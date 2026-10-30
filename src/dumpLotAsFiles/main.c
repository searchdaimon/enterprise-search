#include "../common/reposetory.h"

#include <stdlib.h>




main (int argc, char *argv[]) {


        int LotNr;
        char lotPath[255];
	char FileName[255];

        struct ReposetoryHeaderFormat ReposetoryHeader;
        unsigned int radress;

        char htmlbuffer[524288];
	Bytef  *htmlncompressBuffer;
	int htmlncompressBufferSize;

        char imagebuffer[524288];
	int errornr;


	FILE *IMAGEFILE;
	FILE *HTMLFILE;

        if (argc < 3) {
                printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å dumpe lot 2:\n\trread 2 /tmp/boitho/\n");
                exit(1);
        }

        LotNr = atoi(argv[1]);

        GetFilPathForLot(&lotPath,LotNr);
        printf("Opning lot at: %s\n",lotPath);


	//fetch memory
	htmlncompressBuffer = (Bytef *)malloc(2097152);

	
        //loop through all
        while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,imagebuffer,&radress, 0, 0)) {

	printf("imageSize %i, htmlSize %i\n",ReposetoryHeader.imageSize,ReposetoryHeader.htmlSize);

	if (ReposetoryHeader.response == 200) {

		
		//write image
		sprintf(FileName,"%s%i.jpg",argv[2],ReposetoryHeader.DocID);

		if ((IMAGEFILE = fopen(FileName,"wb")) == NULL) {
                	printf("Open file image file");
                	perror(FileName);
                	exit(1);
        	}

		
		fwrite(imagebuffer,ReposetoryHeader.imageSize,1,IMAGEFILE);

		fclose(IMAGEFILE);
		

		//write html file
		sprintf(FileName,"%s%i.html",argv[2],ReposetoryHeader.DocID);

                if ((HTMLFILE = fopen(FileName,"wb")) == NULL) {
                        printf("Open file image file");
                        perror(FileName);
                        exit(1);
                }

		//decompress the html		
		//should not hardcode here
		htmlncompressBufferSize = 2097152;
		//printf("htmlncompressBufferSize: %i, htmlSize: %i\n",htmlncompressBufferSize, ReposetoryHeader.htmlSize);		
		if ((errornr = uncompress(htmlncompressBuffer,(uLongf *)&htmlncompressBufferSize,htmlbuffer,(uLong)ReposetoryHeader.htmlSize)) != 0) {
			printf("Cant uncompress: %i\n",errornr);
		}
		else {

                fwrite(htmlncompressBuffer,htmlncompressBufferSize,1,HTMLFILE);

                fclose(HTMLFILE);
		}

                printf("DocId: %i url: %s\n",ReposetoryHeader.DocID,ReposetoryHeader.url);

                //clear a bit so we can see if there are any errors in the reading
                ReposetoryHeader.DocID = -1;

        }

	}

	free(htmlncompressBuffer);
}
