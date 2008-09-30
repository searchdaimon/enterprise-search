#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../cgi-util/cgi-util.h"


#include "../common/define.h"
#include "../common/lot.h"
#include "../common/reposetory.h"
#include "../common/DocumentIndex.h"

int main(int argc, char *argv[]) {

	int res;
	int i;
	char FilePath[255];
	FILE *REPOSETORY;
	char *htmlBuf;
	unsigned int htmlBufSize;
	struct ReposetoryHeaderFormat ReposetoryHeader;
        char *aclbuffer_allow = NULL;
        char *aclbuffer_deny = NULL;
	char *attributes;
	char *url;

	int iDocID;
	char subname[maxSubnameLength];
	
        if (getenv("QUERY_STRING") != NULL) {
        

        	 // Initialize the CGI lib
        	res = cgi_init();

        	// Was there an error initializing the CGI???
        	if (res != CGIERR_NONE) {
        	        printf("Error # %d: %s<p>\n", res, cgi_strerror(res));
        	        exit(0);
        	}

        	if (cgi_getentrystr("P") == NULL) {
        	        perror("Did'n receive any query.");
        	}

		iDocID = cgi_getentryint("D");
		strncpy(subname,cgi_getentrystr("subname"),sizeof(subname) -1);

	}
	else if (argc > 2) {
		//iPointer = 9695;
		//iSize = 2288;
		//LotNr = 14;

		iDocID = atoi(argv[1]); 	// D
		strncpy(subname,argv[2],sizeof(subname) -1);

	}
	else {
		printf("no query given. \nUsage: DocID Pointer Size subname\n");
		exit(1);
	}

	printf("Content-type: text/html\n\n");

	struct DocumentIndexFormat DocumentIndexPost;

	DIRead_fmode (&DocumentIndexPost,iDocID,subname, 's');



	htmlBufSize = (DocumentIndexPost.htmlSize * 30);
	if ((htmlBuf = malloc(htmlBufSize)) == NULL) {
		printf("can't malloc space for html buff.\n");
		exit(1);		
	}


	// (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,
	// (*Sider).DocumentIndex.htmlSize,DocID,subname,&ReposetoryHeader,&aclbuffer

	if (rReadHtml(htmlBuf,&htmlBufSize,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize,iDocID,subname,&ReposetoryHeader,&aclbuffer_allow,&aclbuffer_deny,DocumentIndexPost.imageSize, &url, &attributes) != 1) {

		printf("can't read cache file.\n");

	}
	else {
		//printf(htmlBuf);
		fwrite(htmlBuf,htmlBufSize,1,stdout);
	}

	free(htmlBuf);
}
