/*
 * Exchange crawler
 *
 * June, 2007
 */

#include <stdio.h>
#define _GNU_SOURCE 1
#include <string.h>

#include "xml.h"
#include "webdav.h"

#include "../crawl/crawl.h"

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
				char *p, *p2;
				// Let's add it
				crawldocumentAdd.documenturi = crawldocumentExist.documenturi;
				/* Find the subject */
				p = strcasestr(mail.buf, "subject:");
				if (p == NULL || *p == '\0') {
					crawldocumentAdd.title = "";
				} else {
					for (p++; *p == ' '; p++)
						;
					p += 8; // strlen("subject:");
					for (p2 = p; *p2 != '\n' && *p2 != '\r' && *p2 != '\0'; p2++)
						;
					crawldocumentAdd.title = strndup(p, p2 - p);
					if (crawldocumentAdd.title == NULL)
						crawldocumentAdd.title = "";
				}
				crawldocumentAdd.documenttype = "eml";
				crawldocumentAdd.doctype = "";
				crawldocumentAdd.document = mail.buf;
				crawldocumentAdd.dokument_size = mail.size-1; // Last byte is string null terminator
				crawldocumentAdd.lastmodified = cur->modified;
				crawldocumentAdd.acl_allow = "Users"; /* XXX */
				crawldocumentAdd.acl_denied = "";

				printf("Adding: '%s'\n", crawldocumentAdd.title);
				(ci->documentAdd)(ci->collection, &crawldocumentAdd);
			}
			free(crawldocumentAdd.title);
			free(crawldocumentExist.documenturi);
			free(mail.buf);
		}
	}
	freeStringList(urls);

	return 1;
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
	int ret;

	xmlGetWarningsDefaultValue = 0;
	listxml = ex_getContent(ci->collection->resource, ci->collection->user, ci->collection->password);
	if (listxml == NULL)
		return 0;
	ret = grabContent(listxml, ci->collection->resource, ci->collection->user, ci->collection->user, ci);
	free(listxml);

	return ret;
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
