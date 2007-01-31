#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>

#include "define.h"
#include "lot.h"
#include "lotlist.h"

unsigned char *adultWeightMemArray[maxLots];


void adultWeightopenMemArray(char servername[],char subname[]) {

        FILE *FH;
        int i;
        int LocalLots;
        char LotFile[128];
        int branksize;

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
                        if ( (FH = fopen(LotFile,"rb")) == NULL ) {
                                perror(LotFile);
                                adultWeightMemArray[i] = 0;
                        }
                        else {

                                printf("loaded lot %i\n",i);

                                branksize = sizeof(unsigned char) * NrofDocIDsInLot;

                                if ((adultWeightMemArray[i] = (unsigned char*)malloc(branksize)) == NULL) {
                                        printf("malloc eror for lot %i\n",i);
                                        perror("malloc");
                                        exit(1);
                                }

                                fread(adultWeightMemArray[i],branksize,1,FH);

                                //debug: viser alle rankene vi laster
                                //for(y=0;y<branksize;y++) {
                                //      printf("DocID %i, rank %i\n",y,adultWeightMemArray[i][y]);
                                //}

                                fclose(FH);

                                ++LocalLots;

                        }
                }
                else {
                        adultWeightMemArray[i] = 0;
                }
        }
        printf("have %i local lots\n",LocalLots);


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
