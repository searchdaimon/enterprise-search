#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>

#include "../cgi-util/cgi-util.h"

#include "../common/define.h"
#include "../common/lot.h"
#include "../common/reposetory.h"
#include "../common/DocumentIndex.h"
#include "../common/doc_cache.h"

#ifdef BLACK_BOX
	#define URL_EXPIRE_TIME 3600
#endif

int main(int argc, char *argv[]) {

	int res;
	char *htmlBuf;
	uLong htmlBufSize;
	struct ReposetoryHeaderFormat ReposetoryHeader;
        char *aclbuffer_allow = NULL;
        char *aclbuffer_deny = NULL;
	char *attributes;
	char *url;

	docid iDocID;
	char subname[maxSubnameLength];

#ifdef BLACK_BOX
	bool validate_url = true;
	unsigned int url_time;
	unsigned int url_signature;
#endif
	
        if (getenv("QUERY_STRING") != NULL) {
        

        	 // Initialize the CGI lib
        	res = cgi_init();

        	// Was there an error initializing the CGI???
        	if (res != CGIERR_NONE) {
        	        printf("Error # %d: %s<p>\n", res, cgilib_strerror(res));
        	        exit(0);
        	}

		iDocID = cgi_getentryunsignedint("D");
		if (cgi_errno == CGIERR_NOT_UNSIGNED_INTEGER)
			errx(1, "Parameter D missing/invalid");

		
		const char * query_subname = cgi_getentrystr("subname");
		if (cgi_errno != CGIERR_NONE)
			errx(1, "Error with parameter subname");

		strncpy(subname, query_subname ,sizeof(subname) -1);

#ifdef BLACK_BOX
		url_time = cgi_getentryunsignedint("time");
		if (cgi_errno == CGIERR_NOT_UNSIGNED_INTEGER)
			errx(1, "Parameter time missing/invalid");

		url_signature = cgi_getentryunsignedint("sign");
		if (cgi_errno == CGIERR_NOT_UNSIGNED_INTEGER)
			errx(1, "Parameter sign missing/invalid");
#endif


	}
	else if (argc > 2) {
#ifdef BLACK_BOX
		validate_url = false;
#endif

		iDocID = atoi(argv[1]); 	// D
		strncpy(subname,argv[2],sizeof(subname) -1);

	}
	else {
		printf("no query given. \nUsage: DocID subname\n");
		exit(1);
	}


#ifdef BLACK_BOX
	if (validate_url) {
		unsigned int our_sign = sign_cache_params(iDocID, subname, url_time);

		if (url_signature != our_sign)
			errx(1, "Data signature did not match provided signature");
	

		time_t t = time(NULL);
		if (url_time + URL_EXPIRE_TIME < t)
			errx(1, "URL has expired.");
	}
#endif
	
	printf("Content-type: text/html\n\n");

	struct DocumentIndexFormat DocumentIndexPost;

	DIRead_fmode (&DocumentIndexPost,iDocID,subname, 's');


	if (rReadHtml(&htmlBuf,&htmlBufSize,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize2,iDocID,subname,&ReposetoryHeader,&aclbuffer_allow,&aclbuffer_deny,DocumentIndexPost.imageSize, &url, &attributes) != 1) {

		printf("Can't read cache file.\n");
	}
	else {
		fwrite(htmlBuf,htmlBufSize,1,stdout);
	}

	free(htmlBuf);


	return 1;
}
