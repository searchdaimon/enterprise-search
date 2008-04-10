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

#include "../common/define.h"
#include "../common/langdetect.h"
#include "../common/reposetoryNET.h"

#include "../common/crc32.h"
#include "../common/gcrepo.h"

#include "../IndexerRes/IndexerRes.h"
#include "../common/integerindex.h"
#include "../searchFilters/searchFilters.h"

#include "../common/bstr.h"
#include "../common/debug.h"
#include "../common/ir.h"
#include "../common/iindex.h"
#include "../common/revindex.h"
#include "../common/bfileutil.h"

#include "../banlists/ban.h"

#ifdef BLACK_BOKS

#else
	#include "../maincfg/maincfg.h"
	#include "../dispatcher_all/library.h"
#endif

//#include "../parse_summary/summary.h"

#ifdef WITH_THREAD
        #include <pthread.h>

	
        #ifdef BLACK_BOKS
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
	struct adultFormat *adult;
	FILE *ADULTWEIGHTFH;
	FILE *SFH;
	struct alclotFormat *alclot;
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
	char *optQuery;
	char *optNetLot;
	struct DIArrayFormat *DIArray;
	//struct DIArrayFormat DIArray[NrofDocIDsInLot];
	char *reponame;

	#ifndef BLACK_BOKS
		struct addNewUrlhaFormat addNewUrlha[NEWURLFILES_NR];
	#else
	#endif

	#ifdef WITH_THREAD
        	pthread_mutex_t reposetorymutex;
        	pthread_mutex_t restmutex;
       	#endif

	#ifdef PRESERVE_WORDS
		FILE *dictionarywordsfFH;
	#endif
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

};


#ifdef BLACK_BOKS
//acllot includes
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"



struct alclotFormat {
	char subname[maxSubnameLength];
	char openmode[4];
	int lotNr;
	int *h;
};


struct aclusernameFormat {
	char username[MAX_USER_NAME_LEN];
	int len;
};




static unsigned int alclot_hashfromkey(void *ky)
{
    struct aclusernameFormat *k = (struct aclusernameFormat *)ky;
    return crc32boithonl((*k).username,(*k).len);
}

static int alclot_equalkeys(void *k1, void *k2)
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


void alclot_init(struct alclotFormat **alclot,char subname[],char openmode[],int lotNr) {

	(*alclot) = malloc(sizeof(struct alclotFormat));
	strcpy((**alclot).subname,subname);
	strcpy((**alclot).openmode,openmode);
	(**alclot).lotNr = lotNr;

	//struct hashtable *h;
	(**alclot).h = (int *)create_hashtable(200, alclot_hashfromkey, alclot_equalkeys);



}
void alclot_close(struct alclotFormat *alclot) {

	FILE *fp;

	fp = lotOpenFileNoCasheByLotNr((*alclot).lotNr,"acllist",(*alclot).openmode, 'e',(*alclot).subname);

	//struct hashtable **h; //temp

	struct aclusernameFormat *aclusername;
	int *value;
	printf("acls for lot:\n");
	if (hashtable_count((struct hashtable *)(*alclot).h) > 0)
        {
        	struct hashtable_itr *itr;

                itr = hashtable_iterator((struct hashtable *)(*alclot).h);
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
        hashtable_destroy((struct hashtable *)(*alclot).h,1);

	free(alclot);
}

void iiacladd(struct IndexerRes_acls *iiacl,char acl[]) {

  	char **Data;
  	int Count, TokCount;

  	TokCount = split(acl, ",", &Data);


	//legger til acler
	Count = 0;
	while( (Data[Count] != NULL) ) {

		//gruppenavn med spacer skaper problemer. Erstater det med _ i steden
		strsandr(Data[Count]," ","_");

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
void alclot_add(struct alclotFormat *alclot,char acl[]) {


	struct hashtable **h; //temp

  	char **Data;
  	int Count, TokCount;
	int *oldvalue;
	int *value;

  	TokCount = split(acl, ",", &Data);


  	Count = 0;
  	while( (Data[Count] != NULL) ) {

		//gruppenavn med spacer skaper problemer. Erstater det med _ i steden
		strsandr(Data[Count]," ","_");


		#ifdef DEBUG
		printf("god user \"%s\"\n",Data[Count]);
		#endif

		struct aclusernameFormat *aclusername = malloc(sizeof(struct aclusernameFormat));

		(*aclusername).len = strnlen(Data[Count],MAX_USER_NAME_LEN);
		strscpy((*aclusername).username,Data[Count],MAX_USER_NAME_LEN);

		if (NULL == (oldvalue = hashtable_search((struct hashtable *)(*alclot).h,aclusername) )) {
			//printf("not found!. Vil insert first");
			value = malloc(sizeof(int));
			(*value) = 1;

			if (! hashtable_insert((struct hashtable *)(*alclot).h,aclusername,value) ) {
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
	char imagebuffer[],int imagebuffer_size,unsigned long int *radress, char **acl_allow, char **acl_denied,struct ReposetoryHeaderFormat *ReposetoryHeader) {
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
	else if (rGetNext_reponame((*argstruct).lotNr,ReposetoryHeader,htmlcompressdbuffer,htmlcompressdbuffer_size,
			imagebuffer,radress,(*argstruct).FiltetTime,(*argstruct).FileOffset,(*argstruct).subname,acl_allow,acl_denied,argstruct->reponame)) {


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


void *IndexerLot_workthread(void *arg) {

	struct IndexerLot_workthreadFormat *argstruct = (struct IndexerLot_workthreadFormat *)arg;
//void *IndexerLot_workthread(struct IndexerLot_workthreadFormat *argstruct) {

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
	unsigned int crc32;

	Bytef *SummaryBuffer;
	uLong SummaryBufferSize;

	#ifdef BLACK_BOKS
		int i;
	#endif
//dagur:	struct ReposetoryHeaderFormat 

/************************************************************************/
	struct ReposetoryHeaderFormat ReposetoryHeader;
/************************************************************************/

//dagur: ReposetoryHeader;
	struct DocumentIndexFormat *DocumentIndexPost;


	wordsInit(pagewords);


	while (getNextPage(argstruct,htmlcompressdbuffer,sizeofhtmlcompressdbuffer,imagebuffer,sizeofimagebuffer,
		&radress,&acl_allow,&acl_denied,&ReposetoryHeader)) {

       				awvalue = 0;
               			title = NULL;
		                body = NULL;
				metadesc = NULL;

				SummaryBuffer = NULL;

				DocIDPlace = (ReposetoryHeader.DocID - LotDocIDOfset((*argstruct).lotNr));


				if ((DocumentIndexPost = malloc(sizeof(struct DocumentIndexFormat))) == NULL) {
					perror("malloc DocumentIndexPost");
					exit(1);
				}


				if((argstruct->optHandleOld) && ((*argstruct).DIArray[DocIDPlace].oldp != NULL)) {
					memcpy(DocumentIndexPost,(*argstruct).DIArray[DocIDPlace].oldp,sizeof(struct DocumentIndexFormat));
				}
				else if (argstruct->optQuery != NULL) {

					memset(DocumentIndexPost,0, sizeof(struct DocumentIndexFormat));

					if (!DIReadNET(argstruct->optNetLot,DocumentIndexPost,ReposetoryHeader.DocID,argstruct->subname)) {
						printf("can't DIReadNET\n");
						continue;
					}
					if (strcmp(DocumentIndexPost->Url,ReposetoryHeader.url) != 0) {
						printf("Error: urls is not the same: did DIReadNET DocID %u for url \"%s\" == \"%s\" ?\n",ReposetoryHeader.DocID,DocumentIndexPost->Url,ReposetoryHeader.url);
						continue;
					}
					printf("did DIReadNET DocID %u for url \"%s\" == \"%s\" ?\n",ReposetoryHeader.DocID,DocumentIndexPost->Url,ReposetoryHeader.url);				


				}
				else {
					memset(DocumentIndexPost,0, sizeof(struct DocumentIndexFormat));
				}

				HtmlBufferLength = sizeofHtmlBuffer;
				if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)&HtmlBufferLength,(Bytef*)htmlcompressdbuffer,ReposetoryHeader.htmlSize)) != 0) {
					#ifdef DEBUG
               				printf("uncompress error. Code: %i for DocID %u-%i. ReposetoryHeader.htmlSize %i,sizeofHtmlBuffer %i\n",nerror,ReposetoryHeader.DocID,rLotForDOCid(ReposetoryHeader.DocID),ReposetoryHeader.htmlSize,sizeofHtmlBuffer);
					#endif
               				continue;
		                }

				//printf("HtmlBuffer:\n##############\n%s\n#####################\n",HtmlBuffer);

				crc32 = crc32boithonl(HtmlBuffer,HtmlBufferLength);


				if((*argstruct).optHandleOld) {

					if (DocumentIndexPost->htmlSize != 0) {

						if (DocumentIndexPost->RepositoryPointer == radress) {
							#ifdef DEBUG
							printf("have already indexed this dokument\n");
							#endif
							++(*argstruct).optHandleOld_allrediIndexed;

							free(DocumentIndexPost);
							continue;
						}
					        else if (DocumentIndexPost->crc32 == crc32) {
							printf("have a duplicate dokument, but havent changed\n");
							++(*argstruct).optHandleOld_duplicateNochange;

							free(DocumentIndexPost);
							continue;
						}
						else {
							++(*argstruct).optHandleOld_indexed;
						}

					}

				}


				if ((*argstruct).optPrintInfo) {
					printf("url: %s, DocID %u, respons %i\n",ReposetoryHeader.url,ReposetoryHeader.DocID,ReposetoryHeader.response);
				}




				//begynner på en ny side
                                wordsReset(pagewords,ReposetoryHeader.DocID);



				//printf("D: %u, R: %lu\n",ReposetoryHeader.DocID, radress);


				#ifndef BLACK_BOKS
				if (isIpBan(ReposetoryHeader.IPAddress)) {
					printf("ip adrsess %u is on ban list. Url \"%s\"\n",ReposetoryHeader.IPAddress,ReposetoryHeader.url);
					DocumentIndexPost->RepositoryPointer = 0;
					DocumentIndexPost->htmlSize = 0;					
					DocumentIndexPost->response = 0;					
				}
				else if (!find_domain_no_subname(ReposetoryHeader.url,domain,sizeof(domain)) ) {
					debug("can't find domain. Url \"%s\"\n",ReposetoryHeader.url);
				}
				else if (!find_TLD(domain,TLD,sizeof(TLD))) {
					printf("can't find TLD. Url \"%s\"\n",ReposetoryHeader.url);
				}
				else if (IndexerLot_filterOnlyTLD((*argstruct).optOnlyTLD,TLD)) {
					debug("Filter: optOnlyTLD ekskludes \"%s\"\n",TLD);
				}
				else if (filterDomainNrOfLines(domain)) {
					debug("To many lines in domaine. Domain \"%s\"\n",domain);
				}
				else if (filterDomainLength(domain)) {
					debug("To long domaine. Domain \"%s\"\n",domain);
				}
				else if (filterTLDs(domain)) {
					debug("bannet TLD. Domain \"%s\"\n",domain);
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

					#ifndef BLACK_BOKS

					makePreParsedSummary(body,strlen(body),title,strlen(title),metadesc,strlen(metadesc),
						&SummaryBuffer,&SummaryBufferSize);
					#endif


					wordsMakeRevIndexBucket(pagewords,ReposetoryHeader.DocID,&langnr);

					


/************************************************************************/
//denne er gammel, må finne ny
//dagur: la til //
//er adult vekt
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

					#ifdef BLACK_BOKS
						//handel acl
						//trenger dette acl greiene å være her, kan di ikke være lenger opp, der vi ikke har trå lås ?
						alclot_add((*argstruct).alclot,acl_allow);
						
						#ifdef IIACL
						iiacladd(&(*pagewords).acl_allow,acl_allow);
						iiacladd(&(*pagewords).acl_denied,acl_denied);

						aclsMakeRevIndex(&(*pagewords).acl_allow);
						aclsMakeRevIndex(&(*pagewords).acl_denied);

						aclsMakeRevIndexBucket (&(*pagewords).acl_allow,ReposetoryHeader.DocID,&langnr);
						aclsMakeRevIndexBucket (&(*pagewords).acl_denied,ReposetoryHeader.DocID,&langnr);
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

						#ifndef BLACK_BOKS

						SummaryWrite(SummaryBuffer,SummaryBufferSize,&(*DocumentIndexPost).SummaryPointer,
							&(*DocumentIndexPost).SummarySize,ReposetoryHeader.DocID,(*argstruct).SFH);

							linksWrite(pagewords,(*argstruct).addNewUrlha);

						#endif

						//printf("SummaryBufferSize %i\n",SummaryBufferSize);
						


						revindexFilesAppendWords(pagewords,(*argstruct).revindexFilesHa,ReposetoryHeader.DocID,&langnr);


						#ifdef BLACK_BOKS
							#ifdef IIACL
							aclindexFilesAppendWords(&(*pagewords).acl_allow,(*argstruct).acl_allowindexFilesHa,ReposetoryHeader.DocID,&langnr);
							aclindexFilesAppendWords(&(*pagewords).acl_denied,(*argstruct).acl_deniedindexFilesHa,ReposetoryHeader.DocID,&langnr);
							#endif
						#endif

						#ifdef PRESERVE_WORDS
							dictionaryWordsWrite(pagewords,(*argstruct).dictionarywordsfFH, acl_allow, acl_denied);
					
						#endif
						//DocIDPlace = ((ReposetoryHeader.DocID - LotDocIDOfset((*argstruct).lotNr)) * sizeof(unsigned char));
						////printf("DocID %u, DocIDPlace %i\n",ReposetoryHeader.DocID,DocIDPlace);
						//fseek((*argstruct).ADULTWEIGHTFH,DocIDPlace,SEEK_SET);
						//	
	                			//fwrite(&awvalue,sizeof(awvalue),1,(*argstruct).ADULTWEIGHTFH);


					}

					//skiver til DocumentIndex
					//DIWrite(DocumentIndexPost,ReposetoryHeader.DocID,(*argstruct).subname);






				#ifdef WITH_THREAD
					pthread_mutex_unlock(&(*argstruct).restmutex);
				#endif



				(*argstruct).DIArray[DocIDPlace].DocID = ReposetoryHeader.DocID;						

				DocumentIndexPost->crc32 = crc32;
			
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

		}		


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
			printf(". domain\n");
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

			rSendFileToHostname(filname,lotinternfilname,lotNr, "w",subname,server);
			
		}
		else {
			printf("unknown type %i\n",dp->d_type);
		}
		//printf("%s %i\n",dp->d_name,dp->d_type);

	}

	closedir(DIRH);

}


/*******************************************************
rutine for å hente date tilbake til lagringserver.
*******************************************************/
void netlot_end (int lotNr,char subname[], char server[]) {

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

	//sletter reposetor. Vi skal ikke trenge og skrive tilbake det.
	sprintf(reposetuoypath,"%s%s",lotpath,"reposetory");
	printf("unlinking %s\n",reposetuoypath);

	if (unlink(reposetuoypath) != 0) {
		perror(reposetuoypath);
		exit(1);
	}

	netlot_end_recursiveDir(lotpath,"",lotNr,subname, server);

	//sletter loten
	rrmdir(lotpath);
}

/*******************************************************
rutine for å hente en fil på lagringserver
*******************************************************/
void netlot_start_get_file(int lotNr,char subname[],char file[], char server[]) {

	FILE *FH;

	printf("netlotStart: Geting file \"%s\" for lot %i\n",file,lotNr);

	if ((FH = lotOpenFileNoCasheByLotNr(lotNr,file,"wb", 'e',subname)) == NULL) {
		perror("local file");
		exit(1);
	}

	if(!rGetFileByOpenHandlerFromHostName(file, FH, lotNr, subname, server)) {
		perror("file on server");
		exit(1);
	}

	fclose(FH);

	#ifdef DEBUG
	printf("netlotStart: done\n");
	#endif

}


/*******************************************************
rutine for å hente det vi trnger på lagringserver.
*******************************************************/
void netlot_start(int lotNr,char subname[], int optrEindex, char server[]) {

	
	int i;
	char iindexPath[512];



	netlot_start_get_file(lotNr,subname,"IndexTime",server);
	netlot_start_get_file(lotNr,subname,"reposetory",server);

	netlot_start_get_file(lotNr,subname,"DocumentIndex",server);
	netlot_start_get_file(lotNr,subname,"summary",server);
	netlot_start_get_file(lotNr,subname,"domainid",server);
	netlot_start_get_file(lotNr,subname,"brankPageElements",server);


	if (optrEindex == 0) {
		makeLotPath(lotNr,"iindex/Main/index/aa/",subname);

		for(i=0;i<=63;i++) {
			sprintf(iindexPath,"iindex/Main/index/aa/%i.txt",i);
			netlot_start_get_file(lotNr,subname,iindexPath,server);
		
		}
	}

}


void run(int lotNr, char subname[], struct optFormat opt, char reponame[]) {

	struct IndexerLot_workthreadFormat *argstruct;

	int i,n;
	int lotPart;

        unsigned int FiltetTime;
        unsigned int FileOffset;
	//char lotServer[64];
	struct iintegerFormat iinteger;
	off_t DocIDPlace;	
	unsigned int lastIndexTime;
	char openmode[4];
	FILE *brankPageElementsFH;

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


	printf("run: indexing lot %i\n",lotNr);

	#ifndef BLACK_BOKS		
		if (!ipbanLoad()) {
			printf("can't load ip ban list\n");
			exit(1);
		}
	#endif

	for(i=0;i<nrOfHttpResponsCodes;i++) {
		argstruct->httpResponsCodes[i] = 0;
	}

	//temp: må hente dette fra slot server eller fil
	FileOffset = 0;


	argstruct->pageCount = 0;



		//sjekker om vi har nokk palss
		if (!lotHasSufficientSpace(lotNr,4096,subname)) {
			printf("insufficient disk space\n");
			exit(1);
		}

		//netlot: Vi henter det vi trenger
		if ((opt.NetLot != NULL) && (opt.Query == NULL)) {
			netlot_start(lotNr,subname,opt.rEindex,opt.NetLot);
		}

		//finner siste indekseringstid
		lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);

		printf("lastIndexTime %s",ctime((time_t *)&lastIndexTime));

		if (opt.rEindex == 1) {
			FiltetTime = 0;
			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (lastIndexTime == 0) {
			FiltetTime = 0;

			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (opt.MustBeNewerThen != 0) {
			//regner ut hvor ny den må være
			printf("Isnet new. Last indexed %s",ctime((time_t *)&lastIndexTime));
			exit(1);
		}		
		else if(lastIndexTime != 0) {
			printf("lastIndexTime is not 0, but %i\n",lastIndexTime);
			FiltetTime = lastIndexTime;

			//opner for appending
			strcpy(openmode,"ab");
		}
		else {
			printf("sholden happend!\n");
			exit(1);
		}
		printf("openmode\"%s\"\n",openmode);


		revindexFilesOpenLocal(argstruct->revindexFilesHa,lotNr,"Main",openmode,subname);

		#ifdef BLACK_BOKS		

			#ifdef IIACL
				revindexFilesOpenLocal(argstruct->acl_allowindexFilesHa,lotNr,"acl_allow",openmode,subname);
				revindexFilesOpenLocal(argstruct->acl_deniedindexFilesHa,lotNr,"acl_denied",openmode,subname);
			#endif

			alclot_init(&argstruct->alclot,subname,openmode,lotNr);
		#else


			if ((argstruct->ADULTWEIGHTFH = lotOpenFileNoCasheByLotNr(lotNr,"AdultWeight",openmode, 'e',subname)) == NULL) {
				perror("open AdultWeight");
				exit(1);
			}

			if (opt.Query == NULL) {
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

		#ifdef PRESERVE_WORDS
			argstruct->dictionarywordsfFH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords_raw",openmode,'r',subname);
		#endif


		//temp:Søker til problemområdet
		//FileOffset = 334603785;		



		//main work
		argstruct->lotNr 		= lotNr;
		argstruct->subname 		= subname;
        	argstruct->FiltetTime 		= FiltetTime;
        	argstruct->FileOffset 		= FileOffset;
		argstruct->optMaxDocuments	= opt.MaxDocuments;
		argstruct->optPrintInfo		= opt.PrintInfo;
		argstruct->optOnlyTLD		= opt.OnlyTLD;
		argstruct->optMakeWordList 	= opt.MakeWordList;
		argstruct->optHandleOld		= opt.HandleOld;
		argstruct->optQuery		= opt.Query;
		argstruct->optNetLot		= opt.NetLot;
		argstruct->reponame		= reponame;

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

		if (opt.HandleOld) {
			argstruct->optHandleOld_duplicateNochange	= 0;
			argstruct->optHandleOld_allrediIndexed		= 0;
			argstruct->optHandleOld_indexed			= 0;

			printf("will handel old dokuments in reposetory. Loading DocumentIndex...\n");
			for(i=0;i<NrofDocIDsInLot;i++) {
				argstruct->DIArray[i].oldp = malloc(sizeof(struct DocumentIndexFormat));
				DIRead(argstruct->DIArray[i].oldp,LotDocIDOfset(lotNr) + i,subname);
			}
			printf(".. done\n");
		}
		//init mutex
		#ifdef WITH_THREAD

			pthread_mutex_init(&argstruct->reposetorymutex, NULL);
			pthread_mutex_init(&argstruct->restmutex, NULL);

			if (opt.NrofWorkThreads == 0) {
				printf("won't use threads\n");
				IndexerLot_workthread(argstruct);
			}
			else {
				debug("wil use %i threads\n",opt.NrofWorkThreads);
				
				pthread_t *threadid;

				if ((threadid = malloc(sizeof(pthread_t) * opt.NrofWorkThreads)) == NULL) {
					perror("malloc threadid");
					exit(1);
				}

				for(i=0;i<opt.NrofWorkThreads;i++) {
					debug("starting tread nr %i\n",i);
					n = pthread_create(&threadid[i], NULL, IndexerLot_workthread, argstruct);
				}

				//venter på trådene
        			for (i=0;i<opt.NrofWorkThreads;i++) {
                			n = pthread_join(threadid[i], NULL);
        			}
				
				free(threadid);
			}
		#else
			printf("wasent build with threads\n");
			IndexerLot_workthread(argstruct);

		#endif


		revindexFilesCloseLocal(argstruct->revindexFilesHa,"Main"); 

		#ifdef BLACK_BOKS		
			revindexFilesCloseLocal(argstruct->acl_allowindexFilesHa,"Main"); 
			revindexFilesCloseLocal(argstruct->acl_deniedindexFilesHa,"Main"); 
		#endif


		if (opt.Query != NULL) {
			for(i=0;i<NrofDocIDsInLot;i++) {

				if (argstruct->DIArray[i].p != NULL) {
					printf("DocID %u, adult value %hu, url \"%s\"\n",argstruct->DIArray[i].DocID,argstruct->DIArray[i].p->AdultWeight,argstruct->DIArray[i].p->Url);
					DIWriteNET (opt.NetLot , argstruct->DIArray[i].p, argstruct->DIArray[i].DocID, argstruct->subname);

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

				#ifndef BLACK_BOKS

				if (argstruct->DIArray[i].haveawvalue == 1) {

					DocIDPlace = ((argstruct->DIArray[i].DocID - LotDocIDOfset(argstruct->lotNr)) * sizeof(unsigned char));

					//printf("DocID %u, DocIDPlace %i\n",argstruct->DIArray[i].DocID,DocIDPlace);
					fseek(argstruct->ADULTWEIGHTFH,DocIDPlace,SEEK_SET);	
               				fwrite(&argstruct->DIArray[i].awvalue,sizeof(unsigned char),1,argstruct->ADULTWEIGHTFH);

					iintegerSetValue(&iinteger,&argstruct->DIArray[i].DomainDI,sizeof(argstruct->DIArray[i].DomainDI),argstruct->DIArray[i].DocID,subname);
				}


				#endif
			}
		
			closeDICache();
		}

		#ifndef BLACK_BOKS

			if ((brankPageElementsFH = lotOpenFileNoCasheByLotNr(lotNr,"brankPageElements",openmode,'r',subname)) == NULL) {
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

		#ifdef BLACK_BOKS
			alclot_close(argstruct->alclot);
		#else
			fclose(argstruct->ADULTWEIGHTFH);
			if (opt.Query == NULL) {
				fclose(argstruct->SFH);
			}
			iintegerClose(&iinteger);


		#endif

		//skriver riktig indexstide til lotten
		setLastIndexTimeForLot(lotNr,argstruct->httpResponsCodes,subname);
		
		
		#ifdef PRESERVE_WORDS
		fclose(argstruct->dictionarywordsfFH);
		#endif
		// vi må ikke kopiere revindex filene da vi jobber på de lokale direkte


		if (opt.HandleOld) {
			printf("duplicateNochange: %i\n",argstruct->optHandleOld_duplicateNochange);
			printf("allrediIndexed: %i\n",argstruct->optHandleOld_allrediIndexed);
			printf("optHandleOld_indexed: %i\n",argstruct->optHandleOld_indexed);

			for(i=0;i<NrofDocIDsInLot;i++) {
				free(argstruct->DIArray[i].oldp);
			}
		}


		printf("indexed %i pages\n\n\n",argstruct->pageCount);

		free(argstruct->DIArray);
		free(argstruct->adult);


		if (globalIndexerLotConfig.urlfilter != NULL) {
			FreeSplitList(globalIndexerLotConfig.urlfilter);
		}


		//run the Garbage Collection
		#ifdef BLACK_BOKS
		if ((opt.RunGarbageCollection == 1) && (1)) {
		#else
		if ((opt.RunGarbageCollection == 1) && (argstruct->pageCount > 0)) {
		#endif
			printf("running Garbage Collection..\n");
			gcrepo(lotNr, subname);
			printf(".. done\n");

		}

		if (opt.RunIndekser == 1) {

			printf("runing Indekser\n");

                	for (lotPart=0;lotPart<64;lotPart++) {
                	        //printf("indexint part %i for lot %i\n",lotPart,lotNr);

                	        Indekser(lotNr,"Main",lotPart,subname,0,0);

                	}

                	//siden vi nå har lagt til alle andringer fra rev index kan vi nå slettet gced filen også
                	Indekser_deleteGcedFile(lotNr, subname);
		}
	
		//netlot: Vi sender det vi har laget
		if ((opt.NetLot != NULL) && (opt.Query == NULL)) {
			netlot_end(lotNr,subname,opt.NetLot);
		}

		#ifndef BLACK_BOKS
		for (i=0;i<NEWURLFILES_NR;i++) {
			fclose(argstruct->addNewUrlha[i].NYEURLER);
		}	
		#endif

		free(argstruct);

		html_parser_exit();
		langdetectDestroy();

		#ifndef BLACK_BOKS		
			ipbanEnd();
		#endif

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

	#ifdef BLACK_BOKS
		opt.HandleOld = 1;
	#else
		opt.HandleOld = 0;
	#endif

	#ifdef WITH_THREAD
		opt.NrofWorkThreads = DEFAULT_NROF_GENERATEPAGES_THREADS;	
	#endif

	globalIndexerLotConfig.collectUrls = 0;
	globalIndexerLotConfig.urlfilter = NULL;

	extern char *optarg;
       	extern int optind, opterr, optopt;
	char c;
	while ((c=getopt(argc,argv,"neu:t:m:pl:wogh:iq:"))!=-1) {
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
                        case 'm':
                                opt.MaxDocuments = atoi(optarg);
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
		        #ifdef BLACK_BOKS

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

		#ifndef BLACK_BOKS

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

				ReposetoryHeader.htmlSize = strlen(text);

				rApendPostcompress(&ReposetoryHeader,text,image,subname,NULL,NULL,"repo.test");


				dirtylots[rLotForDOCid(Sider[i].iindex.DocID)] = 1;


			}
		}

		free(Sider);

		for (i=0;i<maxLots;i++) {
			if (dirtylots[i]) {
				run (i,subname,opt,"repo.test");
			}
		}

		#endif
	}
	//hvis lotNr er 0 skal vi hente info fra serveren om hvilkene lotter den har som trenger å indekseres.		
	else if ((opt.NetLot != NULL) && (lotNr == 0)) {

		//henter lot nr fra server
		while ((lotNr = getLotToIndex(subname,opt.NetLot)) != 0) {
			printf("starting indexing of lot %i\n",lotNr);
			run (lotNr,subname,opt,"reposetory");
			printf("done indexing lot %i\n",lotNr);
		}

	}
	else {
		run (lotNr,subname,opt,"reposetory");
	}



	return 0;

}

