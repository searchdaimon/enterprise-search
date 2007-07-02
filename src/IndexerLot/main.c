#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "../common/define.h"
#include "../common/langdetect.h"

#include "../common/crc32.h"

#include "../IndexerRes/IndexerRes.h"
#include "../common/integerindex.h"
#include "../searchFilters/searchFilters.h"

#include "../common/bstr.h"
#include "../common/debug.h"
#include "../common/ir.h"


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
	FILE *brankPageElementsFH;
	struct alclotFormat *alclot;
	int pageCount;
	int httpResponsCodes[nrOfHttpResponsCodes];
	int optMaxDocuments;
	int optPrintInfo;
	int optMakeWordList;
	char **optOnlyTLD;
	struct DIArrayFormat *DIArray;
	//struct DIArrayFormat DIArray[NrofDocIDsInLot];

	#ifndef BLACK_BOKS
	struct addNewUrlhaFormat addNewUrlha[NEWURLFILES_NR];
	#endif

	#ifdef WITH_THREAD
        	pthread_mutex_t reposetorymutex;
        	pthread_mutex_t restmutex;
       	#endif

	#ifdef PRESERVE_WORDS
		FILE *dictionarywordsfFH;
	#endif
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
		printf("god acl \"%s\"\n",Data[Count]);
	
		acladd(iiacl, Data[Count]);
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

int SummaryWrite(Bytef *WorkBuffer,uLong WorkBufferSize, unsigned int *SummaryPointer,
		unsigned short *SummarySize,const unsigned int DocID,FILE *SFH) {

	(*SummaryPointer) = ftell(SFH);
        (*SummarySize) = (sizeof(DocID) + WorkBufferSize);

	//write it to disk
        fwrite(&DocID,sizeof(DocID),1,SFH);
        fwrite(WorkBuffer,WorkBufferSize,sizeof(char),SFH);

}

int makePreParsedSummary(const char body[], int bodylen,const  char title[],int titlelen,const char metadesc[], 
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

	#ifdef WITH_THREAD
		pthread_mutex_lock(&(*argstruct).reposetorymutex);
	#endif

	if(((*argstruct).optMaxDocuments) != 0 && ((*argstruct).optMaxDocuments <= (*argstruct).pageCount)) {
		//printf("Exeting after only %i docs\n",(*argstruct).pageCount);
		forreturn = 0;
	}
	else if (lastpage) {
		 forreturn = 0;
	}
	else if (rGetNext((*argstruct).lotNr,ReposetoryHeader,htmlcompressdbuffer,htmlcompressdbuffer_size,
			imagebuffer,radress,(*argstruct).FiltetTime,(*argstruct).FileOffset,(*argstruct).subname,acl_allow,acl_denied)) {

		++(*argstruct).pageCount;

		if (((*argstruct).pageCount % 10000) == 0) {
                	printf("%i\n",(*argstruct).pageCount);
		}

		forreturn = 1;
	}
	else {
		lastpage = 0;
		forreturn = 0;
	}
	
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

	
	int i;
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

	Bytef *SummaryBuffer;
	uLong SummaryBufferSize;

//dagur:	struct ReposetoryHeaderFormat 

/************************************************************************/
	struct ReposetoryHeaderFormat ReposetoryHeader;
/************************************************************************/

//dagur: ReposetoryHeader;
	struct DocumentIndexFormat *DocumentIndexPost;


	wordsInit(pagewords);


	while (getNextPage(argstruct,htmlcompressdbuffer,sizeofhtmlcompressdbuffer,imagebuffer,sizeofimagebuffer,
		&radress,&acl_allow,&acl_denied,&ReposetoryHeader)) {

				DocumentIndexPost = malloc(sizeof(struct DocumentIndexFormat));

				if ((*argstruct).optPrintInfo) {
					printf("url: %s, DocID %u, respons %i\n",ReposetoryHeader.url,ReposetoryHeader.DocID,ReposetoryHeader.response);
				}


				memset(DocumentIndexPost,0, sizeof(struct DocumentIndexFormat));

               			title = NULL;
		                body = NULL;
				metadesc = NULL;

				SummaryBuffer = NULL;

				//begynner på en ny side
                                wordsReset(pagewords,ReposetoryHeader.DocID);

				//printf("D: %u, R: %lu\n",ReposetoryHeader.DocID, radress);


				#ifndef BLACK_BOKS

				if (!find_domain_no_subname(ReposetoryHeader.url,domain,sizeof(domain)) ) {
					debug("can't find domain. Url \"%s\"\n",ReposetoryHeader.url);
				}
				else if (!find_TLD(domain,TLD,sizeof(TLD))) {
					printf("cnat find TLD. Url \"%s\"\n",ReposetoryHeader.url);
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
	
					HtmlBufferLength = sizeofHtmlBuffer;
					if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)&HtmlBufferLength,(Bytef*)htmlcompressdbuffer,ReposetoryHeader.htmlSize)) != 0) {
						#ifdef DEBUG
                       				printf("uncompress error. Code: %i for DocID %u-%i. ReposetoryHeader.htmlSize %i,sizeofHtmlBuffer %i\n",nerror,ReposetoryHeader.DocID,rLotForDOCid(ReposetoryHeader.DocID),ReposetoryHeader.htmlSize,sizeofHtmlBuffer);
						#endif
                       				continue;
			                }

					/*
					if (ReposetoryHeader.DocID == 125768695) {
						FILE *fh;
						fh = fopen("/tmp/125768695.html","w");
						fwrite(HtmlBuffer,1,HtmlBufferLength,fh);
						fclose(fh);
						continue;	
					}
					*/

					//usikker her. Skal det vare +1? strlen() blir da en større en HtmlBufferLength
					//HtmlBuffer[HtmlBufferLength +1] = '\0';
                       			//printf("url: %s\n",ReposetoryHeader.url);


					//printf("document \"%s\" %i b\n",HtmlBuffer,HtmlBufferLength);

					handelPage(pagewords,(*argstruct).lotNr,&ReposetoryHeader,HtmlBuffer,HtmlBufferLength,DocumentIndexPost,ReposetoryHeader.DocID,(*argstruct).httpResponsCodes,(*argstruct).adult,&title,&body);
					//har ikke metadesc enda
					metadesc = strdup("");

					#ifndef BLACK_BOKS

					makePreParsedSummary(body,strlen(body),title,strlen(title),metadesc,strlen(metadesc),
						&SummaryBuffer,&SummaryBufferSize);
					#endif


					(*DocumentIndexPost).crc32 = crc32boithonl(HtmlBuffer,HtmlBufferLength);

					//setter anatll utgående linker
					//bruker en unsigned char. Kan ikke ha flere en 255 
					if ((*pagewords).nrOfOutLinks > 255) {
						(*DocumentIndexPost).nrOfOutLinks = 255;
					}
					else {
						(*DocumentIndexPost).nrOfOutLinks = (*pagewords).nrOfOutLinks;
					}

					wordsMakeRevIndex(pagewords,(*argstruct).adult,&AdultWeight,&langnr);

					sprintf((*DocumentIndexPost).Sprok,"%i",langnr);


					if (AdultWeight > 255) {
                                        	(*DocumentIndexPost).AdultWeight = 255;
                                        }
                                       	else {
                                        	(*DocumentIndexPost).AdultWeight = AdultWeight;
                                        }


					wordsMakeRevIndexBucket (pagewords,ReposetoryHeader.DocID,&langnr);

					


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
				else {
					//ikke 200->299 side
					//egentlig ukjent
					awvalue = 0;
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


				DocIDPlace = (ReposetoryHeader.DocID - LotDocIDOfset((*argstruct).lotNr));

				(*argstruct).DIArray[DocIDPlace].DocID = ReposetoryHeader.DocID;						
			
				(*argstruct).DIArray[DocIDPlace].p = DocumentIndexPost;
			

				(*argstruct).DIArray[DocIDPlace].haveawvalue = 1;
				(*argstruct).DIArray[DocIDPlace].awvalue = awvalue;						


				(*argstruct).DIArray[DocIDPlace].haverankPageElements = 1;
				(*argstruct).DIArray[DocIDPlace].brankPageElements.IPAddress = 		(*DocumentIndexPost).IPAddress;
				(*argstruct).DIArray[DocIDPlace].brankPageElements.nrOfOutLinks = 	(*DocumentIndexPost).nrOfOutLinks;
				(*argstruct).DIArray[DocIDPlace].brankPageElements.response = 		(*DocumentIndexPost).response;
				

				(*argstruct).DIArray[DocIDPlace].DomainDI = DomainDI;
					
				free(SummaryBuffer);


       				free(body);
       				free(title);
				free(metadesc);

		}		


	free(htmlcompressdbuffer);
	free(HtmlBuffer);

	wordsEnd(pagewords);

}

int main (int argc, char *argv[]) {

	struct IndexerLot_workthreadFormat argstruct;

        int lotNr;
	int i,n;


        unsigned int FiltetTime;
        unsigned int FileOffset;
	//char lotServer[64];
	char *subname;
	struct iintegerFormat iinteger;
	
	unsigned int lastIndexTime;
	unsigned int optMustBeNewerThen = 0;
	unsigned int optrEindex = 0;
	unsigned int optMaxDocuments = 0;
	unsigned int optPrintInfo = 0;
	unsigned int optMakeWordList = 0;
	char **optOnlyTLD = NULL;

	off_t DocIDPlace;

	unsigned int optNrofWorkThreads = 0;	

	#ifdef WITH_THREAD
		optNrofWorkThreads = DEFAULT_NROF_GENERATEPAGES_THREADS;	
	#endif

	globalIndexerLotConfig.collectUrls = 0;
	globalIndexerLotConfig.urlfilter = NULL;

	extern char *optarg;
       	extern int optind, opterr, optopt;
	char c;
	while ((c=getopt(argc,argv,"neu:t:m:pl:w"))!=-1) {
                switch (c) {
			case 'l':
				split(optarg, ",", &optOnlyTLD);
				printf("wil only index TLD's:\n");
				i = 0;
				while( (optOnlyTLD[i] != NULL) ) {
				    printf("\t%i\"%s\"\n", i, optOnlyTLD[i]);
					i++;
				}

				break;
			case 'p':
				optPrintInfo = 1;
				break;
			case 'w':
				//lag en ordbok
				optMakeWordList = 1;
				break;

                        case 'n':
                    
/*****************************************************************************/
/*****************************************************************************/
            optMustBeNewerThen = 1;
                                break;
                        case 'e':
                                optrEindex = 1;
                                break;
                        case 't':
                                optNrofWorkThreads = atoi(optarg);
                                break;
                        case 'm':
                                optMaxDocuments = atoi(optarg);
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

	for(i=0;i<nrOfHttpResponsCodes;i++) {
		argstruct.httpResponsCodes[i] = 0;
	}

	lotNr = atoi(argv[1 +optind]);
	//strncpy(subname,argv[2 + optind],sizeof(subname) -1);
	subname = argv[2 + optind];


	printf("subname %s, lotNr %i\n",subname,lotNr);

        langdetectInit();


	//find server based on lotnr
	//lotlistLoad();
	//lotlistGetServer(lotServer,lotNr);

	char openmode[4];

	//printf("vil index lot nr %i at %s\n",lotNr,lotServer);
	argstruct.adult = malloc(sizeof(struct adultFormat));

	adultLoad(argstruct.adult);


	//temp: må hente dette fra slot server eller fil
	FileOffset = 0;

	argstruct.pageCount = 0;

		printf("Wil acess files localy\n");


		//sjekker om vi har nokk palss
		if (!lotHasSufficientSpace(lotNr,10240,subname)) {
			printf("insufficient disk space\n");
			exit(1);
		}


		//finner siste indekseringstid
		lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);

		printf("lastIndexTime %u\n",lastIndexTime);

		if (optrEindex == 1) {
			FiltetTime = 0;
			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (lastIndexTime == 0) {
			FiltetTime = 0;

			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (optMustBeNewerThen != 0) {
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


		revindexFilesOpenLocal(argstruct.revindexFilesHa,lotNr,"Main",openmode,subname);

		#ifdef BLACK_BOKS		

			#ifdef IIACL
				revindexFilesOpenLocal(argstruct.acl_allowindexFilesHa,lotNr,"acl_allow",openmode,subname);
				revindexFilesOpenLocal(argstruct.acl_deniedindexFilesHa,lotNr,"acl_denied",openmode,subname);
			#endif

			alclot_init(&argstruct.alclot,subname,openmode,lotNr);
		#else


			argstruct.ADULTWEIGHTFH = lotOpenFileNoCasheByLotNr(lotNr,"AdultWeight",openmode, 'e',subname);
			argstruct.SFH = lotOpenFileNoCasheByLotNr(lotNr,"summary",openmode,'r',subname);
			argstruct.brankPageElementsFH = lotOpenFileNoCasheByLotNr(lotNr,"brankPageElements",openmode,'r',subname);

			for (i=0;i<NEWURLFILES_NR;i++) {
				addNewUrlOpen(&argstruct.addNewUrlha[i],lotNr,openmode,subname,i);
			}	

			if (!iintegerOpenForLot(&iinteger,"domainid",sizeof(unsigned short),lotNr, ">>", subname)) {
                		perror("iintegerOpenForLot");
        	        	exit(1);
	        	}		

		#endif

		#ifdef PRESERVE_WORDS
			argstruct.dictionarywordsfFH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords_raw",openmode,'r',subname);
		#endif


		//temp:Søker til problemområdet
		//FileOffset = 334603785;		
		html_parser_init();



		//main work
		argstruct.lotNr 		= lotNr;
		argstruct.subname 		= subname;
        	argstruct.FiltetTime 		= FiltetTime;
        	argstruct.FileOffset 		= FileOffset;
		argstruct.optMaxDocuments	= optMaxDocuments;
		argstruct.optPrintInfo		= optPrintInfo;
		argstruct.optOnlyTLD		= optOnlyTLD;
		argstruct.optMakeWordList 	= optMakeWordList;
		//malloc
		argstruct.DIArray = malloc( NrofDocIDsInLot * sizeof(struct DIArrayFormat) );

		for(i=0;i<NrofDocIDsInLot;i++) {
			//printf("i %i\n",i);
			argstruct.DIArray[i].p = NULL;
			argstruct.DIArray[i].haveawvalue = 0;
			argstruct.DIArray[i].haverankPageElements = 0;
		}

		//init mutex
		#ifdef WITH_THREAD

			pthread_mutex_init(&argstruct.reposetorymutex, NULL);
			pthread_mutex_init(&argstruct.restmutex, NULL);

			if (optNrofWorkThreads == 0) {
				printf("won't use threads\n");
				IndexerLot_workthread(&argstruct);
			}
			else {
				printf("wil use %i threads\n",optNrofWorkThreads);
			
				printf("wil use %i threads\n",optNrofWorkThreads);
				pthread_t threadid[optNrofWorkThreads];

				for(i=0;i<optNrofWorkThreads;i++) {
					printf("starting tread nr %i\n",i);
					n = pthread_create(&threadid[i], NULL, IndexerLot_workthread, &argstruct);
				}

				//venter på trådene
        			for (i=0;i<optNrofWorkThreads;i++) {
                			n = pthread_join(threadid[i], NULL);
        			}
			}
		#else
			printf("wasent build with threads\n");
			IndexerLot_workthread(&argstruct);

		#endif

		
		for(i=0;i<NrofDocIDsInLot;i++) {
			if (argstruct.DIArray[i].p != NULL) {
				
				DIWrite(argstruct.DIArray[i].p,argstruct.DIArray[i].DocID,argstruct.subname);

				free(argstruct.DIArray[i].p);
			}

			#ifndef BLACK_BOKS

			if (argstruct.DIArray[i].haveawvalue == 1) {

				DocIDPlace = ((argstruct.DIArray[i].DocID - LotDocIDOfset(argstruct.lotNr)) * sizeof(unsigned char));

				//printf("DocID %u, DocIDPlace %i\n",argstruct.DIArray[i].DocID,DocIDPlace);
				fseek(argstruct.ADULTWEIGHTFH,DocIDPlace,SEEK_SET);	
               			fwrite(&argstruct.DIArray[i].awvalue,sizeof(unsigned char),1,argstruct.ADULTWEIGHTFH);

				iintegerSetValue(&iinteger,&argstruct.DIArray[i].DomainDI,sizeof(argstruct.DIArray[i].DomainDI),argstruct.DIArray[i].DocID,subname);
			}

			if (argstruct.DIArray[i].haverankPageElements == 1) {

				DocIDPlace = ((argstruct.DIArray[i].DocID - LotDocIDOfset(argstruct.lotNr)) * sizeof(struct brankPageElementsFormat));
				//printf("DocIDPlace %i, size %u\n",DocIDPlace,(unsigned int)sizeof(struct brankPageElementsFormat));
				fseek(argstruct.brankPageElementsFH,DocIDPlace,SEEK_SET);
				fwrite(&argstruct.DIArray[i].brankPageElements,sizeof(struct brankPageElementsFormat),1,argstruct.brankPageElementsFH);
			}
			

			#endif
		}
		

		#ifdef BLACK_BOKS
			alclot_close(argstruct.alclot);
		#else
			fclose(argstruct.ADULTWEIGHTFH);
			fclose(argstruct.SFH);
			fclose(argstruct.brankPageElementsFH);
			iintegerClose(&iinteger);


		#endif

		//skriver riktig indexstide til lotten
		setLastIndexTimeForLot(lotNr,argstruct.httpResponsCodes,subname);
		
		
		#ifdef PRESERVE_WORDS
		fclose(argstruct.dictionarywordsfFH);
		#endif
		// vi må ikke kopiere revindex filene da vi jobber på de lokale direkte
//	}


	printf("indexed %i pages\n\n\n",argstruct.pageCount);

	html_parser_exit();
	 langdetectDestroy();
	free(argstruct.DIArray);
	free(argstruct.adult);

	if (globalIndexerLotConfig.urlfilter != NULL) {
		FreeSplitList(globalIndexerLotConfig.urlfilter);
	}

	return 0;
}


