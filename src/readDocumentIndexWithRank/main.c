#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/poprank.h"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	unsigned int DocID;
	int rank;

        if (argc < 4) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex lotNr subname minrank maxrank\n");
               exit(0);
        }

	LotNr = atoi(argv[1]);
	char *subname = argv[2];
	int minrank = atoi(argv[3]);
	int maxrank = atoi(argv[4]);
	DocID = 0;

        popopenMemArray_oneLot(subname, LotNr);

	while (DIGetNext (&DocumentIndexPost,LotNr,&DocID,subname)) {

		if (DocumentIndexPost.response != 200) {
			continue;
		}
		//printf("response: %i\n",(int)DocumentIndexPost.response);

		rank = popRankForDocIDMemArray(DocID);

		if ((rank >= minrank) && (rank <= maxrank)) {
			//printf("DocID: %u, url: %s, rank %i\n",DocID,DocumentIndexPost.Url,rank);
			//printf("rank %i\n",rank);
			printf("%s\n",DocumentIndexPost.Url);
		}
		else {
			//printf("not in range: rank %i\n",rank);

		}
	}


	//DIClose();


}

