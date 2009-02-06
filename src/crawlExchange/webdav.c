/*
 * Exchange crawler
 *
 * June, 2007
 */

#include <sys/types.h>
#include <string.h>


#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "webdav.h"
#include "../common/timediff.h"


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
ex_getEmail(const char *url, struct ex_buffer *buf, CURL ** curl)
{
	CURLcode result;
	struct curl_slist *headers = NULL;
	time_t start_time, end_time;

	printf("ex_getEmail(url=%s)\n",url);

	//curl = curl_easy_init();
	if (*curl == NULL)
		return NULL;

	buf->buf = NULL;
	headers = curl_slist_append(headers, "Translate: f");
	curl_easy_setopt(*curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(*curl, CURLOPT_URL, url);
	curl_easy_setopt(*curl, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(*curl, CURLOPT_WRITEFUNCTION, ex_write_buffer);
	curl_easy_setopt(*curl, CURLOPT_WRITEDATA, buf);
	gettimeofday(&start_time, NULL);
	result = curl_easy_perform(*curl);
	gettimeofday(&end_time, NULL);
	curl_slist_free_all(headers);
	printf("Grab took: %f\n", getTimeDifference(&start_time,&end_time));

	return buf->buf;
}

void ex_logOff(CURL **curl) {

	curl_easy_cleanup(*curl);
}

CURL *ex_logOn(const char *mailboxurl, struct loginInfoFormat *login, char **errorm) {

	CURL *curl;
	CURLcode result;
	char *userpass;
	long code;
	char *redirecttarget;
	char *owaauth;
	char owaauthpath[PATH_MAX];
	*errorm = NULL;
	
	
	printf("ex_logOn(mailboxurl=%s, Exchangeurl=%s, username=%s)\n", mailboxurl, login->Exchangeurl, login->username);

	curl = curl_easy_init();


	if (curl == NULL) {
		asprintf(errorm,"Can't init curl_easy_init()");		
		return NULL;
	}

	#ifdef DEBUG
    	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	#endif

	if (strstr(mailboxurl,"https://") != NULL) {
    		//tilater bugy serfikater.
    		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	//vi prøver førs å gjøre et vanlig kall til urlen for å se hva som skjer. 
    	curl_easy_setopt(curl, CURLOPT_URL, mailboxurl);

	result = curl_easy_perform(curl);
	result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

	#ifdef DEBUG
		//curl skriver av og til ut data uten \n på slutten. 
		printf("\n");
	#endif

	printf("code %i\n",code);

	//hvis vi fikke en redirect finner vi ut til hvor
	if (code == 302) {
    		result = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL , &redirecttarget);
    		printf("res %d\n",result);
	    	printf("redirect's to \"%s\"\n",redirecttarget);
	}

	if (code == 302 && (strstr(redirecttarget,"exchweb/bin/auth/owalogon.asp") != NULL)) {

		printf("we have form based login!\n");

		char *destination;
		char *postData;

	    	/* First set the URL that is about to receive our POST. This URL can
	       	just as well be a https:// URL if that is what should receive the
	       	data. */
		snprintf(owaauthpath,sizeof(owaauthpath),"%s/exchweb/bin/auth/owaauth.dll",login->Exchangeurl);
	    	curl_easy_setopt(curl, CURLOPT_URL, owaauthpath);

	    	//bruke cookies
	    	curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "-");

	    	//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION , 0);

		//url enkoder destination feltet
		destination = curl_easy_escape(curl, mailboxurl, 0);

		//bygger port datane
		// skal være av slik: destination=https%3A%2F%2Fsbs.searchdaimon.com%2FExchange%2Feo&flags=0&username=eo&password=1234Asd&SubmitCreds=Log+On&trusted=0

		asprintf(&postData,"destination=%s&flags=0&username=%s&password=%s&SubmitCreds=Log+On&trusted=0",destination, login->username, login->password);

		printf("postData new: %s\n\n",postData);

	    	/* Now specify the POST data */
	    	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);



	    	/* Perform the request, res will get the return code */
	    	result = curl_easy_perform(curl);

	    	//res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL , &redirecttarget);
	    	//printf("res %d\n");
	    	//printf("red to \"%s\"\n",redirecttarget);

	    	result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
	    	printf("res %d, code %d\n",result,code);

		curl_free(destination);
		free(postData);
		
	}
	else if (code == 401) {
		if ((userpass = make_userpass(login->username, login->password)) == NULL)
			return NULL;
		curl_easy_setopt(curl, CURLOPT_USERPWD, userpass);

		//usikker om vi kan gjøre dette. Kan være at curl trenger datene siden.
		free_userpass(userpass);

		//etter at vi har satt bruker/pass prøver vi å logge på.
    		curl_easy_setopt(curl, CURLOPT_URL, mailboxurl);
	
		result = curl_easy_perform(curl);
		result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

		printf("code %i\n",code);
		if (code == 200) {

		}
		else if (code == 401) {
			asprintf(errorm,"Wrong username or passord");
			return NULL;
		}
		else {
			asprintf(errorm,"Http error code %i\n",code);
			return NULL;
		}

    	}
	else if ((code == 302) && (strstr(mailboxurl,"http://") != NULL) && (strstr(redirecttarget,"https://") != NULL)) {
		asprintf(errorm,"Got redirected to an httpS page. If your server only supports https please use an https url as the resource for Exchange.\n");		
		return NULL;
	}
	else if (code == 200) {
		/* Looks good */
	}
	else {
		asprintf(errorm,"Can't decide login type.\n");
		return NULL;		
	}


	return curl;
}

/* Free the returned value */
char *
ex_getContent(const char *url, CURL **curl, struct loginInfoFormat *login)
{
	CURLcode result;
	struct curl_slist *headers = NULL;
	struct ex_buffer buf;
	long code;

	#ifdef DEBUG
		//warn: skriver ut passord i klartekst.
		printf("ex_getContent(url=%s)\n",url);
	#endif

	buf.buf = NULL;
	buf.size = 0;


	headers = curl_slist_append(headers, "Translate: f");
	headers = curl_slist_append(headers, "Content-type: text/xml");
	headers = curl_slist_append(headers, "Depth: 1");
	curl_easy_setopt(*curl, CURLOPT_HTTPHEADER, headers);
    
	curl_easy_setopt(*curl, CURLOPT_URL, url);
	curl_easy_setopt(*curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");

	curl_easy_setopt(*curl, CURLOPT_WRITEFUNCTION, ex_write_buffer);
	curl_easy_setopt(*curl, CURLOPT_WRITEDATA, &buf);

	curl_easy_setopt(*curl, CURLOPT_POSTFIELDS,
		"<?xml version=\"1.0\"?>"
		"<d:propfind xmlns:d='DAV:' xmlns:c='urn:schemas:httpmail:' xmlns:m='urn:schemas:mailheader:' xmlns:p='http://schemas.microsoft.com/mapi/proptag/' xmlns:ex='http://schemas.microsoft.com/exchange/security/'>"
			"<d:prop>"
				"<d:displayname/>"
				"<d:getcontentlength/>"
				"<d:getlastmodified/>"
				"<p:xfff0102/>"
				"<ex:descriptor/>"
				"<p:x1A001E/>"
			"</d:prop>"
		"</d:propfind>"
	);

//http://schemas.microsoft.com/mapi/proptag/0x0E1D001E

	result = curl_easy_perform(*curl);
	result = curl_easy_getinfo(*curl, CURLINFO_RESPONSE_CODE, &code);
	curl_slist_free_all(headers);

	//printf("Mail\n\n%s\n\n", buf.buf);
	printf("response code %d\n",code);

	if (code == 401) {
		if (buf.buf != NULL) 
			free(buf.buf);

		return NULL;
	}
       //login timeout
       if (code == 440) {
               	char *errorm;
		ex_logOff(curl);
               	*curl = ex_logOn(url, login, &errorm);
		if (buf.buf != NULL) 
               		free(buf.buf);

               	return NULL;

        }

	#ifdef DEBUG
	printf("buf.buf: \"%s\"\n",buf.buf);
	#endif

	return buf.buf;
}

