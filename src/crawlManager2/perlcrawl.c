#include <err.h>
#include <EXTERN.h>
#include <perl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../perlembed/perlembed.h"
#include "../crawl/crawl.h"

#include "../logger/logger.h"
#include "../common/bstr.h"
#include "../common/boithohome.h"
#include "../common/debug.h"

#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../3pLibs/keyValueHash/hashtable.h"

int preprocessAndRun(struct collectionFormat *collection, struct cargsF *cargs, char execute[]) {

	//int retn;
	//antar at rutiner som ikke returnerer noe mislykkes. Dette kan for eks skje hvis vi kaller die, eller ikke trenger retur koden

	char perlfile[PATH_MAX];

	snprintf(perlfile,sizeof(perlfile),"%s/main.pm",collection->crawlLibInfo->resourcepath);

	bblog(DEBUG, "cargs %p\n",cargs);


	#ifdef DEBUG
		//printer ut pekere til colection info, og alle rutinene
		bblog(DEBUG, "collection %p, documentExist %p, documentAdd %p, documentError %p, documentContinue %p",cargs->collection,cargs->documentExist,cargs->documentAdd,cargs->documentError,cargs->documentContinue);
	#endif

	HV *obj_attr = newHV();
	hv_store(obj_attr, "ptr", strlen("ptr"), sv_2mortal(newSViv(PTR2IV(cargs))), 0);


	HV *hv = newHV();

	//sendes altid med
	hv_store(hv, "last_crawl", strlen("last_crawl"), sv_2mortal(newSVuv(collection->lastCrawl)), 0);

	//sendes bare med hvis vi har verdi
	if (collection->resource != NULL)
		hv_store(hv, "resource", strlen("resource"), sv_2mortal(newSVpv(collection->resource, 0)), 0);
	if (collection->connector != NULL)
		hv_store(hv, "connector", strlen("connector"), sv_2mortal(newSVpv(collection->connector, 0)), 0);
	if (collection->password != NULL)
		hv_store(hv, "password", strlen("password"), sv_2mortal(newSVpv(collection->password, 0)), 0);
	if (collection->query1 != NULL)
		hv_store(hv, "query1", strlen("query1"), sv_2mortal(newSVpv(collection->query1, 0)), 0);
	if (collection->query2 != NULL)
		hv_store(hv, "query2", strlen("query2"), sv_2mortal(newSVpv(collection->query2, 0)), 0);
	if (collection->collection_name != NULL)
		hv_store(hv, "collection_name", strlen("collection_name"), sv_2mortal(newSVpv(collection->collection_name, 0)), 0);
	if (collection->user != NULL)
		hv_store(hv, "user", strlen("user"), sv_2mortal(newSVpv(collection->user, 0)), 0);
	if (collection->userprefix != NULL)
		hv_store(hv, "userprefix", strlen("userprefix"), sv_2mortal(newSVpv(collection->userprefix, 0)), 0);
	if (collection->extra != NULL)
		hv_store(hv, "extra", strlen("extra"), sv_2mortal(newSVpv(collection->extra, 0)), 0);
	if (collection->test_file_prefix != NULL)
		hv_store(hv, "test_file_prefix", strlen("test_file_prefix"), sv_2mortal(newSVpv(collection->test_file_prefix, 0)), 0);


        // Add custom params to hash.
	ht_to_perl_ht(hv, collection->params);

	return perl_embed_run(perlfile, execute, hv, "Perlcrawl", obj_attr);

}

int perlcm_crawlupdate(struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
        int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
        int (*documentContinue)(struct collectionFormat *collection)) {

	struct cargsF cargs;
	int ret;

	cargs.collection 	= collection;
	cargs.documentExist 	= documentExist;
	cargs.documentAdd 	= documentAdd;
	cargs.documentError 	= documentError;
	cargs.documentContinue = documentContinue;


	ret = preprocessAndRun(collection,&cargs,"Perlcrawl::crawl_update");

	//hvis vi fik 0 som retur verdi, er alt ok, og vi returnerer 1
	if (ret == 0) {
		return 1;
	}
	else {
		return ret;
	}
}





int perlcm_crawlcanconect( struct collectionFormat *collection,int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...)) 
{
	return 1;
}

int perlcm_crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection)
	) {

	return perlcm_crawlupdate(collection,documentExist,documentAdd,documentError,documentContinue);
}
int perlcm_crawlpatAcces(char resource[], char username[], char password[], int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...), struct collectionFormat *collection)
{

	struct cargsF cargs;

	struct collectionFormat collectionWithUserData;
	
	//siden preprocessAndRun baserer seg på data i collection, må vi koppoerer brukerens data inn i en ny
	// collection struct, og sende den.
	//dette er kansje litt hårete
	memcpy(&collectionWithUserData,collection,sizeof(collectionWithUserData));


	collectionWithUserData.user 	= username;
	collectionWithUserData.password	= password;
	collectionWithUserData.resource = resource;


	return preprocessAndRun(&collectionWithUserData,&cargs,"Perlcrawl::path_access");

}

struct crawlLibInfoFormat *perlCrawlStart(char perlpath[], char name[]) {
	struct crawlLibInfoFormat *crawlLibInfo;

	if((crawlLibInfo = malloc(sizeof(struct crawlLibInfoFormat))) == NULL) {
		perror("malloc crawlLibInfo");
		return NULL;
	}

	crawlLibInfo->crawlinit 	= NULL;
	crawlLibInfo->crawlfirst 	= perlcm_crawlfirst;
	crawlLibInfo->crawlupdate 	= perlcm_crawlupdate;
	crawlLibInfo->crawlcanconect 	= perlcm_crawlcanconect;
	crawlLibInfo->crawlpatAcces 	= perlcm_crawlpatAcces;
	crawlLibInfo->scan 		= NULL;
	crawlLibInfo->rewrite_url 	= NULL;
	crawlLibInfo->crawl_security 	= 0;
	strncpy(crawlLibInfo->shortname, name, sizeof(crawlLibInfo->shortname));
	strncpy(crawlLibInfo->resourcepath, perlpath, sizeof(crawlLibInfo->resourcepath));
	

	return crawlLibInfo;


}




