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

#include "../boitho-bbdn/bbdnclient.h"
//#include "../bbdocument/bbdocument.h"
#include "acl.parser.h"
#include "crawlsmb.h"


#define MAX_URI_SIZE	1024


static void no_auth_data_fn( const char * pServer,
                const char * pShare,
                char * pWorkgroup,
                int maxLenWorkgroup,
                char * pUsername,
                int maxLenUsername,
                char * pPassword,
                int maxLenPassword);

SMBCCTX* context_init();
void context_free( SMBCCTX *context );



/*
    Create prefix for smb-url from username and password ("smb://username:password@")
 */
char* smb_mkprefix( char *username, char *passwd )
{
    const int	buf_size = MAX_URI_SIZE;
    char	buf[buf_size];
    int		pos = 0, i;

    pos = snprintf( buf, buf_size, "smb://" );
    i = smbc_urlencode( buf + pos, username, buf_size - pos );
    pos = buf_size - i - 1;
    pos+= snprintf( buf + pos, buf_size - pos, ":" );
    i = smbc_urlencode( buf + pos, passwd, buf_size - pos );
    pos = buf_size - i - 1;
    pos+= snprintf( buf + pos, buf_size - pos, "@" );

    return strdup(buf);
}


/*
    Recursively get all files from 'dir_name'
 */
void smb_recursive_get( char *collection, char *prefix, char *dir_name , struct bbdnFormat *bbdh)
{
    int                 dh, dirc, dirc_total;
    char                dblock[512], *dbuf, *dbuf_temp;
    struct smbc_dirent  *dirp;
    char                full_name[strlen(prefix) + strlen(dir_name) + 1];
    SMBCCTX		*context;

    context = context_init();

    sprintf(full_name, "%s%s", prefix, dir_name);

    dh = smbc_opendir( full_name );
    if (dh < 0)
        {
            fprintf(stderr, "crawlsmb.c: Error! Could not open directory %s: %s\n", dir_name, strerror(errno));
	    context_free(context);
            return;
        }

    dbuf_temp = NULL;
    dbuf = NULL;
    dirp = (struct smbc_dirent*)dblock;
    dirc_total = 0;

    while ( (dirc=smbc_getdents( dh, dirp, 512 )) != 0 )
        {
            if (dirc < 0)
                {
                    fprintf(stderr, "crawlsmb.c: Error! Could not get directory entries from %s: %s\n", dir_name, strerror(errno));
		    context_free(context);
                    return;
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

    while (dirc_total > 0)
        {
            int         dsize;

            // Skip direntries named "." and ".."
            if (strcmp(dirp->name,".") && strcmp(dirp->name,".."))
                {
                    int         _dname_size = strlen(dir_name) + strlen(dirp->name) + 1;
                    char        entry_name[_dname_size + 1];
                    char        full_entry_name[strlen(prefix) + _dname_size + 1];
                    char        value[1024];
                    struct stat file_stat;
		    char	*parsed_acl;

                    sprintf(entry_name, "%s/%s", dir_name, dirp->name);
                    sprintf(full_entry_name, "%s%s", prefix, entry_name);

                    if ( smbc_getxattr(full_entry_name, "system.nt_sec_desc.*+", value, sizeof(value)) < 0 )
                        {
                            fprintf(stderr, "crawlsmb.c: Error! Could not get attributes for %s: %s\n", entry_name, strerror(errno));
			    context_free(context);
			    context = context_init();
                            goto next_it;
                        }
                    else
                        {
			    parsed_acl = parseacl_read_access( value );
			    printf("Users allowed to read '%s':\t'%s'\n", entry_name, parsed_acl);
                        }

                    if ( smbc_stat(full_entry_name, &file_stat) < 0 )
                        {
                            fprintf(stderr, "crawlsmb.c: Error! Could not get stat for %s: %s\n", entry_name, strerror(errno) );
                            free(parsed_acl);
			    context_free(context);
			    context = context_init();
			    goto next_it;
                        }

                    context_free(context);

                    if (dirp->smbc_type == SMBC_DIR)
                        {
			    smb_recursive_get(collection, prefix, entry_name, bbdh );
                        }
		    else
		    #ifndef NO_BB
			 if (!bbdn_docexist( collection, entry_name, file_stat.st_mtime ))
		    #endif
			{
			    int		fd;

			    // Disse må være med:
			    context = context_init();

			    fd = smbc_open( full_entry_name, O_RDONLY, 0 );

			    if (fd<0)
				{
				    if (errno == EACCES)
					{
					    fprintf(stderr, "crawlsmb.c: Error! We don't have access to %s.\n", entry_name);
					}
				    else
					{
					    fprintf(stderr, "crawlsmb.c: Error! Could not open %s: %s\n", entry_name, strerror(errno));
					}
				}
			    else
				{
				    int		i;
				    char	*fbuf = (char*)malloc(file_stat.st_size+1);

				    i = smbc_read( fd, fbuf, file_stat.st_size );
				    if (i<0)
					{
					    fprintf(stderr, "crawlsmb.c: Error! Could not read %s: %s\n", entry_name, strerror(errno));
					}
				    else
					{
/*
	kodet uri => dekodet uri => kodet uri != opprinnelig kodet uri!!!

					    // The size of the decoded URI will never exceed the size of the encoded version:
					    char	uri[sizeof(entry_name)+1];

					    smbc_urldecode( uri, entry_name, sizeof(entry_name)+1 );
*/
#ifndef NO_BB
					    bbdn_docadd(bbdh, collection, entry_name, "", fbuf, file_stat.st_size, file_stat.st_mtime, parsed_acl, dirp->name ,"");
#endif
//					    printf("%s:\n%s\n\n", entry_name, fbuf);
					}

				    smbc_close(fd);
				    free(fbuf);
				}

			    context_free(context);
			}

		    free(parsed_acl);
		    context = context_init();
                }

next_it:
            dsize = dirp->dirlen;
            dirp = (struct smbc_dirent*)(((char*)dirp) + dsize);
            dirc_total-= dsize;
        }

    free(dbuf);

    context_free(context);
}


/*
    Try to open file, and test if the user has access to it.

    Returns 0 if error or the user doesn't have access to the file.
    Returns 1 if the user has access.
 */
int smb_test_open( char *prefix, char *dir_name )
{
    // Det viser seg at dersom vi koder den dekodete URI-en blir den ødelagt ('/' blir %2F).
    // Har derfor fjernet støtte for å lagre menneskelig lesbar uri.

    int		uri_size = strlen(prefix) + strlen(dir_name) + 1;
    char	uri[uri_size];
    int		fd;
    SMBCCTX	*context;

    context = context_init();

    snprintf(uri, uri_size, "%s%s", prefix, dir_name);

    printf("Opening %s ... ", uri);
    fflush(stdout);

    fd = smbc_open( uri, O_RDONLY, 0 );

    if (fd < 0)
	{
	    printf("failure\n");

	    if (errno != EACCES)
		{
		    fprintf(stderr, "crawlsmb.c: Error! Could not open %s: %s\n", dir_name, strerror(errno));
		}

	    context_free(context);
	    return 0;
	}

    printf("success\n");

    smbc_close(fd);
    context_free(context);
    return 1;
}




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


SMBCCTX* context_init()
{
    SMBCCTX	*context;

    context = smbc_new_context();
    if (!context)
        {
            perror("crawlsmb.c: Error! Could not allocate new smbc context");
            exit(1);
        }

    // debug-level:
    context->debug = 0;
    context->callbacks.auth_fn = no_auth_data_fn;
    context->options.urlencode_readdir_entries = 1;		// Sørg for at alle url-er er kodet.
    smbc_option_set(context, "debug_stderr", (void*)1);

    if (!smbc_init_context(context))
        {
            perror("crawlsmb.c: Error! Could not initialize smbc context");
            exit(1);
        }

    smbc_set_context(context);

    return context;
}


void context_free( SMBCCTX *context )
{
    if (smbc_free_context(context, 0))
	{
	    perror("crawlsmb.c: Error! Could not free smbc context");
	    exit(1);
	}
}

