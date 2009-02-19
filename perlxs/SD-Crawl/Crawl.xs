#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "../../src/crawl/crawl.h"
#include "clib/url.h"
#include "clib/bstr.h"
#include "../../src/common/debug.h"

#include <stdio.h>

int pdocumentContinue(struct cargsF *cargs) {

	if (cargs == NULL) {
		fprintf(stderr,"pdocumentExist: pointer is NULL!\n");
		exit(1);
	}

	return cargs->documentContinue(cargs->collection);	

}


int pdocumentError(struct cargsF *cargs, char error[]) {

	if (cargs == NULL) {
		fprintf(stderr,"pdocumentExist: pointer is NULL!\n");
		exit(1);
	}

	return cargs->documentError(cargs->collection, 1,error);

}


int pdocumentExist(struct cargsF *cargs, char * url, int lastmodified, int dokument_size) {
	int ret;

	debug("pdocumentExist");
	debug("pdocumentExist cargs %p",cargs);

	if (cargs == NULL) {
		fprintf(stderr,"pdocumentExist: pointer is NULL!\n");
		exit(1);
	}

	debug("pdocumentExist cargs->documentExist %p",cargs->documentExist);
	
	debug("pdocumentExist: url: \"%s\", carg p to documentExist %p",url, cargs->documentExist);

	struct crawldocumentExistFormat *crawldocumentExist;

	if ((crawldocumentExist = malloc(sizeof(struct crawldocumentExistFormat))) == NULL) {
		perror("malloc crawldocumentExist");
		return 0;
	}

	crawldocumentExist->documenturi = strdup(url);

	debug("to send \"%s\"",crawldocumentExist->documenturi);
	crawldocumentExist->lastmodified = lastmodified;
	crawldocumentExist->dokument_size = dokument_size;

	ret = cargs->documentExist(cargs->collection,crawldocumentExist);

	free(crawldocumentExist);

	return ret;
}

void pdocumentAdd(struct cargsF *cargs, char * url, int lastmodified, char * document, char title[], 
	char type[], char acl_allow[], char acl_denied[], char attributes[], int doc_size) {

	struct crawldocumentAddFormat *crawldocumentAdd;

	//printf("Adding in XS. Doc size: %d\n", doc_size);

	if (cargs == NULL) {
		fprintf(stderr,"pdocumentExist: pointer is NULL!\n");
		exit(1);
	}

	if ((crawldocumentAdd = malloc(sizeof(struct crawldocumentAddFormat))) == NULL) {
                perror("malloc crawldocumentAdd");
                return;
        }


	crawldocumentAdd->documenturi 	 = url;
	crawldocumentAdd->documenttype   = type;
        crawldocumentAdd->document       = document;
        crawldocumentAdd->dokument_size  = doc_size;
        crawldocumentAdd->lastmodified   = lastmodified;
        crawldocumentAdd->acl_allow      = acl_allow;
        crawldocumentAdd->acl_denied     = acl_denied;
	crawldocumentAdd->title		 = title;
	crawldocumentAdd->doctype        = "";
	crawldocumentAdd->attributes = attributes;

	cargs->documentAdd(cargs->collection ,crawldocumentAdd);

	free(crawldocumentAdd);
}

void
pdocumentChangeCollection(struct cargsF *cargs, char *collection)
{
	cargs->collection->collection_name = collection;
}

MODULE = SD::Crawl		PACKAGE = SD::Crawl		

void
pdocumentChangeCollection(x, collection)
		void * x
		char * collection
	CODE:
		pdocumentChangeCollection(x, collection);

void
pdocumentAdd( x , url , lastmodified, document, title, type, acl_allow, acl_denied, attributes)
		void * x
		char * url
		int lastmodified
		SV * document
		char * title
		char * type
		char * acl_allow
		char * acl_denied
		char * attributes
	CODE:
		//int doc_size = SvLEN(document) - 1; // perl adds \0 ?
		int doc_size;
		char * doc_str = SvPV(document, doc_size);
		pdocumentAdd( x , url , lastmodified, doc_str, title, type, acl_allow, acl_denied, attributes, doc_size);
	

int
pdocumentExist( x , url , lastmodified, dokument_size)
	void * x
	char * url
	int lastmodified
	int dokument_size
	INIT:
		int ret;
	PPCODE:
		ret = pdocumentExist(x, url, lastmodified, dokument_size);
		XPUSHs(sv_2mortal(newSVnv(ret)));


int 
pdocumentContinue(x)

	void * x

        INIT:
                int ret;

        PPCODE:
                ret = pdocumentContinue(x);

                XPUSHs(sv_2mortal(newSVnv(ret)));



int 
pdocumentError(x, errorstring)

	void * x
	char * errorstring


void
htttp_url_normalization(url) 
	char * url

	INIT:
		char urlout[512];

	PPCODE:
		strscpy(urlout,url,sizeof(urlout));
		url_normalization(urlout,sizeof(urlout));

		XPUSHs(sv_2mortal(newSVpv(urlout,0)));
