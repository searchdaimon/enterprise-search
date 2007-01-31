#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "../common/define.h"


int main (int argc, char *argv[]) {

	int i;
	int lotNr;
	int count;

	FILE *udfilefp;
	FILE *newudfilefp;
	struct udfileFormat udpost;

	if (argc <= 3) {
		printf("Usage: ./recoverUrlForLot udfile newudfile lot [lot ...]\n");
		exit(1);
	}


	char *udfile = argv[1];
	char *newudfile = argv[2];
	int nrOfsfLots = (argc - 3);

	int sflot[nrOfsfLots];

	printf("udfile %s, newudfile %s. nrOfsfLots %i\n",udfile,newudfile,nrOfsfLots);

	for (i=0;i<nrOfsfLots;i++) {
		sflot[i] = atoi(argv[i +3]);
		printf("aa %s, %i\n",argv[i +3],sflot[i]);

	}

	if ((udfilefp = fopen(udfile,"rb")) == NULL) {
		perror(udfile);
		exit(1);
	}

	//tester at den ikke finnes. Viktig at vi ikke overskriver noe ved en feil
	if ((newudfilefp = fopen(newudfile,"rb")) != NULL) {
                printf("newudfile exist. It shuldent.\n");
                exit(1);
        }

	if ((newudfilefp = fopen(newudfile,"wb")) == NULL) {
		perror(newudfile);
                exit(1);
        }

	count = 0;
	while (!feof(udfilefp)) {
		fread(&udpost,sizeof(udpost),1,udfilefp);
		lotNr = rLotForDOCid(udpost.DocID);


		for (i=0;i<nrOfsfLots;i++) {
			if (sflot[i] == lotNr) {
				//printf("funnet: url %s, DocID %u-%i\n",udpost.url,udpost.DocID,lotNr);
				fwrite(&udpost,sizeof(udpost),1,newudfilefp);
				++count;
				continue;
			}
		}
	}

	printf("found %i.\n",count);

}	
