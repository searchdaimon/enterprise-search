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


void pdocumentExist(struct cargsF *cargs, char * url, int lastmodified, int dokument_size) {

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
		return;
	}

	crawldocumentExist->documenturi = strdup(url);

	debug("to send \"%s\"",crawldocumentExist->documenturi);
	crawldocumentExist->lastmodified = lastmodified;
	crawldocumentExist->dokument_size = dokument_size;

	cargs->documentExist(cargs->collection,crawldocumentExist);

	free(crawldocumentExist);
}

void pdocumentAdd(struct cargsF *cargs, char * url, int lastmodified, char document[], char title[], char type[], char acl_allow[], char acl_denied[], char attributes[]) {

	struct crawldocumentAddFormat *crawldocumentAdd;
        int document_size = (int) strlen(document);

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
        crawldocumentAdd->dokument_size  = document_size;
        crawldocumentAdd->lastmodified   = lastmodified;
        crawldocumentAdd->acl_allow      = acl_allow;
        crawldocumentAdd->acl_denied     = acl_denied;
	crawldocumentAdd->title		 = title;
	crawldocumentAdd->doctype        = "";

	cargs->documentAdd(cargs->collection ,crawldocumentAdd);

	free(crawldocumentAdd);
}

MODULE = SD::Crawl		PACKAGE = SD::Crawl		

void
pdocumentAdd( x , url , lastmodified, document, title, type, acl_allow, acl_denied, attributes)
	int * x
	char * url
	int lastmodified
	char * document
	char * title
	char * type
	char * acl_allow
	char * acl_denied
	char * attributes

void
pdocumentExist( x , url , lastmodified, dokument_size)
	int * x
	char * url
	int lastmodified
	int dokument_size



int 
pdocumentContinue(x)

	int * x

        INIT:
                int ret;

        PPCODE:
                ret = pdocumentContinue(x);

                XPUSHs(sv_2mortal(newSVnv(ret)));



int 
pdocumentError(x, errorstring)

	int * x
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
