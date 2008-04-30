#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define MMAP_ADULT

#ifdef MMAP_ADULT
        #include <sys/types.h>
        #include <sys/stat.h>
        #include <fcntl.h>
        #include <sys/mman.h>
	#include <sys/resource.h>
#endif

#include "define.h"
#include "lot.h"
#include "lotlist.h"

unsigned char *adultWeightMemArray[maxLots];


void adultWeightopenMemArray2(char subname[]) {

        FILE *FH;
        int i, y ,z;
        int LocalLots;
        char LotFile[128];
        int branksize;
	off_t totsize = 0;
	struct stat inode;

        int locklimit;

        //finner grensen på antal sider vi kan låse i minne
        struct rlimit rlim;

        if (getrlimit(RLIMIT_MEMLOCK, &rlim) < 0) {
                printf("Warning: Cannot raise RLIMIT_MEMLOCK limits.");
                locklimit = 0;
        }
        else {
                locklimit = rlim.rlim_cur;
        }


        //aloker minne for rank for de forskjelige lottene
        LocalLots=0;
        for (i=0;i<maxLots;i++) {
                //sjekekr om dette er en lokal lot

                        adultWeightMemArray[i] = 0;


                        GetFilPathForLot(LotFile,i,subname);
                        strcat(LotFile,"AdultWeight");

                        // prøver å opne
                        if ( (FH = fopen(LotFile,"r+b")) == NULL ) {
                                //perror(LotFile);
				continue;
                        }

			fstat(fileno(FH),&inode);


			if (inode.st_size == 0) {
				printf("adultWeightopenMemArray2: file is emty\n");
				continue;
			}

			//#ifdef DEBUG
                               printf("loaded lot %i\n",i);
			//#endif

                        branksize = sizeof(unsigned char) * NrofDocIDsInLot;

			#ifdef MMAP_ADULT


			if (inode.st_size < branksize) {
				fprintf(stderr,"adultWeightopenMemArray: file is smaler then size. file size %"PRId64", suposed to be %i\n",inode.st_size,branksize);				

                               	/*
                               	Stretch the file size to the size of the (mmapped) array of ints
                               	*/
                               	if (fseek(FH, branksize +1, SEEK_SET) == -1) {
                               	        perror("Error calling fseek() to 'stretch' the file");
                               	        exit(EXIT_FAILURE);
                               	}

                               	/* Something needs to be written at the end of the file to
                               	* have the file actually have the new size.
                               	* Just writing an empty string at the current file position will do.
                               	*
                               	* Note:
                               	*  - The current position in the file is at the end of the stretched
                               	*    file due to the call to fseek().
                               	*  - An empty string is actually a single '\0' character, so a zero-byte
                               	*    will be written at the last byte of the file.
                               	*/
                       	        if (fwrite("", 1, 1, FH) != 1) {
               	                        perror("Error writing last byte of the file");
                                        exit(EXIT_FAILURE);
	                        }

			}

			if ((adultWeightMemArray[i] = mmap(0,branksize,PROT_READ,MAP_SHARED,fileno(FH),0) ) == NULL) {
                               	fprintf(stderr,"adultWeightopenMemArray: can't mmap for lot %i\n",i);
                               	perror("mmap");
                       	}

                        //hvis vi kan låse uendelig med minne gjør vi det
                        if (locklimit == -1) {
                        	//laster all rankerings dataen fra minne, slik ad det går fort å leste den inn siden
                                z = 0;
                                for(y=0;y<NrofDocIDsInLot;y++) {
                                	z += adultWeightMemArray[i][y];
                                }
                                //låser minne
                                if (mlock(adultWeightMemArray[i],branksize) != 0) {
                                	perror("mlock");
                                        //exit(1);
                               	}
                        }


			#else
                        if ((adultWeightMemArray[i] = (unsigned char*)malloc(branksize)) == NULL) {
                                printf("malloc eror for lot %i\n",i);
                                perror("malloc");
                                exit(1);
                        }

                        fread(adultWeightMemArray[i],branksize,1,FH);

			#endif

                        //debug: viser alle rankene vi laster
                        //for(y=0;y<branksize;y++) {
                        //      printf("DocID %i, rank %i\n",y,adultWeightMemArray[i][y]);
                        //}

                        fclose(FH);

			totsize += branksize;

                        ++LocalLots;

                
        }
        printf("adultWeightopenMemArray: have %i local lots\n",LocalLots);
	printf("adultWeightopenMemArray: loaded a total of %"PRId64" bytes\n",totsize);

}

void adultWeightopenMemArray(char servername[],char subname[]) {

        FILE *FH;
        int i;
        int LocalLots;
        char LotFile[128];
        int branksize;
	off_t totsize = 0;
	struct stat inode;

        lotlistLoad();
        lotlistMarkLocals(servername);

        //aloker minne for rank for de forskjelige lottene
        LocalLots=0;
        for (i=0;i<maxLots;i++) {
                //sjekekr om dette er en lokal lot


                if (lotlistIsLocal(i)) {

                        GetFilPathForLot(LotFile,i,subname);
                        strcat(LotFile,"AdultWeight");

                        // prøver å opne
                        if ( (FH = fopen(LotFile,"r+b")) == NULL ) {
                                perror(LotFile);
                                adultWeightMemArray[i] = 0;
                        }
                        else {

				#ifdef DEBUG
                                printf("loaded lot %i\n",i);
				#endif

                                branksize = sizeof(unsigned char) * NrofDocIDsInLot;

				#ifdef MMAP_ADULT

				fstat(fileno(FH),&inode);

				if (inode.st_size < branksize) {
					fprintf(stderr,"adultWeightopenMemArray: file is smaler then size. file size %"PRId64", suposed to be %i\n",inode.st_size,branksize);				

                                	/*
                                	Stretch the file size to the size of the (mmapped) array of ints
                                	*/
                                	if (fseek(FH, branksize +1, SEEK_SET) == -1) {
                                	        perror("Error calling fseek() to 'stretch' the file");
                                	        exit(EXIT_FAILURE);
                                	}

                                	/* Something needs to be written at the end of the file to
                                	* have the file actually have the new size.
                                	* Just writing an empty string at the current file position will do.
                                	*
                                	* Note:
                                	*  - The current position in the file is at the end of the stretched
                                	*    file due to the call to fseek().
                                	*  - An empty string is actually a single '\0' character, so a zero-byte
                                	*    will be written at the last byte of the file.
                                	*/
                        	        if (fwrite("", 1, 1, FH) != 1) {
                	                        perror("Error writing last byte of the file");
        	                                exit(EXIT_FAILURE);
	                                }

				}

				if ((adultWeightMemArray[i] = mmap(0,branksize,PROT_READ,MAP_SHARED,fileno(FH),0) ) == NULL) {
                                	fprintf(stderr,"adultWeightopenMemArray: can't mmap for lot %i\n",i);
                                	perror("mmap");
                        	}

				#else
                                if ((adultWeightMemArray[i] = (unsigned char*)malloc(branksize)) == NULL) {
                                        printf("malloc eror for lot %i\n",i);
                                        perror("malloc");
                                        exit(1);
                                }

                                fread(adultWeightMemArray[i],branksize,1,FH);

				#endif

                                //debug: viser alle rankene vi laster
                                //for(y=0;y<branksize;y++) {
                                //      printf("DocID %i, rank %i\n",y,adultWeightMemArray[i][y]);
                                //}

                                fclose(FH);

				totsize += branksize;

                                ++LocalLots;

                        }
                }
                else {
                        adultWeightMemArray[i] = 0;
                }
        }
        printf("adultWeightopenMemArray: have %i local lots\n",LocalLots);
	printf("adultWeightopenMemArray: loaded a total of %"PRId64" bytes\n",totsize);

}

int adultWeightForDocIDMemArray(int DocID) {
        int LotNr,DocIDPlace;


                //hvis vi har en negativ DocID så er noe galt
                if (DocID < 0) {
                        return -3;
                }

		//filler lot og offset
                LotNr = rLotForDOCid(DocID);
                DocIDPlace = (DocID - LotDocIDOfset(LotNr));

                if (adultWeightMemArray[LotNr] != 0) {
                        return adultWeightMemArray[LotNr][DocIDPlace];
                }
                else {
                        return 0;
                }

}
