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
so_rewrite_url(char *uri, enum platform_type ptype, enum browser_type btype)
{
	char *p;

	return 1;
}


struct crawlLibInfoFormat crawlLibInfo = {
	NULL,
	crawlfirst,
	crawlupdate,
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
crawlGo(struct crawlinfo *ci)
{
	int s, i;
	struct hostent *hostaddr;
	int port;
	struct protoent *protocol;
	struct sockaddr_in socketaddr;
	char *sendbuf, *sendbuflen;
	char *host;

	port = DEFAULTPORT;

	protocol = getprotobyname("tcp");
	if (!protocol) {
		warn("getprotobyname()");
		return 0;
	}
	s = socket(PF_INET, SOCK_STREAM, protocol->p_proto);
	if (s == -1) {
		warn("socket()");
		return 0;
	}
	memset(&socketaddr, '\0', sizeof(socketaddr));
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_port = htons(port);
	host = params_get_char_value(ci->collection->params,"Host");

	hostaddr = gethostbyname(host);
	printf("Connecting to: %s:%d\n", host, port);
	if (!hostaddr) {
		warn("gethostbyname()");
		return 0;
	}
	memcpy(&socketaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);
	i = connect(s, &socketaddr, sizeof(socketaddr));
	if (i == -1) {
		warn("connect()");
		exit(1);
		return 0;
	}

	i = asprintf(&sendbuf, "user %s\npassword %s\ncollection %s\nusersystem %d\nmodule %s\n",
	    ci->collection->user, ci->collection->password,
	    ci->collection->collection_name, ci->collection->usersystem, "superoffice");
	asprintf(&sendbuflen, "%d\n", i);
	if (send(s, sendbuflen, strlen(sendbuflen), 0) == -1)
		warn("send(len)");
	printf("Sending: %slen: %s\n", sendbuf, sendbuflen);
	if (send(s, sendbuf, i, 0) == -1)
		warn("send()");
	else
		close(s);
	free(sendbuflen);
	free(sendbuf);

	return 1;
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
	printf("crawlSO: %s\n", collection->resource);

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

	printf("crawlSO: %s\n", collection->resource);

	return crawlGo(&ci);
}
