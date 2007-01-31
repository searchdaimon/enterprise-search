#include "../common/define.h"
#include "../common/reposetoryNET.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

#define subname "www"

int main() {
	FILE *FH, *LOTFILE;
	struct stat inode;      // lager en struktur for fstat å returnere.
	int nrOfElements;
	int LotNr, DocIDPlace, oldLotNr,i,n,rank;

        if ( (FH = fopen(SHORTPOPFILE,"rb")) == NULL ) {
                perror("open");
        }

        fstat(fileno(FH),&inode);

	

        nrOfElements = inode.st_size;

        oldLotNr = -1;
        for (i=0;i<nrOfElements;i++) {
                if ((n=fread(&rank,sizeof(unsigned char),1,FH)) == -1) {
                        perror("read");
                }
                //finner lot og offset
                LotNr = rLotForDOCid(i);
                DocIDPlace = (i - LotDocIDOfset(LotNr));

                //if (lotlistIsLocal(LotNr)) {
                //        popMemArray[LotNr][DocIDPlace] = rank;
                //}

                /////////////////////////////
                //debug: vise hvilkene lot vi laster
                if (LotNr != oldLotNr) {
			if (oldLotNr != -1) {
				rSendFileByOpenHandler(LOTFILE,"Brank",oldLotNr,"w",subname);
				close(LOTFILE);
			}
			//oppret et midlertidig fil får å holde datane
			LOTFILE = tmpfile();
		
                        printf("lot %i\n",LotNr);
                        //printf("%i rank %i. Lot %i, ofset %i\n",i,(int)rank,LotNr,LotDocIDOfset(LotNr));
                }
                oldLotNr = LotNr;
                ////////////////////////////

		//søker til rikig plass og skiiver
		fseek(LOTFILE,DocIDPlace,SEEK_SET);
		fwrite(&rank,sizeof(unsigned char),1,LOTFILE);

		//printf("DocID %i, rank %i, DocIDPlace %i\n",i,rank,DocIDPlace);


        }

	rSendFileByOpenHandler(LOTFILE,"Brank",oldLotNr,"w",subname);
	close(LOTFILE);
        close(FH);

}
