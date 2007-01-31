#include <stdio.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"

#define subname "www"
#define AdultWeightForXXX 50

int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	unsigned int DocID;
	FILE *LOTIPDB;
	unsigned char awvalue;;

        if (argc < 2) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1\n");
               exit(0);
        }

	LotNr = atoi(argv[1]);

	if (DIHaveIndex(LotNr,subname) == 0) {
		printf("dosent hav DIindex\n");
		exit(1);
	}

	LOTIPDB = lotOpenFileNoCasheByLotNr(LotNr,"ipdb","wb", 'e',subname);

	DocID = 0;
	while (DIGetNext (&DocumentIndexPost,LotNr,&DocID,subname)) {

		fwrite(&DocumentIndexPost.IPAddress,sizeof(unsigned long int),1,LOTIPDB);

		//printf("%u %u\n",DocID,DocumentIndexPost.IPAddress);
	}


	//DIClose();
	fclose(LOTIPDB);

}

