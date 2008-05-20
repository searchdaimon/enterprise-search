#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "../../src/crawl/crawl.h"


#include <stdio.h>

int pdocumentContinue(struct cargsF *cargs) {

	return cargs->documentContinue(cargs->collection);	

}


int pdocumentError(struct cargsF *cargs, char error[]) {

	return cargs->documentError(1,error);

}


void pdocumentExist(struct cargsF *cargs, char * url, int lastmodified, int dokument_size) {


	//printf("pdocumentExist: url: \"%s\"\n",url);
	struct crawldocumentExistFormat *crawldocumentExist;

	if ((crawldocumentExist = malloc(sizeof(struct crawldocumentExistFormat))) == NULL) {
		perror("malloc crawldocumentExist");
		return;
	}

	crawldocumentExist->documenturi = strdup(url);

	printf("to send \"%s\"\n",crawldocumentExist->documenturi);
	crawldocumentExist->lastmodified = lastmodified;
	crawldocumentExist->dokument_size = dokument_size;

	cargs->documentExist(cargs->collection,crawldocumentExist);

	free(crawldocumentExist);
}

void pdocumentAdd(struct cargsF *cargs, char * url, int lastmodified, int dokument_size, char document[], char title[], char type[], char acl_allow[], char acl_denied[]) {

	struct crawldocumentAddFormat *crawldocumentAdd;

	if ((crawldocumentAdd = malloc(sizeof(struct crawldocumentAddFormat))) == NULL) {
                perror("malloc crawldocumentAdd");
                return;
        }

	crawldocumentAdd->documenturi 	 = url;
	crawldocumentAdd->documenttype   = type;
        crawldocumentAdd->document       = document;
        crawldocumentAdd->dokument_size  = dokument_size;
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
pdocumentAdd( x , url , lastmodified, dokument_size, document, title, type, acl_allow, acl_denied)
	int * x
	char * url
	int lastmodified
	int dokument_size
	char * document
	char * title
	char * type
	char * acl_allow
	char * acl_denied

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
