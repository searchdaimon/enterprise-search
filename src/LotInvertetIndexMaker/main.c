#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/mgsort.h"

#include "../common/define.h"
#include "../common/lot.h"

#define subname "www"
#define revIndexArraySize 2000000

struct revIndexArrayFomat {
	unsigned int DocID;
        unsigned long WordID;
	char lang[4];
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

	struct revIndexArrayFomat *revIndexArray; 
	revIndexArray = malloc(sizeof(struct revIndexArrayFomat) * revIndexArraySize);

	if (argc < 2) {
	}

	printf("lot %s, %i\n",argv[1],argc);


	if (argc == 3) {
		lotNr = atoi(argv[2]);

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

		for (lotPart=0;lotPart<63;lotPart++) {
			//printf("indexint part %i for lot %i\n",lotPart,lotNr);

			//"$revindexPath/$revindexFilNr.txt";
			GetFilPathForLot(path,lotNr,subname);
			sprintf(revpath,"%srevindex/%s/%i.txt",path,argv[1],lotPart);
			//ToDo: må sette språk annen plass
			sprintf(iipath,"%siindex/%s/index/aa/",path,argv[1]);

			//oppretter paths
			makePath(iipath);			

			sprintf(iipath,"%s%i.txt",iipath,lotPart);

			Indekser(revpath,iipath,revIndexArray);	

			//sletter revindex. Ingen vits i å ha den fylle opp plass
			//remove(revpath);

		}
	}
	else if (argc == 4) {
		lotNr = atoi(argv[2]);
		lotPart = atoi(argv[3]);

		printf("indexint part %i for lot %i\n",lotPart,lotNr);

		//"$revindexPath/$revindexFilNr.txt";
		GetFilPathForLot(path,lotNr,subname);
		sprintf(revpath,"%srevindex/%s/%i.txt",path,argv[1],lotPart);
		//ToDo: må sette språk annen plass
		sprintf(iipath,"%siindex/%s/index/aa/%i.txt",path,argv[1],lotPart);

		Indekser(revpath,iipath,revIndexArray);	
	
	}
	else {
		printf("usage: ./LotInvertetIndexMaker type lotnr [ lotPart ]\n\n");

	}

	//GetFilPathForLot(lotNr);

}

int Indekser(char revindexPath[],char iindexPath[],struct revIndexArrayFomat revIndexArray[]) {

	int i,y;
	//int mgsort_i,mgsort_k;
	FILE *REVINDEXFH;
	unsigned int nrOfHits;
	unsigned short hit;
	char recordSeperator[4];
	int count;
	char c;
        unsigned int DocID;
	unsigned int lastWordID;
        char lang[4];
	unsigned int nrofDocIDsForWordID[revIndexArraySize];
	int forekomstnr;


	if ((REVINDEXFH = fopen(revindexPath,"rb")) == NULL) {
		perror(revindexPath);
		//exit(1);
	}
	else {
	count = 0;
	while (!feof(REVINDEXFH)) {
	
		fread(&DocID,sizeof(DocID),1,REVINDEXFH);
		fread(lang,sizeof(lang) -1,1,REVINDEXFH);
		lang[3] = '\0';

		//printf("read DocID %u, lang \"%s\"\n",DocID,lang);

		/*
		vi kan ha DocID,Lang,recordseperator, uten noen etterfølgende hit. For
		ådetektere det må vi lese rc så søke ilbake.
		
		lite effektift, må 

		*/

		//så lenge vi ikke går over noen grense.
		//blir som regel avslutte med break, npr ci nåe record seperator
		while (count < revIndexArraySize) {

		//her kan vi enten ha record seperator, eller info om treff
		
		fread(recordSeperator,sizeof(recordSeperator) -1,1,REVINDEXFH);

		if ((recordSeperator[0] == '*') && (recordSeperator[1] == '*') && (recordSeperator[2] == '\n')) {
			//record seperator. Avslutter denne dokiden
			break;
		}
		else {		
			//nå record

			revIndexArray[count].DocID = DocID;
			memcpy(revIndexArray[count].lang,lang,sizeof(lang) -1);


			//leste jo 3 tegn for å lete etter record seperator. Må nå gå tilbake
			fseek(REVINDEXFH,-3,SEEK_CUR);
			
			fread(&revIndexArray[count].WordID,sizeof(revIndexArray[count].WordID),1,REVINDEXFH);
			fread(&revIndexArray[count].nrOfHits,sizeof(revIndexArray[count].nrOfHits),1,REVINDEXFH);

			//printf("\tWordID: %u, %u: ",revIndexArray[count].WordID,revIndexArray[count].nrOfHits);


			if (revIndexArray[count].nrOfHits > MaxsHitsInIndex) {
				printf("nrOfHits lager then MaxsHitsInIndex. Nr was %i for %s\n",revIndexArray[count].nrOfHits,revindexPath);
				return 0;
			}

			//leser antal hist vi skulle ha
			fread(&revIndexArray[count].hits,revIndexArray[count].nrOfHits * sizeof(short),1,REVINDEXFH);


			
			//debug:  hits
			/*
			for (i=0;i<revIndexArray[count].nrOfHits;i++) {
				printf("%hu, ",revIndexArray[count].hits[i]);
			}
			printf("\n");
			*/
			++count;


		}
		}
			
		//hvis vi når grensen
		if (count == revIndexArraySize) {
			printf("revIndexArraySize hit\n");
			break;
			
		}

	}
	--count;

	fclose(REVINDEXFH);

	if ((REVINDEXFH = fopen(iindexPath,"wb")) == NULL) {
		perror(iindexPath);
		exit(1);
	}
	
	//printf("sort\n");
	//sorterer på WordID
	//qsort(revIndexArray, count , sizeof(struct revIndexArrayFomat), compare_elements);
	//int mgsort(void *data, int size, int esize, int i, int k, int (*compare) (const void *key1, const void *key2));
	//må ha en stabil sorteringsalgoritme
	//mgsort_i = 0;	
	//mgsort_k = count -1;
	mgsort(revIndexArray, count , sizeof(struct revIndexArrayFomat),compare_elements);

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
		
			fwrite(&revIndexArray[i].WordID,sizeof(revIndexArray[i].WordID),1,REVINDEXFH);
			fwrite(&nrofDocIDsForWordID[forekomstnr],sizeof(int),1,REVINDEXFH);

			//printf("WordID %u, nr %u\n",revIndexArray[i].WordID,nrofDocIDsForWordID[forekomstnr]);

			
			++forekomstnr;
		}
		lastWordID = revIndexArray[i].WordID;

		//printf("\tDocID %u, nrOfHits %u\n",revIndexArray[i].DocID,revIndexArray[i].nrOfHits);

		//skrive DocID og antall hit vi har
		fwrite(&revIndexArray[i].DocID,sizeof(revIndexArray[i].DocID),1,REVINDEXFH);
		fwrite(&revIndexArray[i].nrOfHits,sizeof(revIndexArray[i].nrOfHits),1,REVINDEXFH);

		//skriver alle hittene		
		for(y=0;y<revIndexArray[i].nrOfHits;y++) {
			//printf("\t\thit %hu\n",revIndexArray[i].hits[y]);
			fwrite(&revIndexArray[i].hits[y],sizeof(short),1,REVINDEXFH);
		}

		//printf("DocID %u, WordID: %u, %u\n",revIndexArray[i].DocID,revIndexArray[i].WordID,revIndexArray[i].nrOfHits);		
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

