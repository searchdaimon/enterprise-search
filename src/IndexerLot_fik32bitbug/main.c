#include <stdio.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>


#include "../common/crc32.h"
#include "../common/define.h"
#include "../common/reposetoryNET.h"
#include "../common/reposetory.h"

#include "../common/DocumentIndex.h"
#include "../common/lot.h"

#include "../parser/html_parser.h"

#define maxWordForPage 4000
#define maxAdultWords 500
#define maxWordlLen 30
#define MaxAdultWordCount 50

//#define DEBUG_ADULT

#define subname "www"

#define AdultWordsVektetFile "data/AdultWordsVektet.txt"
#define AdultFraserVektetFile "data/AdultFraserVektet.txt"

//global veriabel som sier fra om en alarm ble utløst. Trenger bare en da man bare kan ha en alarm av gangen
volatile sig_atomic_t alarm_got_raised;
unsigned int global_curentDocID;
int global_curentUrlIsDynamic;

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf );

struct adultWordFormat {
	char word[maxWordlLen +1];
	unsigned long crc32;
	int weight;
};

struct adultWordFraserFormat {
        char word[maxWordlLen +1];
        unsigned long crc32;
	struct adultWordFormat adultWord[MaxAdultWordCount];
	int adultWordCount;
        
};

struct adultFormat {
	struct adultWordFormat AdultWords[maxAdultWords];
	int adultWordnr;
	struct adultWordFraserFormat adultFraser[maxAdultWords];
	int adultWordFrasernr;
};

struct revIndexFomat {
	unsigned long WordID;
	unsigned long nr;
	unsigned short hits[MaxsHitsInIndex];
};

struct wordsFormat {
        #ifdef DEBUG_ADULT
                char word[maxWordlLen];
        #endif
	unsigned long WordID;
	unsigned long position;
};

struct pagewordsFormat {
	int nr;
	int nextPosition;
	struct wordsFormat words[maxWordForPage];
	struct wordsFormat words_sorted[maxWordForPage];
	int revIndexnr;
	struct revIndexFomat revIndex[maxWordForPage];
};
struct pagewordsFormat pagewords;





int main (int argc, char *argv[]) {

        int lotNr;
	char lotServer[64];
	int pageCount;
	int i;

        unsigned int FiltetTime;
        unsigned int FileOffset;

        char htmlcompressdbuffer[524288];  //0.5 mb
        char imagebuffer[524288];  //0.5 mb
	
	int httpResponsCodes[nrOfHttpResponsCodes];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	struct DocumentIndexFormat DocumentIndexPost;
	unsigned long int radress;
	FILE *revindexFilesHa[NrOfDataDirectorys];
	struct adultFormat adult;
	unsigned int lastIndexTime;
        if (argc < 2) {
                printf("Dette programet indekserer en lot. Gi det et lot nummer\n");
                exit(0);
        }

	for(i=0;i<nrOfHttpResponsCodes;i++) {
		httpResponsCodes[i] = 0;
	}

	lotNr = atoi(argv[1]);



	//find server based on lotnr
	lotlistLoad();
	lotlistGetServer(lotServer,lotNr);


	printf("vil index lot nr %i at %s\n",lotNr,lotServer);

                //finner siste indekseringstid
                lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);


                if(lastIndexTime == 0) {
                        printf("lastIndexTime is 0, skiping.\n");
                        exit(1);
                }

	//temp: må hente dette fra slot server eller fil
	FiltetTime = 0;
	FileOffset = 2140483648;
	//FileOffset = 1997015914;
	
	pageCount = 0;

		printf("Wil acess files localy\n");


		while (rGetNext(lotNr,&ReposetoryHeader,htmlcompressdbuffer,imagebuffer,&radress,FiltetTime,FileOffset,subname)) {


				DIRead(&DocumentIndexPost,ReposetoryHeader.DocID,subname);		

				DocumentIndexPost.RepositoryPointer = radress;

				//skiver til DocumentIndex
				DIWrite(&DocumentIndexPost,ReposetoryHeader.DocID,subname);
				

			++pageCount;
		
		}




	printf("indexed %i pages\n\n\n",pageCount);

	return 0;
}
