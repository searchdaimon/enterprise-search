#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../cgi-util/cgi-util.h"


#include "../common/define.h"
#include "../common/lot.h"
#include "../common/reposetory.h"

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

	off_t iPointer;
	//unsigned long int iPointer;
	unsigned int iSize;
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

		//iPointer = cgi_getentryint("P");
		//iPointer = atol(cgi_getentrystr("P"));
		iSize = strtoul(cgi_getentrystr("S"), (char **)NULL, 10);
		iDocID = cgi_getentryint("D");
		iPointer = strtoul(cgi_getentrystr("P"), (char **)NULL, 10);
		strncpy(subname,cgi_getentrystr("subname"),sizeof(subname) -1);

	}
	else if (argc > 4) {
		//iPointer = 9695;
		//iSize = 2288;
		//LotNr = 14;

		iDocID = atoi(argv[1]); 	// D
		iPointer = atoi(argv[2]); 	// P
		iSize = atoi(argv[3]);		// S
		strncpy(subname,argv[4],sizeof(subname) -1);

	}
	else {
		printf("no query given. \nUsage: DocID Pointer Size subname\n");
		exit(1);
	}

	printf("Content-type: text/html\n\n");


	//må legge til størelsen på hedderen også

	htmlBufSize = (iSize * 10);
	if ((htmlBuf = malloc(htmlBufSize)) == NULL) {
		printf("can't malloc space for html buff.\n");
		exit(1);		
	}


	// (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,
	// (*Sider).DocumentIndex.htmlSize,DocID,subname,&ReposetoryHeader,&aclbuffer

	if (rReadHtml(htmlBuf,&htmlBufSize,iPointer,iSize,iDocID,subname,&ReposetoryHeader,&aclbuffer_allow,&aclbuffer_deny) != 1) {

		printf("can't read cache file.\n");

	}
	else {
		//printf(htmlBuf);
		fwrite(htmlBuf,htmlBufSize,1,stdout);
	}

	free(htmlBuf);
}
