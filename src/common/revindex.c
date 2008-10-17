#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>

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
		return NULL;
       	}

	flock(fileno(revindexFilesHa),LOCK_EX);

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
		if ((revindexFilesHa[i] = revindexFilesOpenLocalPart(lotNr,type,filemode,subname,i)) == NULL ) {
                        exit(1);
                }
        }
}


void revindexFilesCloseLocal(FILE *revindexFilesHa[]) {

	int i;

        for(i=0;i<NrOfDataDirectorys;i++) {

		fclose(revindexFilesHa[i]);
        }
	
}

