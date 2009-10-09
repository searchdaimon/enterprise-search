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
#include "../logger/logger.h"

#define MAX_ATTR_LEN 1024

int crawlcanconnect(struct collectionFormat *collection,
                   int (*documentError)(struct collectionFormat *, int, const char *, ...) __attribute__((unused)));
int crawlfirst(crawlfirst_args);
int crawlupdate(crawlupdate_args);


int 
ex_rewrite_url(struct collectionFormat *collection, char *url, char *uri, char *fulluri, size_t len, enum platform_type ptype, enum browser_type btype)
{
	char *p;

	bblog(DEBUG, "We got: %s", url);
	p = strchr(url, '\x10');
	if (p == NULL)
		return 0;
	if (ptype == WINDOWS) {
		//printf("Wiiiindows!\n");
		*p = '\0';
	} else {
		size_t len;
		//printf("Something else!!\n");
		p++;
		len = strlen(p);
		memmove(url, p, strlen(p));
		url[len] = '\0';
	}
	strcpy(uri, "Exchange");
	strcpy(fulluri, "Exchange");
	//printf("pushing out: %s\n", url);
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
#ifdef WITH_PUBLIC_FOLDERS
	"ExchangePublic",
#else
	"Exchange",
#endif
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

char *parse_mail_header(container *M_header, char **attr_dst, char **title_dst) {
	char *p = NULL, *p2;
	char attr[MAX_ATTR_LEN];
	int pos = 0;

	iterator it = multimap_begin(M_header);
	int num_receivers = 0;

	for (; it.valid; it=multimap_next(it)) {
		if (strcmp("subject", (char *) multimap_key(it).ptr) == 0)
			p = (char*)multimap_val(it).ptr;
#ifdef EMAIL_ADRESS_AS_ATTRIBUTE
		else if (!strcmp("from", (char*) multimap_key(it).ptr)) {
			pos += snprintf(&(attr[pos]), MAX_ATTR_LEN - pos, 
				"from=%s,", (char *) multimap_val(it).ptr);
			//asprintf(&crawldocumentAdd.attributes, "from=%s,",

		}
		else if (strcmp("to", (char *) multimap_key(it).ptr) == 0) {
			// TODO: Support more than one 'to' field
			if (num_receivers++) continue;

			pos += snprintf(&(attr[pos]), MAX_ATTR_LEN - pos, 
				"to=%s,", (char *) multimap_val(it).ptr);
		}
#endif
	}
	pos += snprintf(&(attr[pos]), MAX_ATTR_LEN - pos, 
		"num_receivers=%d,", num_receivers);
	*attr_dst = strdup(attr);

	if (p == NULL) {
		*title_dst = "";
	} else {
		while (isspace(*p)) {
			p++;
		}
		for (p2 = p; *p2 != '\n' && *p2 != '\r' && *p2 != '\0'; p2++)
			;
		if (p2 - p > 0) {
			*title_dst = strndup(p, p2 - p);
			fix_subject(*title_dst, p2-p+1);
		} else {
			*title_dst = NULL;
		}
		if (*title_dst == NULL)
			*title_dst = "";
	}
}

void
grab_email(struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *url, char *sid, size_t contentlen, time_t lastmodified, char *usersid, CURL **curl)
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
#if 0
		iterator	it = multimap_begin(M_header);
		for (; it.valid; it=multimap_next(it))
		    {
			printf("KEY( %s ): VALUE( %s )\n", (char*)multimap_key(it).ptr, (char*)multimap_val(it).ptr);
		    }
		printf("\nANALYZE OFF!!\n");
		destroy(M_header);
		p = NULL;
#endif

		parse_mail_header(M_header, 
			&crawldocumentAdd.attributes, 
			&crawldocumentAdd.title);
		destroy(M_header);
		

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
#if 0
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
#endif

		bblog(INFO, "Adding: '%s'", crawldocumentAdd.title);
		bblog(INFO, "usersid \"%s\"acl_allow \"%s\" acl_denied \"%s\"",usersid,crawldocumentAdd.acl_allow , crawldocumentAdd.acl_denied);
		(ci->documentAdd)(ci->collection, &crawldocumentAdd);
		if (crawldocumentAdd.title[0] != '\0')
			free(crawldocumentAdd.title);
		free(crawldocumentAdd.acl_allow);
		free(crawldocumentAdd.acl_denied);
#ifdef EMAIL_ADRESS_AS_ATTRIBUTE
		bblog(INFO, "attributes: %s", crawldocumentAdd.attributes);
		free(crawldocumentAdd.attributes);
#else
		bblog(WARN, "not compiled with attributes");
#endif

		free(mail.buf);
	}
	free(crawldocumentExist.documenturi);
}

/* Does not detect loops, but they should not happen anyway */
int
grabContent(char *xml, char *url, struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *usersid, CURL **curl, struct loginInfoFormat *login)
{
	acl_allow = malloc(sizeof(*acl_allow));
	acl_deny = malloc(sizeof(*acl_deny));
	set_init(acl_allow);
	set_init(acl_deny);
	getEmailUrls(xml, ci, url, acl_allow, acl_deny, usersid, curl, login);
	set_free_all(acl_allow);
	set_free_all(acl_deny);
	free(acl_allow);
	free(acl_deny);

	return 1;
}

void splitUserString(char *userString,  char **user, char **usersid) {

	char *p;

	bblog(INFO, "userString %s",userString);

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
	
	bblog(INFO, "splitUserString: user \"%s\" splitUserString: usersid \"%s\"",*user,*usersid);
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
	int publicdone;
	struct loginInfoFormat login;

	int n_users = 0;

	if (strstr(collection->resource, "://")) {
		snprintf(origresource, sizeof(origresource), "%s", collection->resource);
	} else {
		snprintf(origresource, sizeof(origresource), "http://%s", collection->resource);
	}

#ifdef WITH_PUBLIC_FOLDERS
	publicdone = 0;
#else
	publicdone = 1;
#endif
	for (users = collection->users; (users && *users) || publicdone == 0; users++) {
		if (users && *users) {
			splitUserString(*users,&user, &usersid);
			user = *users;

			snprintf(resource, sizeof(resource), "%s/exchange/%s/", origresource, user);
		} else { // Public folder
			snprintf(resource, sizeof(resource), "%s/public/", origresource);
			publicdone = 1;
		}
		bblog(INFO, "Resource: %s", resource);

		/* Shut up the xml parser a bit */
		xmlGetWarningsDefaultValue = 0;

		strscpy( login.username,collection->user,sizeof(login.username) );
		strscpy( login.password,collection->password,sizeof(login.password) );
		strscpy( login.Exchangeurl,origresource,sizeof(login.Exchangeurl) );


		if ((curl = ex_logOn(resource, &login, &eerror)) == NULL) {
			documentError(collection, 1, "Unable to connect to %s: %s", origresource, eerror);
			return 0;

		}
		listxml = ex_getContent(resource, &curl, &login);
		//listxml = NULL;
		//printf("%s\n", listxml);
		if (listxml != NULL) {
			/* XXX */
			if (strcmp(listxml, "<html><head><title>Error</title></head><body>Error: Access is Denied.</body></html>") == 0) {
				free(listxml);
				continue;
			}
			free(listxml);
			ex_logOff(&curl);
			return 1;
		}

		ex_logOff(&curl);
	}

	if (n_users == 0)
		return 1;

	documentError(collection, 1, "Unable to connect to: %s", origresource);
	documentError(collection, 1, "Html error: %s", listxml);


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
	char *user, *usersid=NULL;
	char *eerror;
	set *acl_allow, *acl_deny;
	CURL *curl;
	int publicdone;
	struct loginInfoFormat login;

	normalize_url(ci->collection->resource);

	if (strstr(ci->collection->resource, "://")) {
		snprintf(origresource, sizeof(origresource), "%s", ci->collection->resource);
	} else {
		snprintf(origresource, sizeof(origresource), "http://%s", ci->collection->resource);
	}

#ifdef WITH_PUBLIC_FOLDERS
	publicdone = 0;
#else
	publicdone = 1;
#endif
	err = 0;
	for (users = ci->collection->users; (users && *users) || publicdone == 0; (users && *users) ? users++ : publicdone++) {
		if (!ci->documentContinue(ci->collection))
			break;

		if (users && *users) {
			user = *users;
			splitUserString(*users,&user, &usersid);
			snprintf(resource, sizeof(resource), "%s/exchange/%s/", origresource, user);
		} else { // Public folder
			snprintf(resource, sizeof(resource), "%s/public/", origresource);
			publicdone = 1;
		}

		bblog(INFO, "Trying %s", resource);
		/* Shut up the xml parser a bit */
		xmlGetWarningsDefaultValue = 0;

		strscpy( login.username,ci->collection->user,sizeof(login.username) );
		strscpy( login.password,ci->collection->password,sizeof(login.password) );
		strscpy( login.Exchangeurl,origresource,sizeof(login.Exchangeurl) );

		if ((curl = ex_logOn(resource, &login, &eerror)) == NULL) {
			bblog(ERROR, "Can't connect to %s: %s",resource,eerror);
			continue;
		}
		listxml = ex_getContent(resource, &curl, &login);
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
			if (!grabContent(listxml, resource, ci, acl_allow, acl_deny, usersid, &curl, &login))
				err++;
			set_free_all(acl_allow);
			set_free_all(acl_deny);
			free(acl_allow);
			free(acl_deny);
			free(listxml);
		}

		ex_logOff(&curl);
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
	bblog(DEBUG, "crawlEXCHANGE: %s", collection->resource);

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

	bblog(DEBUG, "crawlEXCHANGE: %s", collection->resource);

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
