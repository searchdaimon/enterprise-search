

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "re.h"
#include "gc.h"
#include "lot.h"
#include "define.h"
#include "DocumentIndex.h"

void gc_reduce(struct reformat *re, int LotNr, char subname[]) {

	FILE *GCEDFH;
	int i;
	unsigned int DocID;

        //lagrer hvilkene filer vi har slettet
        GCEDFH =  lotOpenFileNoCasheByLotNr(LotNr,"gced","a", 'e',subname);

        for (i=0;i<NrofDocIDsInLot;i++) {

                if ((REN_DocumentIndex(re, i)->Url[0] != '\0') && DIS_isDeleted(REN_DocumentIndex(re, i))) {
                        printf("Adding url \"%s\" to gc file\n",REN_DocumentIndex(re, i)->Url);

                        DocID = LotDocIDOfset(LotNr) +i;
                        if (fwrite(&DocID,sizeof(DocID),1,GCEDFH) != 1) {
                                perror("can't write gc file");
                        }

                }
        }

        fclose(GCEDFH);


}

