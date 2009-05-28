/*
 * Superoffice crawler
 */

#define _GNU_SOURCE 1
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "../crawl/crawl.h"
#include "../common/daemon.h"
#include "../key/key.h"

int crawlcanconnect(struct collectionFormat *collection,
                   int (*documentError)(struct collectionFormat *, int, const char *, ...) __attribute__((unused)));
int crawlfirst(crawlfirst_args);
int crawlupdate(crawlupdate_args);

struct crawlinfo {
	int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *);
	int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *);
	int (*documentError)(struct collectionFormat *, int, const char *, ...);
	int (*documentContinue)(struct collectionFormat *);
	struct collectionFormat *collection;
	unsigned int timefilter;
};

int
so_rewrite_url(struct collectionFormat *collection, char *url, char *uri, char *fulluri, size_t len, enum platform_type ptype, enum browser_type btype)
{
	strcpy(uri, url);
	strcpy(fulluri, url);

	return 1;
}

struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
	crawlfirst,
	crawlfirst,
	crawlcanconnect,
	NULL,
	NULL,
	so_rewrite_url,
	crawl_security_none,
	"Superoffice",
	"",
};

int
crawlcanconnect(struct collectionFormat *collection,
                   int (*documentError)(struct collectionFormat *, int, const char *, ...))
{
	return 1; // Success
	//documentError(collection, 1, "Unable to connect to: %s\n", origresource);

	return 0;
}

#define DEFAULTPORT 8000

int
crawlGo(struct crawlinfo *ci, int *s)
{
	int i;
	int port;
	char *sendbuf, *sendbuflen;
	char *host;

	port = DEFAULTPORT;

	host = params_get_char_value(ci->collection->params,"Host");

	*s = cconnect(host,DEFAULTPORT);
	if (*s == 0) {
		warn("connect()");
		ci->documentError(ci->collection, 1, "Unable to connect to: %s\n", host);
		return 0;
	}

	printf("connected\n");


	i = asprintf(&sendbuf, "user %s\npassword %s\ncollection %s\nusersystem %d\nmodule %s\nlastcrawl %d\nsystemkey %s",
	    ci->collection->user, ci->collection->password,
	    ci->collection->collection_name, ci->collection->usersystem, "superoffice",
	    ci->collection->lastCrawl, ci->collection->systemkey);
	asprintf(&sendbuflen, "%d\n", i);
	if (send(*s, sendbuflen, strlen(sendbuflen), 0) == -1)
		warn("send(len)");
	printf("Sending: %slen: %s\n", sendbuf, sendbuflen);
	if (send(*s, sendbuf, i, 0) == -1)
		warn("send()");

	free(sendbuflen);
	free(sendbuf);

	return 1;
}

int
crawlfirst(crawlfirst_args)
{

	struct crawlinfo ci;
	int s, ret;
	char buf[1];
	int count = 0;

	ci.documentExist = documentExist;
	ci.documentAdd = documentAdd;
	ci.documentError = documentError;
	ci.documentContinue = documentContinue;
	ci.collection = collection;
	ci.timefilter = 0;
	printf("crawlSO: %s\n", collection->resource);

	ret = crawlGo(&ci,&s);

	if (ret == 1) {
		//read until the so crawler close sock.
		while (( documentContinue(collection) ) && ( read(s, buf, 1) > 0) ) {

			// litt hackis. Akseserer collection strukten direkte slik. Men siden vi ikke 
			// kaller add selv, må vi oppdatere antal dokumenter som er crawletferdig, slik at
			// vi kan avbryte hvis vi har en docsRemaining klausul.

		        if (collection->docsRemaining != -1) {
                		collection->docsRemaining--;
			}


			++count;
			printf("Crawled %i\r",count);
			fflush(stdout);
		}
	}
	close(s);

	//hvis vi ikke fikk sent noe data, er noe feil. Skriver feilmelding og returnerer feil.
	if (count == 0) {
		documentError(collection, 1,"Failed to get data from SuperOffice. Please check the Xml Push error log for additional info.");
		ret = 0;
	}
	printf("~crawlSO:crawlfirst(ret=%i)\n",ret);
	return ret;
}

