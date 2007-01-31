#include "../common/define.h"
#include "../common/DocumentIndex.h"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	unsigned int DocID;

        if (argc < 2) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }

	LotNr = atoi(argv[1]);
	DocID = 0;
	while (DIGetNext (&DocumentIndexPost,LotNr,&DocID)) {
		printf("DocID: %u, url: %s\n",DocID,DocumentIndexPost.Url);
	}


	DIClose();


}

