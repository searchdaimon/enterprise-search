#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include <dirent.h>
#include <err.h>
#include <time.h>

#include "../common/define.h"
#include "../common/langdetect.h"
#include "../common/reposetoryNET.h"

#include "../common/crc32.h"
#include "../common/gcrepo.h"
#include "../common/gcsummary.h"

#include "../IndexerRes/IndexerRes.h"
#include "../common/integerindex.h"
#include "../searchFilters/searchFilters.h"

#include "../common/bstr.h"
#include "../common/debug.h"
#include "../common/ir.h"
#include "../common/iindex.h"
#include "../common/revindex.h"
#include "../common/bfileutil.h"
#include "../common/re.h"
#include "../common/attributes.h"

#include "../banlists/ban.h"
#include "../acls/acls.h"
#include "../common/ht.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../dictionarywordsLot/set.h"
#include "../dictionarywordsLot/acl.h"

#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dset.h"
#include "../ds/dmap.h"

#ifdef BLACK_BOX

#else
	#include "../maincfg/maincfg.h"
	#include "../dispatcher_all/library.h"
#endif


#ifdef WITH_THREAD
        #include <pthread.h>

	
        #ifdef BLACK_BOX
                #define DEFAULT_NROF_GENERATEPAGES_THREADS 5
        #else
                #define DEFAULT_NROF_GENERATEPAGES_THREADS 5
        #endif
	
#endif

struct DIArrayFormat {
	struct DocumentIndexFormat *p;
	struct DocumentIndexFormat *oldp;
	unsigned int DocID;
	char haveawvalue;
	unsigned char awvalue;
	struct brankPageElementsFormat brankPageElements;
	char haverankPageElements;
	unsigned short int DomainDI;
};

struct relocal {
	struct reformat *DocumentIndexFormat;
	struct reformat *crc32map;
	struct reformat	*attributeIndex;
	container	*attributeKeys;
};

struct filteredf {
                int isIpBan;
                int find_domain_no_subname;
                int find_TLD;
                int IndexerLot_filterOnlyTLD;
                int filterDomainNrOfLines;
                int filterDomainLength;
                int filterTLDs;
                int notHttp200;
};
struct IndexerLot_workthreadFormat {
	int lotNr;
	char *subname;
        unsigned int FiltetTime;
        unsigned int FileOffset;
	FILE *revindexFilesHa[NrOfDataDirectorys];
	#ifdef IIACL
	FILE *acl_allowindexFilesHa[NrOfDataDirectorys];
	FILE *acl_deniedindexFilesHa[NrOfDataDirectorys];
	#endif
	#ifdef ATTRIBUTES
	FILE *attribindexFilesHa[NrOfDataDirectorys];
	#endif
	struct adultFormat *adult;
	FILE *ADULTWEIGHTFH;
	FILE *SFH;
	struct acllotFormat *acllot;
	#ifdef ATTRIBUTES
	struct attriblotFormat *attriblot;
	#endif
	int pageCount;
	int httpResponsCodes[nrOfHttpResponsCodes];
	int optMaxDocuments;
	int optPrintInfo;
	int optMakeWordList;
	int optHandleOld;
	char **optOnlyTLD;
	int optHandleOld_duplicateNochange;
	int optHandleOld_allrediIndexed;
	int optHandleOld_indexed;
	int optHandleOld_thencur;
	char *optQuery;
	char *optNetLot;
	int rEindex;
	struct DIArrayFormat *DIArray;
	//struct DIArrayFormat DIArray[NrofDocIDsInLot];
	char *reponame;
	struct hashtable *words;
	struct hashtable *acls;
	

	#ifndef BLACK_BOX
		struct addNewUrlhaFormat addNewUrlha[NEWURLFILES_NR];
	#else
	#endif

	#ifdef WITH_THREAD
        	pthread_mutex_t reposetorymutex;
        	pthread_mutex_t restmutex;
        	pthread_mutex_t countmutex;
		pthread_mutex_t wordsmutex;
       	#endif

	#ifdef PRESERVE_WORDS
		FILE *dictionarywordsfFH;
	#endif

	struct relocal re;
	struct filteredf filtered;
	int n_new, n_recrawled, n_untouched;
	FILE *FNREPO;
	FILE *GCED;
};

struct optFormat {

	unsigned int MustBeNewerThen;
	unsigned int rEindex;
	unsigned int MaxDocuments;
	unsigned int PrintInfo;
	unsigned int MakeWordList;
	unsigned int HandleOld;
	unsigned int RunGarbageCollection;
	char *NetLot;
	unsigned int RunIndekser;
	char **OnlyTLD;
	unsigned int NrofWorkThreads;	
	char *Query;
	int dirty;
	int LotMax;
};


#ifdef BLACK_BOX
//acllot includes
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"



struct acllotFormat {
	char subname[maxSubnameLength];
	char openmode[4];
	int lotNr;
	int *h;
};

struct attriblotFormat {
	char subname[maxSubnameLength];
	char openmode[4];
	container *words;
	// Alt gÃ¥r i lot 1.
};


struct aclusernameFormat {
	char username[MAX_USER_NAME_LEN];
	int len;
};





static unsigned int acllot_hashfromkey(void *ky)
{
    struct aclusernameFormat *k = (struct aclusernameFormat *)ky;
    return crc32boithonl((*k).username,(*k).len);
}

static int acllot_equalkeys(void *k1, void *k2)
{
	//hvis di ikke her samme lengde er de ikke like
	if ((*(struct aclusernameFormat *)k1).len != (*(struct aclusernameFormat *)k2).len) {
		return 0;
	}
	else {
	//hvis vi derimot her samme lenge so må vi sjkke
    		return (0 == memcmp((*(struct aclusernameFormat *)k1).username,(*(struct aclusernameFormat *)k2).username,(*(struct aclusernameFormat *)k1).len));
	}
}


void acllot_init(struct acllotFormat **acllot,char subname[],char openmode[],int lotNr) {

	(*acllot) = malloc(sizeof(struct acllotFormat));
	strcpy((**acllot).subname,subname);
	strcpy((**acllot).openmode,openmode);
	(**acllot).lotNr = lotNr;

	//struct hashtable *h;
	(**acllot).h = (int *)create_hashtable(200, acllot_hashfromkey, acllot_equalkeys);



}
void acllot_close(struct acllotFormat *acllot) {

	FILE *fp;

	fp = lotOpenFileNoCasheByLotNr((*acllot).lotNr,"acllist",(*acllot).openmode, 'e',(*acllot).subname);

	//struct hashtable **h; //temp

	struct aclusernameFormat *aclusername;
	int *value;
	printf("acls for lot:\n");
	if (hashtable_count((struct hashtable *)(*acllot).h) > 0)
        {
        	struct hashtable_itr *itr;

                itr = hashtable_iterator((struct hashtable *)(*acllot).h);
                do {
                	aclusername 	= (struct aclusernameFormat *)hashtable_iterator_key(itr);
                        value 		= (int *)hashtable_iterator_value(itr);

                        printf("acl user \"%s\", len %i, nr %i\n",(*aclusername).username,(*aclusername).len,(*value));
			fwrite((*aclusername).username,(*aclusername).len,1,fp);
			fwrite("\n",1,1,fp);

               	} while (hashtable_iterator_advance(itr));
                free(itr);

    	}
	printf("acl list end\n");
	fclose(fp);
        hashtable_destroy((struct hashtable *)(*acllot).h,1);

	free(acllot);
}

void iiacladd(struct IndexerRes_acls *iiacl,char acl[]) {

  	char **Data;
  	int Count, TokCount;

  	TokCount = split(acl, ",", &Data);


	//legger til acler
	Count = 0;
	while( (Data[Count] != NULL) ) {

		//gruppenavn med spacer skaper problemer. Erstater det med _ i steden
		//strsandr(Data[Count]," ","_");
		//strsandr(Data[Count],"-","_");
		aclElementNormalize(Data[Count]);

		#ifdef DEBUG
		printf("got acl \"%s\"\n",Data[Count]);
		#endif

		if (Data[Count][0] == '\0') {		
			#ifdef DEBUG	
			printf("got emty acl. Ignoring\n");
			#endif
		}
		else {
			acladd(iiacl, Data[Count]);
		}
		++Count;
	}

  	FreeSplitList(Data);

}
void acllot_add(struct acllotFormat *acllot,char acl[]) {


	struct hashtable **h; //temp

  	char **Data;
  	int Count, TokCount;
	int *oldvalue;
	int *value;

  	TokCount = split(acl, ",", &Data);


  	Count = 0;
  	while( (Data[Count] != NULL) ) {

		//gruppenavn med spacer skaper problemer. Erstater det med _ i steden
		//strsandr(Data[Count]," ","_");
		//strsandr(Data[Count],"-","_");
		aclElementNormalize(Data[Count]);


		#ifdef DEBUG
		printf("god user \"%s\"\n",Data[Count]);
		#endif
		struct aclusernameFormat *aclusername;
		if ((aclusername = malloc(sizeof(struct aclusernameFormat))) == NULL) {
			perror("malloc aclusername");
			exit(-1);
		}

		(*aclusername).len = strnlen(Data[Count],MAX_USER_NAME_LEN);
		strscpy((*aclusername).username,Data[Count],MAX_USER_NAME_LEN);

		if (NULL == (oldvalue = hashtable_search((struct hashtable *)(*acllot).h,aclusername) )) {
			//printf("not found!. Vil insert first");
			value = malloc(sizeof(int));
			(*value) = 1;

			if (! hashtable_insert((struct hashtable *)(*acllot).h,aclusername,value) ) {
	                        printf("cant insert!\n");
	                	exit(-1);
	                }
		}
		else {
			//printf("username exist with %i. ++ing\n",(*oldvalue));
			++(*oldvalue);	
		}

		++Count;
	}

  	FreeSplitList(Data);

}


#ifdef ATTRIBUTES
/**
 *	Attributter:
 */

// InitialisÃ©r mapping-fil mellom crc32 og attributter:
void attriblot_init(struct attriblotFormat **attriblot, char *subname, char *openmode)
{
    (*attriblot) = malloc(sizeof(struct attriblotFormat));
    strcpy((*attriblot)->subname, subname);
    strcpy((*attriblot)->openmode, openmode);

    (*attriblot)->words = set_container( string_container() );
}

// Skriv mapping mellom crc32 og attributter til disk:
void attriblot_close(struct attriblotFormat *attriblot)
{
    FILE	*fp = lotOpenFileNoCasheByLotNr(1, "crc32attr.map", attriblot->openmode, 'e', attriblot->subname);
    iterator	it = set_begin(attriblot->words);
    container	*mapping = map_container( int_container(), string_container() );

    #ifdef DEBUG
        printf("IndexerLot: Attributes for lot:\n");
    #endif

    for (; it.valid; it=set_next(it))
	{
	    char	*attribute = (char*)set_key(it).ptr;

	    map_insert(mapping, crc32boithonl(attribute, strlen(attribute)), attribute);

            #ifdef DEBUG
	        printf("%s\n", attribute);
	    #endif
       	}


    it = map_begin(mapping);

    for (; it.valid; it=map_next(it))
	{
	    char	attribute[MAX_ATTRIB_LEN];
	    memset(attribute, 0, MAX_ATTRIB_LEN);
	    strcpy(attribute, (char*)map_val(it).ptr);	// Lengden vil aldri overstige MAX_ATTRIB_LEN.
	    unsigned int crc32 = map_key(it).i;

	    fwrite(&crc32, sizeof(unsigned int), 1, fp);
	    fwrite(attribute, sizeof(char), MAX_ATTRIB_LEN, fp);
	    #ifdef DEBUG
  	        printf("%u -> %s\n", crc32, attribute);
	    #endif
       	}

    fclose(fp);

    destroy((*attriblot).words);
    free(attriblot);
}

// Legg til attributter til crc32-mappingen:
void attriblot_add(struct attriblotFormat *attriblot, char *attributes)
{
    char k[MAX_ATTRIB_LEN], v[MAX_ATTRIB_LEN], kv[MAX_ATTRIB_LEN];
    char *o = NULL;

    while (next_attribute(attributes, &o, k, v, kv))
        {
//	    attribElementNormalize(k);
//	    attribElementNormalize(v);
//	    attribElementNormalize(kv);
	    set_insert(attriblot->words, k);
	    set_insert(attriblot->words, v);
	    set_insert(attriblot->words, kv);
	}
}

// Legg til attributter i egen invertert index:
void iiattribadd(struct IndexerRes_attrib *iiattrib, char *attributes)
{
    char k[MAX_ATTRIB_LEN], v[MAX_ATTRIB_LEN], kv[MAX_ATTRIB_LEN];
    char *o = NULL;

    while (next_attribute(attributes, &o, k, v, kv))
        {
	    if (k[0] != '\0')
		{
//		    attribElementNormalize(k);
//		    attribElementNormalize(v);
//		    attribElementNormalize(kv);
		    attribadd(iiattrib, kv);
		    attribadd(iiattrib, k);
		    attribadd(iiattrib, v);
		}
	}
}

// Legg til attributter per dokument i en egen forward index:
void attributeIndexAdd(struct relocal re, unsigned int DocID, char *attributes)
{
    if (re.attributeIndex==NULL) return;

    #ifdef DEBUG
	    fprintf(stderr, "IndexerLot: attributeIndexAdd( DocID=%i, attributes=%s )\n", DocID, attributes);
    #endif

    char k[MAX_ATTRIB_LEN], v[MAX_ATTRIB_LEN], kv[MAX_ATTRIB_LEN];
    char *o = NULL;
    container	*M = map_container( string_container(), string_container() );

    while (next_attribute(attributes, &o, k, v, kv))
        {
		#ifdef DEBUG
			printf("k=\"%s\", v=\"%s\", kv=\"%s\"\n", k, v, kv);
		#endif
	    map_insert(M, k, v);
	}

    unsigned int *val = reget(re.attributeIndex, DocID);
    iterator	it_s1 = set_begin(re.attributeKeys),
		it_m1 = map_begin(M);

    for (; it_s1.valid; it_s1=set_next(it_s1))
	{
	    if (it_m1.valid && !strcmp((const char*)map_key(it_m1).ptr, (const char*)set_key(it_s1).ptr))
		{
		    *val = crc32boithonl((char*)map_val(it_m1).ptr, strlen((char*)map_val(it_m1).ptr));
		    it_m1 = map_next(it_m1);
		}
	    else
		{
		    *val = 0;
		}

	    val++;
	}
}

#endif // ATTRIBUTES

#endif

void SummaryWrite(Bytef *WorkBuffer,uLong WorkBufferSize, unsigned int *SummaryPointer,
		unsigned short *SummarySize,const unsigned int DocID,FILE *SFH) {

	(*SummaryPointer) = ftell(SFH);
        (*SummarySize) = (sizeof(DocID) + WorkBufferSize);

	//write it to disk
        fwrite(&DocID,sizeof(DocID),1,SFH);
        fwrite(WorkBuffer,WorkBufferSize,sizeof(char),SFH);

}

void makePreParsedSummary(const char body[], int bodylen,const  char title[],int titlelen,const char metadesc[], 
	int metadesclen, Bytef **WorkBuffer,uLong *WorkBufferSize	
) {


	int n;
	char *SummeryBuf;
	int SummeryBufLen;

	//Bytef *WorkBuffer;
	//uLong WorkBufferSize;


	SummeryBuf = malloc(bodylen + titlelen + metadesclen +10);

	SummeryBufLen = sprintf(SummeryBuf,"%s\n%s\n%s",title,metadesc,body);

	(*WorkBufferSize) = (ceil(SummeryBufLen * 1.001) +12);
	(*WorkBuffer) = malloc((*WorkBufferSize));

        if ( (n = compress((*WorkBuffer),WorkBufferSize,(Bytef *)SummeryBuf,SummeryBufLen)) != 0) {
		printf("compress error. Code: %u. WorkBufferSize %u, SummeryBufLen %i\n",n,(unsigned int)WorkBufferSize,(unsigned int)SummeryBufLen);
        }
        else {




        }

	//free(WorkBuffer);
	free(SummeryBuf);


}


int getNextPage(struct IndexerLot_workthreadFormat *argstruct,char htmlcompressdbuffer[],int htmlcompressdbuffer_size, 
	char imagebuffer[],int imagebuffer_size,unsigned long int *radress, char **acl_allow, char **acl_denied,struct ReposetoryHeaderFormat *ReposetoryHeader, char **url, char **attributes) {
	//lock
	int forreturn;
	//må holde status om rGetNext() har sakt at dette er siste. Hvis ikke hamrer vi bortenfor eof
	static int lastpage = 0;
	static int lastLot = 0;

	#ifdef WITH_THREAD
		pthread_mutex_lock(&(*argstruct).reposetorymutex);
	#endif

	//hvis vi har byttet lot må vi prøve igjen
	if(lastLot != (*argstruct).lotNr) {
		lastpage = 0;
	}

	if(((*argstruct).optMaxDocuments) != 0 && ((*argstruct).optMaxDocuments <= (*argstruct).pageCount)) {
		//printf("Exeting after only %i docs\n",(*argstruct).pageCount);
		forreturn = 0;
	}
	else if (lastpage) {
		 forreturn = 0;
	}
	else if (rGetNext_fh((*argstruct).lotNr,ReposetoryHeader,htmlcompressdbuffer,htmlcompressdbuffer_size,
			imagebuffer,radress,(*argstruct).FiltetTime,(*argstruct).FileOffset,(*argstruct).subname,acl_allow,acl_denied,argstruct->FNREPO, url, attributes)) {


		++(*argstruct).pageCount;

		if (((*argstruct).pageCount % 10000) == 0) {
                	printf("%i\n",(*argstruct).pageCount);
		}

		forreturn = 1;
	}
	else {
		printf("rGetNext er ferdig\n");
		lastpage = 1;
		forreturn = 0;
	}

	lastLot = (*argstruct).lotNr;
	
	#ifdef WITH_THREAD
		pthread_mutex_unlock(&(*argstruct).reposetorymutex);
	#endif

	return forreturn;
}

int IndexerLot_filterOnlyTLD (char **optOnlyTLD,char TLD[]) {

	int i;

	if (optOnlyTLD == NULL) {
		return 0;
	}

	i = 0;
	while( (optOnlyTLD[i] != NULL) ) {
		if (strcmp(optOnlyTLD[i],TLD) == 0) {
			return 0;
		}
		++i;
	}

	return 1;
}

typedef struct {
	int hits;
	set acl_allow;
	set acl_denied;
} dictcontent_t;


void
savewords(struct pagewordsFormat *pagewords, 
#ifdef WITH_THREAD
pthread_mutex_t *wordsmutex,
#endif
 struct hashtable *words, struct hashtable *acls,
    char *acl_allow, char *acl_denied)
{
	struct pagewordsFormatPartFormat *wordsPart;
	int i;
	char *filesKey;

	wordsPart = &pagewords->normalWords;

#ifdef WITH_THREAD
	pthread_mutex_lock(wordsmutex);
#endif

	for (i = 0; i < wordsPart->revIndexnr; i++) {
		dictcontent_t *dc;
		int j;

		// Don't save words that are only digits.
		for (j = 0; isdigit(wordsPart->revIndex[i].word[j]) || wordsPart->revIndex[i].word[j] == '\0'; j++) {
			if (wordsPart->revIndex[i].word[j] == '\0') {
				goto skipword;
			}
		}

		if ((dc = hashtable_search(words, wordsPart->revIndex[i].word)) == NULL) {
			filesKey = strdup(wordsPart->revIndex[i].word);
			dc = malloc(sizeof(*dc));
			dc->hits = wordsPart->revIndex[i].wordnr;
			set_init(&dc->acl_allow);
			set_init(&dc->acl_denied);
			add_acls(acl_allow, &dc->acl_allow, acls);
			add_acls(acl_denied, &dc->acl_denied, acls);

			if (!hashtable_insert(words, filesKey, dc)) {
				printf("cant insert\n");
				exit(-1);
			}

		}
		else {
			add_acls(acl_allow, &dc->acl_allow, acls);
			add_acls(acl_denied, &dc->acl_denied, acls);
			dc->hits += wordsPart->revIndex[i].wordnr;
		}
 skipword:
 		;
	}


#ifdef WITH_THREAD
	pthread_mutex_unlock(wordsmutex);
#endif
}


void *IndexerLot_workthread(void *arg) {

	struct IndexerLot_workthreadFormat *argstruct = (struct IndexerLot_workthreadFormat *)arg;

	int sizeofhtmlcompressdbuffer = 524288 * 2;

	char *htmlcompressdbuffer;
        if ((htmlcompressdbuffer = malloc(sizeofhtmlcompressdbuffer)) == NULL) {
		perror("malloc");
		exit(1);
	}

	
	int sizeofimagebuffer = 524288; //0.5 mb
	char *imagebuffer =  malloc(sizeofimagebuffer);
        //char imagebuffer[524288];  //0.5 mb
	unsigned int sizeofHtmlBuffer = 524288 * 2;
	char *HtmlBuffer = malloc(sizeofHtmlBuffer);

	struct pagewordsFormat *pagewords = malloc(sizeof(struct pagewordsFormat));

	int nerror;
	char TLD[5];
	unsigned long int radress;
	char *acl_allow = NULL;
	char *acl_denied = NULL;
	unsigned short int DomainDI;

	unsigned int HtmlBufferLength;
	unsigned char langnr;
	unsigned char awvalue;
	off_t DocIDPlace;
	int AdultWeight;
	//begrenser dette midlertidig. Ser ut til å skape segfeil
	//char domain[129];
	char domain[65];
	char *title;
        char *body;
	char *metadesc;
	char *url, *attributes;
	unsigned int crc32;
	int isnew, isrecrawled, isuntouched;

	Bytef *SummaryBuffer;
	uLong SummaryBufferSize;

	#ifdef BLACK_BOX
		int i;
	#endif

	struct ReposetoryHeaderFormat ReposetoryHeader;
	struct DocumentIndexFormat *DocumentIndexPost;


	wordsInit(pagewords);

	isnew = isrecrawled = isuntouched = 0;
	while (getNextPage(argstruct,htmlcompressdbuffer,sizeofhtmlcompressdbuffer,imagebuffer,sizeofimagebuffer,
		&radress,&acl_allow,&acl_denied,&ReposetoryHeader, &url, &attributes)) {

				int documentUpdate = 0;

       				awvalue = 0;
               			title = NULL;
		                body = NULL;
				metadesc = NULL;
				DocumentIndexPost = NULL;
				SummaryBuffer = NULL;

				DocIDPlace = (ReposetoryHeader.DocID - LotDocIDOfset((*argstruct).lotNr));

				if ((DocumentIndexPost = malloc(sizeof(struct DocumentIndexFormat))) == NULL) {
					perror("malloc DocumentIndexPost");
					exit(1);
				}

				memset(DocumentIndexPost,0, sizeof(struct DocumentIndexFormat));

				if( (argstruct->optHandleOld) && ((*argstruct).DIArray[DocIDPlace].oldp != NULL) ) {
					memcpy(DocumentIndexPost,(*argstruct).DIArray[DocIDPlace].oldp,sizeof(struct DocumentIndexFormat));
				}
				else if (argstruct->optQuery != NULL) {

					// for query indexing
					if (!DIReadNET(argstruct->optNetLot,DocumentIndexPost,ReposetoryHeader.DocID,argstruct->subname)) {
						printf("can't DIReadNET\n");
						goto pageDone;
					}
					if (strcmp(DocumentIndexPost->Url,ReposetoryHeader.url) != 0) {
						printf("Error: urls is not the same: did DIReadNET DocID %u for url \"%s\" == \"%s\" ?\n",ReposetoryHeader.DocID,DocumentIndexPost->Url,ReposetoryHeader.url);
						goto pageDone;
					}
					printf("did DIReadNET DocID %u for url \"%s\" == \"%s\" ?\n",ReposetoryHeader.DocID,DocumentIndexPost->Url,ReposetoryHeader.url);				


				}

				//hvis dette er samme dokumens som vi har fra før kan vi ignorere det.
				//runarb: 28 aug 2008. Legger til en sjekk på at dokumentet ikke er 0 bytes stort. Vi
				//kan ikke ha indeksert det hvis vi ikke har størelsen, men RepositoryPointer == radress == 0 er lov, og skjer for første dokument
				if ( (argstruct->rEindex == 0) && (DocumentIndexPost->RepositoryPointer == radress) && (DocumentIndexPost->htmlSize2 != 0)) {
					#ifdef DEBUG
						printf("have already indexed this dokument\n");
					#endif
					isuntouched++;
					++(*argstruct).optHandleOld_allrediIndexed;
					goto pageDone;
				} else if ((DocumentIndexPost->CrawleDato!=0) && (DocumentIndexPost->CrawleDato > ReposetoryHeader.time)) {
					#ifdef DEBUG
						printf("Dokument is older then indexed. Time DocID %lu > Time this %lu\n",DocumentIndexPost->CrawleDato, ReposetoryHeader.time);
					#endif
					isuntouched++;
					++(*argstruct).optHandleOld_thencur;
					goto pageDone;
				} else if (DocumentIndexPost->RepositoryPointer != radress && radress != 0) {
					documentUpdate = 1;
				}

				HtmlBufferLength = sizeofHtmlBuffer;
				if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)&HtmlBufferLength,(Bytef*)htmlcompressdbuffer,ReposetoryHeader.htmlSize2)) != 0) {
					#ifdef DEBUG
               				printf("uncompress error. Code: %i for DocID %u-%i. ReposetoryHeader.htmlSize2 %i,sizeofHtmlBuffer %i\n",nerror,ReposetoryHeader.DocID,rLotForDOCid(ReposetoryHeader.DocID),ReposetoryHeader.htmlSize2,sizeofHtmlBuffer);
					#endif
               				goto pageDone;
		                }

				//printf("HtmlBuffer:\n##############\n%s\n#####################\n",HtmlBuffer);

				crc32 = crc32boithonl(HtmlBuffer,HtmlBufferLength);

				if((*argstruct).optHandleOld) {


					        if ( (argstruct->rEindex == 0) && (DocumentIndexPost->crc32 == crc32) ) {

							#ifdef DEBUG
								printf("have a duplicate dokument, but havent changed\n");
							#endif
							++(*argstruct).optHandleOld_duplicateNochange;
							isuntouched++;

							//free(DocumentIndexPost);
							goto pageDone;
						}
						else {
							++(*argstruct).optHandleOld_indexed;
						}


				}


				if ((*argstruct).optPrintInfo) {
					printf("url: %s, DocID %u, respons %i\n",ReposetoryHeader.url,ReposetoryHeader.DocID,ReposetoryHeader.response);
				}




				//begynner på en ny side
                                wordsReset(pagewords,ReposetoryHeader.DocID);



				//printf("D: %u, R: %lu\n",ReposetoryHeader.DocID, radress);


				#ifndef BLACK_BOX
				if (isIpBan(ReposetoryHeader.IPAddress)) {
					debug("ip adrsess %u is on ban list. Url \"%s\"\n",ReposetoryHeader.IPAddress,ReposetoryHeader.url);
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.isIpBan;
					goto pageDone;
				}
				else if (!find_domain_no_subname(ReposetoryHeader.url,domain,sizeof(domain)) ) {
					debug("can't find domain. Url \"%s\"\n",ReposetoryHeader.url);
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.find_domain_no_subname;
					goto pageDone;
				}
				else if (!find_TLD(domain,TLD,sizeof(TLD))) {
					printf("can't find TLD. Url \"%s\"\n",ReposetoryHeader.url);
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.find_TLD;
					goto pageDone;
				}
				else if (IndexerLot_filterOnlyTLD((*argstruct).optOnlyTLD,TLD)) {
					debug("Filter: optOnlyTLD ekskludes \"%s\"\n",TLD);
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.IndexerLot_filterOnlyTLD;
					goto pageDone;
				}
				else if (filterDomainNrOfLines(domain)) {
					debug("To many lines in domaine. Domain \"%s\"\n",domain);
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.filterDomainNrOfLines;
					goto pageDone;
				}
				else if (filterDomainLength(domain)) {
					debug("To long domaine. Domain \"%s\"\n",domain);
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.filterDomainLength;
					goto pageDone;
				}
				else if (filterTLDs(domain)) {
					debug("bannet TLD. Domain \"%s\"\n",domain);
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.filterTLDs;
					goto pageDone;

				}
				#else
				if(0) {

				}
				#endif
				else if ((ReposetoryHeader.response >= 200) && (ReposetoryHeader.response <= 299)) {	


                                	(*pagewords).curentDocID = ReposetoryHeader.DocID;
                                	if (strchr(ReposetoryHeader.url,'?') == 0) {
                                        	(*pagewords).curentUrlIsDynamic = 0;
                                	}
                        	        else {
                	                        (*pagewords).curentUrlIsDynamic = 1;
        	                        }
	

					handelPage(pagewords,(*argstruct).lotNr,&ReposetoryHeader,HtmlBuffer,HtmlBufferLength,ReposetoryHeader.DocID,(*argstruct).httpResponsCodes,(*argstruct).adult,&title,&body);





					wordsMakeRevIndex(pagewords,(*argstruct).adult,&AdultWeight,&langnr);

					sprintf((*DocumentIndexPost).Sprok,"%i",langnr);


					if (AdultWeight > 255) {
                                        	(*DocumentIndexPost).AdultWeight = 255;
                                        }
                                       	else {
                                        	(*DocumentIndexPost).AdultWeight = AdultWeight;
                                        }



					//setter anatll utgående linker
					//bruker en unsigned char. Kan ikke ha flere en 255 
					if ((*pagewords).nrOfOutLinks > 255) {
						(*DocumentIndexPost).nrOfOutLinks = 255;
					}
					else {
						(*DocumentIndexPost).nrOfOutLinks = (*pagewords).nrOfOutLinks;
					}


					//har ikke metadesc enda
					metadesc = strdup("");

					#ifndef BLACK_BOX

					makePreParsedSummary(body,strlen(body),title,strlen(title),metadesc,strlen(metadesc),
						&SummaryBuffer,&SummaryBufferSize);
					#endif


					wordsMakeRevIndexBucket(pagewords,ReposetoryHeader.DocID,&langnr);

					

               				if ((*DocumentIndexPost).AdultWeight >= AdultWeightForXXX) {
                       				//printf("DocID: %u, %hu, url: %s\n",DocID,DocumentIndexPost.AdultWeight,DocumentIndexPost.Url);
                       				//mark as adult
                       				awvalue = 1;
               				}
               				else {
                       				//not adult
                       				awvalue = 0;
               				}
				}
				else if (argstruct->optQuery != NULL) {
					//ignorerer bare ikke 200 sider 
					goto pageDone;
				}
				else {
					//ikke 200->299 side
					//egentlig ukjent
					awvalue = 0;
					DIS_delete(DocumentIndexPost);
					++argstruct->filtered.notHttp200;

					goto pageDone;

				}


				if (argstruct->optQuery != NULL) {
					if (DocumentIndexPost->crc32 != crc32) {
						printf("error: crc32 is not the same\n");
					}
					//vi kan nå trykt gå til neste. Vi har hentet det vi vil, som er AdultValue
					(*argstruct).DIArray[DocIDPlace].p = DocumentIndexPost;
					(*argstruct).DIArray[DocIDPlace].haveawvalue = 1;
					(*argstruct).DIArray[DocIDPlace].DocID = ReposetoryHeader.DocID;
					printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
					goto pageDone;
				}

					//data skal kopieres over uanset hva som skjer
					//kopierer over di data
					copyRepToDi(DocumentIndexPost,&ReposetoryHeader);

					(*DocumentIndexPost).RepositoryPointer = radress;


					#ifdef WITH_THREAD
						pthread_mutex_lock(&(*argstruct).restmutex);
					#endif

					#ifdef BLACK_BOX
						//handel acl
						//trenger dette acl greiene å være her, kan di ikke være lenger opp, der vi ikke har trå lås ?
						acllot_add((*argstruct).acllot,acl_allow);
						
						#ifdef IIACL
						iiacladd(&(*pagewords).acl_allow,acl_allow);
						iiacladd(&(*pagewords).acl_denied,acl_denied);

						aclsMakeRevIndex(&(*pagewords).acl_allow);
						aclsMakeRevIndex(&(*pagewords).acl_denied);

						aclsMakeRevIndexBucket (&(*pagewords).acl_allow,ReposetoryHeader.DocID,&langnr);
						aclsMakeRevIndexBucket (&(*pagewords).acl_denied,ReposetoryHeader.DocID,&langnr);
						#endif

						#ifdef ATTRIBUTES
							// attributes:
							if (attributes != NULL) {
								attriblot_add((*argstruct).attriblot, attributes);

								iiattribadd(&(*pagewords).attrib, attributes);

								attribMakeRevIndex(&(*pagewords).attrib);

								attribMakeRevIndexBucket(&(*pagewords).attrib, ReposetoryHeader.DocID, &langnr);

								attributeIndexAdd(argstruct->re, ReposetoryHeader.DocID, attributes);
							}
							// end attributes.
						#endif

						debug("time %u\n",ReposetoryHeader.time);

						iintegerSetValueNoCashe(&ReposetoryHeader.time,sizeof(int),ReposetoryHeader.DocID,"dates",(*argstruct).subname);
						//printf("filtypes \"%c%c%c%c\"\n",ReposetoryHeader.doctype[0],ReposetoryHeader.doctype[1],ReposetoryHeader.doctype[2],ReposetoryHeader.doctype[3]);
						//normaliserer
						for(i=0;i<4;i++) {
							ReposetoryHeader.doctype[i] = btolower(ReposetoryHeader.doctype[i]);
						}
						iintegerSetValueNoCashe(&ReposetoryHeader.doctype,4,ReposetoryHeader.DocID,"filtypes",(*argstruct).subname);

					#else

						DomainDI = calcDomainID(domain);

					#endif


                			if (ReposetoryHeader.response < nrOfHttpResponsCodes) {
                        			++(*argstruct).httpResponsCodes[ReposetoryHeader.response];
                			}



					//lager summery
					if ((body != NULL) && (title != NULL) && (metadesc != NULL)) {

						#ifndef BLACK_BOX

						SummaryWrite(SummaryBuffer,SummaryBufferSize,&(*DocumentIndexPost).SummaryPointer,
							&(*DocumentIndexPost).SummarySize,ReposetoryHeader.DocID,(*argstruct).SFH);

							linksWrite(pagewords,(*argstruct).addNewUrlha);

						#endif

						//printf("SummaryBufferSize %i\n",SummaryBufferSize);
						


						revindexFilesAppendWords(pagewords,(*argstruct).revindexFilesHa,ReposetoryHeader.DocID,&langnr);


						#ifdef BLACK_BOX
							#ifdef IIACL
							aclindexFilesAppendWords(&(*pagewords).acl_allow,(*argstruct).acl_allowindexFilesHa,ReposetoryHeader.DocID,&langnr);
							aclindexFilesAppendWords(&(*pagewords).acl_denied,(*argstruct).acl_deniedindexFilesHa,ReposetoryHeader.DocID,&langnr);
							#endif
							#ifdef ATTRIBUTES
//							attribindexFilesAppendWords(&(*pagewords).attrib,(*argstruct).attribindexFilesHa,ReposetoryHeader.DocID,&langnr);
							attribindexFilesAppendWords(&(*pagewords).attrib,(*argstruct).attribindexFilesHa);
							#endif
						#endif

						#ifdef PRESERVE_WORDS
							savewords(pagewords,
#ifdef WITH_THREAD
							&argstruct->wordsmutex,
#endif
							argstruct->words, argstruct->acls, acl_allow, acl_denied);
					
						#endif
						//DocIDPlace = ((ReposetoryHeader.DocID - LotDocIDOfset((*argstruct).lotNr)) * sizeof(unsigned char));
						////printf("DocID %u, DocIDPlace %i\n",ReposetoryHeader.DocID,DocIDPlace);
						//fseek((*argstruct).ADULTWEIGHTFH,DocIDPlace,SEEK_SET);
						//	
	                			//fwrite(&awvalue,sizeof(awvalue),1,(*argstruct).ADULTWEIGHTFH);


					}

					//skiver til DocumentIndex
					//DIWrite(DocumentIndexPost,ReposetoryHeader.DocID,(*argstruct).subname);


				/*
				Runarb: 20 mai 2011: Koden under var haket ut, og følgende kommentar var lagt til. Dessverre vet jeg ikke hva det betyr
				Vi ser ut til å bure gced under lasting av revindex også. Sikkert brukt ved sletting på web.
				*/
				//da dette er en oppdatering vil vi ha nye ord i den reverserte index, og kan trykt slette de som ligger i invertert index.
				if (documentUpdate) {
					#ifdef DEUG
						printf("DocId %u was updatated. Adding it to the gced file\n",ReposetoryHeader.DocID);
					#endif
					if (fwrite(&ReposetoryHeader.DocID,sizeof(unsigned int),1,argstruct->GCED) != 1) {
						perror("writing DocID to gced file");
					}
				}
				
		
				if (documentUpdate)
					isrecrawled++;
				else
					isnew++;

				#ifdef WITH_THREAD
					pthread_mutex_unlock(&(*argstruct).restmutex);
				#endif



				(*argstruct).DIArray[DocIDPlace].DocID = ReposetoryHeader.DocID;						

				DocumentIndexPost->crc32 = crc32;
				*RE_Uint(argstruct->re.crc32map, ReposetoryHeader.DocID) = crc32;
			
				(*argstruct).DIArray[DocIDPlace].p = DocumentIndexPost;
			

				(*argstruct).DIArray[DocIDPlace].haveawvalue = 1;
				(*argstruct).DIArray[DocIDPlace].awvalue = awvalue;						


				(*argstruct).DIArray[DocIDPlace].brankPageElements.IPAddress = 		DocumentIndexPost->IPAddress;
				(*argstruct).DIArray[DocIDPlace].brankPageElements.nrOfOutLinks = 	DocumentIndexPost->nrOfOutLinks;
				(*argstruct).DIArray[DocIDPlace].brankPageElements.response = 		DocumentIndexPost->response;
				(*argstruct).DIArray[DocIDPlace].haverankPageElements = 1;

				(*argstruct).DIArray[DocIDPlace].DomainDI = DomainDI;

pageDone:

				if (SummaryBuffer != NULL) {
					free(SummaryBuffer);
				}
				if (body != NULL) {
       					free(body);
				}
				if (title != NULL) {
   					free(title);
				}
				if (metadesc != NULL) {
					free(metadesc);
				}
				if (url != NULL)
					free(url);
				if (attributes != NULL)
					free(attributes);
				//hvis vi ikke har peket til DocumentIndexPost, men har allokert den, kan vi slette den
				if ((argstruct->DIArray[DocIDPlace].p == NULL) && (DocumentIndexPost != NULL)) {
					free(DocumentIndexPost);
				}

		}

#ifdef WITH_THREAD
				pthread_mutex_lock(&argstruct->countmutex);
#endif
				argstruct->n_new += isnew;
				argstruct->n_recrawled += isrecrawled;
				argstruct->n_untouched += isuntouched;
#ifdef WITH_THREAD
				pthread_mutex_unlock(&argstruct->countmutex);
#endif


	wordsEnd(pagewords);

	free(htmlcompressdbuffer);
	free(imagebuffer);
	free(HtmlBuffer);
	free(pagewords);

	return NULL;

}
void netlot_end_recursiveDir (char lotpath[],char lotinternpath[],unsigned int lotNr,char subname[], char server[]) {

	DIR *DIRH;
	struct dirent *dp;
	char nextdirname[512];
	char filname[512];
	char dirname[512];
	char lotinternfilname[512];

	sprintf(dirname,"%s%s",lotpath,lotinternpath);

	if ((DIRH = opendir(dirname)) == NULL) {
		perror(dirname);
	}	

	while ((dp = readdir(DIRH)) != NULL) {

		if ((dp->d_type == DT_DIR) && ((strcmp(dp->d_name,".") == 0) || (strcmp(dp->d_name,"..") == 0))) {
			#ifdef DEBUG
			printf(". file/folder/link\n");
			#endif
		}
		else if (strncmp(dp->d_name,"Brank",5) == 0) {
			printf("ignoring file \"%s\"\n",dp->d_name);
		}
		else if (dp->d_type == DT_DIR) {
			//if (lotinternpath[0] == '\0') {
				sprintf(nextdirname,"%s%s/",lotinternpath,dp->d_name);
			//}
			//else {
			//	strcpy(nextdirname,dp->d_name);
			//}
			printf("dir (nextdirname %s)\n",nextdirname);
			rmkdir(nextdirname,lotNr,subname);

			//kaller seg selv rekurift
			netlot_end_recursiveDir(lotpath,nextdirname,lotNr,subname, server);
		}
		else if (dp->d_type == DT_REG) {
			
			sprintf(filname,"%s%s",dirname,dp->d_name);
			sprintf(lotinternfilname,"%s%s",lotinternpath,dp->d_name);

			#ifdef DEBUG
			printf("file %s, %u %s\n",filname,lotNr,lotinternfilname);	
			#endif

			if (!rSendFileToHostname(filname,lotinternfilname,lotNr, "w",subname,server)) {
				printf("can't send file %s\n",filname);	
			}
			
		}
		else {
			printf("unknown type %i\n",dp->d_type);
		}
		//printf("%s %i\n",dp->d_name,dp->d_type);

	}

	closedir(DIRH);

}


/*******************************************************
rutine for å sende date tilbake til lagringserver.
*******************************************************/
void netlot_end (int lotNr,char subname[], char server[], struct optFormat *opt) {

	char lotpath[512];
	char reposetuoypath[512];
	FILE *FH;

	//åpretter en tom dyrty fil, slik at vi får en som overskriver den vi har fra før
	if ((FH = lotOpenFileNoCasheByLotNr(lotNr,"dirty","wb", 'e',subname)) == NULL) {
		perror("local reposetory");
		exit(1);
	}
	fclose(FH);


	GetFilPathForLot(lotpath,lotNr,subname);

	printf("lotpath %s\n",lotpath);

/*

	//sletter reposetor. Vi skal ikke trenge og skrive tilbake det.
	//temp: vi kan ikke slette reposetory når vi også kjører gc
	// da må rApendPost lokke tilkoblingen under gc
	if (opt->RunGarbageCollection == 1) {
		printf("\n##############################\ntemp: remember to delete reposetory before this movs to production\n##############################\n\n");
	}

	sprintf(reposetuoypath,"%s%s",lotpath,"reposetory");
	printf("unlinking %s\n",reposetuoypath);

	if (unlink(reposetuoypath) != 0) {
		perror(reposetuoypath);
		exit(1);
	}
*/
	netlot_end_recursiveDir(lotpath,"",lotNr,subname, server);

	//sletter loten
	rrmdir(lotpath);
}

/*******************************************************
rutine for å hente en fil på lagringserver
*******************************************************/
int netlot_start_get_file(int lotNr,char subname[],char file[], char server[]) {

	FILE *FH;

	printf("netlotStart: Geting file \"%s\" for lot %i\n",file,lotNr);

	if ((FH = lotOpenFileNoCasheByLotNr(lotNr,file,"wb", 'e',subname)) == NULL) {
		perror("local file");
		exit(1);
	}

	if(!rGetFileByOpenHandlerFromHostName(file, FH, lotNr, subname, server)) {
		perror("file on server");
		fclose(FH);
		return 0;
	}

	fclose(FH);

	#ifdef DEBUG
	printf("netlotStart: done\n");
	#endif

	return 1;

}


/*******************************************************
rutine for å hente det vi trnger på lagringserver.
*******************************************************/
int netlot_start(int lotNr,char subname[], int optrEindex, char server[]) {

	
	int i;
	char filePath[512];

	if (!lotHasSufficientSpaceNetToHostname(lotNr, 4096,  subname, server)) {
		fprintf(stderr,"Lot %i has insufficient space. Skipping.\n",lotNr);
		return 0;
	}

	if (!netlot_start_get_file(lotNr,subname,"IndexTime",server)) {
		printf("can't get file \"IndexTime\"\n");
		return 0;
	}
	if (!netlot_start_get_file(lotNr,subname,"reposetory",server)) {
                printf("can't get file \"reposetory\"\n");
		return 0;
        }
	if (!netlot_start_get_file(lotNr,subname,"DocumentIndex",server)) {
                printf("can't get file \"DocumentIndex\"\n");
		return 0;
        }
	if (!netlot_start_get_file(lotNr,subname,"summary",server)) {
                printf("can't get file \"summary\"\n");
		return 0;
        }
	if (!netlot_start_get_file(lotNr,subname,"domainid",server)) {
                printf("can't get file \"domainid\"\n");
		return 0;
        }
	if (!netlot_start_get_file(lotNr,subname,"brankPageElements",server)) {
                printf("can't get file \"brankPageElements\"\n");
		return 0;
        }
	if (!netlot_start_get_file(lotNr,subname,"AdultWeight",server)) {
                printf("can't get file \"AdultWeight\"\n");
		return 0;
        }

	for (i=0;i<NEWURLFILES_NR;i++) {
		sprintf(filePath,"nyeurler.%i",i);
		if (!netlot_start_get_file(lotNr,subname,filePath,server)) {
               		printf("can't get file \"%s\"\n",filePath);
			return 0;
       		}
		
	}


	if (optrEindex == 0) {
		makeLotPath(lotNr,"iindex/Main/index/aa/",subname);

		for(i=0;i<=63;i++) {
			sprintf(filePath,"iindex/Main/index/aa/%i.txt",i);
			if (!netlot_start_get_file(lotNr,subname,filePath,server)) {
                		printf("can't get file \"%s\"\n",filePath);
				return 0;
        		}
		
		}
	}

	return 1;

}


void run(int lotNr, char subname[], struct optFormat *opt, char reponame[]) {

	struct IndexerLot_workthreadFormat *argstruct;

	int i,n;
	int lotPart;
	int flags;
        unsigned int FiltetTime;
        unsigned int FileOffset;
	//char lotServer[64];
	struct iintegerFormat iinteger;
	off_t DocIDPlace;	
	unsigned int lastIndexTime;
	char openmode[4];
	FILE *brankPageElementsFH;
	FILE *INDEXLOCK;

	struct IndekserOptFormat IndekserOpt;

	if ((argstruct = malloc(sizeof(struct IndexerLot_workthreadFormat))) == NULL) {
		perror("malloc argstruct");
		exit(1);
	}

	if ((argstruct->adult = malloc(sizeof(struct adultFormat))) == NULL) {
		perror("malloc argstruct.adult");
		exit(1);
	}


        langdetectInit();
	adultLoad(argstruct->adult);
	html_parser_init();


	printf("run: indexing lot %i %s\n",lotNr, subname);

	#ifndef BLACK_BOX		
		if (!ipbanLoad()) {
			printf("can't load ip ban list\n");
			exit(1);
		}
	#endif

	for(i=0;i<nrOfHttpResponsCodes;i++) {
		argstruct->httpResponsCodes[i] = 0;
	}

	//temp: må hente dette fra lot server eller fil
	FileOffset = 0;


	argstruct->pageCount = 0;



		//sjekker om vi har nokk plass lokalt
		if (!lotHasSufficientSpace(lotNr,4096,subname)) {
			printf("insufficient disk space\n");
			exit(1);
		}

		//netlot: Vi henter det vi trenger
		if ((opt->NetLot != NULL) && (opt->Query == NULL)) {
			if (!netlot_start(lotNr,subname,opt->rEindex,opt->NetLot)) {
				printf("can't netLot for lot nr %i!\n",lotNr);
				return;
			}
		}


		//vi tilater bare at en og en jobber med filen samtidig.
        	if ( (INDEXLOCK = lotOpenFileNoCasheByLotNr(lotNr,"index.lock","wb", 'e',subname)) == NULL) {
			printf("can't loct tje index lock\n");
			exit(-1);
		}
		flock(fileno(INDEXLOCK),LOCK_EX);

		//finner siste indekseringstid
		lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);
		

		printf("lastIndexTime %s",ctime((time_t *)&lastIndexTime));

		if (opt->rEindex == 1) {
			FiltetTime = 0;
			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (lastIndexTime == 0) {
			FiltetTime = 0;

			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (opt->MustBeNewerThen != 0) {
			//regner ut hvor ny den må være
			printf("Isnet new. Last indexed %s",ctime((time_t *)&lastIndexTime));
			exit(1);
		}		
		else if(lastIndexTime != 0) {
			printf("lastIndexTime is not 0, but %i\n",lastIndexTime);
			//23 jul 2008: Vi bruker ikke dette langere. Det skaper problemer for gc.
			// i steden sjekker vi om crc32 og pekere er det samme. Sikkert litt mer overhead, men da fungerer gc :)
			//FiltetTime = lastIndexTime;
			FiltetTime = 0;

			//opner for appending
			strcpy(openmode,"ab");
		}
		else {
			printf("This should not happen!\n");
			exit(1);
		}
		printf("openmode\"%s\"\n",openmode);

		//støtter ikke å apande revindexer langere. Det betyr at vi må lage ny iindex hver gang, hvis ikke blir
		//det overskreved.
		//revindexFilesOpenLocal(argstruct->revindexFilesHa,lotNr,"Main",openmode,subname);
		revindexFilesOpenLocal(argstruct->revindexFilesHa,lotNr,"Main","wb",subname);

		#ifdef BLACK_BOX		

			#ifdef IIACL
				//støtter ikke å apande revindexer langere. Det betyr at vi må lage ny iindex hver gang, hvis ikke blir
				//det overskreved.
				//revindexFilesOpenLocal(argstruct->acl_allowindexFilesHa,lotNr,"acl_allow",openmode,subname);
				//revindexFilesOpenLocal(argstruct->acl_deniedindexFilesHa,lotNr,"acl_denied",openmode,subname);
				revindexFilesOpenLocal(argstruct->acl_allowindexFilesHa,lotNr,"acl_allow","wb",subname);
				revindexFilesOpenLocal(argstruct->acl_deniedindexFilesHa,lotNr,"acl_denied","wb",subname);
			#endif

			acllot_init(&argstruct->acllot,subname,openmode,lotNr);

			#ifdef ATTRIBUTES
			// Åpne reversert index:
			revindexFilesOpenLocal(argstruct->attribindexFilesHa,lotNr,"attributes","wb",subname);
			attriblot_init(&argstruct->attriblot,subname,openmode);

			// Initialiser og åpne vanlig index:

			char		*attrkeys_filename, *attrcols_filename;
			container	*new_keys = set_container( string_container() ),
					*old_keys = set_container( string_container() );

			asprintf(&attrkeys_filename, "%s.new_attribute_keys", reponame);
			FILE	*f_attrkeys = lotOpenFileNoCasheByLotNr(1, attrkeys_filename, "r", 's', subname);

			// Sjekk for nye attributter:
			if (f_attrkeys!=NULL)
			    {
				char		key[MAX_ATTRIB_LEN];

				while (fgets(key, MAX_ATTRIB_LEN, f_attrkeys) != NULL)
				    {
					key[strlen(key)-1] = '\0';
					set_insert(new_keys, key);
				    }

				fclose(f_attrkeys);

				char		attrkeys_fullpath[PATH_MAX];
				GetFilPathForLotFile(attrkeys_fullpath, attrkeys_filename, 1, subname);
				unlink(attrkeys_fullpath);	// Slett fila etter den er lest.
			    }

			asprintf(&attrcols_filename, "%s.attribute_columns", reponame);
			FILE	*f_attrcols = lotOpenFileNoCasheByLotNr(1, attrcols_filename, "r", 's', subname);

			// Sjekk for gamle attributter:
			if (f_attrcols!=NULL)
			    {
				char		key[MAX_ATTRIB_LEN];

				while (fgets(key, MAX_ATTRIB_LEN, f_attrcols) != NULL)
				    {
					key[strlen(key)-1] = '\0';
					set_insert(old_keys, key);
				    }

				fclose(f_attrcols);
			    }


			if (set_size(new_keys)>0)
			    {
				if (set_size(old_keys)>0)
				    {
					iterator	it_s2 = set_begin(old_keys);

					for (; it_s2.valid; it_s2=set_next(it_s2))
					    {
						set_insert(new_keys, (char*)set_key(it_s2).ptr);
					    }

					if (set_size(new_keys) > set_size(old_keys))
					    {
						// Merge gamle og nye attributter i index:
						fprintf(stderr, "**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n");
						fprintf(stderr, "Utvider attributt-indeksen...\n");
						fprintf(stderr, "*ADVARSEL* utestet!\n");
						struct reformat *oldIndex = reopen(lotNr, sizeof(unsigned int)*set_size(old_keys), "attributeIndex", subname, 0);
						struct reformat	*newIndex = reopen(lotNr, sizeof(unsigned int)*set_size(new_keys), "attributeIndex.tmp", subname, 0);

						for (i=0; i<oldIndex->maxsize; i++)
						    {
						        unsigned int *oval = reget(oldIndex, i),
								    *nval = reget(newIndex, i);
						        iterator	it_sn = set_begin(new_keys),
									it_so = set_begin(old_keys);

						        for (; it_sn.valid; it_sn=set_next(it_sn))
							    {
								if (it_so.valid && !strcmp((const char*)set_key(it_so).ptr, (const char*)set_key(it_sn).ptr))
								    {
									*nval = *oval;
									oval++;
									it_so = set_next(it_so);
								    }
								else
								    {
									*nval = 0;
								    }

								nval++;
							    }
						    }

						char	indx_fullpath[PATH_MAX], tmpindx_fullpath[PATH_MAX];
						GetFilPathForLotFile(indx_fullpath, "attributeIndex", lotNr, subname);
						GetFilPathForLotFile(tmpindx_fullpath, "attributeIndex.tmp", lotNr, subname);
						fprintf(stderr, "Deleting '%s'...\n", indx_fullpath);
						unlink(indx_fullpath);
						fprintf(stderr, "Inserting '%s' instead.\n", tmpindx_fullpath);
						rename(tmpindx_fullpath, indx_fullpath);
						fprintf(stderr, "...ferdig\n");
						fprintf(stderr, "**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n**\n");
					    }
				    }

				destroy(old_keys);
			    }
			else	// Ingen nye attributter:
			    {
				destroy(new_keys);
				new_keys = old_keys;
			    }


			if (set_size(new_keys) == 0)
			    {
				fprintf(stderr, "IndexerLot.run: No attributes\n");
				argstruct->re.attributeIndex = NULL;
			    }
			else if ((argstruct->re.attributeIndex = reopen(lotNr, sizeof(unsigned int)*set_size(new_keys), "attributeIndex", subname, 0)) != NULL)
			    {
				// Skriv attributter til fil, samt lagre rekkefølgen i minnet.
				argstruct->re.attributeKeys = new_keys;
				new_keys = NULL;

				f_attrcols = lotOpenFileNoCasheByLotNr(1, attrcols_filename, "w", 'e', subname);
				iterator	it_s1 = set_begin(argstruct->re.attributeKeys);

				for (i=0; it_s1.valid; it_s1=set_next(it_s1),i++)
				    {
					fprintf(f_attrcols, "%s\n", (char*)set_key(it_s1).ptr);
				    }

				fclose(f_attrcols);
			    }
			else
			    {
				err(1, "Can't reopen(attributeIndex)");
			    }

			destroy(new_keys);
			free(attrkeys_filename);
			free(attrcols_filename);

			// @@TODO: Fjerning (og detektering) av tomme kolonner. (Garbage collection)

			#endif	// ATTRIBUTES
		#else


			if ((argstruct->ADULTWEIGHTFH = lotOpenFileNoCasheByLotNr(lotNr,"AdultWeight",">>", 'e',subname)) == NULL) {
				perror("open AdultWeight");
				exit(1);
			}

			if (opt->Query == NULL) {
				if ((argstruct->SFH = lotOpenFileNoCasheByLotNr(lotNr,"summary",openmode,'r',subname)) == NULL) {
					perror("open summary");
					exit(1);
				}
			}


			for (i=0;i<NEWURLFILES_NR;i++) {
				addNewUrlOpen(&argstruct->addNewUrlha[i],lotNr,openmode,subname,i);
			}	

			if (!iintegerOpenForLot(&iinteger,"domainid",sizeof(unsigned short),lotNr, ">>", subname)) {
                		perror("iintegerOpenForLot");
        	        	exit(1);
	        	}		

		#endif


		//main work
		argstruct->lotNr 		= lotNr;
		argstruct->subname 		= subname;
        	argstruct->FiltetTime 		= FiltetTime;
        	argstruct->FileOffset 		= FileOffset;
		argstruct->optMaxDocuments	= opt->MaxDocuments;
		argstruct->optPrintInfo		= opt->PrintInfo;
		argstruct->optOnlyTLD		= opt->OnlyTLD;
		argstruct->optMakeWordList 	= opt->MakeWordList;
		argstruct->optHandleOld		= opt->HandleOld;
		argstruct->optQuery		= opt->Query;
		argstruct->optNetLot		= opt->NetLot;
		argstruct->rEindex		= opt->rEindex;
		argstruct->reponame		= reponame;


                argstruct->filtered.isIpBan 			= 0;
                argstruct->filtered.find_domain_no_subname 	= 0;
                argstruct->filtered.find_TLD 			= 0;
                argstruct->filtered.IndexerLot_filterOnlyTLD 	= 0;
                argstruct->filtered.filterDomainNrOfLines 	= 0;
                argstruct->filtered.filterDomainLength 		= 0;
                argstruct->filtered.filterTLDs 			= 0;
                argstruct->filtered.notHttp200 			= 0;

		argstruct->n_new				= 0;
		argstruct->n_recrawled				= 0;
		argstruct->n_untouched				= 0;

		#ifdef BLACK_BOX
			flags = RE_HAVE_4_BYTES_VERSION_PREFIX;
		#else
			flags = 0;
		#endif
        	if((argstruct->re.DocumentIndexFormat = reopen(lotNr, sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, flags )) == NULL) {
        	        err(1, "can't reopen(DocumentIndex)");
	        }
        	if((argstruct->re.crc32map = reopen(lotNr, sizeof(unsigned int), "crc32map", subname, 0)) == NULL) {
        	        err(1, "can't reopen(crc32map)");
	        }


		//malloc
		if ((argstruct->DIArray = malloc( NrofDocIDsInLot * sizeof(struct DIArrayFormat) )) == NULL) {
			perror("malloc argstruct->DIArray");
			exit(1);
		}

		for(i=0;i<NrofDocIDsInLot;i++) {
			//printf("i %i\n",i);
			argstruct->DIArray[i].p = NULL;
			argstruct->DIArray[i].haveawvalue = 0;
			argstruct->DIArray[i].haverankPageElements = 0;
		}

		if (opt->HandleOld) {
			argstruct->optHandleOld_duplicateNochange	= 0;
			argstruct->optHandleOld_allrediIndexed		= 0;
			argstruct->optHandleOld_indexed			= 0;
			argstruct->optHandleOld_thencur			= 0;

			printf("will handel old dokuments in reposetory. Loading DocumentIndex...\n");
			for(i=0;i<NrofDocIDsInLot;i++) {
				struct DocumentIndexFormat DocumentIndexPost;

				if(!DIRead(&DocumentIndexPost,LotDocIDOfset(lotNr) + i,subname)) {
					argstruct->DIArray[i].oldp = NULL;
				}
				else {
					argstruct->DIArray[i].oldp = malloc(sizeof(struct DocumentIndexFormat));

					memcpy(argstruct->DIArray[i].oldp,&DocumentIndexPost,sizeof(struct DocumentIndexFormat));
				}
			}
			printf(".. done\n");
		}


        	if ( (argstruct->FNREPO = lotOpenFileNoCasheByLotNr(argstruct->lotNr,argstruct->reponame,"rb", 's',argstruct->subname)) == NULL) {
        	        #ifdef DEBUG
        	                printf("lot dont have a reposetory file\n");
        	        #endif

        	        return;
	        }

        	if ( (argstruct->GCED = lotOpenFileNoCasheByLotNr(argstruct->lotNr,"gced","ab", 'e',argstruct->subname)) == NULL) {
        	        #ifdef DEBUG
        	                printf("Can't open the gced file\n");
        	        #endif

        	        return;
	        }

#ifdef PRESERVE_WORDS
		argstruct->words = create_hashtable(1337, ht_stringhash, ht_stringcmp);
		argstruct->acls = create_hashtable(1337, ht_stringhash, ht_stringcmp);

		/* Partial indexing */
		if (strchr(openmode, 'a') != NULL) {
			FILE *FH;
			char line[2048];
			char *filesKey, word[1024];

			FH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords_raw","r",'r',subname);

			while(fgets(line, sizeof(line), FH) != NULL) {
				char acl_allow[DICT_ACL_LENGTH], acl_denied[DICT_ACL_LENGTH];
				dictcontent_t *dc;
				unsigned int nr;


				chomp(line);
				//printf("line \"%s\"\n",line);

				if (!dictionarywordLineSplit(line, word, &nr, acl_allow, acl_denied)) {
					puts("Error");
					continue;
				}

				//printf("word \"%s\", nr %u\n",word,nr);

				if ((dc = hashtable_search(argstruct->words, word)) == NULL) {
					filesKey = strdup(word);
					dc = malloc(sizeof(*dc));
					dc->hits = nr;
					set_init(&dc->acl_allow);
					set_init(&dc->acl_denied);
					add_acls(acl_allow, &dc->acl_allow, argstruct->acls);
					add_acls(acl_denied, &dc->acl_denied, argstruct->acls);

					if (!hashtable_insert(argstruct->words, filesKey, dc)) {
						printf("cant insert\n");
						exit(-1);
					}

				}
				else {
					add_acls(acl_allow, &dc->acl_allow, argstruct->acls);
					add_acls(acl_denied, &dc->acl_denied, argstruct->acls);
					dc->hits += nr;
				}
			}

			fclose(FH);

		}
		
#endif


		#ifdef WITH_THREAD

			//init mutex
			pthread_mutex_init(&argstruct->reposetorymutex, NULL);
			pthread_mutex_init(&argstruct->restmutex, NULL);
			pthread_mutex_init(&argstruct->countmutex, NULL);
			pthread_mutex_init(&argstruct->wordsmutex, NULL);

			if (opt->NrofWorkThreads == 0) {
				printf("won't use threads\n");
				IndexerLot_workthread(argstruct);
			}
			else {
				debug("wil use %i threads\n",opt->NrofWorkThreads);
				
				pthread_t *threadid;

				if ((threadid = malloc(sizeof(pthread_t) * opt->NrofWorkThreads)) == NULL) {
					perror("malloc threadid");
					exit(1);
				}

				//startet tråder
				for(i=0;i<opt->NrofWorkThreads;i++) {
					debug("starting tread nr %i\n",i);
					if ((n = pthread_create(&threadid[i], NULL, IndexerLot_workthread, argstruct)) != 0) {
						perror("pthread_create");
					}
				}

				//venter på trådene
        			for (i=0;i<opt->NrofWorkThreads;i++) {
                			if ((n = pthread_join(threadid[i], NULL)) != 0) {
						perror("pthread_join");
					}
        			}
				
				free(threadid);
			}
		#else
			printf("wasent build with threads\n");
			IndexerLot_workthread(argstruct);

		#endif

		fclose(argstruct->FNREPO);
		fclose(argstruct->GCED);

		revindexFilesCloseLocal(argstruct->revindexFilesHa); 

		#ifdef BLACK_BOX		
			revindexFilesCloseLocal(argstruct->acl_allowindexFilesHa); 
			revindexFilesCloseLocal(argstruct->acl_deniedindexFilesHa); 
			#ifdef ATTRIBUTES
			revindexFilesCloseLocal(argstruct->attribindexFilesHa); 
			#endif
		#endif


		if (opt->Query != NULL) {
			for(i=0;i<NrofDocIDsInLot;i++) {

				if (argstruct->DIArray[i].p != NULL) {
					printf("DocID %u, adult value %hu, url \"%s\"\n",argstruct->DIArray[i].DocID,argstruct->DIArray[i].p->AdultWeight,argstruct->DIArray[i].p->Url);
					DIWriteNET (opt->NetLot , argstruct->DIArray[i].p, argstruct->DIArray[i].DocID, argstruct->subname);

					free(argstruct->DIArray[i].p);

				}
			}
		}
		else {

			for(i=0;i<NrofDocIDsInLot;i++) {

				if (argstruct->DIArray[i].p != NULL) {
					
					DIWrite(argstruct->DIArray[i].p,argstruct->DIArray[i].DocID,argstruct->subname, NULL);
	
					free(argstruct->DIArray[i].p);
				}

				#ifndef BLACK_BOX

				if (argstruct->DIArray[i].haveawvalue == 1) {

					DocIDPlace = ((argstruct->DIArray[i].DocID - LotDocIDOfset(argstruct->lotNr)) * sizeof(unsigned char));

					//printf("DocID %u, DocIDPlace %i\n",argstruct->DIArray[i].DocID,DocIDPlace);
					if(fseek(argstruct->ADULTWEIGHTFH,DocIDPlace,SEEK_SET) != 0) {
						perror("fseek adultweight");
					}	
               				fwrite(&argstruct->DIArray[i].awvalue,sizeof(unsigned char),1,argstruct->ADULTWEIGHTFH);

					iintegerSetValue(&iinteger,&argstruct->DIArray[i].DomainDI,sizeof(argstruct->DIArray[i].DomainDI),argstruct->DIArray[i].DocID,subname);
				}


				#endif
			}
		
			closeDICache();
		}

		#ifndef BLACK_BOX

			if ((brankPageElementsFH = lotOpenFileNoCasheByLotNr(lotNr,"brankPageElements",">>",'r',subname)) == NULL) {
				perror("open brankPageElements");
				exit(1);
			}

			for(i=0;i<NrofDocIDsInLot;i++) {

				if (argstruct->DIArray[i].haverankPageElements == 1) {

					DocIDPlace = ((argstruct->DIArray[i].DocID - LotDocIDOfset(argstruct->lotNr)) * sizeof(struct brankPageElementsFormat));
					#ifdef DEBUG
						printf("DocID %u, lot %i, DocIDPlace %"PRId64", size %i, file %p\n",argstruct->DIArray[i].DocID,argstruct->lotNr,DocIDPlace,sizeof(struct brankPageElementsFormat),brankPageElementsFH);
						printf("IPAddress %u, nrOfOutLinks %d, response %hu\n",argstruct->DIArray[i].brankPageElements.IPAddress,argstruct->DIArray[i].brankPageElements.nrOfOutLinks,argstruct->DIArray[i].brankPageElements.response);
					#endif
					if (fseek(brankPageElementsFH,DocIDPlace,SEEK_SET) == -1) {
						perror("fseek brankPageElementsFH");
						exit(1);
					}
		
					if (fwrite(&argstruct->DIArray[i].brankPageElements,sizeof(struct brankPageElementsFormat),1,brankPageElementsFH) != 1) {
						perror("fwrite brankPageElementsFH");
						exit(1);
					}
				}
				
			}

			fclose(brankPageElementsFH);

		#endif

		#ifdef BLACK_BOX
			acllot_close(argstruct->acllot);
			#ifdef ATTRIBUTES
			attriblot_close(argstruct->attriblot);
			#endif
		#else
			fclose(argstruct->ADULTWEIGHTFH);
			if (opt->Query == NULL) {
				fclose(argstruct->SFH);
			}
			iintegerClose(&iinteger);


		#endif

		//skriver riktig indexs tid til lotten
		setLastIndexTimeForLot(lotNr,argstruct->httpResponsCodes,subname);

		#ifdef PRESERVE_WORDS

		argstruct->dictionarywordsfFH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords_raw", "w",'r',subname);

		// Loop over and write all words here...
		{
			FILE *resultFH = argstruct->dictionarywordsfFH;

			printf("Got words: %d\n", hashtable_count(argstruct->words));

			if (hashtable_count(argstruct->words) > 0) {
				struct hashtable_itr *itr;

				itr = hashtable_iterator(argstruct->words);
				do {
					char *p;
					int i;
					char *filesKey;
					dictcontent_t *dc;

					filesKey = hashtable_iterator_key(itr);
					dc = hashtable_iterator_value(itr);

					//printf("\"%s\": %i\n",filesKey,dc->hits);
					fprintf(resultFH,"%s %u ",filesKey,dc->hits);
					//printf("acl allow:\n");
					SET_FOREACH(i, &dc->acl_allow, p) {
						if (i > 0)
							fprintf(resultFH, ",");
						fprintf(resultFH, "%s", p);
						//printf("Got soemthing here: %s\n", p);
						//printf("\t%s\n", p);
					}
					fprintf(resultFH, " ");
					//printf("acl denied:\n");
					SET_FOREACH(i, &dc->acl_denied, p) {
						if (i > 0)
							fprintf(resultFH, ",");
						fprintf(resultFH, "%s", p);
						//printf("\t%s\n", p);
					}
					fprintf(resultFH, "\n");

				} while (hashtable_iterator_advance(itr) != 0);
				free(itr);
			}
		}

		fclose(argstruct->dictionarywordsfFH);
		#endif
		// vi må ikke kopiere revindex filene da vi jobber på de lokale direkte


		if (opt->HandleOld) {
			printf("duplicateNochange: %i\n",argstruct->optHandleOld_duplicateNochange);
			printf("allrediIndexed: %i\n",argstruct->optHandleOld_allrediIndexed);
			printf("optHandleOld_indexed: %i\n",argstruct->optHandleOld_indexed);
			printf("optHandleOld_thencur: %i\n",argstruct->optHandleOld_thencur);

			for(i=0;i<NrofDocIDsInLot;i++) {
				free(argstruct->DIArray[i].oldp);
			}
		}

		printf("filters:\n");
                printf("\tisIpBan: %i\n",argstruct->filtered.isIpBan);
                printf("\tfind_domain_no_subname: %i\n",argstruct->filtered.find_domain_no_subname);
                printf("\tfind_TLD: %i\n",argstruct->filtered.find_TLD);
                printf("\tIndexerLot_filterOnlyTLD: %i\n",argstruct->filtered.IndexerLot_filterOnlyTLD);
                printf("\tfilterDomainNrOfLines: %i\n",argstruct->filtered.filterDomainNrOfLines);
                printf("\tfilterDomainLength: %i\n",argstruct->filtered.filterDomainLength);
                printf("\tfilterTLDs: %i\n",argstruct->filtered.filterTLDs);
                printf("\tnotHttp200: %i\n",argstruct->filtered.notHttp200);


		printf("\nindexed %i pages\n\n\n",argstruct->pageCount);

		free(argstruct->DIArray);
		free(argstruct->adult);


		if (globalIndexerLotConfig.urlfilter != NULL) {
			FreeSplitList(globalIndexerLotConfig.urlfilter);
		}

		//run the Garbage Collection
		#ifdef BLACK_BOX
		if ((opt->RunGarbageCollection == 1) && (1)) {
		#else
		if ((opt->RunGarbageCollection == 1) && (argstruct->pageCount > 0)) {
		#endif
			printf("running repo Garbage Collection..\n");
				gcrepo(lotNr, subname);
			printf(".. done\n");

			printf("running summary Garbage Collection..\n");
			        gcsummary(lotNr, subname);
			printf(".. done\n");

			printf("running reduce Garbage Collection..\n");
				gc_reduce(argstruct->re.DocumentIndexFormat, lotNr, subname);
			printf(".. done\n");

		}

		if (opt->RunIndekser == 1) {

			printf("runing Indekser\n");

			IndekserOpt.optMustBeNewerThen = 0;
        		IndekserOpt.optAllowDuplicates = 0;
        		IndekserOpt.optValidDocIDs = NULL;
        		IndekserOpt.sequenceMode =1;
		        IndekserOpt.garbareCollection = 1;

                	for (lotPart=0;lotPart<64;lotPart++) {
                	        //printf("indexint part %i for lot %i of type Main\n",lotPart,lotNr);
                	        Indekser(lotNr,"Main",lotPart,subname,&IndekserOpt);
                	}
			#ifdef BLACK_BOX		
                	for (lotPart=0;lotPart<64;lotPart++) {
                	        //printf("indexint part %i for lot %i of type acl_allow\n",lotPart,lotNr);
                	        Indekser(lotNr,"acl_allow",lotPart,subname,&IndekserOpt);
                	}
                	for (lotPart=0;lotPart<64;lotPart++) {
                	        //printf("indexint part %i for lot %i of type acl_denied\n",lotPart,lotNr);
                	        Indekser(lotNr,"acl_denied",lotPart,subname,&IndekserOpt);
                	}

			#ifdef ATTRIBUTES
                	for (lotPart=0;lotPart<64;lotPart++) {
                	        //printf("indexint part %i for lot %i of type attributes\n",lotPart,lotNr);
                	        Indekser(lotNr,"attributes",lotPart,subname,&IndekserOpt);
                	}
			#endif

			#endif

                	//siden vi nå har lagt til alle andringer fra rev index kan vi nå slettet gced filen også
                	//Indekser_deleteGcedFile(lotNr, subname);
			lotDeleteFile("gced", lotNr, subname);
		}


		reclose(argstruct->re.DocumentIndexFormat);
		reclose(argstruct->re.crc32map);

		#ifdef ATTRIBUTES
		if (argstruct->re.attributeIndex!=NULL)
		    {
			reclose(argstruct->re.attributeIndex);
			destroy(argstruct->re.attributeKeys);
		    }
		#endif
	
		//netlot: Vi sender det vi har laget
		if ((opt->NetLot != NULL) && (opt->Query == NULL)) {
			//lokker eventuelt åpne filer, som etter gced.
			lotCloseFiles();
			
			netlot_end(lotNr,subname,opt->NetLot, opt);
		}

		#ifndef BLACK_BOX
		for (i=0;i<NEWURLFILES_NR;i++) {
			fclose(argstruct->addNewUrlha[i].NYEURLER);
		}	
		#endif


		{
			FILE *indexlog;

			if ((indexlog = lotOpenFileNoCasheByLotNr(lotNr,"indexlog.txt","a",'e',subname)) == NULL) {
				warn("Unable to write index log");
			} else {
				fprintf(indexlog, "%d new=%d,recrawled=%d,untouched=%d\n", time(NULL),
				    argstruct->n_new, argstruct->n_recrawled, argstruct->n_untouched);
				fclose(indexlog);
			}
		}
		printf("New: %d\nRecrawled: %d\nUntouched: %d\n", argstruct->n_new, argstruct->n_recrawled, argstruct->n_untouched);

		free(argstruct);

		html_parser_exit();
		langdetectDestroy();

		#ifndef BLACK_BOX		
			ipbanEnd();
		#endif


		fclose(INDEXLOCK);
}






int main (int argc, char *argv[]) {

        int lotNr;
	char *subname;
	struct optFormat opt;
	int i;

	opt.MustBeNewerThen = 0;
	opt.rEindex = 0;
	opt.MaxDocuments = 0;
	opt.PrintInfo = 0;
	opt.MakeWordList = 0;
	opt.RunGarbageCollection = 0;
	opt.NetLot = NULL;
	opt.RunIndekser = 0;
	opt.OnlyTLD = NULL;
	opt.NrofWorkThreads = 0;	
	opt.Query = NULL;
	opt.dirty = 10000;
	opt.LotMax = 0;

	#ifdef BLACK_BOX
		opt.HandleOld = 1;
	#else
		opt.HandleOld = 1;
	#endif

	#ifdef WITH_THREAD
		opt.NrofWorkThreads = DEFAULT_NROF_GENERATEPAGES_THREADS;	
	#endif

	globalIndexerLotConfig.collectUrls = 0;
	globalIndexerLotConfig.urlfilter = NULL;

	extern char *optarg;
       	extern int optind, opterr, optopt;
	char c;
	while ((c=getopt(argc,argv,"neu:t:m:pl:wogh:iq:d:z:"))!=-1) {
                switch (c) {
			case 'l':
				split(optarg, ",", &opt.OnlyTLD);
				printf("wil only index TLD's:\n");
				i = 0;
				while( (opt.OnlyTLD[i] != NULL) ) {
				    printf("\t%i\"%s\"\n", i, opt.OnlyTLD[i]);
					i++;
				}

				break;
			case 'p':
				opt.PrintInfo = 1;
				break;
			case 'i':
				opt.RunIndekser = 1;
				break;
			case 'g':
				opt.RunGarbageCollection = 1;
				break;
			case 'h':
				opt.NetLot = optarg;
				break;
			case 'w':
				//lag en ordbok
				opt.MakeWordList = 1;
				break;

                        case 'n':
      			        opt.MustBeNewerThen = 1;
                                break;
                        case 'o':
      			        opt.HandleOld = 1;
                                break;
                        case 'e':
                                opt.rEindex = 1;
                                break;
                        case 't':
                                opt.NrofWorkThreads = atoi(optarg);
                                break;
                        case 'd':
                                opt.dirty = atoi(optarg);
                                break;
                        case 'm':
                                opt.MaxDocuments = atoi(optarg);
                                break;
                        case 'z':
                                opt.LotMax = atoi(optarg);
                                break;
                        case 'u':
				printf("optopt \"%s\"\n",optarg);
				globalIndexerLotConfig.collectUrls = 1;
				if (strcmp(optarg,"-") == 0) {
					printf("Will collect all urls\n");

				}
				else {
					split(optarg,",",&globalIndexerLotConfig.urlfilter);
					printf("will only collect url of ending:\n");
					i=0;
					while( (globalIndexerLotConfig.urlfilter[i] != NULL) ) {
    						printf("\t\t%i\t\"%s\"\n", i, globalIndexerLotConfig.urlfilter[i]);
						i++;
					}

				}

				//exit(1);
                                break;
		        #ifdef BLACK_BOX

			#else
			case 'q':
				opt.Query = optarg;
				break;
			#endif

			default:
                                          exit(1);
                }
	}
	--optind;

	printf("argc %i, optind %i\n",argc,optind);


       	if ((argc - optind)!= 3) {
       	        printf("Dette programet indekserer en lot. Usage:\n\tIndexerLot lotNr subname\n");
       	        exit(0);
       	}

	lotNr = atoi(argv[1 +optind]);

	subname = argv[2 + optind];

	printf("subname %s, lotNr %i\n",subname,lotNr);

	if ((opt.Query != NULL) && (opt.NetLot != NULL)) {

		#ifndef BLACK_BOX

        	struct config_t maincfg;
		struct SiderFormat *Sider;
		int pageNr;
		size_t len;
		int len2;
		char *text;
		struct ReposetoryHeaderFormat ReposetoryHeader;
		//toDo: vi må hente bilde fra readHTMLNET()
		char image[] = "";
		int lastLot = 0;
		char dirtylots[maxLots];

		//setter opt som er nødvendige
		opt.rEindex = 1;
		//diread er ikke tråsikker
		opt.NrofWorkThreads = 1;

		len = 1048576 * 5; //5MB
		if ((text = malloc(len)) == NULL) {
                	perror("malloc");
                	exit(1);
        	}

        	maincfg = maincfgopen();

	        int searchport = maincfg_get_int(&maincfg,"BSDPORT");

		for (i=0;i<maxLots;i++) {
			dirtylots[i] = 0;
		}

		//int bsConectAndQueryOneServer(char server[], int searchport, char query[], char subname[], int maxHits, int start, struct SiderFormat *Sider)
		bsConectAndQueryOneServer(opt.NetLot,searchport,opt.Query,subname,50,1,&Sider,&pageNr);
		
		for (i=0;i<pageNr;i++) {
			if (!Sider[i].deletet) {
				printf("%i: DocID: %u, Url \"%s\"\n",i,Sider[i].iindex.DocID,Sider[i].url);
				len2 = readHTMLNET_toHost(subname, Sider[i].iindex.DocID, text, len, &ReposetoryHeader, opt.NetLot);
				if (text[0] == '\0') {
					printf("diden manage to read html\n");
					continue;
				}

				//if (Sider[i].iindex.DocID == 125606568) {
				//	printf("#########################\n%s\n######################\n",text);
				//	printf("len %i\n",strlen(text));
				//}

				//toDo: vi må hente bilde fra readHTMLNET()				
				ReposetoryHeader.imageSize = 0;

				ReposetoryHeader.htmlSize2 = strlen(text);

				rApendPostcompress(&ReposetoryHeader,text,image,subname,NULL,NULL,"repo.test", ReposetoryHeader.url , "", NULL);


				dirtylots[rLotForDOCid(Sider[i].iindex.DocID)] = 1;


			}
		}

		free(Sider);

		for (i=0;i<maxLots;i++) {
			if (dirtylots[i]) {
				run (i,subname,&opt,"repo.test");
			}
		}

		#endif
	}
	//hvis lotNr er 0 skal vi hente info fra serveren om hvilkene lotter den har som trenger å indekseres.		
	else if ((opt.NetLot != NULL) && (lotNr == 0)) {

		//henter lot nr fra server
		i = 0;
		while (((lotNr = getLotToIndex(subname,opt.NetLot, opt.dirty)) != 0) && (opt.LotMax == 0 || opt.LotMax > i)) {
			printf("starting indexing of lot %i. nr %i\n",lotNr,i);
			run (lotNr,subname,&opt,"reposetory");
			printf("done indexing lot %i\n",lotNr);
			++i;
		}
		printf("main: Have no more lot's to index\n");

	}
	else {
		run (lotNr,subname,&opt,"reposetory");
	}


	return 0;

}

