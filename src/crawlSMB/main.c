
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "crawlsmb.h"
#include "scan.h"
#include "cleanresource.h"

//#include "../boitho-bbdn/bbdnclient.h"


#include "../crawl/crawl.h"



int crawlpatAcces(char resource[], const char username[], const char password[], int (*documentError)(int level, const char *fmt, ...) ) 
{
        //tester om vi kan koble til
	char        *prefix;
	char	*resourcereal;
	int status;
	int no_auth = 0;

	printf("aa username \"%s\"\n",username);

	resourcereal = resource;

	//fjerner file: i begyndelsen
	if (strncmp("file:",resourcereal,strlen("file:")) == 0) {
		resourcereal += strlen("file:");
	}

	//fp char bug fiks:
	cleanresourceWinToUnix(resourcereal);

        printf("crawlSMB: \n\tresourcereal: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",resourcereal,username,password);


	prefix = smb_mkprefix( username, password );

	status =  smb_test_open( prefix, resourcereal,documentError);

	free(prefix);

	if (status == 1) {
		printf("can open :) . (status %i)\n",status);
	}
	else {
		printf("can't open. (status %i)\n",status);
	}

	return status;
}

int crawlcanconect( struct collectionFormat *collection,int (*documentError)(int level, const char *fmt, ...)) 
{
        //tester om vi kan koble til
	char        *prefix;
	int status;
	int no_auth = 0;

	cleanresourceWinToUnix((*collection).resource);

        printf("crawlSMB: \n\tresource: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",(*collection).resource,(*collection).user,(*collection).password);

	if ((*collection).user == NULL) {
		no_auth = 1;
	}

	prefix = smb_mkprefix( (*collection).user, (*collection).password );

	status =  smb_test_conect( prefix, (*collection).resource,no_auth , documentError);

	free(prefix);

	return status;
}

int crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection)
	) {

        struct crawldocumentExistFormat crawldocumentExist;
        struct crawldocumentAddFormat crawldocumentAdd;

	int no_auth = 0;

	char        *prefix;
	int result;

	printf("crawlfirst: start\n");

	cleanresourceWinToUnix((*collection).resource);
	if ((*collection).user == NULL) {
		no_auth = 1;
	}


        printf("crawlSMB: \n\tresource: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",(*collection).resource,(*collection).user,(*collection).password);
    	prefix = smb_mkprefix( (*collection).user, (*collection).password );

    	result = smb_recursive_get(  prefix, (*collection).resource, collection, documentExist,documentAdd, documentError, documentContinue, 0,no_auth);

    	free(prefix);


	printf("crawlfirst: end\n");

        return result;
}

int crawlupdate(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(int level, const char *fmt, ...),
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

	printf("crawlupdate: start\n");

        printf("crawlSMB: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",(*collection).resource,(*collection).user,(*collection).password);
    	prefix = smb_mkprefix( (*collection).user, (*collection).password );

    	result = smb_recursive_get(  prefix, (*collection).resource, collection, documentExist,documentAdd, documentError, documentContinue,(*collection).lastCrawl,no_auth);

    	free(prefix);


	printf("crawlupdate: end\n");

        return result;
}

int
smb_rewrite_url(char *uri, enum platform_type ptype, enum browser_type btype)
{

	printf("smb_rewrite_url1: raw url: \"%s\"\n",uri);
	smbc_urldecode( uri, uri, strlen(uri)+1 );
	cleanresourceUnixToWin(uri);
	printf("smb_rewrite_url2: raw url: \"%s\"\n",uri);

	char *tmpuri = strdup(uri+7);

	if (ptype == MAC) {
		int i;

		for (i = 0; i < strlen(tmpuri); i++)
			if (tmpuri[i] == '\\')
				tmpuri[i] = '/';

		sprintf(uri, "sdsmb://%s", tmpuri);
	} else if (btype == MOZILLA) {
		sprintf(uri, "file://///%s", tmpuri);
	}

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
        "SMB",
	strcrawlError
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
