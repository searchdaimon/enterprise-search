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

int Indekser(char revindexPath[]);
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



	printf("lot %s, %i\n",argv[1],argc);


	if ((argc) == 5) {
		lotNr = atoi(argv[2]);
		strncpy(subname,argv[3],sizeof(subname) -1);
		lotPart = atoi(argv[4]);
		printf("indexint part %i for lot %i\n",lotPart,lotNr);

		GetFilPathForLot(path,lotNr,subname);

		sprintf(revpath,"%srevindex/%s/%i.txt",path,argv[1],lotPart);

		Indekser(revpath);	

	
	}
	else {
		printf("usage: ./LotInvertetIndexMaker type lotnr subname lotPart\n\n");

	}


}

int Indekser(char revindexPath[]) {

	struct revIndexArrayFomat *revIndexArray; 
	revIndexArray = malloc(sizeof(struct revIndexArrayFomat) * revIndexArraySize);

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

//	#ifdef DEBUG
		printf("revindexPath \"%s\"\n",revindexPath);
//	#endif

	if ((REVINDEXFH = fopen(revindexPath,"rb")) == NULL) {
		perror(revindexPath);
		//exit(1);
	}
	else {
	count = 0;
	while ((!feof(REVINDEXFH)) && (count < revIndexArraySize)) {
	



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

			//#ifdef DEBUG
				printf("%i\n",count);
				printf("\tDocID %u lang %i\n",revIndexArray[count].DocID,(int)revIndexArray[count].langnr);
				printf("\tread WordID: %u, nrOfHits %u\n",revIndexArray[count].WordID,revIndexArray[count].nrOfHits);
			//#endif

			if (revIndexArray[count].nrOfHits > MaxsHitsInIndex) {
				printf("nrOfHits lager then MaxsHitsInIndex. Nr was %i for %s\n",revIndexArray[count].nrOfHits,revindexPath);
				return 0;
			}

			//leser antal hist vi skulle ha
			fread(&revIndexArray[count].hits,revIndexArray[count].nrOfHits * sizeof(short),1,REVINDEXFH);


			
			//debug:  hits
			//#ifdef DEBUG
			printf("\tread hits: ");
			for (i=0;i<revIndexArray[count].nrOfHits;i++) {
				printf("%hu, ",revIndexArray[count].hits[i]);
			}
			printf("\n");
			//#endif
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


	fclose(REVINDEXFH);

	}
	
}
