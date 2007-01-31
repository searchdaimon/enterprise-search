#include "../common/define.h"
#include "../common/DocumentIndex.h"

#include <stdio.h>

main (int argc, char *argv[]) {


        int LotNr;
        struct DocumentIndexFormat DocumentIndexPost;
	int DocID;
	FILE *IPDBHA;

        if (argc < 2) {
                printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2\n");
                exit(1);
        }

	LotNr = atoi(argv[1]);

        if ((IPDBHA = fopen(IPDBPATH,"r+b")) == NULL) {
                perror(IPDBPATH);
                exit(1);
        }

	
	while (DIGetNext (&DocumentIndexPost, LotNr, &DocID)) {
		//printf("%s\n",DocumentIndexPost.Url);

		fseek(IPDBHA,DocID * sizeof(DocumentIndexPost.IPAddress),SEEK_SET);

        	fwrite(&DocumentIndexPost.IPAddress,sizeof(DocumentIndexPost.IPAddress),1,IPDBHA);

	}

	fclose(IPDBHA);

}

