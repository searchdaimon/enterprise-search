/*
 * Exchange crawler
 *
 * June, 2007
 * Oktober, 2008:	Registrerer attributter.
 */

#define _GNU_SOURCE 1
#include <sys/param.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "xml.h"
#include "webdav.h"
#include "excrawler.h"
#include "analyze_header.h"

#include "../base64/base64.h"
#include "../crawl/crawl.h"
#include "../common/subject.h"
#include "../common/sid.h"
#include "../common/bprint.h"
#include "../dictionarywordsLot/set.h"
#include "../ds/dcontainer.h"
#include "../ds/dmultimap.h"

int crawlcanconnect(struct collectionFormat *collection,
                   int (*documentError)(struct collectionFormat *, int, const char *, ...) __attribute__((unused)));
int crawlfirst(crawlfirst_args);
int crawlupdate(crawlupdate_args);


int 
ex_rewrite_url(char *uri, enum platform_type ptype, enum browser_type btype)
{
	char *p;

	printf("We got: %s\n", uri);
	p = strchr(uri, '\x10');
	if (p == NULL)
		return 0;
	if (ptype == WINDOWS) {
		printf("Wiiiindows!\n");
		*p = '\0';
	} else {
		size_t len;
		printf("Something else!!\n");
		p++;
		len = strlen(p);
		memmove(uri, p, strlen(p));
		uri[len] = '\0';
	}
	printf("pushing out: %s\n", uri);
	return 1;
}


struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
	crawlfirst,
	crawlupdate,
	crawlcanconnect,
	NULL,
	NULL,
	ex_rewrite_url,
	crawl_security_none,
	"Exchange",
	"",
};



char *
make_crawl_uri(char *uri, char *id)
{
	int len;
	char *p;
	char out[2048], some[5];
	char outlookid[2048];

#if 1
	len = base64_decode(out, id, 1024);
	p = out;
	outlookid[0] = '\0';
	while (len--) {
		sprintf(some, "%.2x", (*p & 0xff));
		strcat(outlookid, some);
		p++;
	}
#endif

	snprintf(out, sizeof(out), "outlook:%s\x10%s", outlookid, uri);
	//sprintf(out, "%s\x10%s", id, uri);
	p = strdup(out);

	return p;
}

void
grab_email(struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *url, char *sid, size_t contentlen, time_t lastmodified, char *usersid, CURL *curl)
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
		(ci->documentError)(ci->collection ,1, "Could not allocate memory for documenturi");
		return;
	}
	if ((ci->documentExist)(ci->collection, &crawldocumentExist)) {
		// This document already exists
	} else {
		char *p, *p2;

		if (ex_getEmail(url, &mail, curl) == NULL) {
			free(crawldocumentExist.documenturi);
			return;
		}

		// Let's add it
		crawldocumentAdd.documenturi = crawldocumentExist.documenturi;
		/* Find the subject */
		//printf("RAW EMAIL:\n%.4096s\n", mail.buf);
		//printf("ANALYZE ON!!\n\n");
		container	*M_header = mail_analyze_header(mail.buf, mail.size-1);
		//iterator	it = multimap_begin(M_header);
		//for (; it.valid; it=multimap_next(it))
		//    {
		//	printf("KEY( %s ): VALUE( %s )\n", (char*)multimap_key(it).ptr, (char*)multimap_val(it).ptr);
		//    }
		//printf("\nANALYZE OFF!!\n");
		//destroy(M_header);
		/*
		p = NULL;
		if (strcasecmp(mail.buf, "subject:") == 0) {
			p = mail.buf;
		} else if ((p = strcasestr(mail.buf, "\nsubject:")) != NULL) {
			p++;
		} else if ((p = strcasestr(mail.buf, "\rsubject:")) != NULL) {
			p++;
		}
		*/
		iterator	it = multimap_begin(M_header);
		p = NULL;
		crawldocumentAdd.attributes = "";
		for (; it.valid; it=multimap_next(it))
		    {
			if (!strcmp("subject", (char*)multimap_key(it).ptr))
			    p = (char*)multimap_val(it).ptr;
			#ifdef EMAIL_ADRESS_AS_ATTRIBUTE
			else if (!strcmp("from", (char*)multimap_key(it).ptr))
			    asprintf(&crawldocumentAdd.attributes, "from=%s",(char*)multimap_val(it).ptr);
			#endif
		    }
		
		if (p == NULL) {
			crawldocumentAdd.title = "";
		} else {
			//p += 8; // strlen("subject:");
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

		//hvis vi har blitt sendt en user sid så bruker vi den som acl.
		if (usersid == NULL) {
			crawldocumentAdd.acl_allow = set_to_string(acl_allow, ",");
			crawldocumentAdd.acl_denied = set_to_string(acl_deny, ",");
		}
		else {
			crawldocumentAdd.acl_allow = strdup(usersid);
			crawldocumentAdd.acl_denied = strdup("");
		}
/*
		buffer		*B = buffer_init(-1);
		int		to_i=1, from_i=1;
		it = multimap_begin(M_header);
		for (; it.valid; it=multimap_next(it))
		    {
			if (!strcmp("from", (char*)multimap_key(it).ptr) && from_i<=4)
			    bprintf(B, "from_%i=%s,", from_i++, multimap_val(it).ptr);
			else if (!strcmp("to", (char*)multimap_key(it).ptr) && to_i<=4)
			    bprintf(B, "to_%i=%s,", to_i++, multimap_val(it).ptr);
		    }

		crawldocumentAdd.attributes = buffer_exit(B);
*/
		destroy(M_header);

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
grabContent(char *xml, char *url, struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *usersid, CURL *curl)
{
	acl_allow = malloc(sizeof(*acl_allow));
	acl_deny = malloc(sizeof(*acl_deny));
	set_init(acl_allow);
	set_init(acl_deny);
	getEmailUrls(xml, ci, url, acl_allow, acl_deny, usersid, curl);
	set_free_all(acl_allow);
	set_free_all(acl_deny);
	free(acl_allow);
	free(acl_deny);

	return 1;
}

void splitUserString(char *userString,  char **user, char **usersid) {

	char *p;
	printf("userString %s\n",userString);

	p = strchr(userString,':');
	if (p == NULL) {
		*user = userString;
		*usersid = NULL;
	}
	else {
		p[0] = '\0';
		++p;
		*user = userString;
		*usersid = p;
	}
	
	printf("user \"%s\"\nusersid \"%s\"\n",*user,*usersid);
}

int
crawlcanconnect(struct collectionFormat *collection,
                   int (*documentError)(struct collectionFormat *, int, const char *, ...) __attribute__((unused)))
{
	char *listxml;
	char origresource[PATH_MAX];
	char resource[PATH_MAX];
	char *userString;
	char *user, *usersid;
	char **users;
	char *eerror;
	CURL * curl;

	int n_users = 0;

	if (strstr(collection->resource, "://")) {
		snprintf(origresource, sizeof(origresource), "%s", collection->resource);
	} else {
		snprintf(origresource, sizeof(origresource), "http://%s", collection->resource);
	}

	for (users = collection->users; users && *users; users++) {
		splitUserString(*users,&user, &usersid);
		user = *users;

		snprintf(resource, sizeof(resource), "%s/exchange/%s/", origresource, user);
		printf("Resource: %s\n", resource);
		/* Shut up the xml parser a bit */
		xmlGetWarningsDefaultValue = 0;
		if ((curl = ex_logOn(resource, origresource, collection->user, collection->password, &eerror)) == NULL) {
			documentError(collection, 1, "Unable to connect to %s: %s\n", origresource, eerror);
			return 0;

		}
		listxml = ex_getContent(resource, curl);
		//listxml = NULL;
		//printf("%s\n", listxml);
		if (listxml != NULL) {
			if (strcmp(listxml, "<html><head><title>Error</title></head><body>Error: Access is Denied.</body></html>") == 0) {
				free(listxml);
				continue;
			}
			free(listxml);
			ex_logOff(curl);
			return 1;
		}

		ex_logOff(curl);
	}

	if (n_users == 0)
		return 1;

	documentError(collection, 1, "Unable to connect to: %s\n", origresource);
	documentError(collection, 1, "Html error: %s\n", listxml);


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
	char **users;
	char *user, *usersid;
	char *eerror;
	set *acl_allow, *acl_deny;
	CURL *curl;

	normalize_url(ci->collection->resource);

	if (strstr(ci->collection->resource, "://")) {
		snprintf(origresource, sizeof(origresource), "%s", ci->collection->resource);
	} else {
		snprintf(origresource, sizeof(origresource), "http://%s", ci->collection->resource);
	}

	err = 0;
	for (users = ci->collection->users; users && *users; users++) {
		//user = *users;
		splitUserString(*users,&user, &usersid);

		if (!ci->documentContinue(ci->collection))
			break;

		snprintf(resource, sizeof(resource), "%s/exchange/%s/", origresource, user);
		printf("Trying %s\n", resource);
		/* Shut up the xml parser a bit */
		xmlGetWarningsDefaultValue = 0;
		if ((curl = ex_logOn(resource, origresource, ci->collection->user, ci->collection->password, &eerror)) == NULL) {
			fprintf(stderr,"Can't connect to %s: %s\n",resource,eerror);
			continue;
		}
		listxml = ex_getContent(resource, curl);
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
			if (!grabContent(listxml, resource, ci, acl_allow, acl_deny, usersid, curl))
				err++;
			set_free_all(acl_allow);
			set_free_all(acl_deny);
			free(acl_allow);
			free(acl_deny);
			free(listxml);
		}

		ex_logOff(curl);
	}

	return 1;
	//return err == 0 ? 0 : 1;
}

int
crawlfirst(crawlfirst_args)
{

	struct crawlinfo ci;

	ci.documentExist = documentExist;
	ci.documentAdd = documentAdd;
	ci.documentError = documentError;
	ci.documentContinue = documentContinue;
	ci.collection = collection;
	ci.timefilter = 0;
	printf("crawlEXCHANGE: %s\n", collection->resource);

	return crawlGo(&ci);
}


int
crawlupdate(crawlupdate_args)
{
	struct crawlinfo ci;

	ci.documentExist = documentExist;
	ci.documentAdd = documentAdd;
	ci.documentError = documentError;
	ci.documentContinue = documentContinue;
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
	listxml = ex_getContent("http://213.179.58.125/exchange/en/", "exchangeCrawler", "1234Asd" );
	//listxml = ex_getContent("http://213.179.58.125/exchange/exchangeCrawler/", "exchangeCrawler", "1234Asd");
	if (listxml == NULL)
		return 1;
	printf("Got: %s\n", listxml);
	//grabContent(listxml, "http://129.241.50.208/exchange/rb/", USERNAME, PASSWORD, NULL);
	free(listxml);

	return 0;
}

#endif
