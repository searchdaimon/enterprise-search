#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../common/mgsort.h"

#include "../common/define.h"
#include "../common/lot.h"
#include "../common/revindex.h"

//#define subname "www"
#define revIndexArraySize 2000000

struct revIndexArrayFomat {
	unsigned int DocID;
        unsigned long WordID;
	unsigned char langnr;
        unsigned long nrOfHits;
        unsigned short hits[MaxsHitsInIndex];
};

//int Indekser(char revindexPath[],char iindexPath[],struct revIndexArrayFomat revIndexArray[]);
int Indekser(char iindexPath[],struct revIndexArrayFomat revIndexArray[],int lotNr,char type[],int part,char subname[]);

int compare_elements (const void *p1, const void *p2);

int main (int argc, char *argv[]) {


	int lotNr;
	int lotPart;
	char path[256];
	char iipath[256];
	unsigned lastIndexTime;
	int optMustBeNewerThen = 0;

	struct revIndexArrayFomat *revIndexArray; 
	revIndexArray = malloc(sizeof(struct revIndexArrayFomat) * revIndexArraySize);

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"n"))!=-1) {
                switch (c) {
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

	char *type = argv[1 +optind];
	lotNr = atoi(argv[2 +optind]);
	char *subname = argv[3 +optind];

	if ((argc -optind)== 4) {

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
			//ToDo: må sette språk annen plass
			sprintf(iipath,"%siindex/%s/index/aa/",path,argv[1 +optind]);

			//oppretter paths
			makePath(iipath);			

			sprintf(iipath,"%s%i.txt",iipath,lotPart);

			if ((optMustBeNewerThen != 0)) {
				if (fopen(iipath,"r") != NULL) {
					printf("we all redy have a iindex.\n");
					continue;
				}
			}


			Indekser(iipath,revIndexArray,lotNr,type,lotPart,subname);	



		}
	}
	else if ((argc - optind) == 5) {
		lotPart = atoi(argv[4 +optind]);

		printf("indexint part %i for lot %i\n",lotPart,lotNr);

		//"$revindexPath/$revindexFilNr.txt";
		GetFilPathForLot(path,lotNr,subname);
		//ToDo: må sette språk annen plass
		//aa sprintf(iipath,"%siindex/%s/index/aa/%i.txt",path,argv[1 +optind],lotPart);
                //ToDo: må sette språk annen plass
                sprintf(iipath,"%siindex/%s/index/aa/",path,argv[1 +optind]);

                //oppretter paths
                makePath(iipath);

                sprintf(iipath,"%s%i.txt",iipath,lotPart);


		if ((optMustBeNewerThen != 0)) {
			if (fopen(iipath,"r") != NULL) {
				printf("we all redy have a iindex.\n");
				exit(1);
			}
		}

		Indekser(iipath,revIndexArray,lotNr,type,lotPart,subname);	

	
	}
	else {
		printf("usage: ./LotInvertetIndexMaker type lotnr subname [ lotPart ]\n\n");

	}

	//GetFilPathForLot(lotNr);

}

int Indekser(char iindexPath[],struct revIndexArrayFomat revIndexArray[],int lotNr,char type[],int part,char subname[]) {

	int i,y;
	int mgsort_i,mgsort_k;
	FILE *REVINDEXFH;
	unsigned int nrOfHits;
	unsigned short hit;
	char recordSeperator[4];
	int count;
	char c;
        unsigned int DocID;
	unsigned int lastWordID;
        //char lang[4];
	unsigned int nrofDocIDsForWordID[revIndexArraySize];
	int forekomstnr;

	#ifdef DEBUG
		printf("revindexPath \"%s\"\n",revindexPath);
	#endif


	//if ((REVINDEXFH = fopen(revindexPath,"rb")) == NULL) {
	if ((REVINDEXFH = revindexFilesOpenLocalPart(lotNr,type,"rb",subname,part)) == NULL) {
		perror("revindexFilesOpenLocalPart");
		//exit(1);
	}
	else {
	count = 0;
	while ((!feof(REVINDEXFH)) && (count < revIndexArraySize)) {
	


		//så lenge vi ikke går over noen grense.
		//while (count < revIndexArraySize) {

		//her kan vi enten ha record seperator, eller info om treff
		
		

			if (fread(&revIndexArray[count].DocID,sizeof(revIndexArray[count].DocID),1,REVINDEXFH) != 1) {
				printf("can't read any more data\n");
				perror("revindex");
				break;
			}
			//v3
			fread(&revIndexArray[count].langnr,sizeof(char),1,REVINDEXFH);
			//printf("lang1 %i\n",(int)revIndexArray[count].langnr);


			fread(&revIndexArray[count].WordID,sizeof(revIndexArray[count].WordID),1,REVINDEXFH);
			fread(&revIndexArray[count].nrOfHits,sizeof(revIndexArray[count].nrOfHits),1,REVINDEXFH);

			#ifdef DEBUG
				printf("%i\n",count);
				printf("\tDocID %u lang %i\n",revIndexArray[count].DocID,(int)revIndexArray[count].langnr);
				printf("\tread WordID: %u, nrOfHits %u\n",revIndexArray[count].WordID,revIndexArray[count].nrOfHits);
			#endif

			if (revIndexArray[count].nrOfHits > MaxsHitsInIndex) {
				printf("nrOfHits lager then MaxsHitsInIndex. Nr was %i\n",revIndexArray[count].nrOfHits);
				return 0;
			}

			//leser antal hist vi skulle ha
			fread(&revIndexArray[count].hits,revIndexArray[count].nrOfHits * sizeof(short),1,REVINDEXFH);


			
			//debug:  hits
			#ifdef DEBUG
			printf("\tread hits: ");
			for (i=0;i<revIndexArray[count].nrOfHits;i++) {
				printf("%hu, ",revIndexArray[count].hits[i]);
			}
			printf("\n");
			#endif
			++count;


		//}
		//}
			
		////hvis vi når grensen
		//if (count == revIndexArraySize) {
		//	printf("revIndexArraySize hit\n");
		//	break;
		//	
		//}

	}

	printf("Documents in index: %i\n",count);

	
	//runarb: 17 aug 2007: hvorfor har vi med -- her. Ser ut til at vi da mksiter siste dokumentet. haker ut for nå
	//--count;

	fclose(REVINDEXFH);

	
	//printf("sort\n");
	//sorterer på WordID
	//qsort(revIndexArray, count , sizeof(struct revIndexArrayFomat), compare_elements);
	//int mgsort(void *data, int size, int esize, int i, int k, int (*compare) (const void *key1, const void *key2));
	//må ha en stabil sorteringsalgoritme
	//mgsort_i = 0;	
	//mgsort_k = count -1;
	//mgsort(revIndexArray, count , sizeof(struct revIndexArrayFomat),mgsort_i,mgsort_k,compare_elements);

	mgsort(revIndexArray, count , sizeof(struct revIndexArrayFomat),compare_elements);

	//int mgsort(void *data, int size, int esize, int (*compare) (const void *key1, const void *key2));

	if ((REVINDEXFH = fopen(iindexPath,"wb")) == NULL) {
		perror(iindexPath);
		exit(1);
	}

	//teller forkomster av DocID's pr WordID
	lastWordID = 0;
	forekomstnr = 0;
	for(i=0;i<count;i++) {
		if (lastWordID != revIndexArray[i].WordID) {
			nrofDocIDsForWordID[forekomstnr] = 1;
			++forekomstnr;			
		}
		else {
			++nrofDocIDsForWordID[forekomstnr -1];
		}
		lastWordID = revIndexArray[i].WordID;

	}

	lastWordID = 0;
	forekomstnr = 0;
	for(i=0;i<count;i++) {
		
		if (lastWordID != revIndexArray[i].WordID) {

			#ifdef DEBUG
				printf("write WordID %u, nr %u\n",revIndexArray[i].WordID,nrofDocIDsForWordID[forekomstnr]);
			#endif
		
			fwrite(&revIndexArray[i].WordID,sizeof(revIndexArray[i].WordID),1,REVINDEXFH);
			fwrite(&nrofDocIDsForWordID[forekomstnr],sizeof(int),1,REVINDEXFH);

			++forekomstnr;
		}
		lastWordID = revIndexArray[i].WordID;

		//printf("\tDocID %u, nrOfHits %u\n",revIndexArray[i].DocID,revIndexArray[i].nrOfHits);

		//skrive DocID og antall hit vi har
		fwrite(&revIndexArray[i].DocID,sizeof(revIndexArray[i].DocID),1,REVINDEXFH);
		//v3
		fwrite(&revIndexArray[i].langnr,sizeof(char),1,REVINDEXFH);

		fwrite(&revIndexArray[i].nrOfHits,sizeof(revIndexArray[i].nrOfHits),1,REVINDEXFH);

		//skriver alle hittene		
		for(y=0;y<revIndexArray[i].nrOfHits;y++) {
			#ifdef DEBUG		
				printf("\t\thit %hu\n",revIndexArray[i].hits[y]);
			#endif
			fwrite(&revIndexArray[i].hits[y],sizeof(short),1,REVINDEXFH);
		}
		#ifdef DEBUG
		printf("write: DocID %u, WordID: %u, %u\n",revIndexArray[i].DocID,revIndexArray[i].WordID,revIndexArray[i].nrOfHits);		
		#endif
	}

	fclose(REVINDEXFH);
	} //else filsjekk
}

//sortere først på WordID, så DocID
//krever en stabil algoritme
int compare_elements (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

	
	if (((struct revIndexArrayFomat*)p1)->WordID == ((struct revIndexArrayFomat*)p2)->WordID) {

		if (((struct revIndexArrayFomat*)p1)->DocID < ((struct revIndexArrayFomat*)p2)->DocID) {
        	        return -1;
        	}
        	else {
        	        return ((struct revIndexArrayFomat*)p1)->DocID > ((struct revIndexArrayFomat*)p2)->DocID;
	        }


	}
        else if (((struct revIndexArrayFomat*)p1)->WordID < ((struct revIndexArrayFomat*)p2)->WordID) {
                return -1;
	}
        else {
                return ((struct revIndexArrayFomat*)p1)->WordID > ((struct revIndexArrayFomat*)p2)->WordID;
	}
}

