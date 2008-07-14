#include <EXTERN.h>
#include <perl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "perlxsi.h"
#include "../crawl/crawl.h"

#include "../common/bstr.h"
#include "../common/boithohome.h"
#include "../common/debug.h"

#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../3pLibs/keyValueHash/hashtable.h"

static PerlInterpreter *my_perl;


void params_to_perlhash(HV *perl_ht, struct hashtable *params) {
    if (!hashtable_count(params)) return;

    struct hashtable_itr *itr;
    itr = hashtable_iterator(params);


    do {
        char *param = hashtable_iterator_key(itr);
        char *value = hashtable_iterator_value(itr);
        
        // check if key already exists
        if (hv_exists(perl_ht, param, strlen(param))) {
            fprintf(stderr, "Parameter '%s' is already defined. Ignoring.\n", param);
            continue;
        }

        hv_store(perl_ht, param, strlen(param),
            sv_2mortal(newSVpv(value, 0)), 0);
        
    } while (hashtable_iterator_advance(itr));
    free(itr);
}

int preprocessAndRun(struct collectionFormat *collection, struct cargsF *cargs, char execute[]) {

	int retn;
	//antar at rutiner som ikke returnerer noe mislykkes. Dette kan for eks skje hvis vi kaller die, eller ikke trenger retur koden
	int success = 0;


	char perlfile[PATH_MAX];
	int perlCache = 0;

	snprintf(perlfile,sizeof(perlfile),"%s/main.pm",collection->crawlLibInfo->resourcepath);

	debug("cargs %p\n",cargs);


	#ifdef DEBUG
		//printer ut pekere til colection info, og alle rutinene
		printf("collection %p, documentExist %p, documentAdd %p, documentError %p, documentContinue %p\n",cargs->collection,cargs->documentExist,cargs->documentAdd,cargs->documentError,cargs->documentContinue);
	#endif

	dSP;
	int i;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);

	//filnavnet
	XPUSHs(sv_2mortal(newSVpv(perlfile, 0) ));

	//mappen, for å inkludere
	XPUSHs(sv_2mortal(newSVpv(collection->crawlLibInfo->resourcepath, 0) ));

	//om vi skal cache perl ting i minne
	XPUSHs(sv_2mortal(newSViv(perlCache))); 

	//hva den skal kalle
	XPUSHs(sv_2mortal(newSVpv(execute, 0) ));

	//pekere til c funksjonene.
	XPUSHs(sv_2mortal(newSViv(PTR2IV(cargs))));

	//lager en ny perl hash
	HV *hv = newHV();
	//hv_store(hv, "key1", 4, sv_2mortal(newSVpv("test", 0)), 0);


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
        params_to_perlhash(hv, collection->params);

	//push the option has inn as a subrutine arg.
	XPUSHs(sv_2mortal(newRV((SV *)hv)));

	PUTBACK;

	//kaller perl
	//call_pv("Perlcrawl::crawlupdate", G_DISCARD | G_EVAL);
	retn = call_pv("Embed::Persistent::eval_file", G_SCALAR | G_EVAL);

	SPAGAIN; //refresh stack pointer

	if (SvTRUE(ERRSV)) {
		printf("An error occurred in the Perl preprocessor: %s",SvPV_nolen(ERRSV));
	}
	else if (retn == 0) {

	}
	else if (retn == 1) {
		//pop the return value, as a int
		success = POPi;
	}
	else {
		fprintf(stderr,"Did return %i valus, but we are supposed to only return 1!\n", retn);
	}

	debug("preprocessAndRun: return nr %i, returning value %i\n",retn,success);

	FREETMPS;
	LEAVE;



	return success;
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
	char blib[512];

	printf("perlCrawlStart(perlpath=%s, name=%s)\n",perlpath, name);


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
	strcpy(crawlLibInfo->shortname,name);
	strcpy(crawlLibInfo->resourcepath,perlpath);

	snprintf( blib,sizeof(blib),"-Mblib=%s",bfile("perlxs/SD-Crawl") );

        //char *perl_args[] = { "", "-Mblib=/home/boitho/boitho/websearch/perlxs/SD-Crawl",  "-I", collection->crawlLibInfo->resourcepath, perlfile, NULL };
        //char *perl_args[] = { "", "-Mblib=/home/boitho/boitho/websearch/perlxs/SD-Crawl",  "-I", bfile("crawlers/Modules/"), bfile2("perl/persistent.pl"), NULL };
        char *perl_args[] = { "", blib,  "-I", bfile("crawlers/Modules/"), bfile2("perl/persistent.pl"), NULL };


	int perl_argsc = 4;
	

	extern char **environ;



        PERL_SYS_INIT3(&argc,&argv,&environ);
        my_perl = perl_alloc();
        perl_construct(my_perl);

        perl_parse(my_perl, xs_init, perl_argsc, perl_args, NULL);

        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	//hvis vi ikke sender med perl coden med -e må den kjøres her, men da får i mindre feil melidinger
	//eval_pv(collection->perlcode, FALSE);


        /*** skipping perl_run() ***/


//        perl_destruct(my_perl);
//        perl_free(my_perl);
//        PERL_SYS_TERM();


	return crawlLibInfo;


}




