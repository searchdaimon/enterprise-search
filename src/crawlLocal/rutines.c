#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <limits.h>

#include "../crawl/crawl.h"
#include "../common/define.h"
#include "../crawl/crawlLocalFiles.h"


int crawlcanconect( struct collectionFormat *collection,int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...)) {

	//tester om vi kan koble til
	return 1;
}

int crawlfirst(struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
        int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
        int (*documentContinue)(struct collectionFormat *collection)
        ) {

	
	printf("crawlLocal: crawlfirst: start\n");

	crawlLocalFiles(collection,documentExist,documentAdd,"",(*collection).resource);

	printf("crawlLocal: crawlfirst: end\n");

	return 1;
}


struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
        crawlfirst,
	crawlfirst,
        crawlcanconect,
	NULL,
	NULL,
	NULL,
        crawl_security_acl,
        "Local",
};

