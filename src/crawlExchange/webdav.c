/*
 * Exchange crawler
 *
 * June, 2007
 */

#include <sys/types.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "webdav.h"


char *
make_userpass(const char *username, const char *password)
{
	char *buf;

	buf = malloc(strlen(username) + strlen(password) + 1 + 1);
	if (buf == NULL)
		return NULL;
	sprintf(buf, "%s:%s", username, password);

	return buf;
}

void
free_userpass(char *buf)
{
	free(buf);
}
 
int
ex_write_buffer(void *buffer, size_t size, size_t nmemb, void *stream)
{
	struct ex_buffer *buf;
	size_t origsize;

	if (size != 1)
		fprintf(stderr, "ex_write_buffer(): size not equal to one!\n");
	buf = stream;

	origsize = 0;
	if (buf->buf == NULL) {
		buf->buf = malloc(size * (nmemb + 1));
		buf->size = size * (nmemb + 1);
		buf->buf[0] = '\0';
	} else {
		origsize = buf->size;
		buf->size += (size * nmemb);
		buf->buf = realloc(buf->buf, buf->size);
	}
	if (buf->buf == NULL)
		return -1;

	/* XXX: Assuming size == sizeof(char) */
	//strncat(buf->buf, buffer, size * nmemb);
	strncpy(buf->buf + origsize - (origsize == 0 ? 0 : size), buffer, (size * nmemb)+1);
	buf->buf[buf->size-1] = '\0';
	return nmemb;
}

/* Free the returned value */
char *
ex_getEmail(const char *url, const char *username, const char *password, struct ex_buffer *buf)
{
	CURL * curl;
	CURLcode result;
	struct curl_slist *headers = NULL;
	char *userpass;

	curl = curl_easy_init();
	if (curl == NULL)
		return NULL;

	buf->buf = NULL;
	headers = curl_slist_append(headers, "Translate: f");
	//headers = curl_slist_append(headers, "Content-type: text/xml");
	//headers = curl_slist_append(headers, "Depth: 1");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
	if ((userpass = make_userpass(username, password)) == NULL)
		return NULL;
	curl_easy_setopt(curl, CURLOPT_USERPWD, userpass);
	//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ex_write_buffer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	//curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	result = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	free_userpass(userpass);

	return buf->buf;
}

/* Free the returned value */
char *
ex_getContent(const char *url, const char *username, const char *password)
{
	CURL * curl;
	CURLcode result;
	struct curl_slist *headers = NULL;
	struct ex_buffer buf;
	char *userpass;

	curl = curl_easy_init();
	buf.buf = NULL;
	buf.size = 0;
	if (curl == NULL)
		return NULL;

	headers = curl_slist_append(headers, "Translate: f");
	headers = curl_slist_append(headers, "Content-type: text/xml");
	headers = curl_slist_append(headers, "Depth: 1");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
	if ((userpass = make_userpass(username, password)) == NULL)
		return NULL;
	curl_easy_setopt(curl, CURLOPT_USERPWD, userpass);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ex_write_buffer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	//curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
		"<?xml version=\"1.0\"?>"
		"<d:propfind xmlns:d='DAV:' xmlns:c='urn:schemas:httpmail:' xmlns:p='http://schemas.microsoft.com/mapi/proptag/' xmlns:ex='http://schemas.microsoft.com/exchange/security/'>"
			"<d:prop>"
				"<d:displayname/>"
				"<d:getcontentlength/>"
				"<d:getlastmodified/>"
//				"<c:normalizedsubject/>"
				"<p:xfff0102/>"
				"<ex:descriptor/>"
			"</d:prop>"
		"</d:propfind>"
	);

//http://schemas.microsoft.com/mapi/proptag/0x0E1D001E

	result = curl_easy_perform(curl);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	free_userpass(userpass);

	//printf("Mail\n\n%s\n\n", buf.buf);

	return buf.buf;
}

