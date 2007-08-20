/*
 * Exchange crawler
 *
 * June, 2007
 */

#include <sys/param.h>

#include <stdio.h>
#define _GNU_SOURCE 1
#include <string.h>

#include "xml.h"
#include "webdav.h"

#include "../base64/base64.h"
#include "../crawl/crawl.h"

struct crawlinfo {
	int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *);
	int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *);
	int (*documentError)(int, const char *, ...);
	struct collectionFormat *collection;
	unsigned int timefilter;
};

int crawlcanconect(struct collectionFormat *collection,
                   int (*documentError)(int, const char *, ...) __attribute__((unused)));
int crawlfirst(struct collectionFormat *collection,
		int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *),
		int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *),
		int (*documentError)(int, const char *, ...));
int crawlupdate(struct collectionFormat *collection,
		int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *),
		int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *),
		int (*documentError)(int, const char *, ...));


int 
ex_rewrite_url(char *uri, enum platform_type ptype, enum browser_type btype)
{

	//sprintf(uri, "%s", uri);

	return 1;
}


struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
	crawlfirst,
	crawlupdate,
	crawlcanconect,
	NULL,
	NULL,
	NULL,
	crawl_security_none,
	"Exchange",
	strcrawlError
};



char *
make_crawl_uri(char *uri, char *id)
{
	int len;
	int i;
	char *p;
	char out[1024], some[5];
	char outlookid[1024];

	len = base64_decode(out, id, 1024);
	p = out;
	outlookid[0] = '\0';
	while (len--) {
		sprintf(some, "%.2x", (*p & 0xff));
		strcat(outlookid, some);
		p++;
	}

	sprintf(out, "outlook:%s", outlookid);
/*
	i = strlen(crawlLibInfo.shortname);
	len = strlen(uri) + i + 1 + 1;
	p = malloc(len);
	strcpy(p, crawlLibInfo.shortname);
	strcpy(p + i, "|");
	strcpy(p + i + 1, uri);
	*/
	//p = strdup(uri);
	p = strdup(out);

	return p;
}

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

			crawldocumentExist.documenturi = make_crawl_uri((char *)cur->str, cur->id);
			crawldocumentExist.lastmodified = cur->modified;
			if (crawldocumentExist.documenturi == NULL) {
				(ci->documentError)(1, "Could not allocate memory for documenturi");
				free(crawldocumentExist.documenturi);
				free(mail.buf);
				continue;
			}
			if (ci->timefilter > 0 && ci->timefilter > crawldocumentExist.lastmodified) {
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
				crawldocumentAdd.acl_allow = NULL;
				p = strstr(url, "/exchange/");
				if (p != NULL) {
					p += strlen("/exchange/");
					p2 = strchr(p, '/');
					if (p2 != NULL) 
						crawldocumentAdd.acl_allow = strndup(p, p2 - p); /* XXX: Anything that should be added? */
				}
				if (crawldocumentAdd.acl_allow == NULL)
					crawldocumentAdd.acl_allow = "";
				crawldocumentAdd.acl_denied = "";

				printf("Adding: '%s'\n", crawldocumentAdd.title);
				(ci->documentAdd)(ci->collection, &crawldocumentAdd);
			}
			free(crawldocumentAdd.title);
			free(crawldocumentExist.documenturi);
			if (crawldocumentAdd.acl_allow[0] != '\0')
				free(crawldocumentAdd.acl_allow);
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
	char *listxml;
	char origresource[PATH_MAX];
	char resource[PATH_MAX];
	char *user;
	char **users;
	
	if (strstr(collection->resource, "://")) {
		snprintf(origresource, sizeof(origresource), "%s/exchange", collection->resource);
	} else {
		snprintf(origresource, sizeof(origresource), "http://%s/exchange", collection->resource);
	}

	for (users = collection->users; users && *users; users++) {
		user = *users;

		snprintf(resource, sizeof(resource), "%s/%s/", origresource, user);
		printf("Resource: %s\n", resource);
		/* Shut up the xml parser a bit */
		xmlGetWarningsDefaultValue = 0;
		listxml = ex_getContent(resource, collection->user, collection->password);
		//listxml = NULL;
		if (listxml != NULL) {
			free(listxml);
			return 1;
		}
	}

	return 0;
}

int
crawlGo(struct crawlinfo *ci)
{
	char *listxml;
	int err;
	char origresource[PATH_MAX];
	char resource[PATH_MAX];
	char *user;
	char **users;
#if 0
	char *fakeusers[] = {
		"rb",
		"eirik",
		"dagur",
		NULL
	};
#endif
	
	//users = "eirik rb runarb";
	if (strstr(ci->collection->resource, "://")) {
		snprintf(origresource, sizeof(origresource), "%s/exchange", ci->collection->resource);
	} else {
		snprintf(origresource, sizeof(origresource), "http://%s/exchange", ci->collection->resource);
	}

	err = 0;
	for (users = ci->collection->users; users && *users; users++) {
	//for (users = fakeusers; *users; users++) {
		user = *users;

		snprintf(resource, sizeof(resource), "%s/%s/", origresource, user);
		/* Shut up the xml parser a bit */
		xmlGetWarningsDefaultValue = 0;
		listxml = ex_getContent(resource, ci->collection->user, ci->collection->password);
		//listxml = NULL;
		if (listxml == NULL) {
			err++;
		} else {
			if (!grabContent(listxml, resource, ci->collection->user, ci->collection->password, ci))
				err++;
			free(listxml);
		}
	}

	return 1;
	//return err == 0 ? 0 : 1;
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
	ci.timefilter = 0;
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
	ci.timefilter = collection->lastCrawl;

	printf("crawlEXCHANGE: %s\n", collection->resource);

	return crawlGo(&ci);
}
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
