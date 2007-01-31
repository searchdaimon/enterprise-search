#include <stdio.h>

#define nrOfFiles 64

#include "../common/define.h"
#include "../common/crc32.h"


int main (int argc, char *argv[]) {

	struct udfileFormat udfilePost;
	FILE *UDFILE;
	int count;
	int i,y;
	FILE *SPLITFILE[nrOfFiles];

        unsigned int lastTime;
        unsigned int currentTime;
        double runtime;
	int FileForUrl;

        if (argc < 3) {
                printf("Dette programet splitter en ud file i deler slik at den kan lett indekseres\n\n\tBruke\n\tUrlToDocIDIndexer mappeForFilene/ udfile\n\n");
                exit(0);
        }

//	for(i=0;i<nrOfFiles;i++) {
//		SPLITFILE[i] = malloc(sizeof(FILE));
//	}


	//DB *dbp;
	unsigned long crc32Value;
	char fileName[255];

	/********************************************************************
	* Opening nrOfFiles to stor det data in
	********************************************************************/
	for (i=0; i < nrOfFiles;i++) {

		sprintf(fileName,"%s%i",argv[1],i);

		printf("openig: %s\n",fileName);

		if ((SPLITFILE[i] = fopen(fileName,"a")) == NULL) {
                	 perror(fileName);
                	 exit(1);
         	}


	}
	/********************************************************************/


         if ((UDFILE = fopen(argv[2],"r")) == NULL) {
                 printf("Cant read udfile ");
                 perror(argv[1]);
                 exit(1);
         }


	lastTime = (unsigned int)time(NULL);	
	count = 0;
        while(!feof(UDFILE)) {
//	  for (y=0;y<7000;y++) {



		fread(&udfilePost,sizeof(struct udfileFormat),1,UDFILE);


		
		//lager en has verdi slik at vi kan velge en av filene
		crc32Value = crc32(udfilePost.url);
		FileForUrl = (crc32Value % nrOfFiles);

		//printf("Url %s, DocID %u lemgth %i\n",udfilePost.url,udfilePost.DocID,strlen(udfilePost.url));

                if (fwrite(&udfilePost,sizeof(struct udfileFormat),1,SPLITFILE[FileForUrl]) != 1) {
                        perror("Can't write");
                        exit(1);
                }

		//printf("Url %s, DocID %u lemgth %i\n",udfilePost.url,udfilePost.DocID,strlen(udfilePost.url));

	

		if ((count % 100000) == 0) {
			currentTime = (unsigned int)time(NULL);

			runtime = (currentTime - lastTime);

			printf("komet til %i, time %f s\n", count,runtime);

			lastTime = currentTime;
		}

		count++;
	}

	fclose(UDFILE);

	//lokker alle filene
	for (i=0; i < nrOfFiles;i++) {
		fclose(SPLITFILE[i]);
	}

}
