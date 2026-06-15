#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../crawl/crawl.h"

int crawlcanconect(struct collectionFormat *collection) {
	//test if we can connect
}

int crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {

	struct crawldocumentExistFormat crawldocumentExist;
	struct crawldocumentAddFormat crawldocumentAdd;
	

	printf("crawlTEMPLATE: %s\n",(*collection).resource);
	//while (al docs) {

		if (!(*documentExist)(collection,&crawldocumentExist)) {

			if (!(*documentAdd)(collection,&crawldocumentAdd)) {
				printf("cant add doc\n");
			}
		}
	//}

	return 1;
}

int crawlupdate(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {
	//update
	
	
	
}
struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
        crawlfirst,
        crawlupdate,
        crawlcanconect,
	NULL,
	NULL,
        crawl_security_acl,
        "temp",
        strcrawlError
};


