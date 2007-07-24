
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

#include "../common/adultWeight.h"
#include "../common/DocumentIndex.h"
//#include "../parse_summary/summary.h"
#include "../parser/html_parser.h"
//#include "../parse_summary/highlight.h"
#include "../generateSnippet/snippet.parser.h"
#include "../common/ir.h"
#include "../common/timediff.h"
#include "../common/bstr.h"
#include "../query/query_parser.h"
#include "../common/integerindex.h"

#include "../getdate/getdate.h"


#include "shortenurl.h"

#include "../utf8-filter/utf8-filter.h"

#ifdef WITH_THREAD
        #include <pthread.h>
	#define NROF_GENERATEPAGES_THREADS 5

#endif

#include "searchkernel.h"
#include "../searchFilters/searchFilters.h"

//#include "parseEnv.h"

#include "../boithoadClientLib/liboithoaut.h"
#include "../crawlManager/client.h"

#include "htmlstriper.h"
#include "search.h"

#include "../common/reposetory.h"
//#include "../common/lot.h"

//#include "../common/define.h"

//#include "cgi-util.h"

	//struct iindexFormat *TeffArray; //[maxIndexElements];



void utfclean(char test[],int len) {


	char        *ny = utf8_filter(test);

	strscpy(test,ny,len);

	free(ny);
}

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf, void* wordlist )
{
	//trenger ikke å se på hvert ord her
/*
    if (pos > 25) return;

    printf("\t%s (%i) ", word, pos);

    switch (pu)
        {
            case pu_word: printf("[word]"); break;
            case pu_linkword: printf("[linkword]"); break;
            case pu_link: printf("[link]"); break;
            case pu_baselink: printf("[baselink]"); break;
            case pu_meta_keywords: printf("[meta keywords]"); break;
            case pu_meta_description: printf("[meta description]"); break;
            case pu_meta_author: printf("[meta author]"); break;
            case pu_meta_redirect: printf("[meta redirect]"); break;
            default: printf("[...]");
        }

    switch (puf)
        {
            case puf_none: break;
            case puf_title: printf(" +title"); break;
            case puf_h1: printf(" +h1"); break;
            case puf_h2: printf(" +h2"); break;
            case puf_h3: printf(" +h3"); break;
            case puf_h4: printf(" +h4"); break;
            case puf_h5: printf(" +h5"); break;
            case puf_h6: printf(" +h6"); break;
        }

    printf("\n");
*/
}


enum platform_type
get_platform(char *useragent)
{
	if (strstr(useragent, "Windows"))
		return WINDOWS;
	if (strstr(useragent, "Linux"))
		return UNIX;
	if (strstr(useragent, "Mac OS X"))
		return MAC;
	return UNKNOWN_PLATFORM;
}

enum browser_type
get_browser(char *useragent)
{
	if (strstr(useragent, "MSIE"))
		return IE;
	if (strstr(useragent, "Opera"))
		return OPERA;
	if (strstr(useragent, "Mozilla"))
		return MOZILLA;
	return UNKNOWN_BROWSER;
}

#ifdef BLACK_BOKS
int
handle_url_rewrite(char *url_in, size_t lenin, enum platform_type ptype, enum browser_type btype, char *collection,
           char *url_out, size_t len, int sock, pthread_mutex_t *lock)
{

#ifdef BLACK_BOX
#ifdef WITH_THREAD
	pthread_mutex_lock(lock);
#endif

	cmc_rewrite_url(sock, collection, url_in, lenin, ptype, btype, url_out, len);

#ifdef WITH_THREAD
	pthread_mutex_unlock(lock);
#endif
#endif

	return 1;
}
#endif


int popResult (struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,int antall,unsigned int DocID,
	struct iindexMainElements *TeffArray,struct QueryDataForamt QueryData, char *htmlBuffer,
	unsigned int htmlBufferSize, char servername[],char subname[], unsigned int getRank) {

	int y;
	char        *titleaa, *body, *metakeyw, *metadesc;
	struct ReposetoryHeaderFormat ReposetoryHeader;
	char *acl_allowbuffer = NULL;
	char *acl_deniedbuffer = NULL;
	off_t imagep;

	titleaa = body = metakeyw = metadesc = NULL;
        //alloc on heap, not stack
	//temp, må kalle fre her. Bør ha en fast, ikke kalle hele tiden

	char *strpointer;
   
	int termpos;
	//char queryelement[MaxQueryLen];
	int returnStatus = 0;	
	//for (i = 0; ((i < 10) && (i < antall)); i++) {
	//while (((*SiderHeder).showabal < PagesResults.MaxsHits) && (i < antall)) {


		(*Sider).cacheLink[0] = '\0';


		htmlBuffer[0] = '\0';

			
			//printf("DocID: %i, url: \"%s\"\n",DocID,(*Sider).DocumentIndex.Url);

			//ser om vi har bilde, og at det ikke er på 65535 bytes. (65535  er maks støresle 
			//for unsigned short. Er bilde så stort skyldes det en feil)
			if (((*Sider).DocumentIndex.imageSize != 0) && ((*Sider).DocumentIndex.imageSize != 65535)) {
				
				imagep =  getImagepFromRadres((*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize);
				printf("imakep %u\n",(unsigned int)imagep);
				#ifdef BLACK_BOKS
			sprintf((*Sider).thumbnale,"/cgi-bin/ShowThumbbb?L=%i&amp;P=%u&amp;S=%i&amp;C=%s",
						rLotForDOCid(DocID),
						(unsigned int)imagep,
						(*Sider).DocumentIndex.imageSize,
						subname);

				#else
			sprintf((*Sider).thumbnale,"http://%s/cgi-bin/ShowThumb?L=%i&amp;P=%u&amp;S=%i&amp;C=%s",
						servername,
						rLotForDOCid(DocID),
						(unsigned int)imagep,
						(*Sider).DocumentIndex.imageSize,
						subname);
				#endif
				//sprintf((*Sider).thumbnale,"http://%s/cgi-bin/ShowThumb?L=%i&amp;P=%u&amp;S=%i&amp;C=%s",servername,rLotForDOCid(DocID),4 + sizeof(struct ReposetoryHeaderFormat) + ((*Sider).DocumentIndex.RepositoryPointer + (*Sider).DocumentIndex.htmlSize),(*Sider).DocumentIndex.imageSize,subname);
				(*Sider).thumbnailwidth = 100;
				(*Sider).thumbnailheight = 100;
			}
			else {
				//sprintf((*Sider).thumbnale,"http://www.boitho.com/images/spacer.jpg");
				(*Sider).thumbnale[0] = '\0';
			}

			//tester om vi har en url
			if ((*Sider).DocumentIndex.Url[0] == '\0') {
				printf("Cant read url for %i-%i\n",DocID,rLotForDOCid(DocID));
				//temp: blie denne kalt også ved popresult sjekken i dosearch?
				//(*SiderHeder).filtered++;
				returnStatus = 0;
			}
			//tester om vi har noe innhold
			else if ((*Sider).DocumentIndex.htmlSize == 0) {
				//har ingen reposetroy data, ikke kravlet anda?
				//(*SiderHeder).filtered++;
				
				//setter utlen som title
				
				strscpy((*Sider).title,(*Sider).DocumentIndex.Url,sizeof((*Sider).title));
				(*Sider).DocumentIndex.AdultWeight = 0;
		
				(*Sider).description[0] = '\0'; 
				
				//(*Sider).thumbnale[0] = '\0';
				//printf("teter for inhole: %i rank: %i, i= %i showabal= %i\n",DocID,allrank,i,(*SiderHeder).showabal);
				memcpy(&(*Sider).iindex,TeffArray,sizeof(*TeffArray));
				
				(*SiderHeder).showabal++;
				returnStatus = 1;
			}
			else {
				//int rread (struct ReposetoryFormat *ReposetoryData,unsigned int *radress,unsigned int *rsize,unsigned long DocID)			

											
				htmlBuffer[0] = '\0';
		
				if ((*Sider).DocumentIndex.response == 404) {
					//404 sider kan dukke opp i athor indeksen, da den som kjent er basert på data som ikke ser på om siden er crawlet
					printf("Page has 404 status %i-%i\n",DocID,rLotForDOCid(DocID));
					(*SiderHeder).filtered++;
					returnStatus = 0;
				}
				else if (((*Sider).DocumentIndex.response == 302) || ((*Sider).DocumentIndex.response == 301)) {

					if (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize,DocID,subname,&ReposetoryHeader,&acl_allowbuffer,&acl_deniedbuffer) != 1) {
						//kune ikke lese html. Pointer owerflow ?
						printf("error reding html for %s\n",(*Sider).DocumentIndex.Url);
						sprintf((*Sider).description,"Html error. Can't read html");
						(*Sider).title[0] = '\0';
						(*SiderHeder).showabal++;
						returnStatus = 0;
					}
					else {
										
						for(y=0;y<htmlBufferSize;y++) {
							//printf("1 y: %i of %i\n",y,strlen(htmlBuffer));
							if (!(
								isalnum(htmlBuffer[y])  
								|| (47 ==(int)htmlBuffer[y]) 
								|| (46 ==(int)htmlBuffer[y])
								|| (38 ==(int)htmlBuffer[y]) // &
								|| (63 ==(int)htmlBuffer[y]) // ?
								|| (61 ==(int)htmlBuffer[y]) // =
								|| (58 ==(int)htmlBuffer[y]))) {
								
								htmlBuffer[y] = 'r';
							}
						}
						
						if ((*Sider).DocumentIndex.response == 302) {
							sprintf((*Sider).title,"302 temporarily redirect.");
						}
						else {
							sprintf((*Sider).title,"301 Permanent redirect.");
						}
						//temp: er sider som gir binørutput her, må filtreres
					
						//hvis det er forholdsvis få tegn her så er det nokk en url. Hvis det 
						//er mange er det nokk binær støy
						if (strlen(htmlBuffer) < 100) {
							sprintf((*Sider).description,"Redirecting to %200s",htmlBuffer);
						}
						else {
							//sprintf((*Sider).description,"i %i",htmlBufferSize);
							(*Sider).description[0] = '\0';
						}
						memcpy(&(*Sider).iindex,TeffArray,sizeof(*TeffArray));
						

						(*SiderHeder).showabal++;
						returnStatus = 1;
					}
				}
				else if ((*Sider).DocumentIndex.response == 200) {
			
					
					sprintf((*Sider).cacheLink,"http://%s/cgi-bin/ShowCache?D=%i&amp;P=%u&amp;S=%i&amp;subname=%s",servername,DocID,(*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize,subname);
			
					//printf("cacheLink: %s\n",(*Sider).cacheLink);

					
					if (((*Sider).DocumentIndex.SummaryPointer != 0) && 
							((rReadSummary(DocID,&metadesc, &titleaa,&body,(*Sider).DocumentIndex.SummaryPointer,(*Sider).DocumentIndex.SummarySize,subname) != 0))) {

							printf("hav Summary on disk\n");
	
							//ToDo minne kopiering er kansje ikke det raskeste her
							//strcpy(htmlBuffer,body);
							//htmlBufferSize = strlen(htmlBuffer);
							//printf("titleaa %s\n",titleaa);
						
							//må være med når No title buggen er fikset. Slett dette og hakk ut det over
							//ToDo minne kopiering er kansje ikke det raskeste her
							/*
							//vi bruker ikke html buffer engdernede, men body
							strcpy(htmlBuffer,body);
							htmlBufferSize = strlen(htmlBuffer);
							*/
							//printf("titleaa %s\n",titleaa);

							(*Sider).HtmlPreparsed = 1;

					
						///////////////////////////////////////
					}
					else {

						debug("dont hav Summary on disk. Will hav to read html\n");	
						if (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize,DocID,subname,&ReposetoryHeader,&acl_allowbuffer,&acl_deniedbuffer) != 1) {
							//kune ikke lese html. Pointer owerflow ?
							printf("error reding html for %s\n",(*Sider).DocumentIndex.Url);
							sprintf((*Sider).description,"Html error. Can't read html");
							(*Sider).title[0] = '\0';
							(*SiderHeder).showabal++;
							returnStatus = 1;
							return 0;

						}
						

						//generate_summary( htmlBuffer, htmlBufferSize, &titleaa, &body, 
						//		&metakeyw, &metadesc );

						//void html_parser_run( char *url, char text[], int textsize, char **output_title, 
						//char **output_body,void (*fn), void* wordlist );


						html_parser_run( (*Sider).DocumentIndex.Url, htmlBuffer, htmlBufferSize, 
							&titleaa, &body, fn, NULL);

						
						(*Sider).HtmlPreparsed = 0;

                        	                /*
						//Debug: får å sette vedier riktig hvis man skal hake ut sumery og hilitning
						metadesc = metakeyw = NULL;
	
						titleaa = malloc(512);
						titleaa[0] = '\0';

						body = malloc(512);
						sprintf(body,"aaaaaaaaaa");;						
						*/

					}



					
					
					
					char        *summary;
			                //generate_highlighting( QueryData.queryParsed, body, strlen(body)+1, &summary );
					//temp: bug generate_snippet er ikke ut til å takle og ha en tom body
					if (strlen(body) == 0 || getRank) {
						(*Sider).description[0] = '\0';
					}
					else {

						#ifdef DEBUG
							printf("calling generate_snippet with strlen body %i\n",strlen(body));
						#endif

						generate_snippet( QueryData.queryParsed, body, strlen(body), &summary, "<b>", "</b>" , 160);
					
						//printf("summary len %i\nsummary:\n-%s-\n",strlen(summary),summary);
					
						if (strlen(summary) > (sizeof((*Sider).description) -1) ) {
							sprintf((*Sider).description,"Error: Sumary to large. Was %i but only space for %i.",strlen(summary),sizeof((*Sider).description) -1);
						}
						else {
							strscpy((*Sider).description,summary,sizeof((*Sider).description));
						}
					
						utfclean((*Sider).description,sizeof((*Sider).description));

						//sjekker om vi har en & på slutten. Hvis vi har skal det også være en ;. Dette får å ungå at
						//vi har klart å kappe av før ; en kommer, eks: v Frp&rsquo ...
						char *andcp;
						if ((andcp = strrchr((*Sider).description,'&')) != NULL) {
							if (strchr(andcp,';') == NULL) {
								andcp[0] = '\0';
							}
						}

						free(summary);
					}

					debug("%u -%s-, len %i\n",DocID,titleaa,strlen(titleaa));



					if (titleaa[0] == '\0') {
						sprintf((*Sider).title,"No title");
					}
					else if (strlen(titleaa) > (sizeof((*Sider).title) -1)) {
					    
					    int copylen = (sizeof((*Sider).title) -4);

					    
					    strscpy(&((*Sider).title[0]),titleaa,copylen);
					    
					    //ToDo, underlig nokk finker ikke strncpy her, men kopierer mer en n. 
					    //Nå kopiere manuelt.
					    //har nå begynt å virke ???
					    //magnus forelså &((*Sider).title[0]) i steden for (*Sider).title 
					    //for (y=0;y<copylen;y++) {
					    //	(*Sider).title[y] = titleaa[y];
					    //}
					    //(*Sider).title[y] = '\0';
					    


					    //søker oss til siste space , eller ; og avslutter der
					    if ((strpointer = strrchr((*Sider).title,' ')) != NULL) {
						printf("aa strpointer %u\n",(unsigned int)strpointer);
						//midlertidg fiks på at title altid begynner med space på bb.
						//vil dermed altidd føre til treff i første tegn, og
						// dermed bare vise ".." som title
						if ( ((int)(*Sider).title - (int)strpointer) > 10) {
							strpointer[0] = '\0';
						}
						printf("fant space at %i\n",((int)(*Sider).title - (int)strpointer));
					    }						
					    else if ((strpointer = strrchr((*Sider).title,';')) != NULL) {
						++strpointer; //pekeren peker på semikolonet. SKal ha det med, så må legge il en
						strpointer[0] = '\0';
						
						printf("fant semi colon at %i\n",((int)(*Sider).title - (int)strpointer));
					    }
					    strncat((*Sider).title,"..",2);    

    					    printf("title to long choped. now %i len. size %i\n",strlen((*Sider).title),sizeof((*Sider).title));

					}
					else {
					    strscpy((*Sider).title,titleaa,sizeof((*Sider).title) -1);
					}
					
								
                                        /*
					//Debug: får å sette vedier riktig hvis man skal hake ut sumery og hilitning
					metadesc = metakeyw = body = NULL;
	
					titleaa = malloc(512);
					titleaa[0] = '\0';
					sprintf((*Sider).description,"aaaaaaaaaaa");
					*/


					//frigjør minne
					//printf("frinng buffers from parsers\n");
					
					//temp: ser ut til at vi får problemer her hvis vi har starus "Html error. Can't read"
					if (titleaa != NULL) free(titleaa);
					if (body != NULL) free(body);
					if (metakeyw != NULL) free(metakeyw);
					if (metadesc != NULL) free(metadesc);

					//printf("TermRank %i\n",*TeffArray.TermRank);
					memcpy(&(*Sider).iindex,TeffArray,sizeof(*TeffArray));					
					(*SiderHeder).showabal++;
					returnStatus = 1;

				}
				else {
					//ved 601 og andre feil der vi ikke har crawlet
					printf("Status error for page %i-%i. Status %i\n",DocID,rLotForDOCid(DocID),(*Sider).DocumentIndex.response);
					sprintf((*Sider).title,"%i error.",(*Sider).DocumentIndex.response);
					returnStatus = 0;
				}



					//temp:
					//printf("legger til DocID: %i rank: %i, i= %i showabal= %i\n",DocID,allrank,i,(*SiderHeder).showabal);
					//memcpy(&(*Sider).iindex,&TeffArray,sizeof(TeffArray));
				

			}
						
		//}


	//temp:
	if (acl_allowbuffer != NULL) {free(acl_allowbuffer);}		
	if (acl_deniedbuffer != NULL) {free(acl_deniedbuffer);}		

	return returnStatus;
}
struct PagesResultsFormat {
		struct SiderFormat *Sider;
		struct SiderHederFormat *SiderHeder;
		int antall;
		struct iindexFormat *TeffArray;
		int showabal;
		int nextPage;
		int filterOn;
		int adultpages;
		int noadultpages;
		struct QueryDataForamt QueryData;
		char *servername;
		//int godPages;
		int MaxsHits;
		int start;
		int indexnr;
		int cmcsocketha;
		char search_user[64];
		char password[64];
		char useragent[64];
		struct iintegerMemArrayFormat *DomainIDs;
		int getRank;

		int filtered;
		int memfiltered;

		#ifdef WITH_THREAD
		//pthread_mutexattr_t mutex;
		pthread_mutex_t mutex;
		#endif


		//struct filtypesFormat *filtypes;
		//int filtypesnrof;
};

int nextIndex(struct PagesResultsFormat *PagesResults) {

	int forreturn;

	#ifdef WITH_THREAD
	//printf("nextIndex: waiting for lock: start\n");
	pthread_mutex_lock(&(*PagesResults).mutex);
	//printf("nextIndex: waiting for lock: end\n");
	#endif

	if ((*PagesResults).filtered > 300) {
		debug("nextIndex: filtered (%i) > 300",(*PagesResults).filtered);
		forreturn = -1;
	}
	//mister vi en her? var før >, bytet til >=
	else if ((*PagesResults).indexnr >= (*PagesResults).antall) {
		debug("nextIndex: indexnr (%i) > antall (%i)",(*PagesResults).indexnr,(*PagesResults).antall);

		forreturn = -1;
	}
	else {

		forreturn = (*PagesResults).indexnr++;
	}	

	debug("nextIndex: returning %i as index nr. Nr of hist is %i",forreturn,(*PagesResults).antall);

	#ifdef WITH_THREAD
	//printf("nextIndex: waiting for UNlock: start\n");
	pthread_mutex_unlock(&(*PagesResults).mutex);
	//printf("nextIndex: waiting for UNlock: end\n");
	#endif

	return forreturn;
}

int nextPage(struct PagesResultsFormat *PagesResults) {

	int forreturn;

	#ifdef WITH_THREAD
	//printf("nextPage: waiting for lock: start\n");
	pthread_mutex_lock(&(*PagesResults).mutex);
	//printf("nextPage: waiting for lock: end\n");
	#endif

	//tread lock
	if ((*PagesResults).nextPage >= (*PagesResults).MaxsHits) {
		debug("nextPage: nextPage (%i) >= MaxsHits (%i)",(*PagesResults).nextPage,(*PagesResults).MaxsHits);

		forreturn = -1;
	}
	//mister vi en her? Var før >, men måte elgge til >= får å håntere at vi kan ha 0. Da er 
	//indexnr og antall, like, men ikke støre en. Hvis dette er 0 så skal vi da ikke gå vire. Men hva hvis det
	//er 1?? Vil vi ha 0 først, og det er elemenet 1 ????
	else if ((*PagesResults).indexnr >= (*PagesResults).antall) {
		debug("nextPage: indexnr (%i) > antall (%i)",(*PagesResults).indexnr,(*PagesResults).antall);

		forreturn = -1;
	}
	else {
		forreturn = (*PagesResults).nextPage++;
	}
	
	debug("nextPage: returning %i as next page.",forreturn);

	#ifdef WITH_THREAD
	//printf("nextPage:waiting for UNlock: start\n");
	pthread_mutex_unlock(&(*PagesResults).mutex);
	//printf("nextPage:waiting for UNlock: end\n");

	#endif

	return forreturn;

}

int foundGodPage(struct PagesResultsFormat *PagesResults) {
	#ifdef WITH_THREAD
	//printf("nextPage: waiting for lock: start\n");
	pthread_mutex_lock(&(*PagesResults).mutex);
	//printf("nextPage: waiting for lock: end\n");
	#endif

		printf("this is a good page\n");
		++(*PagesResults).showabal;

	#ifdef WITH_THREAD
	//printf("nextPage:waiting for UNlock: start\n");
	pthread_mutex_unlock(&(*PagesResults).mutex);
	//printf("nextPage:waiting for UNlock: end\n");

	#endif

}
void increaseMemFiltered(struct PagesResultsFormat *PagesResults,int *whichFilterTraped, 
	int *nrInSubname,struct iindexMainElements *iindex) {


	#ifdef WITH_THREAD
	pthread_mutex_lock(&(*PagesResults).mutex);
	#endif

	//++(*(*PagesResults).SiderHeder).filtered;
	++(*PagesResults).memfiltered;

	++(*whichFilterTraped);

	//runarb:1 13 mars. Hvorfor var denne komentert ut???
	//runarb:2 for de vi nå bruker subname som filter
	//runarb:3 kan det være det er nyttig for web søket og ha rikitig tall her? 
	--(*nrInSubname);

	#ifdef BLACK_BOKS
		(*iindex).deleted = 1;
	#endif

	#ifdef WITH_THREAD
	pthread_mutex_unlock(&(*PagesResults).mutex);
	#endif


}
void increaseFiltered(struct PagesResultsFormat *PagesResults,int *whichFilterTraped, 
	int *nrInSubname,struct iindexMainElements *iindex) {


	#ifdef WITH_THREAD
	pthread_mutex_lock(&(*PagesResults).mutex);
	#endif

	//++(*(*PagesResults).SiderHeder).filtered;
	++(*PagesResults).filtered;

	++(*whichFilterTraped);

	//runarb:1 13 mars. Hvorfor var denne komentert ut???
	//runarb:2 for de vi nå bruker subname som filter
	//runarb:3 kan det være det er nyttig for web søket og ha rikitig tall her? 
	--(*nrInSubname);

	#ifdef BLACK_BOKS
		(*iindex).deleted = 1;
	#endif

	#ifdef WITH_THREAD
	pthread_mutex_unlock(&(*PagesResults).mutex);
	#endif


}

#ifdef BLACK_BOKS
int pathaccess(struct PagesResultsFormat *PagesResults, int socketha,char collection_in[], char uri_in[], char user_in[], char password_in[]) {


	int ret = 0;

	#ifdef WITH_THREAD
		pthread_mutex_lock(&(*PagesResults).mutex);
	#endif


	ret = cmc_pathaccess(socketha,collection_in,uri_in,user_in,password_in);

	#ifdef WITH_THREAD
		pthread_mutex_unlock(&(*PagesResults).mutex);
	#endif

	printf("pathaccess: %i\n",ret);

	return ret;


}
#endif
//int generatePagesResults( struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,int antall, 
//struct iindexFormat *TeffArray, int showabal, int filterOn, int adultpages, int noadultpages,struct 
//QueryDataForamt QueryData, char servername[],int godPages,int MaxsHits,int start) 
//void *generatePagesResults(struct PagesResultsFormat *PagesResults) 
void *generatePagesResults(void *arg) 
{

	struct PagesResultsFormat *PagesResults = (struct PagesResultsFormat *)arg;

	int i;
                //ToDo: ikke hardkode her
        unsigned int htmlBufferSize = 900000;

	char *htmlBuffer;
	unsigned short *DomainID;

	struct timeval start_time, end_time;
	int localshowabal;
	//tread lock

#if BLACK_BOKS

	enum platform_type ptype; 
	enum browser_type btype; 

	if (!PagesResults->getRank) {
		ptype = get_platform(PagesResults->useragent);
		btype = get_browser(PagesResults->useragent);
	}

#endif

	if ((htmlBuffer = malloc(htmlBufferSize)) == NULL) {
		perror("can't malloc");
		return;
	}

	#ifdef WITH_THREAD
		pthread_t tid;
		tid = pthread_self();
		printf("is thread id %u\n",(unsigned int)tid);

	#else
		unsigned int tid=0;
	#endif
	//for (i=0;(i<(*PagesResults).antall) && ((*(*PagesResults).SiderHeder).filtered < 300);i++) {
	while ( (localshowabal = nextPage(PagesResults)) != -1 ) {
	debug("localshowabal %i",localshowabal);

	while ((i=nextIndex(PagesResults)) != -1) {
	debug("i %i, DocID %u\n",i,(*PagesResults).TeffArray->iindex[i].DocID);

		//if ((*SiderHeder).filtered > 300) {
		//	(*PagesResults).filterOn = 0;
		//}

		#ifdef BLACK_BOKS
		//hvis index filter tidligere har funet ut at dette ikke er et pra treff går vi til neste
		if ((*PagesResults).TeffArray->iindex[i].indexFiltered.filename == 1) {
			printf("index filtered\n");
			continue;
		}
		if ((*PagesResults).TeffArray->iindex[i].indexFiltered.date == 1) {
			printf("index filtered\n");
			continue;
		}
		if ((*PagesResults).TeffArray->iindex[i].indexFiltered.subname == 1) {
			printf("index filtered\n");
			continue;
		}
		#endif

		#ifndef BLACK_BOKS
		//pre DIread filter
		printf("adult %u: %i\n",(*PagesResults).TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID));
		if (((*PagesResults).filterOn) && (filterAdultWeight_bool(adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID),(*PagesResults).adultpages,(*PagesResults).noadultpages) == 1)) {
			//#ifdef DEBUG
			printf("%u is adult whith %i\n",(*PagesResults).TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID));
			//#endif
			increaseMemFiltered(PagesResults,
				&(*(*PagesResults).SiderHeder).filtersTraped.filterAdultWeight_bool,
				&(*(*PagesResults).TeffArray->iindex[i].subname).hits,
				&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}

		//uanset om vi har filter eller ikke så slår vi opp domainid. Men hvis vi har filter på så teller vi også
		if (!iintegerMemArrayGet ((*PagesResults).DomainIDs,&DomainID,sizeof(*DomainID),(*PagesResults).TeffArray->iindex[i].DocID) ) {
			#ifdef DEBUG
			printf("can't lookup DomainID\n");
			#endif
			(*PagesResults).Sider[localshowabal].DomainID = 0;

			if (((*PagesResults).filterOn)) {
				increaseMemFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.getingDomainID,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

				continue;
			}
		}
		else {
			(*PagesResults).Sider[localshowabal].DomainID = (*DomainID);
			//printf("DomainID %ho\n",(*PagesResults).Sider[localshowabal].DomainID);
			
		}

                if (((*PagesResults).filterOn) && (filterSameDomainID(localshowabal,&(*PagesResults).Sider[localshowabal],(*PagesResults).Sider))) {
			#ifdef DEBUG
			printf("Have same DomainID\n");
			#endif
			increaseMemFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.sameDomainID,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
			
		}

		#endif

		//leser DI
		if (!DIRead(&(*PagesResults).Sider[localshowabal].DocumentIndex,(*PagesResults).TeffArray->iindex[i].DocID,(*(*PagesResults).TeffArray->iindex[i].subname).subname)) 
		{
                        //hvis vi av en eller annen grun ikke kunne gjøre det kalger vi
                        printf("Cant read post for %u-%i\n",(*PagesResults).TeffArray->iindex[i].DocID,rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID));
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cantDIRead,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        continue;
                }

		printf("[tid: %u] looking on  DocID: %u url: \"%s\"\n",(unsigned int)tid,(*PagesResults).TeffArray->iindex[i].DocID,(*PagesResults).Sider[localshowabal].DocumentIndex.Url);

		//adult fra di
		if (((*PagesResults).filterOn) && (filterAdultWeight_value((*PagesResults).Sider[localshowabal].DocumentIndex.AdultWeight,(*PagesResults).adultpages,(*PagesResults).noadultpages)) ) {
			printf("Hav seen url befor. Url \"%s\"\n",(*PagesResults).Sider[localshowabal].DocumentIndex.Url);
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterAdultWeight_value,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        continue;
		}

		//filtrerer ut dublikater fra med crc32 fra DocumentIndex
		if ((*PagesResults).Sider[localshowabal].DocumentIndex.crc32 != 0) {
                       
                        if (filterSameCrc32(localshowabal,&(*PagesResults).Sider[localshowabal],(*PagesResults).Sider)) {
                        	printf("hav same crc32. crc32 from DocumentIndex\n");
				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameCrc32_1,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        	continue;
                        }
                }

		//sjekker tilgang (midlertidig flyttet)
/*
		//int cmc_pathaccess(int socketha,char collection_in[], char uri_in, char user_in[], char password_in[]);

		gettimeofday(&start_time, NULL);
		if (!cmc_pathaccess((*PagesResults).cmcsocketha,(*(*PagesResults).TeffArray->iindex[i].subname).subname,(*PagesResults).Sider[localshowabal].DocumentIndex.Url,search_user,password)) {
			printf("dident hav acces to that one\n");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cmc_pathaccess,&(*(*PagesResults).TeffArray->iindex[i].subname).hits);
			continue;
		}
		gettimeofday(&end_time, NULL);
		(*(*PagesResults).SiderHeder).queryTime.crawlManager += getTimeDifference(&start_time,&end_time);

*/


		#ifndef BLACK_BOKS

		if ((*PagesResults).Sider[localshowabal].DocumentIndex.Url[0] == '\0') {
			printf("DocumentIndex url is emty\n");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterNoUrl,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			continue;
		}

		//fjerner eventuelt like urler
		if (((*PagesResults).filterOn) && (filterSameUrl(localshowabal,(*PagesResults).Sider[localshowabal].DocumentIndex.Url,(*PagesResults).Sider)) ) {
			printf("Hav seen url befor. Url \"%s\"\n",(*PagesResults).Sider[localshowabal].DocumentIndex.Url);
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameUrl,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        continue;
		}

		//finner domene
		if (!find_domain_no_subname((*PagesResults).Sider[localshowabal].DocumentIndex.Url,(*PagesResults).Sider[localshowabal].domain,sizeof((*PagesResults).Sider[localshowabal].domain))) {
		//if (!find_domain_no_www((*PagesResults).Sider[localshowabal].DocumentIndex.Url,(*PagesResults).Sider[localshowabal].domain)) {
			printf("cant find domain. Bad url?. Url \"%s\". localshowabal %i\n",(*PagesResults).Sider[localshowabal].DocumentIndex.Url,localshowabal);
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.find_domain_no_subname,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}

		if (((*PagesResults).filterOn) && (filterSameDomain(localshowabal,&(*PagesResults).Sider[localshowabal],(*PagesResults).Sider))) {
			//#ifdef DEBUG
			printf("hav same domain. Domain: %s. Url %s\n",(*PagesResults).Sider[localshowabal].domain,(*PagesResults).Sider[localshowabal].DocumentIndex.Url);
			//#endif
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameDomain,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;		
		}

		if (((*PagesResults).filterOn) && (filterTLDs((*PagesResults).Sider[localshowabal].domain))) {
			#ifdef DEBUG
			printf("banned TLD\n");
			#endif
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterTLDs,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}
		#endif


		//(*PagesResults).Sider[localshowabal].posisjon = localshowabal +1;

		#ifndef BLACK_BOKS
		//DI filtere
		if (((*PagesResults).filterOn) && (filterResponse((*PagesResults).Sider[localshowabal].DocumentIndex.response) )) {
			debug("bad respons kode %i\n",(*PagesResults).Sider[localshowabal].DocumentIndex.response);
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterResponse,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}
		//if (((*PagesResults).filterOn) && (filterSameIp(localshowabal,&(*PagesResults).Sider[localshowabal],(*PagesResults).Sider))) {
		//	printf("hav same IP\n");
		//	increaseFiltered(PagesResults);
		//	continue;		
		//}
		#endif
		
		if (PagesResults->getRank == 0 && !popResult(&(*PagesResults).Sider[localshowabal], (*PagesResults).SiderHeder,(*PagesResults).antall,(*PagesResults).TeffArray->iindex[i].DocID,&(*PagesResults).TeffArray->iindex[i],(*PagesResults).QueryData,htmlBuffer,htmlBufferSize,(*PagesResults).servername,(*(*PagesResults).TeffArray->iindex[i].subname).subname, PagesResults->getRank)) {
			//#ifdef DEBUG
                        	printf("cant popResult\n");
                        //#endif
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cantpopResult,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;

		}

		gettimeofday(&start_time, NULL);
		#ifdef BLACK_BOKS

		printf("pathaccess: start\n");
		//temp: kortslutter får å implementere sudo. Må implementeres skikkelig, men å spørre boithoad
		if (strcmp((*PagesResults).password,"water66") == 0) {
			printf("pathaccess: have sodo password. Won't do pathaccess\n");
		}
		else if (!pathaccess(PagesResults, (*PagesResults).cmcsocketha,(*(*PagesResults).TeffArray->iindex[i].subname).subname,(*PagesResults).Sider[localshowabal].DocumentIndex.Url,(*PagesResults).search_user,(*PagesResults).password)) {
			printf("dident hav acces to that one\n");

			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cmc_pathaccess,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			//temp:
			//strcpy((*PagesResults).Sider[localshowabal].title,"Access denied!");
			//strcpy((*PagesResults).Sider[localshowabal].description,"");
			continue;

		}
		printf("pathaccess: done\n");

		printf("url rewrite: start\n");

#ifdef BLACK_BOKS
		if (!PagesResults->getRank) {
			handle_url_rewrite(PagesResults->Sider[localshowabal].DocumentIndex.Url, sizeof(PagesResults->Sider[localshowabal].DocumentIndex.Url), ptype, btype, PagesResults->TeffArray->iindex[i].subname->subname, PagesResults->Sider[localshowabal].DocumentIndex.Url, sizeof(PagesResults->Sider[localshowabal].DocumentIndex.Url), PagesResults->cmcsocketha, 
#ifdef WITH_THREAD
			&PagesResults->mutex
#else
			NULL
#endif
			);
		}
#endif

		printf("url rewrite: done\n");

		#endif
		gettimeofday(&end_time, NULL);
		(*(*PagesResults).SiderHeder).queryTime.crawlManager += getTimeDifference(&start_time,&end_time);

		//Så lenge vi ikke har crc32 for alle domener
		if ((*PagesResults).Sider[localshowabal].DocumentIndex.crc32 == 0) {
			(*PagesResults).Sider[localshowabal].DocumentIndex.crc32 = crc32boitho(htmlBuffer);
		

			if (((*PagesResults).filterOn) && (filterSameCrc32(localshowabal,&(*PagesResults).Sider[localshowabal],(*PagesResults).Sider))) {
	                      	printf("hav same crc32\n");
				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameCrc32_2,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                	      	continue;
                	}
		}
		///////////

		if (1 || !PagesResults->getRank) {
			strscpy((*PagesResults).Sider[localshowabal].uri,(*PagesResults).Sider[localshowabal].DocumentIndex.Url,sizeof((*PagesResults).Sider[localshowabal].uri));
			strscpy((*PagesResults).Sider[localshowabal].url,(*PagesResults).Sider[localshowabal].DocumentIndex.Url,sizeof((*PagesResults).Sider[localshowabal].url));

			(*PagesResults).Sider[localshowabal].pathlen = find_domain_path_len((*PagesResults).Sider[localshowabal].uri);

			shortenurl((*PagesResults).Sider[localshowabal].uri,sizeof((*PagesResults).Sider[localshowabal].uri));

			strcpy((*PagesResults).Sider[localshowabal].servername,(*PagesResults).servername);
		}

		//kopierer over subname
		(*PagesResults).Sider[localshowabal].subname = (*(*PagesResults).TeffArray->iindex[i].subname);

		(*PagesResults).Sider[localshowabal].posisjon = localshowabal +1;


		(*PagesResults).Sider[localshowabal].bid = 0;


		(*PagesResults).Sider[localshowabal].type = siderType_normal;
		
		(*PagesResults).Sider[localshowabal].deletet = 0;


/*
temp: 25 des 2006
		if ((*PagesResults).godPages >= ((*PagesResults).start * (*PagesResults).MaxsHits)) {
			++(*PagesResults).showabal;
		}
		
		++(*PagesResults).godPages;
*/
		//gjør post god side ting, som å øke showabal
		foundGodPage(PagesResults);

		break; //går ut av loopen. Vi har funnet at vår index hit var brukenes, vi trenger da en ny side
	}
	}

	//printf("******************************\nfreeing htmlBuffer\n******************************\n");
	free(htmlBuffer);

	//return 1;
}

int dosearch(char query[], int queryLen, struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,
char *hiliteQuery, char servername[], struct subnamesFormat subnames[], int nrOfSubnames, 
int MaxsHits, int start, int filterOn, char languageFilter[],char orderby[],int dates[], 
char search_user[],struct filtersFormat *filters,struct searchd_configFORMAT *searchd_config, char *errorstr,int *errorLen,
	struct iintegerMemArrayFormat *DomainIDs, char *useragent
	) { 

	struct PagesResultsFormat PagesResults;
	struct filteronFormat filteron;

	PagesResults.Sider = Sider;
	PagesResults.SiderHeder = SiderHeder;
	PagesResults.antall = 0;
	//PagesResults.TeffArray;
	//PagesResults.showabal;
	PagesResults.filterOn = filterOn;
	//PagesResults.adultpages
	//PagesResults.noadultpages
	//PagesResults.QueryData
	PagesResults.servername = servername;
	//PagesResults.godPages
	PagesResults.MaxsHits = MaxsHits;
	PagesResults.start = start;
	//hvr vi skal begynne. Vå bruker dog navn som 1, 2 osv til brukeren, men starter på 0 internt 
	//dette har dog dispatsher_all allerede håntert, ved å trekke fra en
	PagesResults.indexnr = (start * MaxsHits); 
	strscpy(PagesResults.search_user,search_user,sizeof(PagesResults.search_user));
	//PagesResults.password
	PagesResults.DomainIDs = DomainIDs;
	PagesResults.filtered		= 0;
	PagesResults.memfiltered	= 0;	
	PagesResults.getRank		= 0;
	strcpy(PagesResults.useragent, useragent);


         (*SiderHeder).filtersTraped.cantDIRead = 0;
         (*SiderHeder).filtersTraped.getingDomainID = 0;
         (*SiderHeder).filtersTraped.sameDomainID = 0;

         (*SiderHeder).filtersTraped.filterAdultWeight_bool = 0;
         (*SiderHeder).filtersTraped.filterAdultWeight_value = 0;
         (*SiderHeder).filtersTraped.filterSameCrc32_1 = 0;
         (*SiderHeder).filtersTraped.filterSameUrl = 0;
         (*SiderHeder).filtersTraped.find_domain_no_subname = 0;
         (*SiderHeder).filtersTraped.filterSameDomain = 0;
         (*SiderHeder).filtersTraped.filterTLDs = 0;
         (*SiderHeder).filtersTraped.filterResponse = 0;
         (*SiderHeder).filtersTraped.cantpopResult = 0;
         (*SiderHeder).filtersTraped.cmc_pathaccess = 0;
         (*SiderHeder).filtersTraped.filterSameCrc32_2 = 0;
         (*SiderHeder).filtersTraped.filterNoUrl = 0;

	//struct iindexFormat *TeffArray;

	#ifdef BLACK_BOKS
	searchFilterInit(filters,dates);
	#endif

	//int godPages; //antal sider som kan vises. (ikke det samme som antal sider som faktisk skal vises )
	int readedFromIndex;
	int i, y, j;
	int x;

	PagesResults.TeffArray = malloc(sizeof(struct iindexFormat));

	//int adultpages, noadultpages;
	//struct QueryDataForamt QueryData;

	#ifdef DEBUG
		memset(&PagesResults.QueryData,' ',sizeof(PagesResults.QueryData));
	#endif

	struct timeval start_time, end_time;
	struct timeval popResult_start_time, popResult_end_time;

	int rank;
	int ret;

	//7int filtered; //antal sider som ble filtrert ut
	//int showabal; //holder antal sider som kunne vises

	struct DocumentIndexFormat DocumentIndexPost;
	
	//struct ReposetoryFormat ReposetoryData;
	//char ReposetoryHtml[10][5000];

	//5: printf("Start time was %d uSec.\n", start_time.tv_usec);

	//7int TotaltTreff;
	
	//char buff[64]; //generell buffer

	#ifdef BLACK_BOKS

		//henter brukerens passord fra boithoad
		gettimeofday(&start_time, NULL);
		//henter inn brukerens passord
		printf("geting pw for \"%s\"\n",PagesResults.search_user);
		if (!boithoad_getPassword(PagesResults.search_user,PagesResults.password)) {
			//printf("Can't boithoad_getPassword. Brukeren er ikke logget inn??\n");
			(*errorLen) = snprintf(errorstr,(*errorLen),"Can't get user info from authentication backend");
			return(0);
		}
		//printf("got pw \"%s\" -> \"%s\"\n",PagesResults.search_user,PagesResults.password);
		gettimeofday(&end_time, NULL);
	        (*SiderHeder).queryTime.getUserObjekt = getTimeDifference(&start_time,&end_time);

		/****************************************************************/
		//hent alle grupper
		char **groups_respons_list;
		int groups_responsnr;
		char groupOrQuery[1024];

		//boithoad_listGroups(&groups_respons_list,&groups_responsnr);
		if (!boithoad_groupsForUser(PagesResults.search_user,&groups_respons_list,&groups_responsnr)) {
                        perror("Error: boithoad_groupsForUser");
                        return 0;
                }
		groupOrQuery[0] = '\0';
                printf("groups: %i\n",groups_responsnr);
                for (i=0;i<groups_responsnr;i++) {

			strsandr(groups_respons_list[i]," ","X");

                        printf("group: %s\n",groups_respons_list[i]);

			strlcat(groupOrQuery," |",sizeof(groupOrQuery));
			strlcat(groupOrQuery,groups_respons_list[i],sizeof(groupOrQuery));

                }

		//legger til brukernavnet
		strlcat(groupOrQuery," |",sizeof(groupOrQuery));
		strlcat(groupOrQuery,PagesResults.search_user,sizeof(groupOrQuery));

		printf("groupOrQuery \"%s\"\n",groupOrQuery);


		/****************************************************************/


		gettimeofday(&start_time, NULL);
		//int socketha;
		int errorbufflen = 512;
		char errorbuff[errorbufflen];
		printf("making a connection to crawlerManager\n");
		if (!cmc_conect(&PagesResults.cmcsocketha,errorbuff,errorbufflen,(*searchd_config).cmc_port)) {
                        //printf("Error: %s:%i\n",errorbuff,(*searchd_config).cmc_port);
			(*errorLen) = snprintf(errorstr,(*errorLen),"Can't connect to crawler manager: \"%s\", port %i",errorbuff,(*searchd_config).cmc_port);

                        return(0);
	        }
		gettimeofday(&end_time, NULL);
	        (*SiderHeder).queryTime.cmc_conect = getTimeDifference(&start_time,&end_time);

		(*SiderHeder).queryTime.crawlManager = 0;

	#else
		(*SiderHeder).queryTime.cmc_conect = 0;
		(*SiderHeder).queryTime.crawlManager = 0;
	#endif


	//kopierer query inn i strukturen som holder query date
	strscpy(PagesResults.QueryData.query,query,sizeof(PagesResults.QueryData.query));

	get_query( PagesResults.QueryData.query, queryLen, &PagesResults.QueryData.queryParsed );

	#ifdef BLACK_BOKS
	get_query( groupOrQuery, strlen(groupOrQuery), &PagesResults.QueryData.search_user_as_query );
	#endif


        printf("ll: query %s\n",PagesResults.QueryData.query);


/*
//temp: skrur av detektering av stopord
    for (i=0; i<PagesResults.QueryData.queryParsed.size; i++)
        {
            struct text_list *t_it = PagesResults.QueryData.queryParsed.elem[i];

            printf("(%c)", PagesResults.QueryData.queryParsed.operand[i] );

            while ( t_it!=NULL )
                {
                    printf(" %s", t_it->text);

                        if (isStoppWord(t_it->text)) {
                                printf("fant stoppord 1\n");
                                t_it->stopword = 1;
                        }
                        else {
                                t_it->stopword = 0;
                        }

                    t_it = t_it->next;
                }

            printf("\n");
        }

*/
	int languageFilterAsNr[5];

	int languageFilternr;

	if (languageFilter[0] != '\0') {
		printf("languageFilter: %s\n",languageFilter);
		languageFilterAsNr[0] = getLangNr(languageFilter);
		languageFilternr = 1;
	}
	else {
		languageFilternr = 0;
	}

	// temp:
	int h;
	for (h=0;h<languageFilternr;h++) {
		printf("ll %i\n",languageFilterAsNr[h]);
	}
	printf("languageFilternr %i\n",languageFilternr);
	// :temp

	#ifdef WITH_THREAD
	pthread_t threadid[NROF_GENERATEPAGES_THREADS];

	//ret = pthread_mutexattr_init(&PagesResults.mutex);
	ret = pthread_mutex_init(&PagesResults.mutex, NULL);

	//låser mutex. Vi er jo enda ikke kalre til å kjøre
	pthread_mutex_lock(&PagesResults.mutex);

	//start som thread that can get the pages
	for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
		//pthread_create(&chld_thr, NULL, do_chld, (void *) newsockfd);
		ret = pthread_create(&threadid[i], NULL, generatePagesResults, &PagesResults);		
	}
	#endif

	printf("searchSimple\n");
	
	searchSimple(&PagesResults.antall,PagesResults.TeffArray,&(*SiderHeder).TotaltTreff,
			&PagesResults.QueryData.queryParsed,&(*SiderHeder).queryTime,
			subnames,nrOfSubnames,languageFilternr,languageFilterAsNr,
			orderby,
			filters,&filteron,&PagesResults.QueryData.search_user_as_query);

	printf("end searchSimple\n");

	/*
	for(i=0;i<PagesResults.antall;i++) {
		printf("index DociD %u\n",PagesResults.TeffArray[i].DocID);
	}
	*/

	//intresnag debug info
	#ifdef BLACK_BOKS
	//viser hvordan treffene er i subnames
		printf("subname records:\n");
		for (i=0;i<nrOfSubnames;i++) {
			printf("\t\"%s\": %i\n",subnames[i].subname,subnames[i].hits);
		}
	#endif

	//setter alle sidene som sletett
	for (i=0;i<PagesResults.MaxsHits;i++) {
		Sider[i].deletet = 1;
	}

       	// vi bruker ikke denne mer før siden. Vi henter verdien til å lagre i den fra   PagesResults.showabal
	//(*SiderHeder).showabal = 0;

       	readedFromIndex = 0;
	y=0;
       	(*SiderHeder).filtered = 0;

	gettimeofday(&start_time, NULL);
	//kalkulerer antall adult sier i første n resultater
	PagesResults.noadultpages = 0;
	PagesResults.adultpages = 0;
	for (i=0;((i<100) && (i<PagesResults.antall));i++) {

		//printf("DocID %u, aw %i\n",PagesResults.TeffArray[i].DocID,adultWeightForDocIDMemArray(PagesResults.TeffArray[i].DocID));
		if (adultWeightForDocIDMemArray(PagesResults.TeffArray->iindex[i].DocID)) {
			++PagesResults.adultpages;	
		}
		else {
			++PagesResults.noadultpages;
		}
	}
	printf("noadultpages %i, PagesResults.adultpages %i\n",PagesResults.noadultpages,PagesResults.adultpages);
	gettimeofday(&end_time, NULL);
        (*SiderHeder).queryTime.adultcalk = getTimeDifference(&start_time,&end_time);
	////////

	//går i utbangspungete gjenon alle sider, men eskaper når vi når anatllet vi vil ha
	PagesResults.showabal = 0;
	//PagesResults.godPages = 0;
	PagesResults.nextPage = 0;

//cuted
	gettimeofday(&popResult_start_time, NULL);



	#ifdef WITH_THREAD

	//vi har data. Lå tårdene jobbe med det
	pthread_mutex_unlock(&PagesResults.mutex);

	//venter på trådene
	for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
		ret = pthread_join(threadid[i], NULL);
	}

	//free mutex'en
	ret = pthread_mutex_destroy(&PagesResults.mutex);

	#else 
		generatePagesResults(&PagesResults);		
	#endif

	#ifdef BLACK_BOKS
		cmc_close(PagesResults.cmcsocketha);
	#endif


	(*SiderHeder).showabal = PagesResults.showabal;

	//lager en filtered verdi
	(*SiderHeder).filtered = PagesResults.filtered + PagesResults.memfiltered;	


	gettimeofday(&popResult_end_time, NULL);
        (*SiderHeder).queryTime.popResult = getTimeDifference(&popResult_start_time,&popResult_end_time);

	
	//5: printf("end\n");
	


	//teller opp filteree
	/*
searchSimple(&PagesResults.antall,PagesResults.TeffArray,&(*SiderHeder).TotaltTreff,
                        &PagesResults.QueryData.queryParsed,&(*SiderHeder).queryTime,
                        subnames,nrOfSubnames,languageFilternr,languageFilterAsNr,
                        orderby,dates,
                        filters)
	*/

	#ifdef BLACK_BOKS
		searchFilterCount(&PagesResults.antall,PagesResults.TeffArray,filters,subnames,nrOfSubnames,&filteron,dates,&(*SiderHeder).queryTime);
	
	#endif



	//fjerner filtered fra total oversikten
	//ToDo: bør dette også gjøres for web?
	(*SiderHeder).TotaltTreff = (*SiderHeder).TotaltTreff - (*SiderHeder).filtered;

	//lager en liste med ordene som ingikk i queryet til hiliting
	hiliteQuery[0] = '\0';
	//printf("size %i\n",PagesResults.QueryData.queryParsed.size);

	printf("hiliteQuery\n");
	//x=0;
	//for (y=0; y<PagesResults.QueryData.queryParsed.size; y++) {
	//	struct text_list *t_it = PagesResults.QueryData.queryParsed.elem[y];
	for (i=0; i<PagesResults.QueryData.queryParsed.n; i++) {

		//while ( t_it!=NULL )
		for (j=0; j<PagesResults.QueryData.queryParsed.query[i].n; j++)
                {

			//if (!t_it->stopword) {
                		//ToDo vanlig strcat gir fare for buffer overflov her
				//strncat(hiliteQuery,t_it->text,sizeof(*hiliteQuery) -1); 
		        	strcat(hiliteQuery,PagesResults.QueryData.queryParsed.query[i].s[j]);      	

				//appender et komma, slik at vi får en komma separert liste med ord
				strncat(hiliteQuery,",",sizeof(*hiliteQuery) -1);		
				strcat(hiliteQuery,",");
			//}

			//t_it = t_it->next;

			//temp: midlertidig fiks får å slippe problemer med querys som har mange \\\\\\ i seg
			//if (x>10) {
			//	break;
			//}
			//++x;
		}

	}
	printf("hiliteQuery END\n");

	//det vil bli et komma for mye på slutten, fjerner det.
	//ToDo: er det altid et komma for mye ?	
	if (hiliteQuery[strlen(hiliteQuery) -1] == ',') {hiliteQuery[strlen(hiliteQuery) -1] = '\0';}

	printf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" filtered=\"%i\" showabal=\"%i\"/>\n",(*SiderHeder).TotaltTreff,PagesResults.QueryData.query,hiliteQuery,(*SiderHeder).filtered,(*SiderHeder).showabal);
	
	printf("free TeffArray\n");
	free(PagesResults.TeffArray);

	destroy_query( &PagesResults.QueryData.queryParsed );


	//printer ut info om brukt tid
	printf("Time\n");
	//printf("\tAthorSearch %f\n",(*SiderHeder).queryTime.AthorSearch);
	printf("\t%-40s %f\n","AthorSearch",(*SiderHeder).queryTime.AthorSearch);
	printf("\t%-40s %f\n","AthorRank",(*SiderHeder).queryTime.AthorRank);
	printf("\t%-40s %f\n","MainSearch",(*SiderHeder).queryTime.MainSearch);
	printf("\t%-40s %f\n","MainRank",(*SiderHeder).queryTime.MainRank);
	printf("\t%-40s %f\n","MainAthorMerge",(*SiderHeder).queryTime.MainAthorMerge);
	printf("\n");
	printf("\t%-40s %f\n","popRank",(*SiderHeder).queryTime.popRank);
	printf("\t%-40s %f\n","responseShortning",(*SiderHeder).queryTime.responseShortning);
	printf("\n");
	printf("\t%-40s %f\n","allrankCalc",(*SiderHeder).queryTime.allrankCalc);
	printf("\t%-40s %f\n","indexSort",(*SiderHeder).queryTime.indexSort);
	printf("\t%-40s %f\n","popResult",(*SiderHeder).queryTime.popResult);
	printf("\t%-40s %f\n","adultcalk",(*SiderHeder).queryTime.adultcalk);

	

	#ifdef BLACK_BOKS
	printf("\tfiletypes %f\n",(*SiderHeder).queryTime.filetypes);
	printf("\tiintegerGetValueDate %f\n",(*SiderHeder).queryTime.iintegerGetValueDate);
	printf("\tdateview %f\n",(*SiderHeder).queryTime.dateview);
	printf("\tcrawlManager %f\n",(*SiderHeder).queryTime.crawlManager);

	printf("\tgetUserObjekt %f\n",(*SiderHeder).queryTime.getUserObjekt);
	printf("\tcmc_conect %f\n",(*SiderHeder).queryTime.cmc_conect);

	#endif

	printf("filters:\n");

	printf("\t%-40s %i\n","cantDIRead",(*SiderHeder).filtersTraped.cantDIRead);
	printf("\t%-40s %i\n","getingDomainID",(*SiderHeder).filtersTraped.getingDomainID);
	printf("\t%-40s %i\n","sameDomainID",(*SiderHeder).filtersTraped.sameDomainID);

	printf("\t%-40s %i\n","filterAdultWeight_bool",(*SiderHeder).filtersTraped.filterAdultWeight_bool);
	printf("\t%-40s %i\n","filterAdultWeight_value",(*SiderHeder).filtersTraped.filterAdultWeight_value);
	printf("\t%-40s %i\n","filterSameCrc32_1",(*SiderHeder).filtersTraped.filterSameCrc32_1);
	printf("\t%-40s %i\n","filterSameUrl",(*SiderHeder).filtersTraped.filterSameUrl);
	printf("\t%-40s %i\n","filterNoUrl",(*SiderHeder).filtersTraped.filterNoUrl);
	printf("\t%-40s %i\n","find_domain_no_subname",(*SiderHeder).filtersTraped.find_domain_no_subname);
	printf("\t%-40s %i\n","filterSameDomain",(*SiderHeder).filtersTraped.filterSameDomain);
	printf("\t%-40s %i\n","filterTLDs",(*SiderHeder).filtersTraped.filterTLDs);
	printf("\t%-40s %i\n","filterResponse",(*SiderHeder).filtersTraped.filterResponse);
	printf("\t%-40s %i\n","cantpopResult",(*SiderHeder).filtersTraped.cantpopResult);
	printf("\t%-40s %i\n","cmc_pathaccess",(*SiderHeder).filtersTraped.cmc_pathaccess);
	printf("\t%-40s %i\n","filterSameCrc32_2",(*SiderHeder).filtersTraped.filterSameCrc32_2);

	printf("\n\n");

	#ifdef EXPLAIN_RANK
	printf("|%-10s|%-10s|%-10s||%-10s|%-10s|%-10s|%-25s|%-10s|%-10s|\n",
		"AllRank",
		"TermRank",
		"PopRank",
		"Body",
		"Headline",
		"Tittel",
		"Athor (nr nrp)",
		"UrlM",
		"Url"
		);
	printf("|----------|----------|----------||----------|----------|----------|------------------------|----------|----------|\n");

	for(i=0;i<(*SiderHeder).showabal;i++) {
                        printf("|%10i|%10i|%10i||%10i|%10i|%10i|%10i (%5i %5i)|%10i|%10i| %s (DocID %u-%i)\n",

				Sider[i].iindex.allrank,
				Sider[i].iindex.TermRank,
				Sider[i].iindex.PopRank,

				Sider[i].iindex.rank_explaind.rankBody,
				Sider[i].iindex.rank_explaind.rankHeadline,
				Sider[i].iindex.rank_explaind.rankTittel,
				Sider[i].iindex.rank_explaind.rankAthor,
				Sider[i].iindex.rank_explaind.nrAthor,
				Sider[i].iindex.rank_explaind.nrAthorPhrase,
				Sider[i].iindex.rank_explaind.rankUrl_mainbody,
				Sider[i].iindex.rank_explaind.rankUrl,
				Sider[i].DocumentIndex.Url,
				Sider[i].iindex.DocID,
				rLotForDOCid(Sider[i].iindex.DocID)
				);
	}
	#endif

	return 1;
}



int
dorank(char query[], int queryLen, struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,
char *hiliteQuery, char servername[], struct subnamesFormat subnames[], int nrOfSubnames, 
int MaxsHits, int start, int filterOn, char languageFilter[],char orderby[],int dates[], 
char search_user[],struct filtersFormat *filters,struct searchd_configFORMAT *searchd_config, char *errorstr,int *errorLen,
	struct iintegerMemArrayFormat *DomainIDs, int rankType, unsigned int rankDocId, int *ranking)
{ 
	struct PagesResultsFormat PagesResults;
	struct filteronFormat filteron;

	PagesResults.Sider = Sider;
	PagesResults.SiderHeder = SiderHeder;
	PagesResults.antall = 0;
	//PagesResults.TeffArray;
	//PagesResults.showabal;
	PagesResults.filterOn = filterOn;
	//PagesResults.adultpages
	//PagesResults.noadultpages
	//PagesResults.QueryData
	PagesResults.servername = servername;
	//PagesResults.godPages
	PagesResults.MaxsHits = MaxsHits;
	PagesResults.start = 0;
	//hvr vi skal begynne. Vå bruker dog navn som 1, 2 osv til brukeren, men starter på 0 internt 
	//dette har dog dispatsher_all allerede håntert, ved å trekke fra en
	PagesResults.indexnr = 0;//(start * MaxsHits); 
	strscpy(PagesResults.search_user,search_user,sizeof(PagesResults.search_user));
	//PagesResults.password
	PagesResults.DomainIDs = DomainIDs;
	PagesResults.filtered		= 0;
	PagesResults.memfiltered	= 0;	
	PagesResults.getRank		= 1;


         (*SiderHeder).filtersTraped.cantDIRead = 0;
         (*SiderHeder).filtersTraped.getingDomainID = 0;
         (*SiderHeder).filtersTraped.sameDomainID = 0;

         (*SiderHeder).filtersTraped.filterAdultWeight_bool = 0;
         (*SiderHeder).filtersTraped.filterAdultWeight_value = 0;
         (*SiderHeder).filtersTraped.filterSameCrc32_1 = 0;
         (*SiderHeder).filtersTraped.filterSameUrl = 0;
         (*SiderHeder).filtersTraped.find_domain_no_subname = 0;
         (*SiderHeder).filtersTraped.filterSameDomain = 0;
         (*SiderHeder).filtersTraped.filterTLDs = 0;
         (*SiderHeder).filtersTraped.filterResponse = 0;
         (*SiderHeder).filtersTraped.cantpopResult = 0;
         (*SiderHeder).filtersTraped.cmc_pathaccess = 0;
         (*SiderHeder).filtersTraped.filterSameCrc32_2 = 0;
         (*SiderHeder).filtersTraped.filterNoUrl = 0;

	//struct iindexFormat *TeffArray;

	#ifdef BLACK_BOKS
	searchFilterInit(filters,dates);
	#endif

	//int godPages; //antal sider som kan vises. (ikke det samme som antal sider som faktisk skal vises )
	int readedFromIndex;
	int i, y, j;
	int x;

	PagesResults.TeffArray = malloc(sizeof(struct iindexFormat));

	//int adultpages, noadultpages;
	//struct QueryDataForamt QueryData;

	#ifdef DEBUG
		memset(&PagesResults.QueryData,' ',sizeof(PagesResults.QueryData));
	#endif

	struct timeval start_time, end_time;
	struct timeval popResult_start_time, popResult_end_time;

	int rank;
	int ret;

	//7int filtered; //antal sider som ble filtrert ut
	//int showabal; //holder antal sider som kunne vises

	struct DocumentIndexFormat DocumentIndexPost;
	
	//struct ReposetoryFormat ReposetoryData;
	//char ReposetoryHtml[10][5000];

	//5: printf("Start time was %d uSec.\n", start_time.tv_usec);

	//7int TotaltTreff;
	
	//char buff[64]; //generell buffer

 /* False */
	#ifdef BLACK_BOKS
		//henter brukerens passord fra boithoad
		gettimeofday(&start_time, NULL);
		//henter inn brukerens passord
		printf("geting pw for \"%s\"\n",PagesResults.search_user);
		if (!boithoad_getPassword(PagesResults.search_user,PagesResults.password)) {
			//printf("Can't boithoad_getPassword. Brukeren er ikke logget inn??\n");
			(*errorLen) = snprintf(errorstr,(*errorLen),"Can't get user info from authentication backend");
			return(0);
		}
		//printf("got pw \"%s\" -> \"%s\"\n",PagesResults.search_user,PagesResults.password);
		gettimeofday(&end_time, NULL);
	        (*SiderHeder).queryTime.getUserObjekt = getTimeDifference(&start_time,&end_time);

		/****************************************************************/
		//hent alle grupper
		char **groups_respons_list;
		int groups_responsnr;
		char groupOrQuery[1024];

		//boithoad_listGroups(&groups_respons_list,&groups_responsnr);
		if (!boithoad_groupsForUser(PagesResults.search_user,&groups_respons_list,&groups_responsnr)) {
                        perror("Error: boithoad_groupsForUser");
                        return;
                }
		groupOrQuery[0] = '\0';
                printf("groups: %i\n",groups_responsnr);
                for (i=0;i<groups_responsnr;i++) {
                        printf("group: %s\n",groups_respons_list[i]);

			strlcat(groupOrQuery," |",sizeof(groupOrQuery));
			strlcat(groupOrQuery,groups_respons_list[i],sizeof(groupOrQuery));
                }

		//legger til brukernavnet
		strlcat(groupOrQuery," |",sizeof(groupOrQuery));
		strlcat(groupOrQuery,PagesResults.search_user,sizeof(groupOrQuery));

		printf("groupOrQuery \"%s\"\n",groupOrQuery);
		/****************************************************************/


		gettimeofday(&start_time, NULL);
		//int socketha;
		int errorbufflen = 512;
		char errorbuff[errorbufflen];
		printf("making a connection to crawlerManager\n");
		if (!cmc_conect(&PagesResults.cmcsocketha,errorbuff,errorbufflen,(*searchd_config).cmc_port)) {
                        //printf("Error: %s:%i\n",errorbuff,(*searchd_config).cmc_port);
			(*errorLen) = snprintf(errorstr,(*errorLen),"Can't connect to crawler manager: \"%s\", port %i",errorbuff,(*searchd_config).cmc_port);

                        return(0);
	        }
		gettimeofday(&end_time, NULL);
	        (*SiderHeder).queryTime.cmc_conect = getTimeDifference(&start_time,&end_time);

		(*SiderHeder).queryTime.crawlManager = 0;

	#else
		(*SiderHeder).queryTime.cmc_conect = 0;
		(*SiderHeder).queryTime.crawlManager = 0;
	#endif


	//kopierer query inn i strukturen som holder query date
	strscpy(PagesResults.QueryData.query,query,sizeof(PagesResults.QueryData.query));

	get_query( PagesResults.QueryData.query, queryLen, &PagesResults.QueryData.queryParsed );

	#ifdef BLACK_BOKS
	get_query( groupOrQuery, strlen(groupOrQuery), &PagesResults.QueryData.search_user_as_query );
	#endif


        printf("ll: query %s\n",PagesResults.QueryData.query);

	int languageFilterAsNr[5];

	int languageFilternr;

	if (languageFilter[0] != '\0') {
		printf("languageFilter: %s\n",languageFilter);
		languageFilterAsNr[0] = getLangNr(languageFilter);
		languageFilternr = 1;
	}
	else {
		languageFilternr = 0;
	}


	#ifdef WITH_THREAD
	pthread_t threadid[NROF_GENERATEPAGES_THREADS];

	//ret = pthread_mutexattr_init(&PagesResults.mutex);
	ret = pthread_mutex_init(&PagesResults.mutex, NULL);

	//låser mutex. Vi er jo enda ikke kalre til å kjøre
	pthread_mutex_lock(&PagesResults.mutex);


	//start som thread that can get the pages
	for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
		//pthread_create(&chld_thr, NULL, do_chld, (void *) newsockfd);
		ret = pthread_create(&threadid[i], NULL, generatePagesResults, &PagesResults);		
	}
	#endif

	printf("searchSimple\n");
	
	searchSimple(&PagesResults.antall,PagesResults.TeffArray,&SiderHeder->TotaltTreff,
			&PagesResults.QueryData.queryParsed,&SiderHeder->queryTime,
			subnames,nrOfSubnames,languageFilternr,languageFilterAsNr,
			orderby,
			filters,&filteron,&PagesResults.QueryData.search_user_as_query);
	//&rankDocId);

	printf("end searchSimple\n");

	printf("rankDocId: %d\n", rankDocId);
	if (rankType == RANK_TYPE_FIND) {
		*ranking = -1;
		printf("Some document: %d\n", PagesResults.TeffArray->iindex[i].DocID);
		for (i = 0; i < PagesResults.antall; i++) {
			//printf("Found document: %d\n", PagesResults.TeffArray->iindex[i].DocID);
			if (rankDocId == PagesResults.TeffArray->iindex[i].DocID) {
				*ranking = PagesResults.TeffArray->iindex[i].allrank;
				printf("This is actually what we wanted rank is: %d\n", PagesResults.TeffArray->iindex[i].allrank);
				break;
			}
		}
	}
	//intresnag debug info
	#ifdef BLACK_BOKS
	//viser hvordan treffene er i subnames
		printf("subname records:\n");
		for (i=0;i<nrOfSubnames;i++) {
			printf("\t\"%s\": %i\n",subnames[i].subname,subnames[i].hits);
		}
	#endif

	//setter alle sidene som sletett
	for (i=0;i<PagesResults.MaxsHits;i++) {
		Sider[i].deletet = 1;
	}

       	readedFromIndex = 0;
	y=0;
       	(*SiderHeder).filtered = 0;

	gettimeofday(&start_time, NULL);
	//kalkulerer antall adult sier i første n resultater
	PagesResults.noadultpages = 0;
	PagesResults.adultpages = 0;
	for (i=0;((i<100) && (i<PagesResults.antall));i++) {

		//printf("DocID %u, aw %i\n",PagesResults.TeffArray[i].DocID,adultWeightForDocIDMemArray(PagesResults.TeffArray[i].DocID));
		if (adultWeightForDocIDMemArray(PagesResults.TeffArray->iindex[i].DocID)) {
			++PagesResults.adultpages;	
		}
		else {
			++PagesResults.noadultpages;
		}
	}
	printf("noadultpages %i, PagesResults.adultpages %i\n",PagesResults.noadultpages,PagesResults.adultpages);
	gettimeofday(&end_time, NULL);
        (*SiderHeder).queryTime.adultcalk = getTimeDifference(&start_time,&end_time);
	////////

	//går i utbangspungete gjenon alle sider, men eskaper når vi når anatllet vi vil ha
	PagesResults.showabal = 0;
	//PagesResults.godPages = 0;
	PagesResults.nextPage = 0;

//cuted
	gettimeofday(&popResult_start_time, NULL);



	#ifdef WITH_THREAD

	//vi har data. Lå tårdene jobbe med det
	pthread_mutex_unlock(&PagesResults.mutex);

	//venter på trådene
	for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
		ret = pthread_join(threadid[i], NULL);
	}

	//free mutex'en
	ret = pthread_mutex_destroy(&PagesResults.mutex);

	#else 
	generatePagesResults(&PagesResults);		
	#endif

	#ifdef BLACK_BOKS
		cmc_close(PagesResults.cmcsocketha);
	#endif


	if (rankType == RANK_TYPE_SUM) {
		/* If we are getting the current rank we sum up the pages better ranked
		 * than the rank we put in and return that result */
		int ranksum = 0;

		for (i = 0; i < SiderHeder->TotaltTreff; i++) {
			if (PagesResults.TeffArray->iindex[i].allrank >= *ranking) {
				/* XXX */
				if (PagesResults.Sider[i].deletet == 0 && strlen(PagesResults.Sider[i].DocumentIndex.Url) > 0) {
					//printf("Page: %d <<%s>>\n", PagesResults.TeffArray->iindex[i].allrank, PagesResults.Sider[i].DocumentIndex.Url);
					ranksum++;
				}
			}
		}
		printf("Position: %d\n", ranksum);
		*ranking = ranksum;
	}

	(*SiderHeder).showabal = PagesResults.showabal;

	//lager en filtered verdi
	(*SiderHeder).filtered = PagesResults.filtered + PagesResults.memfiltered;	


	gettimeofday(&popResult_end_time, NULL);
        (*SiderHeder).queryTime.popResult = getTimeDifference(&popResult_start_time,&popResult_end_time);

	
	//5: printf("end\n");
	


	//teller opp filteree
	/*
searchSimple(&PagesResults.antall,PagesResults.TeffArray,&(*SiderHeder).TotaltTreff,
                        &PagesResults.QueryData.queryParsed,&(*SiderHeder).queryTime,
                        subnames,nrOfSubnames,languageFilternr,languageFilterAsNr,
                        orderby,dates,
                        filters)
	*/

	#ifdef BLACK_BOKS
		searchFilterCount(&PagesResults.antall,PagesResults.TeffArray,filters,subnames,nrOfSubnames,&filteron,dates,&(*SiderHeder).queryTime);
	
	#endif



	//fjerner filtered fra total oversikten
	//ToDo: bør dette også gjøres for web?
	(*SiderHeder).TotaltTreff = (*SiderHeder).TotaltTreff - (*SiderHeder).filtered;

	//lager en liste med ordene som ingikk i queryet til hiliting
	hiliteQuery[0] = '\0';
	//printf("size %i\n",PagesResults.QueryData.queryParsed.size);

	//det vil bli et komma for mye på slutten, fjerner det.
	//ToDo: er det altid et komma for mye ?	
	if (hiliteQuery[strlen(hiliteQuery) -1] == ',') {hiliteQuery[strlen(hiliteQuery) -1] = '\0';}

	printf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" filtered=\"%i\" showabal=\"%i\"/>\n",(*SiderHeder).TotaltTreff,PagesResults.QueryData.query,hiliteQuery,(*SiderHeder).filtered,(*SiderHeder).showabal);
	
	printf("free TeffArray\n");
	free(PagesResults.TeffArray);

	destroy_query( &PagesResults.QueryData.queryParsed );


	//printer ut info om brukt tid
	printf("Time\n");
	//printf("\tAthorSearch %f\n",(*SiderHeder).queryTime.AthorSearch);
	printf("\t%-40s %f\n","AthorSearch",(*SiderHeder).queryTime.AthorSearch);
	printf("\t%-40s %f\n","AthorRank",(*SiderHeder).queryTime.AthorRank);
	printf("\t%-40s %f\n","UrlSearch",(*SiderHeder).queryTime.UrlSearch);
	printf("\t%-40s %f\n","MainSearch",(*SiderHeder).queryTime.MainSearch);
	printf("\t%-40s %f\n","MainRank",(*SiderHeder).queryTime.MainRank);
	printf("\t%-40s %f\n","MainAthorMerge",(*SiderHeder).queryTime.MainAthorMerge);
	printf("\n");
	printf("\t%-40s %f\n","popRank",(*SiderHeder).queryTime.popRank);
	printf("\t%-40s %f\n","responseShortning",(*SiderHeder).queryTime.responseShortning);
	printf("\n");
	printf("\t%-40s %f\n","allrankCalc",(*SiderHeder).queryTime.allrankCalc);
	printf("\t%-40s %f\n","indexSort",(*SiderHeder).queryTime.indexSort);
	printf("\t%-40s %f\n","popResult",(*SiderHeder).queryTime.popResult);
	printf("\t%-40s %f\n","adultcalk",(*SiderHeder).queryTime.adultcalk);

	

	#ifdef BLACK_BOKS
	printf("\tfiletypes %f\n",(*SiderHeder).queryTime.filetypes);
	printf("\tiintegerGetValueDate %f\n",(*SiderHeder).queryTime.iintegerGetValueDate);
	printf("\tdateview %f\n",(*SiderHeder).queryTime.dateview);
	printf("\tcrawlManager %f\n",(*SiderHeder).queryTime.crawlManager);

	printf("\tgetUserObjekt %f\n",(*SiderHeder).queryTime.getUserObjekt);
	printf("\tcmc_conect %f\n",(*SiderHeder).queryTime.cmc_conect);

	#endif

	printf("filters:\n");

	printf("\t%-40s %i\n","cantDIRead",(*SiderHeder).filtersTraped.cantDIRead);
	printf("\t%-40s %i\n","getingDomainID",(*SiderHeder).filtersTraped.getingDomainID);
	printf("\t%-40s %i\n","sameDomainID",(*SiderHeder).filtersTraped.sameDomainID);

	printf("\t%-40s %i\n","filterAdultWeight_bool",(*SiderHeder).filtersTraped.filterAdultWeight_bool);
	printf("\t%-40s %i\n","filterAdultWeight_value",(*SiderHeder).filtersTraped.filterAdultWeight_value);
	printf("\t%-40s %i\n","filterSameCrc32_1",(*SiderHeder).filtersTraped.filterSameCrc32_1);
	printf("\t%-40s %i\n","filterSameUrl",(*SiderHeder).filtersTraped.filterSameUrl);
	printf("\t%-40s %i\n","filterNoUrl",(*SiderHeder).filtersTraped.filterNoUrl);
	printf("\t%-40s %i\n","find_domain_no_subname",(*SiderHeder).filtersTraped.find_domain_no_subname);
	printf("\t%-40s %i\n","filterSameDomain",(*SiderHeder).filtersTraped.filterSameDomain);
	printf("\t%-40s %i\n","filterTLDs",(*SiderHeder).filtersTraped.filterTLDs);
	printf("\t%-40s %i\n","filterResponse",(*SiderHeder).filtersTraped.filterResponse);
	printf("\t%-40s %i\n","cantpopResult",(*SiderHeder).filtersTraped.cantpopResult);
	printf("\t%-40s %i\n","cmc_pathaccess",(*SiderHeder).filtersTraped.cmc_pathaccess);
	printf("\t%-40s %i\n","filterSameCrc32_2",(*SiderHeder).filtersTraped.filterSameCrc32_2);

	printf("\n\n");

	#ifdef EXPLAIN_RANK
	printf("|%-10s|%-10s|%-10s||%-10s|%-10s|%-10s|%-25s|%-10s|%-10s|\n",
		"AllRank",
		"TermRank",
		"PopRank",
		"Body",
		"Headline",
		"Tittel",
		"Athor (nr nrp)",
		"UrlM",
		"Url"
		);
	printf("|----------|----------|----------||----------|----------|----------|------------------------|----------|----------|\n");

	for(i=0;i<(*SiderHeder).showabal;i++) {
                        printf("|%10i|%10i|%10i||%10i|%10i|%10i|%10i (%5i %5i)|%10i|%10i| %s (DocID %u-%i)\n",

				Sider[i].iindex.allrank,
				Sider[i].iindex.TermRank,
				Sider[i].iindex.PopRank,

				Sider[i].iindex.rank_explaind.rankBody,
				Sider[i].iindex.rank_explaind.rankHeadline,
				Sider[i].iindex.rank_explaind.rankTittel,
				Sider[i].iindex.rank_explaind.rankAthor,
				Sider[i].iindex.rank_explaind.nrAthor,
				Sider[i].iindex.rank_explaind.nrAthorPhrase,
				Sider[i].iindex.rank_explaind.rankUrl_mainbody,
				Sider[i].iindex.rank_explaind.rankUrl,
				Sider[i].DocumentIndex.Url,
				Sider[i].iindex.DocID,
				rLotForDOCid(Sider[i].iindex.DocID)
				);
	}
	#endif

	return 1;
}


