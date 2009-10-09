
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "crawlsmb.h"
#include "scan.h"
#include "cleanresource.h"
#include "../crawlManager2/shortenurl.h"

//#include "../boitho-bbdn/bbdnclient.h"


#include "../crawl/crawl.h"
#include "../logger/logger.h"



int crawlpatAcces   (char resource[], char username[], char password[], int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...) , struct collectionFormat *collection) 
{
        //tester om vi kan koble til
	char        *prefix;
	char	*resourcereal;
	int status;
	int no_auth = 0;

	bblog(INFO, "aa username \"%s\"", username);

	resourcereal = resource;

	//fjerner file: i begyndelsen
	if (strncmp("file:",resourcereal,strlen("file:")) == 0) {
		resourcereal += strlen("file:");
	}

	//fp char bug fiks:
	cleanresourceWinToUnix(resourcereal);

        bblog(INFO, "crawlSMB: resourcereal: \"%s\" user \"%s\" Password \"%s\"", resourcereal,username,password);


	prefix = smb_mkprefix( username, password );

	status =  smb_test_open(collection,  prefix, resourcereal,documentError);

	free(prefix);

	if (status == 1) {
		bblog(INFO, "can open :) . (status %i)", status);
	}
	else {
		bblog(ERROR, "can't open. (status %i)",status);
	}

	return status;
}

int crawlcanconect( struct collectionFormat *collection,int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...)) 
{
        //tester om vi kan koble til
	char        *prefix;
	int status;
	int no_auth = 0;

	cleanresourceWinToUnix((*collection).resource);

        bblog(INFO, "crawlSMB: resource: \"%s\" user \"%s\" Password \"%s\"",(*collection).resource,(*collection).user,(*collection).password);

	if ((*collection).user == NULL) {
		no_auth = 1;
	}

	prefix = smb_mkprefix( (*collection).user, (*collection).password );

	status =  smb_test_conect(collection,  prefix, (*collection).resource,no_auth , documentError);

	free(prefix);

	return status;
}

int crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection)
	) {

        struct crawldocumentExistFormat crawldocumentExist;
        struct crawldocumentAddFormat crawldocumentAdd;

	int no_auth = 0;

	char        *prefix;
	int result;

	bblog(INFO, "crawlfirst: start");

	cleanresourceWinToUnix((*collection).resource);
	if ((*collection).user == NULL) {
		no_auth = 1;
	}


        bblog(INFO, "crawlSMB: resource: \"%s\" user \"%s\" Password \"%s\"",(*collection).resource,(*collection).user,(*collection).password);
    	prefix = smb_mkprefix( (*collection).user, (*collection).password );

    	result = smb_recursive_get(  prefix, (*collection).resource, collection, documentExist,documentAdd, documentError, documentContinue, no_auth);

    	free(prefix);


	bblog(INFO, "crawlfirst: end");

        return result;
}

int crawlupdate(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection)
	) {

        struct crawldocumentExistFormat crawldocumentExist;
        struct crawldocumentAddFormat crawldocumentAdd;

	int no_auth = 0;

	int result;
	char        *prefix;
	
	cleanresourceWinToUnix((*collection).resource);

	if ((*collection).user == NULL) {
		no_auth = 1;
	}

	bblog(INFO, "crawlupdate: start");

        bblog(INFO, "crawlSMB: \"%s\" user \"%s\" Password \"%s\"",(*collection).resource,(*collection).user,(*collection).password);
    	prefix = smb_mkprefix( (*collection).user, (*collection).password );

    	result = smb_recursive_get(  prefix, (*collection).resource, collection, documentExist,documentAdd, documentError, documentContinue,no_auth);

    	free(prefix);


	bblog(INFO, "crawlupdate: end");

        return result;
}

int
smb_rewrite_url(struct collectionFormat *collection, char *url, char *uri, char *fulluri, size_t len, enum platform_type ptype, enum browser_type btype)
{
	int valid_alias;

	bblog(INFO, "smb_rewrite_url1: raw url: \"%s\"", url);
	smbc_urldecode( url, url, strlen(url)+1 );
	cleanresourceUnixToWin(url);
	bblog(INFO, "smb_rewrite_url2: raw url: \"%s\"", url);

	char *tmpurl = strdup(url+7);

	if (collection->alias && collection->alias[0])
		valid_alias = 1;
	else
		valid_alias = 0;


	if (ptype == MAC) {
		int i;

		for (i = 0; i < strlen(tmpurl); i++)
			if (tmpurl[i] == '\\')
				tmpurl[i] = '/';

		sprintf(url, "sdsmb://%s", tmpurl);
		if (valid_alias) {
			sprintf(fulluri, "%s/%s", collection->alias, tmpurl);
		} else {
			sprintf(fulluri, "sdsmb://%s", tmpurl);
		}
	} else {
		char *p;
		if (btype == MOZILLA) {
			sprintf(url, "file://///%s", tmpurl);
			p = tmpurl;
		} else {
			p = strstr(url, ":\\\\");
			if (p != NULL) {
				p+=3;
			}
		}
		{
			char *p2, *p3;
			/* Go beyond file://ip/sharename/ */
			if (p != NULL && (p2 = strchr(p, '\\')) && (p3 = strchr(p2+1, '\\')) && valid_alias) {
				p3++;
				sprintf(fulluri, "[%s]\\%s", collection->alias, p3);
			} else {
				sprintf(fulluri, "file://%s", tmpurl);
			}
		}
	}
	strcpy(uri, fulluri);
	shortenurl(uri, len);

	free(tmpurl);

	return 1;
}



struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
        crawlfirst,
	crawlupdate,
        crawlcanconect,
	crawlpatAcces,
	scanSMB,
	smb_rewrite_url,
        crawl_security_acl,
        "SMB"
};



/*
int main( int argc, char *argv[] )
{
    char	*prefix;

    if (argc != 5)
	{
	    printf("usage ./crawlSMB collection username password path\n");
	    exit(1);
	}
	struct bbdnFormat bbdh;

#ifndef NO_BB

	if (!bbdn_conect(&bbdh,"")) {
                printf("cant conetc to bbdh.\n");
                exit(1);
	}
#endif

    prefix = smb_mkprefix( argv[2], argv[3] );

//    // Eksempel på bruk av smb_test_open:
//
//    if (smb_test_open(prefix, argv[4]))
//	{
//	    printf("User %s has access to %s\n", argv[2], argv[4]);
//	}
//    else
//	{
//	    printf("User %s has NOT access to %s\n", argv[2], argv[4]);
//	}

    smb_recursive_get( argv[1], prefix, argv[4],&bbdh );
    free(prefix);
}
*/
