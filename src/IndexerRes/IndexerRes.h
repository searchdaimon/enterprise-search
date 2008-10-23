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

#include "../parser2/html_parser.h"

#define IndexerMaxLinks 4048


#ifdef BLACK_BOKS
	#define maxWordForPage 40000
	#define maxAclForPage 10
	#define maxAttribForPage 20
#else
	#define maxWordForPage 4000
#endif

#define maxAdultWords 5000
#define maxWordlLen 30
#define MaxAdultWordCount 250


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
	int addedAllReady;
};

struct revHitsFormat {
	unsigned short pos;
	unsigned short realpos;
};

struct revIndexFomat {
	//#ifdef DEBUG_ADULT
	#ifdef PRESERVE_WORDS
		char word[maxWordlLen +1];
		unsigned int wordnr;
	#endif
        unsigned long WordID;
        unsigned long nr;
        struct revHitsFormat hits[MaxsHitsInIndex];
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


struct outlinksFormat {
	char linktext[50];
	char url[200];
	int linktextlen;
	int good;
};

#ifdef BLACK_BOKS

struct IndexerRes_acls {
	int aclnr;
	struct wordsFormat acls[maxAclForPage];
	struct wordsFormat acls_sorted[maxAclForPage];
	int aclIndexnr;
        struct revIndexFomat aclIndex[maxAclForPage];
	struct nrofBucketElementsFormat nrofAclBucketElements[NrOfDataDirectorys];
};

struct IndexerRes_attrib {
	int attribnr;
	struct wordsFormat attrib[maxAttribForPage];
	struct wordsFormat attrib_sorted[maxAttribForPage];
	int attribIndexnr;
        struct revIndexFomat attribIndex[maxAttribForPage];
	struct nrofBucketElementsFormat nrofAttribBucketElements[NrOfDataDirectorys];
};
#endif

struct pagewordsFormatPartFormat{
	int nr;
	struct wordsFormat words[maxWordForPage];
	struct wordsFormat words_sorted[maxWordForPage];

	int revIndexnr;
	struct revIndexFomat revIndex[maxWordForPage];

	int nextPosition;

	struct nrofBucketElementsFormat nrofBucketElements[NrOfDataDirectorys];

};

struct pagewordsFormat {

	struct pagewordsFormatPartFormat normalWords;
	struct pagewordsFormatPartFormat linkWords;
	struct pagewordsFormatPartFormat spamWords;

	int nrOfOutLinks;
	char lasturl[201];
	int curentUrlIsDynamic;
	unsigned int curentDocID;
	unsigned int DocID;
	//struct updateFormat updatePost[IndexerMaxLinks];
	//struct outlinksFormat outlinks[IndexerMaxLinks];
	struct outlinksFormat *outlinks;

	#ifdef BLACK_BOKS
		struct IndexerRes_acls acl_allow;
		#ifdef IIACL
		struct IndexerRes_acls acl_denied;
		#endif
		struct IndexerRes_attrib attrib;
	#endif
};

//struct pagewordsFormat pagewords;

void html_parser_timout( int signo );
void pagewordsSortOnOccurrence();

void copyRepToDi(struct DocumentIndexFormat *DocumentIndexPost,struct ReposetoryHeaderFormat *ReposetoryHeader);

void revindexFilesAppendWords(struct pagewordsFormat *pagewords,FILE *revindexFilesHa[],unsigned int DocID,unsigned char *langnr);

void wordsMakeRevIndexBucket (struct pagewordsFormat *pagewords,unsigned int DocID,unsigned char *langnr) ;

void handelPage(struct pagewordsFormat *pagewords, unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader,
                char HtmlBuffer[],int HtmlBufferLength,
                int DocID,int httpResponsCodes[], struct adultFormat *adult,
                char **title, char **body);

void wordsReset(struct pagewordsFormat *pagewords,unsigned int DocID);
void dictionaryWordsWrite (struct pagewordsFormat *pagewords, FILE *FH, char *, char *);
void linksWrite(struct pagewordsFormat *pagewords,struct addNewUrlhaFormat addNewUrlha[]);

void wordsMakeRevIndex(struct pagewordsFormat *pagewords, struct adultFormat *adult,int *adultWeight, unsigned char *langnr);
void wordsInit(struct pagewordsFormat *pagewords);
void wordsEnd(struct pagewordsFormat *pagewords);

void adultLoad (struct adultFormat *adult);

#ifdef BLACK_BOKS
void acladd(struct IndexerRes_acls *acl, char word[]);
void aclsMakeRevIndex(struct IndexerRes_acls *acl);
void aclsMakeRevIndexBucket (struct IndexerRes_acls *acl,unsigned int DocID,unsigned char *langnr);
void aclindexFilesAppendWords(struct IndexerRes_acls *acl,FILE *aclindexFilesHa[],unsigned int DocID,unsigned char *langnr);

#ifdef ATTRIBUTE
void attribadd(struct IndexerRes_attrib *attrib, char word[]);
void attribMakeRevIndex(struct IndexerRes_attrib *attrib);
void attribMakeRevIndexBucket (struct IndexerRes_attrib *attrib,unsigned int DocID,unsigned char *langnr);
void attribindexFilesAppendWords(struct IndexerRes_attrib *attrib,FILE *attribindexFilesHa[]);
#endif

#endif
