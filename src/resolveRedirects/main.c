#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/reposetory.h"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	unsigned int DocID;
	char HtmlBuffer[5000];
	unsigned int HtmlBufferLen;
	struct ReposetoryHeaderFormat ReposetoryHeader;

	char *acl_allowbuffer, *acl_deniedbuffer;

        if (argc < 3) {
                printf("Dette programet identifiserer 301 og 302 redirects i DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1 www\n");
               exit(0);
        }

	LotNr = atoi(argv[1]);
	char *subname = argv[2];
	DocID = 0;


	while (DIGetNext (&DocumentIndexPost,LotNr,&DocID,subname)) {

		if ((DocumentIndexPost.response == 301) || (DocumentIndexPost.response == 301)) {
			HtmlBufferLen = sizeof(HtmlBuffer);
			rReadHtml (HtmlBuffer,&HtmlBufferLen,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize,
				DocID, subname,&ReposetoryHeader,&acl_allowbuffer,&acl_deniedbuffer);

			if (HtmlBufferLen < 200) {
				printf("DocID: %u, url: \"%s\", redirects of type %hu to \"%s\"\n",DocID,DocumentIndexPost.Url,DocumentIndexPost.response,HtmlBuffer);

			}
		}
	}


	//DIClose();


}

