#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../crawl/crawl.h"
#include "crawlmysql.h"
#include "scan.h"

int crawlcanconect(struct collectionFormat *collection) {
	//tester om vi kan koble til
	return 1;
}

int crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {

	struct crawldocumentExistFormat crawldocumentExist;
	struct crawldocumentAddFormat crawldocumentAdd;
	
	mysql_recurse((*collection).host,(*collection).user,(*collection).password,(*collection).query1,collection,documentExist,documentAdd);

	return 1;
}

/*
int crawlupdate(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {
	//opdate
	
	
	return 1;
}
*/

struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
        crawlfirst,
        crawlfirst, // bruker bare crawlfirst da vi ikke har noen måte og finne ut hvis gammel en record er
        crawlcanconect,
	NULL,
	scanMySQL,
        crawl_security_acl,
        "MySQL",
        strcrawlError
};


