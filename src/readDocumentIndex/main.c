#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "../common/define.h"
#include "../common/DocumentIndex.h"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	unsigned int DocID;

        if (argc < 3) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }

	LotNr = atoi(argv[1]);
	char *subname = argv[2];
	DocID = 0;

	printf("DocumentIndexPost size: %i\n",sizeof(DocumentIndexPost));

	while (DIGetNext (&DocumentIndexPost,LotNr,&DocID,subname)) {
		if (DocumentIndexPost.RepositoryPointer != 0) {
			printf("DocID: %u, url: %s, RepositoryPointer %u, lastSeen %s",DocID,DocumentIndexPost.Url,DocumentIndexPost.RepositoryPointer,ctime(&DocumentIndexPost.lastSeen));
		}
	}


	//DIClose();


}

