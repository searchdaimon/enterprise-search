/*
 * Exchange crawler
 *
 * June, 2007
 */

#include <stdio.h>
#include <string.h>

#include "xml.h"
#include "webdav.h"

#include "../crawl/crawl.h"

#define USERNAME "rb"
#define PASSWORD "Hurra123"

struct crawlinfo {
	int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *);
	int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *);
	int (*documentError)(int, const char *, ...);
	struct collectionFormat *collection;
	unsigned int timefilter;
};

/* Does not detect loops, but they should not happen anyway */
int
grabContent(char *xml, char *url, const char *username, const char *password, struct crawlinfo *ci)
{
	stringListElement *urls, *cur;
	
	urls = getEmailUrls(xml);
	if (urls == NULL)
		return -1;
	//printf("Xml: \n%s\n", xml);
	for (cur = urls; cur; cur = cur->next) {
		size_t len;

		/* Is it the parent? */
		if (strcmp((char *)cur->str, url) == 0)
			continue;
		len = strlen((char *)cur->str);
		if (cur->str[len - 1] == '/') { /* A directory */
			char *listxml;

			listxml = ex_getContent((char *)cur->str, username, password);
			if (listxml == NULL)
				continue;
			grabContent(listxml, (char *)cur->str, username, password, ci);
			free(listxml);
		} else if (len > 4 && strcasecmp((char *)cur->str + (len-4), ".eml") == 0) { /* A mail */
			struct ex_buffer mail;
			struct crawldocumentExistFormat crawldocumentExist;
			struct crawldocumentAddFormat crawldocumentAdd;

			printf("%s\n", cur->str);
			if (ex_getEmail((char *)cur->str, username, password, &mail) == NULL)
				continue;

			crawldocumentExist.documenturi = strdup((char *)cur->str);
			if (crawldocumentExist.documenturi == NULL) {
				(ci->documentError)(1, "Could not allocate memory for documenturi");
				free(crawldocumentExist.documenturi);
				free(mail.buf);
				continue;
			}
			if (ci->timefilter > 0 && ci->timefilter >= crawldocumentExist.lastmodified) {
				printf("Have the same or newer version of %s\n", cur->str);
			} else if ((ci->documentExist)(ci->collection, &crawldocumentExist)) {
				// This document already exists
			} else {
				// Let's add it
				crawldocumentAdd.documenturi = crawldocumentExist.documenturi;
				crawldocumentAdd.title = "SOMETITLE";
				crawldocumentAdd.documenttype = "eml";
				crawldocumentAdd.document = mail.buf;
				crawldocumentAdd.dokument_size = mail.size-1; // Last byte is string null terminator
				crawldocumentAdd.lastmodified = cur->modified;
				crawldocumentAdd.acl_allow = "Users";
				crawldocumentAdd.acl_denied = "";
				//crawldocumentAdd.acl = NULL;

				(ci->documentAdd)(ci->collection, &crawldocumentAdd);
			}

			free(crawldocumentExist.documenturi);
			free(mail.buf);
		}
	}
	freeStringList(urls);

	return 0;
}


int
crawlcanconect(struct collectionFormat *collection,
                   int (*documentError)(int, const char *, ...) __attribute__((unused)))
{
	int connected;
	char *listxml;

	/* XXX: Should we do some more checking here? */
	listxml = ex_getContent(collection->resource, collection->user, collection->password);

	connected = (listxml != NULL);
	free(listxml);

	return connected;
}

int
crawlGo(struct crawlinfo *ci)
{
	char *listxml;

	xmlGetWarningsDefaultValue = 0;
	listxml = ex_getContent(ci->collection->resource, ci->collection->user, ci->collection->password);
	if (listxml == NULL)
		return 1;
	grabContent(listxml, ci->collection->resource, USERNAME, PASSWORD, ci);
	free(listxml);

	return 0;
}

int
crawlfirst(struct collectionFormat *collection,
		int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *),
		int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *),
		int (*documentError)(int, const char *, ...))
{

	struct crawlinfo ci;

	ci.documentExist = documentExist;
	ci.documentAdd = documentAdd;
	ci.documentError = documentError;
	ci.collection = collection;
	printf("crawlEXCHANGE: %s\n", collection->resource);

	return crawlGo(&ci);
}


int
crawlupdate(struct collectionFormat *collection,
		int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *),
		int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *),
		int (*documentError)(int, const char *, ...))
{
	struct crawlinfo ci;

	ci.documentExist = documentExist;
	ci.documentAdd = documentAdd;
	ci.documentError = documentError;
	ci.collection = collection;

	printf("crawlEXCHANGE: %s\n", collection->resource);

	return crawlGo(&ci);
}

struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
	crawlfirst,
	crawlupdate,
	crawlcanconect,
	NULL,
	NULL,
	crawl_security_none,
	"Exchange",
	strcrawlError
};

#if 0

int
main(int argc, char * argv[])
{
	char *listxml;

	xmlGetWarningsDefaultValue = 0;
	listxml = ex_getContent("http://129.241.50.208/exchange/rb/", USERNAME, PASSWORD);
	if (listxml == NULL)
		return 1;
	grabContent(listxml, "http://129.241.50.208/exchange/rb/", USERNAME, PASSWORD, NULL);
	free(listxml);

	return 0;
}

#endif
