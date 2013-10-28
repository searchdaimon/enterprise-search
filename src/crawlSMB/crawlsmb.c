//asprintf
#define _GNU_SOURCE


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
#include "../crawl/crawl.h"
#include "../logger/logger.h"
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
    if (smbc_free_context(context, 1) != 0)
	{
	    bblog(ERROR, "crawlsmb.c: Could not free smbc context: %s", strerror(errno));
		//ToDo: returnerer free uanset. Får nemlig altid feil her
                //hvis ikke. Er dette en minne lekasj prblem?
	    return 1;

	}

	return 1;
}

int smbc_readloop(int sockfd, void *buf, off_t len) {

	bblog(INFO, "smbc_readloop: start");
	
        off_t total = 0;
        off_t bytesleft = len; // how many we have left to send
        int n;

        #ifdef DEBUG
        bblog(DEBUG, "will read %i",len);
        #endif

        while(total < len) {

                if ((n = smbc_read(sockfd, buf+total, 65000)) <= 0) {
			bblog(ERROR, "can't smbc_read()");
                        return 0;
                }

                #ifdef DEBUG
                bblog(DEBUG, "reading %.5i bytes. total red %.8"PRId64", left %.8"PRId64", total to get %.8"PRId64" ( %f\% )",n,total,bytesleft,len,((float)total/(float)len)*100.00);
                #endif

                total += n;
                bytesleft -= n;
        }

	bblog(INFO, "smbc_readloop: end. did read %"PRId64"",total);
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
static int smb_recursive_get_next( char *prefix, char *dir_name,
	struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection),
	int no_auth
	 )
{

    bblog(INFO, "smb_recursive_get_next(prefix=\"%s\", dir_name=\"%s\")", prefix, dir_name);

    static int count = 0;

    int                 dh, dirc, dirc_total, n;
    char                dblock[512], *dbuf, *dbuf_temp;
    struct smbc_dirent  *dirp;
    char                full_name[strlen(prefix) + strlen(dir_name) + 1];
//2012    SMBCCTX		*context;

    struct crawldocumentExistFormat crawldocumentExist;
    struct crawldocumentAddFormat crawldocumentAdd;


//2012    context = context_init(no_auth);

    sprintf(full_name, "%s%s", prefix, dir_name);

    dh = smbc_opendir( full_name );
    if (dh < 0)
    {
            documentError(collection, 1,"crawlsmb.c: Error! Could not open directory %s for dir \"%s\" at %s:%d", dir_name,dir_name,__FILE__,__LINE__);
//2012	    context_free(context);
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
                    documentError(collection, 1,"crawlsmb.c: Error! Could not get directory entries from %s", dir_name);
		    smbc_closedir(dh);
//2012		    context_free(context);
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

    while (documentContinue(collection) && (dirc_total > 0)) {
            int         dsize;
            int         _dname_size = strlen(dir_name) + strlen(dirp->name) + 1;
	    int		fd=0;

            char        *entry_name;
            char        *full_entry_name;
	    char 	*uri;


            asprintf(&entry_name, "%s/%s", dir_name, dirp->name);
            asprintf(&full_entry_name, "%s%s", prefix, entry_name);
            asprintf(&uri, "file:%s",entry_name);

	    #ifdef DEBUG
	    bblog(DEBUG, "entry_name raw: \"%s\"",entry_name);
	    #endif

            // Skip direntries named "." and "..". 
	    // tar heller ikke med filer som begynner på ~ da det er temp filer i windows.
            if ((dirp->name[0] == '.') || (dirp->name[0] == '~')) {

	    }
       	    else if (dirp->smbc_type == SMBC_DIR) {
	   	smb_recursive_get_next( prefix, entry_name, collection, documentExist, documentAdd , documentError, documentContinue, no_auth);
            }
            else {

                    	char        value[1024];
                    	struct stat file_stat;
		    	char	**parsed_acl;
			
	
			crawldocumentExist.documenturi = uri;

			//kaller documentExist første gang, men indikerer at vi ikke her noe ide om lastmodified eller dokument_size
			//documentExist vil da avgjøre om vi skal gå videre eller ikke.
			crawldocumentExist.lastmodified = 0;
			crawldocumentExist.dokument_size = 0;

		    	if ((documentExist)(collection, &crawldocumentExist )) {
				goto next_it;
			}

		    	//på grunn av en bug i smblib må vi kalle dette før hver opperasjon
               	    	//context_free(context);
		    	//context = context_init(no_auth);

		    	bblog(INFO, "opening full_entry_name: \"%s\"",full_entry_name);
		    	fd = smbc_open( full_entry_name, O_RDONLY, 0 );
		    	if (fd<0) {
				if (errno == EACCES) {
				    documentError(collection, 1,"crawlsmb.c: Error! We don't have access to %s.", entry_name);
				}
				else {
				    documentError(collection, 1,"crawlsmb.c: Error! Could not open file %s. Error code %i", entry_name, errno);
				}
//2012			    	context_free(context);
//2012			    	context = context_init(no_auth);
			    	goto next_it;
			}


			//på grunn av en bug i smblib må vi kalle dette før hver opperasjon
                    	//context_free(context);
			//context = context_init(no_auth);

			bblog(INFO, "stating \"%s\"",full_entry_name);
                    	if ( smbc_fstat(fd, &file_stat) < 0 ) {
                            	documentError(collection, 1,"crawlsmb.c: Warning[no fatal]! Could not get stat for %s. Skipping document, continue crawl.", entry_name);
//2012			    	context_free(context);
//2012			    	context = context_init(no_auth);
			    	goto next_it;
                        }
			


			//kaller documentExist andre gang
			crawldocumentExist.lastmodified = file_stat.st_mtime;
			crawldocumentExist.dokument_size = file_stat.st_size;
		    	if ((documentExist)(collection, &crawldocumentExist )) {
				goto next_it;
			}


			#ifdef DEBUG
				bblog(DEBUG, "times: st_atime %s ",ctime(&file_stat.st_atime));
				bblog(DEBUG, "times: st_mtime %s ",ctime(&file_stat.st_mtime));
				bblog(DEBUG, "times: st_ctime %s ",ctime(&file_stat.st_ctime));
			#endif




				    	int	i;
				    	char	*fbuf;

					//tester størelsen på filen. Hvis den er for stor dropper vi å laste den ned.
					if (file_stat.st_size > MAX_FILE_SIZE) {
						bblog(WARN, "Warning: document \"%s\" is larger then maximum size of %i. Size is %"PRId64"",entry_name,MAX_FILE_SIZE,file_stat.st_size);

						fbuf = strdup("");
						//setter at dokumentet er 0 bytes
						file_stat.st_size = 0;

					}
					else {

				    		if ((fbuf = malloc(file_stat.st_size+1)) == NULL) {
							bblog_errno(ERROR,"can't malloc %i bytes for file buffer",file_stat.st_size);
							goto next_it;
				    		}

				    		i = smbc_readloop( fd, fbuf, file_stat.st_size );

				    		if (i<0) {
							documentError(collection, 1,"crawlsmb.c: Error! Could not read %s", entry_name);
							goto next_it;
				    		}
					}

					#ifndef NO_BB
					        //på grunn av en bug i smblib må vi kalle dette før hver opperasjon
                		    		//context_free(context);
					        //context = context_init(no_auth);


			                    	if (smbc_fgetxattr(fd, "system.nt_sec_desc.*", value, sizeof(value)) < 0) {

							if (errno == ENOATTR) {
                        				    documentError(collection, 1,"crawlsmb.c: Warn! Could not get attributes for %s. If the attribute does not exist and the flag SMBC_XATTR_FLAG_REPLACE was specified. Will use an emty acl.", entry_name);
							    // Acl (file attribut) dos not exist. Make an emty acl
							    parsed_acl = malloc(sizeof(char*)*2);
							    parsed_acl[0] = strdup("");
							    parsed_acl[1] = strdup("");
							}
							else if (errno == EINVAL) {
	                			            documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. The client library is not properly initialized or one of the parameters is not of a correct form.", entry_name);
							    goto next_it;
							}
							else if (errno == ENOMEM) {
        	                			    documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. No memory was available for internal needs.", entry_name);
							    goto next_it;
							}
							else if (errno == EEXIST) {
                	        			    documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. If the attribute already exists and the flag SMBC_XATTR_FLAG_CREAT was specified.", entry_name);
							    goto next_it;
							}
							else if (errno == EPERM) {
	                        			    documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. Permission was denied.", entry_name);
							    goto next_it;
							}
							else if (errno == ENOTSUP) {
	                        			    documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. The referenced file system does not support extended attributes", entry_name);
							    goto next_it;
							}
							else {
	                        			    documentError(collection, 1,"crawlsmb.c: Error! Could not get attributes for %s. Unknown error code \"%i\", name \"%s\"", entry_name, errno, strerror(errno));
							    goto next_it;
							}

                        			}
                    				else {
			    				parsed_acl = parseacl_read_access( value );
						}

			    			#ifdef DEBUG
			    				bblog(DEBUG, "crawlsmb.c: Users allowed '%s'", parsed_acl[0]);
			    				bblog(DEBUG, "crawlsmb.c: Users denied  '%s'", parsed_acl[1]);
			    			#endif
                        			


        					
						crawldocumentAdd.documenturi = uri;

        					crawldocumentAdd.documenttype	= "";
        					crawldocumentAdd.document	= fbuf;
        					crawldocumentAdd.dokument_size	= file_stat.st_size;
        					crawldocumentAdd.lastmodified	= file_stat.st_mtime;
        					crawldocumentAdd.acl_allow 	= parsed_acl[0];
						crawldocumentAdd.acl_denied 	= parsed_acl[1];

        					crawldocumentAdd.title	= malloc((strlen(dirp->name) *2) +1);
						smbc_urldecode( crawldocumentAdd.title, dirp->name, strlen(dirp->name) +1);
						

					        crawldocumentAdd.doctype	= "";
					        crawldocumentAdd.attributes	= "";


						(*documentAdd)(collection ,&crawldocumentAdd);
					
						free(crawldocumentAdd.title);



			    			free(parsed_acl[0]);
					    	free(parsed_acl[1]);
					    	free(parsed_acl);

					#endif

					
				    	free(fbuf);

				   

	



	         		//context_free(context);
		    		//context = context_init(no_auth);
                }

next_it:
	    if (fd>0) {
            	smbc_close(fd);
	    }
	    free(uri);
	    free(entry_name);
	    free(full_entry_name);

            dsize = dirp->dirlen;
            dirp = (struct smbc_dirent*)(((char*)dirp) + dsize);
            dirc_total-= dsize;

	    ++count;
	    if ((count % 1000) == 0) {
		bblog(INFO, "progres %i",count);
	    }
        }

    free(dbuf);

//2012    if (!context_free(context)) {
//2012      	documentError(collection, 1,"crawlsmb.c-smb_recursive_get_next: Error! Could not free smbc context at %s:%d",__FILE__,__LINE__);
//2012        return 0;
//2012    }

    bblog(INFO,"~smb_recursive_get_next(prefix=\"%s\", dir_name=\"%s\")", prefix, dir_name);

    return 1;
}


int smb_recursive_get( char *prefix, char *dir_name,
	struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection),
	int no_auth
	 )
{

    SMBCCTX             *context;
    int ret;
    context = context_init(no_auth);

	ret = smb_recursive_get_next(prefix, dir_name, collection,documentExist,documentAdd,documentError,documentContinue,no_auth);

    if (!context_free(context)) {
        documentError(collection, 1,"crawlsmb.c-smb_recursive_get: Error! Could not free smbc context at %s:%d",__FILE__,__LINE__);
    }

    return ret;

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

    bblog(INFO, "smb_test_conect(prefix=%s, dir_name=%s)",prefix,dir_name);

    context = context_init(no_auth);

    sprintf(full_name, "%s%s", prefix, dir_name);

    dh = smbc_opendir( full_name );

    if (dh < 0)
        {
            documentError(collection, 1,"crawlsmb.c: Error! Could not open directory %s: for dir \"%s\" at %s:%d", dir_name, dir_name,__FILE__,__LINE__);
	    context_free(context);
            return 0;
        }




    bblog(INFO, "successfully opened: %s", full_name);

	struct smbc_dirent  *dirp;
	int dirc;
char                dblock[512];
dirp = (struct smbc_dirent*)dblock;

	bblog(INFO, "smb_test_conect traverse: start");

	while ( (dirc=smbc_getdents( dh, dirp, 512 )) != 0 )
	{
		if (dirc < 0)
                {
                    documentError(collection, 1,"crawlsmb.c: Error! Could not get directory entries from %s", dir_name);
		    smbc_closedir(dh);
                    context_free(context);
                    return 0;
                }

		bblog(INFO, "name \"%s\"",dirp->name);

	}
	bblog(INFO, "smb_test_conect traverse: end");


    	if (smbc_closedir(dh) != 0) {
		documentError(collection, 1,"crawlsmb.c-smb_test_conect: Error! Could not close dir");
	}

	if (!context_free(context)) {
		documentError(collection, 1,"crawlsmb.c-smb_test_conect: Error! Could not free smbc context at %s:%d",__FILE__,__LINE__);
    	}

	bblog(INFO, "smb_test_conect(): returnin 1");
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
    	fd = smbc_open( uri, O_RDONLY, 0 );

    	if (fd < 0)
	{
	    bblog(WARN, "failed to open: %s", uri);

	    if (errno != EACCES)
		{
		    documentError(collection, 1,"crawlsmb.c: Error! Could not open \"%s\": %s", uri,strerror(errno));
		}

	    context_free(context);
	    return 0;
	}

    	bblog(INFO, "successfully opened: %s", uri);

    	smbc_close(fd);
    	if (!context_free(context)) {
		documentError(collection, 1,"crawlsmb.c-smb_test_open: Error! Could not free smbc context at %s:%d",__FILE__,__LINE__);	
		return 0;
    	}

    	return 1;
}
