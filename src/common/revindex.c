#include <stdio.h>
#include <stdlib.h>

#include "define.h"
#include "lot.h"

FILE *revindexFilesOpenLocalPart(int lotNr,char type[],char filemode[],char subname[],int part) {

        char lotPath[128];
        char revfile[128];
	FILE *revindexFilesHa;

        GetFilPathForLot(lotPath,lotNr,subname);

        //oppretter path
        sprintf(revfile,"%srevindex/%s/",lotPath,type);
        makePath(revfile);

       sprintf(revfile,"%srevindex/%s/%i.txt",lotPath,type,part);

       if ((revindexFilesHa = fopen(revfile,filemode)) == NULL) {
                perror(revfile);
       		exit(1);
       }

	return revindexFilesHa;
}

void revindexFilesOpenLocal(FILE *revindexFilesHa[],int lotNr,char type[],char filemode[],char subname[]) {

        int i;
        char lotPath[128];
        char revfile[128];

        GetFilPathForLot(lotPath,lotNr,subname);

        //oppretter path
        sprintf(revfile,"%srevindex/%s/",lotPath,type);
        makePath(revfile);

        for(i=0;i<NrOfDataDirectorys;i++) {
                sprintf(revfile,"%srevindex/%s/%i.txt",lotPath,type,i);

                if ((revindexFilesHa[i] = fopen(revfile,filemode)) == NULL) {
                        perror(revfile);
                        exit(1);
                }
        }
}

