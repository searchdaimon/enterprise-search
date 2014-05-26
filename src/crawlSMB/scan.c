#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <stdlib.h>
#include <libsmbclient.h>
#include "get_auth_data_fn.h"

#include "../crawl/crawl.h"
#include "../logger/logger.h"

static void
no_auth_data_fn(const char * pServer,
                const char * pShare,
                char * pWorkgroup,
                int maxLenWorkgroup,
                char * pUsername,
                int maxLenUsername,
                char * pPassword,
                int maxLenPassword);

int browse(int (*scan_found_share)(char share[]),char * path, int scan, int indent);

static void
get_auth_data_with_context_fn(SMBCCTX * context,
                              const char * pServer,
                              const char * pShare,
                              char * pWorkgroup,
                              int maxLenWorkgroup,
                              char * pUsername,
                              int maxLenUsername,
                              char * pPassword,
                              int maxLenPassword);


int scanSMB(int (*scan_found_share)(char share[]),char host[],char username[], char password[], int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...))
{
    int                         debug = 0;
    int                         debug_stderr = 0;
    int                         no_auth = 0;
    int                         context_auth = 0;
    int                         scan = 0;
    int                         iterations = -1;
    int                         again;
    int                         opt;
    char *                      p;
    char *                      q;
    char                        buf[1024];
    poptContext                 pc;
    SMBCCTX *                   context;

    bblog(INFO, "scan.c: host %s username %s password %s",host,username,password);


    /* Allocate a new context */
    context = smbc_new_context();
    if (!context) {
        bblog(ERROR, "Could not allocate new smbc context");
        return 0;
    }
        
    /* If we're scanning, do no requests for authentication data */
    if (scan) {
        no_auth = 1;
    }

    /* Set mandatory options (is that a contradiction in terms?) */
    context->debug = debug;
    if (context_auth) {
        context->callbacks.auth_fn = NULL;
        smbc_option_set(context,
                        "auth_function",
                        (void *) get_auth_data_with_context_fn);
        smbc_option_set(context, "user_data", "hello world");
    } else {
        context->callbacks.auth_fn =
            (no_auth ? no_auth_data_fn : get_auth_data_fn);
    }

    /* If we've been asked to log to stderr instead of stdout... */
    if (debug_stderr) {
        /* ... then set the option to do so */
        smbc_option_set(context, "debug_stderr", (void *) 1);
    }
	
    /* Initialize the context using the previously specified options */
    if (!smbc_init_context(context)) {
        smbc_free_context(context, 0);
        bblog(ERROR, "Could not initialize smbc context");
        return 0;
    }

    /* Tell the compatibility layer to use this context */
    smbc_set_context(context);

    sprintf(buf,"smb://%s:%s@%s",username,password,host);
            
    if (!browse(scan_found_share,buf, scan, 0)) {
	bblog(ERROR, "cant browse");
	return 0;
    }


    return 1;
    //exit(0);
}


static void
no_auth_data_fn(const char * pServer,
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


static void
get_auth_data_with_context_fn(SMBCCTX * context,
                              const char * pServer,
                              const char * pShare,
                              char * pWorkgroup,
                              int maxLenWorkgroup,
                              char * pUsername,
                              int maxLenUsername,
                              char * pPassword,
                              int maxLenPassword)
{
    bblog(INFO, "Authenticating with context 0x%lx", context);
    if (context != NULL) {
        char *user_data = smbc_option_get(context, "user_data");
        bblog(INFO, "> with user data %s", user_data);
    }

    get_auth_data_fn(pServer, pShare, pWorkgroup, maxLenWorkgroup,
                     pUsername, maxLenUsername, pPassword, maxLenPassword);
}

int browse(int (*scan_found_share)(char share[]),char * path, int scan, int indent)
{
    char *                      p;
    char                        buf[1024];
    int                         dir;
    struct stat                 stat;
    struct smbc_dirent *        dirent;

    if (! scan)
    {
        bblog(INFO, "Opening (%s)...", path);
    }
        
    if ((dir = smbc_opendir(path)) < 0)
    {
        bblog(ERROR, "Could not open directory [%s] (%d:%s)", path, errno, strerror(errno));
        return 0;
    }

    while ((dirent = smbc_readdir(dir)) != NULL)
    {
        bblog(INFO, "%*.*s%-30s", indent, indent, "", dirent->name);

        switch(dirent->smbc_type)
        {
        case SMBC_WORKGROUP:
            bblog(INFO, "WORKGROUP");
            break;
            
        case SMBC_SERVER:
            bblog(INFO, "SERVER");
            break;
            
        case SMBC_FILE_SHARE:
            bblog(INFO, "FILE_SHARE");
	    (*scan_found_share)(dirent->name);
            break;
            
        case SMBC_PRINTER_SHARE:
            bblog(INFO, "PRINTER_SHARE");
            break;
            
        case SMBC_COMMS_SHARE:
            bblog(INFO, "COMMS_SHARE");
            break;
            
        case SMBC_IPC_SHARE:
            bblog(INFO, "IPC_SHARE");
            break;
            
        case SMBC_DIR:
            bblog(INFO, "DIR");
            break;
            
        case SMBC_FILE:
            bblog(INFO, "FILE");

            p = path + strlen(path);
            strcat(p, "/");
            strcat(p+1, dirent->name);
            if (smbc_stat(path, &stat) < 0)
            {
                bblog(WARN, " unknown size (reason %d: %s)",
                       errno, strerror(errno));
            }
            else
            {
                bblog(INFO, " size %lu", (unsigned int) stat.st_size);
            }
            *p = '\0';

            break;
            
        case SMBC_LINK:
            bblog(INFO, "LINK");
            break;
        }

        if (scan &&
            (dirent->smbc_type == SMBC_WORKGROUP ||
             dirent->smbc_type == SMBC_SERVER))
        {
            /*
             * don't append server name to workgroup; what we want is:
             *
             *   smb://workgroup_name
             * or
             *   smb://server_name
             *
             */
            snprintf(buf, sizeof(buf), "smb://%s", dirent->name);
            browse(scan_found_share,buf, scan, indent + 2);
        }
    }

    smbc_closedir(dir);

	return 1;
}
    
