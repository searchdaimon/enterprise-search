#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "../common/define.h"
#include "../common/lot.h"
#include "../common/lotlist.h"


int main (int argc, char *argv[]) {

	int i;
	int lotNr;
	int count;

	FILE *newudfilefp;
	struct udfileFormat udpost;

	if (argc <= 2) {
		printf("Usage: zcat udfile.gz | ./recoverUrlForLot newudfile lot [lot ...]\n");
		printf("OR\n");
		printf("Usage: zcat udfile.gz | ./recoverUrlForLot -s server newudfile\n");
		exit(1);
	}


        char *optServer = NULL;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"s:"))!=-1) {
                switch (c) {
                        case 's':
                                optServer = optarg;
                                break;
                        default:
                                exit(1);
                }

        }
        --optind;


	char *newudfile = argv[1 +optind];

	//tester at den ikke finnes. Viktig at vi ikke overskriver noe ved en feil
	if ((newudfilefp = fopen(newudfile,"rb")) != NULL) {
       	        printf("newudfile exist. It shuldent. Delete it of you don't need it\n");
       	        exit(1);
       	}

	if ((newudfilefp = fopen(newudfile,"wb")) == NULL) {
		perror(newudfile);
       	        exit(1);
       	}


	count = 0;

	if (optServer != NULL) {

		printf("will recover lot for server \"%s\"\n",optServer);

   		lotlistLoad();
        	lotlistMarkLocals(optServer);
				

		while (fread(&udpost,sizeof(udpost),1,stdin) == 1) {

			lotNr = rLotForDOCid(udpost.DocID);

			if (lotlistIsLocal(lotNr)) {
				//printf("funnet: url %s, DocID %u-%i\n",udpost.url,udpost.DocID,lotNr);
				fwrite(&udpost,sizeof(udpost),1,newudfilefp);
				++count;
				continue;
			}
			else {
				//printf("no match: url %s, DocID %u-%i\n",udpost.url,udpost.DocID,lotNr);
			}

		}
	}
	else {

		int nrOfsfLots = (argc - 2);

		int sflot[nrOfsfLots];

		printf("newudfile %s. nrOfsfLots %i\n",newudfile,nrOfsfLots);

		for (i=0;i<nrOfsfLots;i++) {
			sflot[i] = atoi(argv[i +2]);
			printf("aa %s, %i\n",argv[i +2],sflot[i]);

		}



		while (fread(&udpost,sizeof(udpost),1,stdin) == 1) {

			lotNr = rLotForDOCid(udpost.DocID);


			for (i=0;i<nrOfsfLots;i++) {
				if (sflot[i] == lotNr) {
					//printf("funnet: url %s, DocID %u-%i\n",udpost.url,udpost.DocID,lotNr);
					fwrite(&udpost,sizeof(udpost),1,newudfilefp);
					++count;
					continue;
				}
				else {
					printf("no match: url %s, DocID %u-%i\n",udpost.url,udpost.DocID,lotNr);
				}
			}
		}

		if (!feof(stdin)) {
			perror("stdin");
		}


	}

	printf("found %i.\n",count);

	return EXIT_SUCCESS;
}	
