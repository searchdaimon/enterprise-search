#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>

#include "../common/define.h"

struct lotinfoFomrat {
	int lot;
	int nr;
};


static inline int isin(int lot, int nroflotToSearch, struct lotinfoFomrat lotsToSearch[]) {

	int i;

	for(i=0;i<nroflotToSearch;i++) {
		if (lot==lotsToSearch[i].lot) {
			++lotsToSearch[i].nr;
			return 1;
		}
	}
	
	return 0;
}


main (int argc, char *argv[]) {

        char lotpath[512];
        int lotNr;
        int i, y;

        FILE *OUTFILE_remove;
        FILE *OUTFILE_keep;
	struct udfileFormat udpost;
	int nroflotToSearch;

        if (argc < 3) {
               	printf("Dette programet går gjenom en udfile og finner lots som mangler\n\n");
		printf("usage: ./missinglotGetFormUdfile outfile_remove outfile_keep lotnr [ lotnr2 lotnr3 .. n] < udfile\n\n");
               	exit(0);
        }


        char *outfile_remove = argv[1];
        char *outfile_keep = argv[2];
	nroflotToSearch = argc -3;
	struct lotinfoFomrat lotsToSearch[nroflotToSearch];

	y=0;
	for (i=3;i<argc;i++) {
		printf("i: %i, \"%s\"\n",i,argv[i]);
		lotsToSearch[y].lot = atoi(argv[i]);
		lotsToSearch[y].nr = 0;
		++y;
	}


	if ((OUTFILE_remove = fopen(outfile_remove,"ab")) == NULL) {
		perror(outfile_remove);
		exit(1);
	}
	if ((OUTFILE_remove = fopen(outfile_keep,"ab")) == NULL) {
		perror(outfile_keep);
		exit(1);
	}

	while (fread(&udpost,sizeof(udpost),1,stdin) > 0) {
		if (isin(rLotForDOCid(udpost.DocID),nroflotToSearch,lotsToSearch)) {
			#ifdef DEBUG
			printf("DocID %u-%i, Url \"%s\"\n",udpost.DocID,rLotForDOCid(udpost.DocID),udpost.url);
			#endif
			if (fwrite(&udpost,sizeof(udpost),1,OUTFILE_remove) != 1) {
				perror("write");
				exit(1);
			}
		}
		else {
			if (fwrite(&udpost,sizeof(udpost),1,OUTFILE_keep) != 1) {
				perror("write");
				exit(1);
			}
		}
	}

	fclose(OUTFILE_remove);
	fclose(OUTFILE_keep);


	for(i=0;i<nroflotToSearch;i++) {
		printf("lot %i nr %i\n",lotsToSearch[i].lot,lotsToSearch[i].nr);
	}

}
