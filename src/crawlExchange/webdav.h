#ifndef _WEBDAV__H_
#define _WEBDAV__H_

/*
 * Exchange crawler
 *
 * June, 2007
 */

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>


struct ex_buffer {
	size_t size;
	unsigned int modified;
	char *buf;
};

char *ex_getEmail(const char *url, struct ex_buffer *buf, CURL * curl);
char *ex_getContent(const char *url, CURL *curl);

void ex_logOff(CURL *curl);
CURL *ex_logOn(const char *mailboxurl, const char *Exchangeurl ,const char *username, const char *password, char **errorm);

#endif
