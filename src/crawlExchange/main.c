/*
 * Exchange crawler
 *
 * June, 2007
 */

#define _GNU_SOURCE 1
#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "xml.h"
#include "webdav.h"
#include "excrawler.h"

#include "../base64/base64.h"
#include "../crawl/crawl.h"
#include "../common/subject.h"
#include "../common/sid.h"
#include "../dictionarywordsLot/set.h"

int crawlcanconnect(struct collectionFormat *collection,
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
	return 1;
}


struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
	crawlfirst,
	crawlupdate,
	crawlcanconnect,
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
	p = strdup(out);

	return p;
}

void
grab_email(struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *url, char *sid, size_t contentlen, time_t lastmodified)
{
	size_t len;

	/* Is it the parent? */
	struct ex_buffer mail;
	struct crawldocumentExistFormat crawldocumentExist;
	struct crawldocumentAddFormat crawldocumentAdd;

	len = strlen(url);

	//printf("%s\n", cur->str);
	crawldocumentExist.documenturi = make_crawl_uri(url, sid);
	crawldocumentExist.lastmodified = lastmodified;
	if (crawldocumentExist.documenturi == NULL) {
		(ci->documentError)(1, "Could not allocate memory for documenturi");
		return;
	}
	if ((ci->documentExist)(ci->collection, &crawldocumentExist)) {
		// This document already exists
	} else {
		char *p, *p2;

		if (ex_getEmail(url, ci->collection->user, ci->collection->password, &mail) == NULL) {
			free(crawldocumentExist.documenturi);
			return;
		}

		// Let's add it
		crawldocumentAdd.documenturi = crawldocumentExist.documenturi;
		/* Find the subject */
		p = NULL;
		if (strcasecmp(mail.buf, "subject:") == 0) {
			p = mail.buf;
		} else if ((p = strcasestr(mail.buf, "\nsubject:")) != NULL) {
			p++;
		} else if ((p = strcasestr(mail.buf, "\rsubject:")) != NULL) {
			p++;
		} 
		
		if (p == NULL) {
			crawldocumentAdd.title = "";
		} else {
			p += 8; // strlen("subject:");
			while (isspace(*p)) {
				p++;
			}
			for (p2 = p; *p2 != '\n' && *p2 != '\r' && *p2 != '\0'; p2++)
				;
			if (p2 - p > 0) {
				crawldocumentAdd.title = strndup(p, p2 - p);
				fix_subject(crawldocumentAdd.title, p2-p+1);
			} else {
				crawldocumentAdd.title = NULL;
			}
			if (crawldocumentAdd.title == NULL)
				crawldocumentAdd.title = "";
		}
		crawldocumentAdd.documenttype = "eml";
		crawldocumentAdd.doctype = "";
		crawldocumentAdd.document = mail.buf;
		crawldocumentAdd.dokument_size = mail.size-1; // Last byte is string null terminator
		crawldocumentAdd.lastmodified = lastmodified;
		crawldocumentAdd.acl_allow = set_to_string(acl_allow, ",");
		crawldocumentAdd.acl_denied = set_to_string(acl_deny, ",");

		printf("Adding: '%s'\n", crawldocumentAdd.title);
		(ci->documentAdd)(ci->collection, &crawldocumentAdd);
		if (crawldocumentAdd.title[0] != '\0')
			free(crawldocumentAdd.title);
		free(crawldocumentAdd.acl_allow);
		free(crawldocumentAdd.acl_denied);

		free(mail.buf);
	}
	free(crawldocumentExist.documenturi);
}

/* Does not detect loops, but they should not happen anyway */
int
grabContent(char *xml, char *url, struct crawlinfo *ci, set *acl_allow, set *acl_deny)
{
	acl_allow = malloc(sizeof(*acl_allow));
	acl_deny = malloc(sizeof(*acl_deny));
	set_init(acl_allow);
	set_init(acl_deny);
	getEmailUrls(xml, ci, url, acl_allow, acl_deny);
	set_free_all(acl_allow);
	set_free_all(acl_deny);
	free(acl_allow);
	free(acl_deny);

	return 1;
}


int
crawlcanconnect(struct collectionFormat *collection,
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
		//printf("%s\n", listxml);
		if (listxml != NULL) {
			if (strcmp(listxml, "<html><head><title>Error</title></head><body>Error: Access is Denied.</body></html>") == 0) {
				free(listxml);
				continue;
			}
			free(listxml);
			return 1;
		}
	}

	documentError(1, "Unable to connect to: %s\n", origresource);

	return 0;
}

static void
normalize_url(char *res)
{
	size_t len, i, cur;
	char *p;

	/* Normalize the url a bit */
	cur = i = 0;

	/* Skip past the protocol */
	if ((p = strstr(res, "://")) == NULL)
		p = res;
	else
		p += 3;

	/* Remove duplicate /'es */
	while (p[i] != '\0') {
		if (p[cur-1] == '/' && p[i] == '/') {
			i++;
		} else {
			p[cur] = p[i];
			i += 1;
			cur += 1;
		}
	}
	p[cur] = '\0';

	/* Remove trailing / if any */
	len = strlen(res);
	if (res[len-1] == '/') {
		res[len-1] = '\0';
	}
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
	set *acl_allow, *acl_deny;
	
	normalize_url(ci->collection->resource);

	if (strstr(ci->collection->resource, "://")) {
		snprintf(origresource, sizeof(origresource), "%s/exchange", ci->collection->resource);
	} else {
		snprintf(origresource, sizeof(origresource), "http://%s/exchange", ci->collection->resource);
	}

	err = 0;
	for (users = ci->collection->users; users && *users; users++) {
		user = *users;

		snprintf(resource, sizeof(resource), "%s/%s/", origresource, user);
		printf("Trying %s\n", resource);
		/* Shut up the xml parser a bit */
		xmlGetWarningsDefaultValue = 0;
		listxml = ex_getContent(resource, ci->collection->user, ci->collection->password);
		//listxml = NULL;
		if (listxml == NULL) {
			err++;
		} else {
			//printf("Got xml: \n%s\n", listxml);
			// Set up ACL lists
			acl_allow = malloc(sizeof(*acl_allow));
			acl_deny = malloc(sizeof(*acl_deny));
			set_init(acl_allow);
			set_init(acl_deny);
			if (!grabContent(listxml, resource, ci, acl_allow, acl_deny))
				err++;
			set_free_all(acl_allow);
			set_free_all(acl_deny);
			free(acl_allow);
			free(acl_deny);
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
	//listxml = ex_getContent("http://129.241.50.208/exchange/rb/", USERNAME, PASSWORD);
	//listxml = ex_getContent("http://213.179.58.125/exchange/en/", "en", "1234Asd");
	listxml = ex_getContent("http://213.179.58.125/exchange/en/", "exchangeCrawler", "1234Asd");
	//listxml = ex_getContent("http://213.179.58.125/exchange/exchangeCrawler/", "exchangeCrawler", "1234Asd");
	if (listxml == NULL)
		return 1;
	printf("Got: %s\n", listxml);
	//grabContent(listxml, "http://129.241.50.208/exchange/rb/", USERNAME, PASSWORD, NULL);
	free(listxml);

	return 0;
}

#endif
