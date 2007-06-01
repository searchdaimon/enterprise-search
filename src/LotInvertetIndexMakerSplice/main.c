#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../common/mgsort.h"

#include "../common/define.h"
#include "../common/lot.h"

//#define subname "www"
#define revIndexArraySize 2000000

struct revIndexArrayFomat {
	unsigned int DocID;
        unsigned long WordID;
	unsigned char langnr;
        unsigned long nrOfHits;
        unsigned short hits[MaxsHitsInIndex];
};

int Indekser(char revindexPath[],char iindexPath[],struct revIndexArrayFomat revIndexArray[]);
int compare_elements (const void *p1, const void *p2);

int main (int argc, char *argv[]) {


	int lotNr;
	int lotPart;
	char path[256];
	char revpath[256];
	char iipath[256];
	unsigned lastIndexTime;
	char subname[maxSubnameLength];
	int optMustBeNewerThen = 0;
	int optUnlink = 0;

	struct revIndexArrayFomat *revIndexArray; 
	revIndexArray = malloc(sizeof(struct revIndexArrayFomat) * revIndexArraySize);

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"nu"))!=-1) {
                switch (c) {
			case 'u':
				optUnlink = 1;
				break;
                        case 'n':
                                optMustBeNewerThen = 1;
                                break;
                        case 'v':
                                break;
                        default:
                                          exit(1);
                }
        }
        --optind;

	printf("lot %s, %i\n",argv[1],argc);


	if ((argc -optind)== 4) {
		lotNr = atoi(argv[2 +optind]);
		strncpy(subname,argv[3 +optind],sizeof(subname) -1);

                //finner siste indekseringstid
                lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);


                if(lastIndexTime == 0) {
                        printf("lastIndexTime is 0\n");
                        exit(1);
                }

               //sjekker om vi har nokk palss
                if (!lotHasSufficientSpace(lotNr,4096,subname)) {
                        printf("insufficient disk space\n");
                        exit(1);
                }


        	printf("Indexing all buvkets for lot %i\n",lotNr);

		for (lotPart=0;lotPart<64;lotPart++) {
			//printf("indexint part %i for lot %i\n",lotPart,lotNr);

			//"$revindexPath/$revindexFilNr.txt";
			GetFilPathForLot(path,lotNr,subname);
			sprintf(revpath,"%srevindex/%s/%i.txt",path,argv[1 +optind],lotPart);
			//ToDo: må sette språk annen plass
			sprintf(iipath,"%siindex/%s/index/aa/",path,argv[1 +optind]);

			//oppretter paths
			makePath(iipath);			

			sprintf(iipath,"%s%i.txt",iipath,lotPart);

			if ((optMustBeNewerThen != 0)) {
				if (fopen(iipath,"r") != NULL) {
					printf("we al redy hav a iindex.\n");
					continue;
				}
			}

			Indekser(revpath,iipath,revIndexArray);	

			if (optUnlink) {
				unlink(revpath);
			}

			//sletter revindex. Ingen vits i å ha den fylle opp plass
			//remove(revpath);

		}
	}
	else if ((argc - optind) == 5) {
		lotNr = atoi(argv[2 +optind]);
		strncpy(subname,argv[3 +optind],sizeof(subname) -1);
		lotPart = atoi(argv[4 +optind]);
		printf("indexint part %i for lot %i\n",lotPart,lotNr);

		//"$revindexPath/$revindexFilNr.txt";
		GetFilPathForLot(path,lotNr,subname);
		sprintf(revpath,"%srevindex/%s/%i.txt",path,argv[1 +optind],lotPart);
		//ToDo: må sette språk annen plass
		//aa sprintf(iipath,"%siindex/%s/index/aa/%i.txt",path,argv[1 +optind],lotPart);
                        //ToDo: må sette språk annen plass
                        sprintf(iipath,"%siindex/%s/index/aa/",path,argv[1 +optind]);

                        //oppretter paths
                        makePath(iipath);

                        sprintf(iipath,"%s%i.txt",iipath,lotPart);


			if ((optMustBeNewerThen != 0)) {
				if (fopen(iipath,"r") != NULL) {
					printf("we al redy hav a iindex.\n");
					exit(1);
				}
			}

		Indekser(revpath,iipath,revIndexArray);	

			if (optUnlink) {
				unlink(revpath);
			}
	
	}
	else {
		printf("usage: ./LotInvertetIndexMaker type lotnr subname [ lotPart ]\n\n");

	}

	//GetFilPathForLot(lotNr);

}

int Indekser(char revindexPath[],char iindexPath[],struct revIndexArrayFomat revIndexArray[]) {

	int i,y;

	FILE *fileha;

        unsigned long DocID;
        unsigned long TermAntall;
        unsigned short hit;

	unsigned long term;
	unsigned long Antall;;
	unsigned char langnr;


	if ((fileha = fopen(revindexPath,"rb")) == NULL) {
		perror(revindexPath);
		//exit(1);
	}
	else {



	while (!feof(fileha)) {
		//wordid hedder
        	fread(&term,sizeof(unsigned long),1,fileha);
        	fread(&Antall,sizeof(unsigned long),1,fileha);

		printf("term: %u antall: %u\n",term,Antall);

		for (i=0;i<Antall;i++) {
			//side hedder
			fread(&DocID,sizeof(unsigned long),1,fileha);
			fread(&langnr,sizeof(char),1,fileha);
        		fread(&TermAntall,sizeof(unsigned long),1,fileha);

			printf("DocID: %u, langnr: %i, nr: %u. Hits: ",DocID,(int)langnr,TermAntall);

			for (y = 0;y < TermAntall; y++) {
                		        fread(&hit,sizeof(unsigned short),1,fileha);

					printf("%i,",hit);

			}
			printf("\n");
		}	
		//printf("\n\n");
	}
	fclose(fileha);

	}


}

