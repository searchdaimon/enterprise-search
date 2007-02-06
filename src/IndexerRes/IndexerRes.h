#include <stdio.h>
#include <string.h>
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
#include "../common/revindex.h"
#include "../common/url.h"
#include "../common/DocumentIndex.h"
#include "../common/lot.h"

#include "../parser/html_parser.h"

#define maxWordForPage 4000
#define maxAdultWords 500
#define maxWordlLen 30
#define MaxAdultWordCount 60

//#define DEBUG_ADULT

//#define subname "www"

#define AdultWordsVektetFile "data/AdultWordsVektet.txt"
#define AdultFraserVektetFile "data/AdultFraserVektet.txt"

struct indexerLotConfigFormat {
        int collectUrls;
        char **urlfilter;
};

struct indexerLotConfigFormat globalIndexerLotConfig;


//global veriabel som sier fra om en alarm ble utløst. Trenger bare en da man bare kan ha en alarm av gangen
volatile sig_atomic_t alarm_got_raised;
unsigned int global_curentDocID;
int global_curentUrlIsDynamic;
char *subname;
int global_source_url_havpri;

struct addNewUrlhaFormat global_addNewUrlha;
struct addNewUrlhaFormat global_addNewUrlha_pri1;
struct addNewUrlhaFormat global_addNewUrlha_pri2;


struct adultWordFormat {
	char word[maxWordlLen +1];
	unsigned long crc32;
	int weight;
};

struct revIndexFomat {
	#ifdef DEBUG_ADULT
		char word[maxWordlLen +1];
	#endif
        unsigned long WordID;
        unsigned long nr;
        unsigned short hits[MaxsHitsInIndex];
	int bucket;

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

//struct wordsFormat {
//        #ifdef DEBUG_ADULT
//                char word[maxWordlLen];
//        #endif
//	unsigned int WordID;
//	unsigned int position;
//	unsigned short occurrence;
//};

struct nrofBucketElementsFormat {
            int records;
            int hits;
            void *bucketbuff;
            int bucketbuffsize;
            void *p;
};

struct pagewordsFormat {
	int nr;
	int nextPosition;
	struct wordsFormat words[maxWordForPage];
	struct wordsFormat words_sorted[maxWordForPage];
	int revIndexnr;
	struct revIndexFomat revIndex[maxWordForPage];
	int nrOfOutLinks;
	char lasturl[201];
	int curentUrlIsDynamic;
	unsigned int curentDocID;
	struct nrofBucketElementsFormat nrofBucketElements[NrOfDataDirectorys];
};

struct pagewordsFormat pagewords;

void html_parser_timout( int signo );
void pagewordsSortOnOccurrence();

void copyRepToDi(struct DocumentIndexFormat *DocumentIndexPost,struct ReposetoryHeaderFormat *ReposetoryHeader);

void revindexFilesAppendWords(struct pagewordsFormat *pagewords,FILE *revindexFilesHa[],unsigned int DocID,unsigned char *langnr);

void wordsMakeRevIndexBucket (struct pagewordsFormat *pagewords,unsigned int DocID,unsigned char *langnr) ;

void handelPage(struct pagewordsFormat *pagewords, unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader,
                char HtmlBuffer[],int HtmlBufferLength,struct DocumentIndexFormat *DocumentIndexPost,
                int DocID,int httpResponsCodes[], struct adultFormat *adult, unsigned char *langnr,
                char **title, char **body);

void wordsReset(struct pagewordsFormat *pagewords);
