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
#include "../common/boithohome.h"

#include "../3pLibs/keyValueHash/hashtable.h"



static int crawlNFS_keys_equal_fn(void *k1, void *k2)
{

	char *k1s = (char *)k1; 
	char *k2s = (char *)k2; 

    	return (0 == strcmp(k1s,k2s));
}


int crawlinit() {


	return 1;
}

int crawlcanconect(struct collectionFormat *collection) {
	//tester om vi kan koble til
	return 1;
}

int crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {

	struct crawldocumentExistFormat crawldocumentExist;
	struct crawldocumentAddFormat crawldocumentAdd;
	
	char *localfs;

	printf("crawlfirst: start\n");
	

	return 1;
}


int crawlupdate(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {
	//opdate
	printf("crawlupdate: start\n");
	
	
	return 1;
}



struct crawlLibInfoFormat crawlLibInfo = {
        NULL,
        crawlfirst,
        crawlupdate,
        crawlcanconect,
        NULL,
        NULL,
        NULL,
        crawl_security_acl,
        "Push"
};


