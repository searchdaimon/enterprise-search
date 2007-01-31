#include <stdio.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"

#define subname "www"

int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	unsigned int DocID;
	FILE *ADULTWEIGHTFH;
	unsigned char awvalue;;

        if (argc < 2) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }

	LotNr = atoi(argv[1]);

	if (DIHaveIndex(LotNr,subname) == 0) {
		printf("dosent hav DIindex\n");
		exit(1);
	}

	ADULTWEIGHTFH = lotOpenFileNoCasheByLotNr(LotNr,"AdultWeight","wb", 'e',subname);

	DocID = 0;
	while (DIGetNext (&DocumentIndexPost,LotNr,&DocID,subname)) {

		if (DocumentIndexPost.AdultWeight >= AdultWeightForXXX) {
			//printf("DocID: %u, %hu, url: %s\n",DocID,DocumentIndexPost.AdultWeight,DocumentIndexPost.Url);
			//mark as adult
			awvalue = 1;
		}
		else {
			//not adult
			awvalue = 0;
		}

		fwrite(&awvalue,sizeof(awvalue),1,ADULTWEIGHTFH);

	}


	//DIClose();
	fclose(ADULTWEIGHTFH);

}

