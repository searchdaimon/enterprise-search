
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "crawlsmb.h"
#include "scan.h"

//#include "../boitho-bbdn/bbdnclient.h"


#include "../crawl/crawl.h"

int cleanresource(char resource[]) {

	char *cp;

	//gjør om på formatet slik at \\ blir //
	if (strchr(resource,'\\') != NULL) {
		crawlWarn("collection \"%s\" contains \"\\\" characters. Corect format is //host/shares not \\\\host\\shares. Will convert \"\\\" to \"/\"",resource);

		while((cp = strchr(resource,'\\')) != NULL) {
			cp[0] = '/';
		}		

	}

}


int crawlpatAcces(char resource[], char username[], char password[]) 
{
        //tester om vi kan koble til
	char        *prefix;
	int status;
	int no_auth = 0;

	cleanresource(resource);

        printf("crawlSMB: \n\tresource: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",resource,username,password);



	prefix = smb_mkprefix( username, password );

	status =  smb_test_open( prefix, resource);

	free(prefix);

	if (status == 1) {
		printf("can open :) . (status %i)\n",status);
	}
	else {
		printf("can't open. (status %i)\n",status);
	}

	return status;
}

int crawlcanconect( struct collectionFormat *collection) 
{
        //tester om vi kan koble til
	char        *prefix;
	int status;
	int no_auth = 0;

	cleanresource((*collection).resource);

        printf("crawlSMB: \n\tresource: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",(*collection).resource,(*collection).user,(*collection).password);


	if ((*collection).user == NULL) {
		no_auth = 1;
	}

	prefix = smb_mkprefix( (*collection).user, (*collection).password );

	status =  smb_test_conect( prefix, (*collection).resource,no_auth );

	free(prefix);

	return status;
}

int crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {

        struct crawldocumentExistFormat crawldocumentExist;
        struct crawldocumentAddFormat crawldocumentAdd;

	int no_auth = 0;

	char        *prefix;
	int result;

	printf("crawlfirst: start\n");

	cleanresource((*collection).resource);
	if ((*collection).user == NULL) {
		no_auth = 1;
	}


        printf("crawlSMB: \n\tresource: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",(*collection).resource,(*collection).user,(*collection).password);
    	prefix = smb_mkprefix( (*collection).user, (*collection).password );

    	result = smb_recursive_get(  prefix, (*collection).resource, collection, documentExist,documentAdd, 0,no_auth);

    	free(prefix);


	printf("crawlfirst: end\n");

        return result;
}

int crawlupdate(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {

        struct crawldocumentExistFormat crawldocumentExist;
        struct crawldocumentAddFormat crawldocumentAdd;

	int no_auth = 0;

	int result;
	char        *prefix;
	
	cleanresource((*collection).resource);

	if ((*collection).user == NULL) {
		no_auth = 1;
	}

	printf("crawlupdate: start\n");

        printf("crawlSMB: \"%s\"\n\tuser \"%s\"\n\tPassword \"%s\"\n",(*collection).resource,(*collection).user,(*collection).password);
    	prefix = smb_mkprefix( (*collection).user, (*collection).password );

    	result = smb_recursive_get(  prefix, (*collection).resource, collection, documentExist,documentAdd, (*collection).lastCrawl,no_auth);

    	free(prefix);


	printf("crawlupdate: end\n");

        return result;
}

struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
        crawlfirst,
	crawlupdate,
        crawlcanconect,
	crawlpatAcces,
	scanSMB,
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
