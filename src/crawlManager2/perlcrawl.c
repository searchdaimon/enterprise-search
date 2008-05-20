#include <EXTERN.h>
#include <perl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "perlxsi.h"
#include "../crawl/crawl.h"

static PerlInterpreter *my_perl;



void preprocess_crawlupdate(struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
        int (*documentError)(int level, const char *fmt, ...),
        int (*documentContinue)(struct collectionFormat *collection)) {


	struct cargsF *cargs = malloc(sizeof(struct cargsF));

	cargs->collection 	= collection;
	cargs->documentExist 	= documentExist;
	cargs->documentAdd 	= documentAdd;
	cargs->documentError 	= documentError;
	cargs->documentContinue = documentContinue;

	#ifdef DEBUG
		//printer ut pekere til colection info, og alle rutinene
		printf("collection %p, documentExist %p, documentAdd %p, documentError %p, documentContinue %p\n",cargs->collection,cargs->documentExist,cargs->documentAdd,cargs->documentError,cargs->documentContinue);
	#endif

	dSP;
	int i;
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);


	XPUSHs(sv_2mortal(newSViv(PTR2IV(cargs))));


	PUTBACK;

	//kaller perl
	call_pv("Perlcrawl::crawlupdate", G_DISCARD | G_EVAL);

	if (SvTRUE(ERRSV)) {
		printf("An error occurred in the Perl preprocessor: %s",SvPV_nolen(ERRSV));
		return; 
	}


	FREETMPS;
	LEAVE;

	free(cargs);
}

int perlcm_crawlupdate(struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
        int (*documentError)(int level, const char *fmt, ...),
        int (*documentContinue)(struct collectionFormat *collection)) {


        //char *perl_args[] = { "", "-Mblib=/home/boitho/boitho/websearch/perlxs/SD-Crawl", "/home/boitho/boitho/websearch/src/crawlManager2/Hermes.pl", NULL };
        //char *perl_args[] = { "", "-Mblib=/home/boitho/boitho/websearch/perlxs/SD-Crawl", "-e", "0", NULL };
        char *perl_args[] = { "", "-Mblib=/home/boitho/boitho/websearch/perlxs/SD-Crawl", "-e", collection->perlcode, NULL };
	int perl_argsc = 3;
	

	extern char **environ;


        PERL_SYS_INIT3(&argc,&argv,&environ);
        my_perl = perl_alloc();
        perl_construct(my_perl);

        perl_parse(my_perl, xs_init, perl_argsc, perl_args, NULL);

        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	//hvis vi ikke sender med perl coden med -e må den kjøres her, men da får i mindre feil melidinger
	eval_pv(collection->perlcode, FALSE);


        /*** skipping perl_run() ***/

	preprocess_crawlupdate(collection,documentExist,documentAdd,documentError,documentContinue);

        perl_destruct(my_perl);
        perl_free(my_perl);
        PERL_SYS_TERM();

	return 1;
}





int perlcm_crawlcanconect( struct collectionFormat *collection,int (*documentError)(int level, const char *fmt, ...)) 
{
	return 1;
}

int perlcm_crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(int level, const char *fmt, ...),
	int (*documentContinue)(struct collectionFormat *collection)
	) {

	return perlcm_crawlupdate(collection,documentExist,documentAdd,documentError,documentContinue);
}



struct crawlLibInfoFormat *perlCrawlStart() {


	static struct crawlLibInfoFormat crawlLibInfo = {
		NULL,
        	perlcm_crawlfirst,
		perlcm_crawlupdate,
        	perlcm_crawlcanconect,
		NULL,
		NULL,
		NULL,
        	NULL,
        	"PerlCrawl",
		strcrawlError
	};

	printf("perlCrawlStart\n");

	return &crawlLibInfo;
}




