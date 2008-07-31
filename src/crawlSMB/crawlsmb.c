#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libsmbclient.h>
#include <inttypes.h>


#include "get_auth_data_fn.h"
#include "cleanresource.h"

//#include "../boitho-bbdn/bbdnclient.h"
//#include "../bbdocument/bbdocument.h"
#include "../common/iconv.h"

#include "../crawl/crawl.h"

#include "acl.parser.h"
#include "crawlsmb.h"

#define MAX_URI_SIZE	1024
// 100 mb
#define MAX_FILE_SIZE 104857600

static void no_auth_data_fn( const char * pServer,
                const char * pShare,
                char * pWorkgroup,
                int maxLenWorkgroup,
                char * pUsername,
                int maxLenUsername,
                char * pPassword,
                int maxLenPassword);

static void no_auth_data_fn( const char * pServer,
                const char * pShare,
                char * pWorkgroup,
                int maxLenWorkgroup,
                char * pUsername,
                int maxLenUsername,
                char * pPassword,
                int maxLenPassword)
{
    return;
}

static SMBCCTX* context_init(int no_auth)
{
    SMBCCTX	*context;

    context = smbc_new_context();
    if (!context)
        {
            crawlperror("crawlsmb.c: Error! Could not allocate new smbc context");
            exit(1);
        }

    // debug-level:
    context->debug = 0;
    //context->callbacks.auth_fn = no_auth_data_fn;
    context->callbacks.auth_fn = (no_auth ? no_auth_data_fn : get_auth_data_fn);
    context->options.urlencode_readdir_entries = 1;		// Sørg for at alle url-er er kodet.
    smbc_option_set(context, "debug_stderr", (void*)1);

    if (!smbc_init_context(context))
        {
            crawlperror("crawlsmb.c: Error! Could not initialize smbc context");
            exit(1);
        }

    smbc_set_context(context);

    return context;
}


static int context_free( SMBCCTX *context )
{
    printf("#####################\nFreeing context...\n");
    if (smbc_free_context(context, 0) != 0)
	{
	    perror("crawlsmb.c: Error! Could not free smbc context");
		//ToDo: returnerer free uanset. Får nemlig altid feil her
                //hvis ikke. Er dette en minne lekasj prblem?
	    return 1;

	}

	return 1;
}

int smbc_readloop(int sockfd, void *buf, off_t len) {

	printf("smbc_readloop: start\n");
	
        off_t total = 0;
        off_t bytesleft = len; // how many we have left to send
        int n;

        #ifdef DEBUG
        printf("will read %i",len);
        #endif

        while(total < len) {

                if ((n = smbc_read(sockfd, buf+total, 65000)) <= 0) {
			printf("error: can't smbc_read()\n");
                        return 0;
                }

                //#ifdef DEBUG
                printf("reading %i bytes. \ttotal red %"PRId64", \tleft %"PRId64", \ttotal to get %"PRId64" ( %f\% )\n",n,total,bytesleft,len,((float)total/(float)len)*100.00);
                //#endif

                total += n;
                bytesleft -= n;
        }

	printf("smbc_readloop: end. did read %"PRId64"\n",total);
        return total;

}


/*
 * boithosmbc_wholeurlencode()
 * By Runarb
 *
 * Convert any characters not specifically allowed in a URL into their %xx
 * equivalent.
 *
 * Based on libsmbs smbc_urlencode(), but do not encode / sow it can be used on a whole url
 *
 * Returns the remaining buffer length.
 */
int
boithosmbc_wholeurlencode(char * dest, char * src, int max_dest_len)
{
        char hex[] = "0123456789ABCDEF";

        for (; *src != '\0' && max_dest_len >= 3; src++) {

                if ((*src < '0' &&
                     *src != '-' &&
                     *src != '/' &&
                     //*src != ' ' &&
                     *src != '.') ||
                    (*src > '9' &&
                     *src < 'A') ||
                    (*src > 'Z' &&
                     *src < 'a' &&
                     *src != '_') ||
                    (*src > 'z')) {
                        *dest++ = '%';
                        *dest++ = hex[(*src >> 4) & 0x0f];
                        *dest++ = hex[*src & 0x0f];
                        max_dest_len -= 3;
                } else {
                        *dest++ = *src;
                        max_dest_len--;
                }
        }

        *dest++ = '\0';
        max_dest_len--;
        
        return max_dest_len;
}




/*
    Create prefix for smb-url from username and password ("smb://username:password@")
 */
char* smb_mkprefix( const char *username, const char *passwd )
{
    const int	buf_size = MAX_URI_SIZE;
    char	buf[buf_size];
    int		pos = 0, i;


	//håndterer at vi ikke altid har en bruker og passord
	if (username != 0) {
	    	pos = snprintf( buf, buf_size, "smb://" );
	    	i = smbc_urlencode( buf + pos, username, buf_size - pos );
	    	pos = buf_size - i - 1;
	    	pos+= snprintf( buf + pos, buf_size - pos, ":" );
	    	i = smbc_urlencode( buf + pos, passwd, buf_size - pos );
	    	pos = buf_size - i - 1;
	    	pos+= snprintf( buf + pos, buf_size - pos, "@" );

	}
	else {
		sprintf(buf,"smb://");
	}

    return strdup(buf);
}


/*
    Recursively get all files from 'dir_name'
 */
int smb_recursive_get( char *prefix, char *dir_name,
	struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection),
	int no_auth
	 )
{

	printf("dir \"%s\"\n",dir_name);

    int                 dh, dirc, dirc_total, n;
    char                dblock[512], *dbuf, *dbuf_temp;
    struct smbc_dirent  *dirp;
    char                full_name[strlen(prefix) + strlen(dir_name) + 1];
    SMBCCTX		*context;

    struct crawldocumentExistFormat crawldocumentExist;
    struct crawldocumentAddFormat crawldocumentAdd;
    int isize;
    iconv_t isoconp;
    if ( (isoconp = iconv_open("UTF-8","ISO-8859-15")) ==  (iconv_t)(-1) ) {
                perror("iconv_open");
    }

    context = context_init(no_auth);

    sprintf(full_name, "%s%s", prefix, dir_name);

    dh = smbc_opendir( full_name );
    if (dh < 0)
        {
            documentError(collection, 1,"crawlsmb.c: Error! Could not open directory %s for dir \"%s\" at %s:%d", dir_name,dir_name,__FILE__,__LINE__);
	    context_free(context);
            return 0;
        }

    dbuf_temp = NULL;
    dbuf = NULL;
    dirp = (struct smbc_dirent*)dblock;
    dirc_total = 0;

    while (  (dirc=smbc_getdents( dh, dirp, 512 )) != 0  )
        {
            if (dirc < 0)
                {
                    documentError(collection, 1,"crawlsmb.c: Error! Could not get directory entries from %s\n", dir_name);
		    context_free(context);
                    return 0;
                }

            dbuf_temp = dbuf;
            dbuf = (char*)malloc(dirc_total + dirc);
            memcpy(dbuf, dbuf_temp, dirc_total);
            memcpy(&(dbuf[dirc_total]), dblock, dirc);
            if (dbuf_temp!=NULL) free(dbuf_temp);

            dirc_total+= dirc;
        }

    smbc_closedir(dh);

    dirp = (struct smbc_dirent*)dbuf;

    while (documentContinue(collection) && (dirc_total > 0))
        {
            int         dsize;

            // Skip direntries named "." and "..". 
	    // tar heller ikke med filer som begynner på ~ da det er temp filer i windows.
            if ((dirp->name[0] != '.') && (dirp->name[0] != '~'))
                {
                    int         _dname_size = strlen(dir_name) + strlen(dirp->name) + 1;
                    char        entry_name[_dname_size + 1];
                    char        full_entry_name[strlen(prefix) + _dname_size + 1];
                    char        value[1024];
                    struct stat file_stat;
		    char	**parsed_acl;

                    sprintf(entry_name, "%s/%s", dir_name, dirp->name);
                    sprintf(full_entry_name, "%s%s", prefix, entry_name);

                    if ( (n = smbc_getxattr(full_entry_name, "system.nt_sec_desc.*", value, sizeof(value))) < 0 )
                        {

				if (n == EINVAL) {
	                            documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. The client library is not properly initialized or one of the parameters is not of a correct form.\n", entry_name);
				}
				else if (n == ENOMEM) {
        	                    documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. No memory was available for internal needs.\n", entry_name);
				}
				else if (n == EEXIST) {
                	            documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. If the attribute already exists and the flag SMBC_XATTR_FLAG_CREAT was specified.\n", entry_name);
				}
				else if (n == ENOATTR) {
                        	    documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. If the attribute does not exist and the flag SMBC_XATTR_FLAG_REPLACE was specified.\n", entry_name);
				}
				else if (n == EPERM) {
	                            documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. Permission was denied.\n", entry_name);
				}
				else if (n == ENOTSUP) {
	                            documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. The referenced file system does not support extended attributes\n", entry_name);
				}
				else {
	                            documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. Unknown error code \"%i\"\n", entry_name,n);
				}

			    	context_free(context);
			    	context = context_init(no_auth);
                            	goto next_it;
                        }
                    else
                        {
			    parsed_acl = parseacl_read_access( value );
			    #ifdef DEBUG
			    printf("crawlsmb.c: Users allowed \t'%s'\n", parsed_acl[0]);
			    printf("crawlsmb.c: Users denied  \t'%s'\n", parsed_acl[1]);
			    #endif
                        }

                    if ( smbc_stat(full_entry_name, &file_stat) < 0 )
                        {
                            documentError(collection, 1,"crawlsmb.c: Error! Could not get stat for %s", entry_name);
			    free(parsed_acl[0]);
			    free(parsed_acl[1]);
                            free(parsed_acl);
			    context_free(context);
			    context = context_init(no_auth);
			    goto next_it;
                        }

                    context_free(context);

			//runarb: 20 nov 2007: denne er ikke komentert ut, det fører til at vi allokerer minne to ganger.
			// er det riktig å konvertere til utf-8 her??? Fører ikke det til problemer med tegnsett?? slik det vi ser i fp nå?
			//hva hvis vi får en iso-? inn, og konverterer den til utf, konverterer vi den rikit tilbake til iso-? da ?
			//crawldocumentExist.documenturi = malloc(strlen(entry_name) + strlen("file:") +1);
			//sprintf(crawldocumentExist.documenturi,"file:%s",entry_name);
			printf("entry_name raw: \"%s\"\n",entry_name);
			
			char        uri[sizeof(entry_name)+1];
			//fp char bug fiks:
			//smbc_urldecode( uri, entry_name, sizeof(entry_name)+1 );
        		strscpy(uri,entry_name,sizeof(uri));
	
			isize = (strlen(uri) *2)+ strlen("file:");
			if ((crawldocumentExist.documenturi = malloc(isize)) == NULL) {
				perror("malloc");
				//runarb: 20 nov 2007: usikker på om dette er rikitg feilhontering.
	                   	goto next_it;
			}

			sprintf(crawldocumentExist.documenturi,"file:%s",uri);

			#ifdef URLDECODE
			iconv_convert(isoconp ,&crawldocumentExist.documenturi, isize);
			#endif
			
			crawldocumentExist.lastmodified = file_stat.st_mtime;
			crawldocumentExist.dokument_size = file_stat.st_size;

			//runarb: 26 feb. Vi kjører denne på crawldocumentExist ~urlen, men ikke på add. Noe som fører til at de blir forskjelige.
			//cleanresourceUnixToWin(crawldocumentExist.documenturi);

			#ifdef DEBUG
				printf("times: st_atime %s ",ctime(&file_stat.st_atime));
				printf("times: st_mtime %s ",ctime(&file_stat.st_mtime));
				printf("times: st_ctime %s ",ctime(&file_stat.st_ctime));
			#endif


                    	if (dirp->smbc_type == SMBC_DIR) {
			    smb_recursive_get( prefix, entry_name, collection, documentExist, documentAdd , documentError, documentContinue, no_auth);
                        }
		    	else if ((documentExist)(collection, &crawldocumentExist )) {
				//doc exist
				printf("Note: smb_recursive_get: document exist\n");
			}
			else 
			{
			    int		fd;

			    // Disse må være med:
			    context = context_init(no_auth);

				printf("opening full_entry_name: \"%s\"\n",full_entry_name);
			    fd = smbc_open( full_entry_name, O_RDONLY, 0 );

			    if (fd<0)
				{
				    if (errno == EACCES)
					{
					    documentError(collection, 1,"crawlsmb.c: Error! We don't have access to %s.\n", entry_name);
					}
				    else
					{
					    documentError(collection, 1,"crawlsmb.c: Error! Could not open %s\n", entry_name);
					}
				}
			    else
				{

				    	int	i;
				    	char	*fbuf;
					char    uri[sizeof(entry_name)+1];

					//tester størelsen på filen. Hvis den er for stor dropper vi å laste den ned.
					if (file_stat.st_size > MAX_FILE_SIZE) {
						fprintf(stderr,"Warning: document \"%s\" is larger then maximum size of %i. Size is %"PRId64"\n",entry_name,MAX_FILE_SIZE,file_stat.st_size);

						fbuf = strdup("");
						//setter at dokumentet er 0 bytes
						file_stat.st_size = 0;

					}
					else {

				    		if ((fbuf = malloc(file_stat.st_size+1)) == NULL) {
							fprintf(stderr,"can't malloc %i bytes for file buffer\n",file_stat.st_size);
							perror("malloc");

							goto closefile;
				    		}

				    		i = smbc_readloop( fd, fbuf, file_stat.st_size );

				    		if (i<0) {
							documentError(collection, 1,"crawlsmb.c: Error! Could not read %s", entry_name);
							goto closefile;
				    		}
					}

					#ifndef NO_BB
						//fp char bug fiks:
						//smbc_urldecode( uri, entry_name, sizeof(entry_name)+1 );
						strscpy( uri, entry_name, sizeof(uri) );

						printf("url after smbc_urldecode(): \"%s\"\n",uri);
        					
						isize = (strlen(uri) *2)+ strlen("file:");
						crawldocumentAdd.documenturi = malloc(isize);
						sprintf(crawldocumentAdd.documenturi,"file:%s",uri);

						#ifdef URLDECODE
							iconv_convert(isoconp ,&crawldocumentAdd.documenturi, isize);
						#endif

        					crawldocumentAdd.documenttype	= "";
        					crawldocumentAdd.document	= fbuf;
        					crawldocumentAdd.dokument_size	= file_stat.st_size;
        					crawldocumentAdd.lastmodified	= file_stat.st_mtime;
        					crawldocumentAdd.acl_allow 	= parsed_acl[0];
						crawldocumentAdd.acl_denied 	= parsed_acl[1];

						isize = ((strlen(dirp->name) *2) +1);
        					crawldocumentAdd.title	= malloc(isize);
						smbc_urldecode( crawldocumentAdd.title, dirp->name, strlen(dirp->name) +1);
						
						#ifdef URLDECODE
							iconv_convert(isoconp ,&crawldocumentAdd.title, isize);
						#endif

					        crawldocumentAdd.doctype	= "";
					        crawldocumentAdd.attributes	= "";

						//fp char bug fiks:
						//cleanresourceUnixToWin(crawldocumentAdd.documenturi);

						(*documentAdd)(collection ,&crawldocumentAdd);
					
						free(crawldocumentAdd.documenturi);
						free(crawldocumentAdd.title);

		    				//documentAdd(bbdh, collection, entry_name, "", fbuf, file_stat.st_size, file_stat.st_mtime, parsed_acl, dirp->name ,"");
					#endif

					

				    	free(crawldocumentExist.documenturi); //usikker om dette er rikit plass

				    	free(fbuf);

				    	closefile:
				    	smbc_close(fd);

				}

			    context_free(context);
			}

		    free(parsed_acl[0]);
		    free(parsed_acl[1]);
		    free(parsed_acl);
		    context = context_init(no_auth);
                }

next_it:
            dsize = dirp->dirlen;
            dirp = (struct smbc_dirent*)(((char*)dirp) + dsize);
            dirc_total-= dsize;
        }

    free(dbuf);

    //context_free(context);
    if (!context_free(context)) {
      	documentError(collection, 1,"crawlsmb.c-smb_recursive_get: Error! Could not free smbc context at %s:%d",__FILE__,__LINE__);
        return 0;
    }

    iconv_close(isoconp);

    return 1;
}


/*
    Try to conect, basicly the same as smb_test_open

    Returns 0 if error.
    Returns 1 if we has access.
 */
int smb_test_conect(struct collectionFormat *collection, char *prefix, char *dir_name , int no_auth, int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...))
{
    // Det viser seg at dersom vi koder den dekodete URI-en blir den ødelagt ('/' blir %2F).
    // Har derfor fjernet støtte for å lagre menneskelig lesbar uri.

    char        full_name[strlen(prefix) + strlen(dir_name) + 1];
    int		dh;
    SMBCCTX	*context;

    printf("smb_test_conect(prefix=%s, dir_name=%s)\n",prefix,dir_name);

    context = context_init(no_auth);

    sprintf(full_name, "%s%s", prefix, dir_name);

    dh = smbc_opendir( full_name );

    if (dh < 0)
        {
            documentError(collection, 1,"crawlsmb.c: Error! Could not open directory %s: for dir \"%s\" at %s:%d", dir_name, dir_name,__FILE__,__LINE__);
	    context_free(context);
            return 0;
        }



    printf("Opening %s ... ", full_name);
    fflush(stdout);

    dh = smbc_opendir( full_name );


    //fd = smbc_open( uri, O_RDONLY, 0 );

    if (dh < 0)
	{
	    printf("failure\n");

	    if (errno != EACCES)
		{
		    documentError(collection, 1,"crawlsmb.c: Error! Could not open %s", dir_name);
		}

	    context_free(context);
	    return 0;
	}

    printf("success\n");

	struct smbc_dirent  *dirp;
	int dirc;
char                dblock[512];
dirp = (struct smbc_dirent*)dblock;

	printf("traverse: start\n");

	while ( (dirc=smbc_getdents( dh, dirp, 512 )) != 0 )
	{
		if (dirc < 0)
                {
                    documentError(collection, 1,"crawlsmb.c: Error! Could not get directory entries from %s", dir_name);
                    context_free(context);
                    return 0;
                }

		printf("name \"%s\"\n",dirp->name);

	}
	printf("traverse: end\n");


    	if (smbc_closedir(dh) != 0) {
		documentError(collection, 1,"crawlsmb.c-smb_test_conect: Error! Could not close dir");
	}

	if (!context_free(context)) {
		documentError(collection, 1,"crawlsmb.c-smb_test_conect: Error! Could not free smbc context at %s:%d",__FILE__,__LINE__);	
		return 0;
    	}

    	return 1;
}


/*
    Try to open file, and test if the user has access to it.

    Returns 0 if error or the user doesn't have access to the file.
    Returns 1 if the user has access.
 */
int smb_test_open(struct collectionFormat *collection,  char *prefix, char *dir_name, int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...))
{

    	// Det viser seg at dersom vi koder den dekodete URI-en blir den ødelagt ('/' blir %2F).
    	// Har derfor fjernet støtte for å lagre menneskelig lesbar uri.
    	int		uri_size = strlen(prefix) + (strlen(dir_name) *2) + 1;
    	char	uri[uri_size];
    	int		fd;
    	SMBCCTX	*context;
    	int no_auth = 0; //kan ikke ha manglende bruker og passord her


    	context = context_init(no_auth);


	snprintf(uri, uri_size, "%s%s", prefix, dir_name);


	printf("urip: \"%s\"\n",uri);



    	printf("Opening %s ... ", uri);
    	fflush(stdout);

    	fd = smbc_open( uri, O_RDONLY, 0 );

    	if (fd < 0)
	{
	    printf("failure\n");

	    if (errno != EACCES)
		{
		    documentError(collection, 1,"crawlsmb.c: Error! Could not open \"%s\": %s", uri,strerror(errno));
		}

	    context_free(context);
	    return 0;
	}

    	printf("success\n");

    	smbc_close(fd);
    	if (!context_free(context)) {
		documentError(collection, 1,"crawlsmb.c-smb_test_open: Error! Could not free smbc context at %s:%d",__FILE__,__LINE__);	
		return 0;
    	}

    	return 1;
}
