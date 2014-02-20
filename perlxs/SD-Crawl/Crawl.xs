#include <err.h>

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "../../src/crawl/crawl.h"
#include "clib/url.h"
#include "clib/bstr.h"
#include "../../src/common/debug.h"
#include "../../src/boitho-bbdn/bbdnclient.h"

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
	char type[], char acl_allow[], char acl_denied[], char attributes[], int doc_size, char *image, int image_size) {

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
	crawldocumentAdd->attributes 	 = attributes;
        crawldocumentAdd->image       	 = image;
        crawldocumentAdd->image_size   	 = image_size;

	cargs->documentAdd(cargs->collection ,crawldocumentAdd);

	free(crawldocumentAdd);
}

void
pdocumentChangeCollection(struct cargsF *cargs, char *collection)
{
	cargs->collection->collection_name = collection;
}

int
paddForeignUser(struct cargsF *cargs, char *user, char *group)
{
	return addForeignUser(cargs->collection->collection_name, user, group);
}

int
premoveForeignUsers(struct cargsF *cargs) 
{
	return removeForeignUsers(cargs->collection->collection_name);
}

void
pcloseCurrentCollection(struct  cargsF *cargs)
{
	extern int global_bbdnport;
	bbdn_closecollection(cargs->collection->socketha, cargs->collection->collection_name);
	bbdn_close(cargs->collection->socketha);
	cargs->collection->socketha = 0;
	bbdn_conect(&cargs->collection->socketha, "", global_bbdnport);
}

unsigned int
pgetLastCrawl(struct cargsF *cargs)
{
	return cargs->collection->lastCrawl;
}

int
pdeleteuri(struct cargsF *cargs, char *subname, char *uri)
{
	return bbdn_deleteuri(cargs->collection->socketha, subname, uri);
}

void
paddwhisper(struct cargsF *cargs, char *subname, int value)
{
	bbdn_addwhisper(cargs->collection->socketha, subname, value);
}

MODULE = SD::Crawl		PACKAGE = SD::Crawl		

void
pcloseCurrentCollection(x)
		void * x
	CODE:
		pcloseCurrentCollection(x);

void
pdocumentChangeCollection(x, collection)
		void * x
		char * collection
	CODE:
		pdocumentChangeCollection(x, collection);

void
pdocumentAdd( x , url , lastmodified, document, title, type, acl_allow, acl_denied, attributes, image)
		void * x
		char * url
		int lastmodified
		SV * document
		char * title
		char * type
		char * acl_allow
		char * acl_denied
		char * attributes
		SV * image
	CODE:
		int doc_size, image_size;
		char * doc_str = SvPV(document, doc_size);
		char * image_str = SvPV(image, image_size);
		pdocumentAdd( x , url , lastmodified, doc_str, title, type, acl_allow, acl_denied, attributes, doc_size, image_str, image_size);
	

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
paddForeignUser(x, user, group)
	void *x
	char *user
	char *group
	INIT:
		int ret;
	PPCODE:
		ret = paddForeignUser(x, user, group);
		
		XPUSHs(sv_2mortal(newSVnv(ret)));

int
premoveForeignUsers(x)
	void *x
	INIT:
		int ret;
	PPCODE:
		ret = premoveForeignUsers(x);
		
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

unsigned int
pget_last_crawl(x)
	void *x
	INIT:
		int ret;
	PPCODE:
		ret = pgetLastCrawl(x);
		
		XPUSHs(sv_2mortal(newSVnv(ret)));

int
pdeleteuri(x, subname, uri)
	void *x
	char *subname
	char *uri
	INIT:
		int ret;
	PPCODE:
		ret = pdeleteuri(x, subname, uri);

		XPUSHs(sv_2mortal(newSVnv(ret)));

void
paddwhisper(x, subname, value)
	void *x
	char *subname
	int value
	CODE:
		paddwhisper(x, subname, value);
