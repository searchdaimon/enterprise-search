#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "../common/define.h"
#include "../common/DocumentIndex.h"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	unsigned int DocID;
	int i;
	int forrecrawl;

	struct udfileFormat ud;
	FILE *UDFILE;

        if (argc < 5) {
                printf("Dette programet leser en DocumentIndex og velger ut sider for recrawling. \n\n\tUsage: ./readDocumentIndex lotfrom lotto udfile subname");
               exit(0);
        }

	int lotfrom = atoi(argv[1]);
	int lotto = atoi(argv[2]);

	char *addudfile = argv[3];
	char *subname = argv[4];


	if ((UDFILE = fopen(addudfile,"wb")) == NULL) {
		perror(addudfile);
		exit(1);
	}

	for (LotNr=lotfrom;LotNr<=lotto;LotNr++) {

		printf("lot nr %i\n",LotNr);

		DocID = 0;
		forrecrawl = 0;

		while (DIGetNext (&DocumentIndexPost,LotNr,&DocID,subname)) {
			if (DocumentIndexPost.Url[0] == '\0') {
				continue;
			}

			
			if (DocumentIndexPost.response == 0) {

			}
			else if (0) {

			}
			else {
				//hvis ingen tester slo ut så går vi bare til neste
				continue;
			}
			#ifdef DEBUG
			printf("recrawl: DocID %u, \"%s\"\n",DocID,DocumentIndexPost.Url);
			#endif

			strcpy(ud.url,DocumentIndexPost.Url);
			ud.DocID = DocID;

			if (fwrite(&ud,sizeof(ud),1,UDFILE) != 1) {
				perror("fwrite");
				exit(1);
			}

			++forrecrawl;
			//printf("DocID: %u, url: %s\n",DocID,DocumentIndexPost.Url);
		}

		printf("for recrawl: %i\n",forrecrawl);
		//DIClose();
	}

	fclose(UDFILE);
}

