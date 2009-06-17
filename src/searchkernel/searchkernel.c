
			// Rank siden til slutt hvis den har en hÃyere rank enn siden vi skal ranke
#include "verbose.h"

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>


#include "../common/adultWeight.h"
#include "../common/DocumentIndex.h"
//#include "../parse_summary/summary.h"
#include "../parser/html_parser.h"
//#include "../parse_summary/highlight.h"
#include "../generateSnippet/snippet.parser.h"
#include "../common/ir.h"
#include "../common/timediff.h"
#include "../common/attributes.h"
#include "../common/bstr.h"
#include "../query/query_parser.h"
#include "../query/stemmer.h"
#include "../common/integerindex.h"
#include "../ds/dcontainer.h"

#include "../common/ht.h"

#ifdef WITH_SPELLING
#include "../newspelling/spelling.h"
#endif

#ifdef BLACK_BOKS
	#include "../getdate/getdate.h"
#endif

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

#include "shortenurl.h"

#include "../utf8-filter/utf8-filter.h"

#ifdef WITH_THREAD
        #include <pthread.h>
	#ifdef BLACK_BOKS
		#define NROF_GENERATEPAGES_THREADS 5
	#else
		#define NROF_GENERATEPAGES_THREADS 5
	#endif
#endif

#include "searchkernel.h"
#include "../searchFilters/searchFilters.h"

//#include "parseEnv.h"

#include "../boithoadClientLib/liboithoaut.h"
#include "../crawlManager/client.h"

#include "htmlstriper.h"
#include "search.h"

#include "../common/reposetory.h"
#include "../common/reposetoryNET.h"
//#include "../common/lot.h"

//#include "../common/define.h"

//#include "cgi-util.h"

#include "../ds/dcontainer.h"
#include "../ds/dvector.h"
#include "../ds/dpair.h"

	//struct iindexFormat *TeffArray; //[maxIndexElements];

#ifdef WITH_SPELLING
	extern spelling_t spelling;
#endif

#ifdef DEBUG_TIME
struct popResultTimesFormat {
        double time;
        int nr;
};

struct popResultBreakDownTimeFormat {

        struct popResultTimesFormat DocumentIndex;
        struct popResultTimesFormat ReadSummary;
        struct popResultTimesFormat html_parser_run;
        struct popResultTimesFormat generate_snippet;
        struct popResultTimesFormat ReadHtml;
        struct popResultTimesFormat memGetDomainID;
        struct popResultTimesFormat totalpopResult;
        struct popResultTimesFormat makecrc32;
        struct popResultTimesFormat treadSyncFilter;

        struct popResultTimesFormat titleClean;
        struct popResultTimesFormat bodyClean;
        struct popResultTimesFormat iindexMemcpy;
        struct popResultTimesFormat popResultFree;

};
#endif

#define MAX_CM_CONSUMERS 2
#define SUMMARY_LEN 160

struct socket_pool {
	int sock[MAX_CM_CONSUMERS];
	int used[MAX_CM_CONSUMERS];
	int consumers;
	pthread_mutex_t mutex;
	pthread_cond_t cv;
};


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
		int filteredsilent;

		int memfiltered;


		#ifdef WITH_THREAD
			pthread_mutex_t mutex;
			pthread_mutex_t mutextreadSyncFilter;
			struct socket_pool cmConn;
		#endif

		int activetreads;

		struct searchd_configFORMAT *searchd_config;
		//struct filtypesFormat *filtypes;
		//int filtypesnrof;
		#ifdef DEBUG_TIME
		struct popResultBreakDownTimeFormat popResultBreakDownTime;
		#endif
		struct hashtable *crc32maphash;
		struct duplicate_docids *dups;
		enum platform_type ptype; 
		enum browser_type btype; 
		int anonymous;
};


static unsigned int hash_domainid_fn(void *k) { /* XXX: Make a proper hash function here */
	return *(unsigned short int *)k;
}
static int equal_domainid_fn(void *key1, void *key2) {
	return (*(unsigned short int *)key1) == (*(unsigned short int *)key2);
}


char *generate_summary(char summary_cfg, query_array query_parsed, char *body)  {
	char *summary;
	size_t body_len = strlen(body);

	if (summary_cfg == SUMMARY_DB) {
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , db_snippet, SUMMARY_LEN, 4, 80);
	}
	else if (summary_cfg == SUMMARY_SNIPPET) {
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , plain_snippet, SUMMARY_LEN, 4, 80);
	}
	else if (summary_cfg == SUMMARY_START) {
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , first_snippet, SUMMARY_LEN, 4, 80);
	    /*
		// asuming *5 is enough to also clean out htmltags at
		// the beginning of the document
		char stripbuff[SUMMARY_LEN * 5];
		htmlstrip(body, stripbuff, SUMMARY_LEN * 5);
		size_t striplen = strlen(stripbuff);

		size_t summary_len = (striplen > SUMMARY_LEN)
			? SUMMARY_LEN
			: striplen;
		
		
		summary = malloc(sizeof(char) * (summary_len + 1));
		strlcpy(summary, stripbuff, summary_len +1);
		if (striplen > SUMMARY_LEN) {
			summary[SUMMARY_LEN - 4] = '.';
			summary[SUMMARY_LEN - 3] = '.';
			summary[SUMMARY_LEN - 2] = '.';
			summary[SUMMARY_LEN - 1] = '\0';
		}
	    */
	}
	/*
	  ++Ax:
	    Snippet for database-records:
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , db_snippet, SUMMARY_LEN, 4, 80);
	    De to siste tallene er maks antall rader og maks antall tegn per kolonne.
	*/
	else { 
		warnx("Unknown snippet/summery cfg: %d\n", summary_cfg); 
		summary = strdup("");
	}

	return summary;

}


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

#ifdef WITH_THREAD
static inline int
get_sock_from_pool(struct socket_pool *pool, int *index)
{
	int i;

	pthread_mutex_lock(&pool->mutex);
	while (pool->consumers == MAX_CM_CONSUMERS) {
		pthread_cond_wait(&pool->cv, &pool->mutex);
	}
	pool->consumers++;

	for (i = 0; i < MAX_CM_CONSUMERS; i++) {
		if (pool->used[i] == 0) {
			pool->used[i] = 1;
			pthread_mutex_unlock(&pool->mutex);
			*index = i;
			return pool->sock[i];
		}
	}
	
	assert(1 == 0);
}

static inline int
release_sock_to_pool(struct socket_pool *pool, int index)
{
	pthread_mutex_lock(&pool->mutex);
	pool->used[index] = 0;
	
	pool->consumers--;
	pthread_cond_signal(&pool->cv);
	pthread_mutex_unlock(&pool->mutex);
}
#endif
#endif



#ifdef BLACK_BOKS
static inline int
handle_url_rewrite(const char *url_in, size_t lenin, enum platform_type ptype, enum browser_type btype, char *collection,
           char *url_out, size_t len, char *uri_out, size_t uri_out_len, char *fulluri_out, size_t fulluri_out_len, int sock, struct socket_pool *pool)
{

	int ret = 1;

#ifndef _24SEVENOFFICE

#ifdef WITH_THREAD
	int index;

	sock = get_sock_from_pool(pool, &index);
#endif

	ret = cmc_rewrite_url(sock, collection, url_in, lenin, ptype, btype, url_out, len, uri_out, uri_out_len, fulluri_out, fulluri_out_len);
	if (ret == 0) {
		fprintf(stderr,"Cant rewrite url %s\n",url_in);
	}
	else {
		vboprintf("handle_url_rewrite: Did rewrite \"%s\" -> \"%s\"\n",url_in,url_out);
	}
#ifdef WITH_THREAD
	release_sock_to_pool(pool, index);
#endif

#endif

	return ret;
}
#endif


int
popResult(struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,int antall,unsigned int DocID,
	struct iindexMainElements *TeffArray,struct QueryDataForamt QueryData, char *htmlBuffer,
	unsigned int htmlBufferSize, char servername[], struct subnamesFormat *subname, unsigned int getRank,
	struct queryTimeFormat *queryTime, int summaryFH, struct PagesResultsFormat *PagesResults)
{

	vboprintf("searchkernel: popResult(antall=%i, DocID=%i, subname=%s)\n", antall, DocID, subname->subname);

	char *url = NULL, *attributes = NULL;
	int y;
	char        *titleaa, *body, *metakeyw, *metadesc;
	struct ReposetoryHeaderFormat ReposetoryHeader;
	char *acl_allowbuffer = NULL;
	char *acl_deniedbuffer = NULL;
	off_t imagep;
	struct timeval start_time, end_time;
	titleaa = body = metakeyw = metadesc = NULL;

	char *strpointer;

	int termpos;
	int returnStatus = 0;	



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
				subname->subname);

#else
		sprintf((*Sider).thumbnale,"http://%s/cgi-bin/ShowThumb?L=%i&amp;P=%u&amp;S=%i&amp;C=%s",
				servername,
				rLotForDOCid(DocID),
				(unsigned int)imagep,
				(*Sider).DocumentIndex.imageSize,
				subname->subname);
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
		printf("Cant read url for DocID=%i-%i, Subname=\"%s\"\n",DocID,rLotForDOCid(DocID),subname->subname);
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
		//memcpy(&(*Sider).iindex,TeffArray,sizeof(*TeffArray));

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
			if (Sider->DocumentIndex.ResourceSize != 0) {
				char *resbuf;
				size_t ressize;
				int LotNr;
				char *snippet;

				LotNr = rLotForDOCid(DocID);
				ressize = getResource(LotNr, "www", DocID, NULL, -1);
				resbuf = malloc(ressize+1);
				if (getResource(LotNr, "www", DocID, resbuf, ressize+1) == 0) {
					fprintf(stderr, "Unable to get resource for %d\n", DocID);
					(*Sider).title[0] = '\0';
					(*SiderHeder).showabal++;
					returnStatus = 0;
				} else {

					gettimeofday(&start_time, NULL);

					html_parser_run( (*Sider).DocumentIndex.Url, resbuf, ressize, 
							&titleaa, &body, fn, NULL);

					gettimeofday(&end_time, NULL);
#ifdef DEBUG_TIME
					PagesResults->popResultBreakDownTime.html_parser_run.time += getTimeDifference(&start_time,&end_time);
					++PagesResults->popResultBreakDownTime.html_parser_run.nr;
#endif

					(*Sider).HtmlPreparsed = 0;

					gettimeofday(&start_time, NULL);


					snippet = generate_summary(subname->config.summary, QueryData.queryParsed, body);

					gettimeofday(&end_time, NULL);
#ifdef BLACK_BOKS
					queryTime->generate_snippet += getTimeDifference(&start_time,&end_time);
#endif

					strcpy(Sider->title, titleaa);
					strcpy(Sider->description, snippet);
					//memcpy(&(*Sider).iindex,TeffArray,sizeof(*TeffArray));

					(*SiderHeder).showabal++;
					returnStatus = 1;

					if (titleaa != NULL) free(titleaa);
					if (body != NULL) free(body);
					free(snippet);
				}
				free(resbuf);
			} else if (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize,DocID,subname->subname,&ReposetoryHeader,&acl_allowbuffer,&acl_deniedbuffer,(*Sider).DocumentIndex.imageSize, &url, &attributes) != 1) {
				//kune ikke lese html. Pointer owerflow ?
				//printf("Fii faa foo: %s\n", url);
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
				//memcpy(&(*Sider).iindex,TeffArray,sizeof(*TeffArray));


				(*SiderHeder).showabal++;
				returnStatus = 1;
			}
		}
		else if ((*Sider).DocumentIndex.response == 200) {

#ifdef BLACK_BOKS
                       // Include time and sign the parameters.

			time_t u_time = time(NULL);
			unsigned int signature;
			signature = sign_cache_params(DocID, subname->subname, u_time);

			sprintf((*Sider).cacheLink, "http://%s/cgi-bin/ShowCache2bb?D=%u&amp;subname=%s&amp;time=%u&amp;sign=%u",
					servername, DocID, subname->subname, u_time, signature);
#else
			sprintf((*Sider).cacheLink,"http://%s/cgi-bin/ShowCache2?D=%u&amp;subname=%s",servername,DocID,subname->subname);
#endif


			gettimeofday(&start_time, NULL);						


			if (((*Sider).DocumentIndex.SummaryPointer != 0) && 
					((rReadSummary_l(DocID,&metadesc, &titleaa,&body,(*Sider).DocumentIndex.SummaryPointer,(*Sider).DocumentIndex.SummarySize,subname->subname,summaryFH) != 0))) {

				vboprintf("hav Summary on disk\n");

				(*Sider).HtmlPreparsed = 1;

				gettimeofday(&end_time, NULL);				
#ifdef DEBUG_TIME
				PagesResults->popResultBreakDownTime.ReadSummary.time += getTimeDifference(&start_time,&end_time);
				++PagesResults->popResultBreakDownTime.ReadSummary.nr;					
#endif

			}
			else {
				debug("don't hav Summary on disk. Will hav to read html\n");	
				if (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize,DocID,subname->subname,&ReposetoryHeader,&acl_allowbuffer,&acl_deniedbuffer,(*Sider).DocumentIndex.imageSize, &url, &attributes) != 1) {
					//printf("Fii faa foo: %s\n", url);
					//kune ikke lese html. Pointer owerflow ?
					printf("error reding html for %s\n",(*Sider).DocumentIndex.Url);
					sprintf((*Sider).description,"Html error. Can't read html");
					(*Sider).title[0] = '\0';
					(*SiderHeder).showabal++;
					returnStatus = 1;
					vboprintf("searchkernel: ~popResult()\n");
					return 0;

				}
				/* XXX: Probably no longer needed */
				strlcpy(Sider->uri, url, sizeof(Sider->uri));
				strlcpy(Sider->url, url, sizeof(Sider->uri));

				gettimeofday(&end_time, NULL);				
#ifdef DEBUG_TIME
				PagesResults->popResultBreakDownTime.ReadHtml.time += getTimeDifference(&start_time,&end_time);
				++PagesResults->popResultBreakDownTime.ReadHtml.nr;					
#endif


				gettimeofday(&start_time, NULL);						

				html_parser_run(url, htmlBuffer, htmlBufferSize, &titleaa, &body, fn, NULL);

				gettimeofday(&end_time, NULL);
#ifdef DEBUG_TIME
				PagesResults->popResultBreakDownTime.html_parser_run.time += getTimeDifference(&start_time,&end_time);
				++PagesResults->popResultBreakDownTime.html_parser_run.nr;
#endif

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
					//generate_highlighting(QueryData.queryParsed, body, strlen(body)+1, &summary);
			//temp: bug generate_snippet er ikke ut til å takle og ha en tom body
			if (strlen(body) == 0 || getRank) {
				(*Sider).description[0] = '\0';
			}
			else if (strlen(body) < 15) {
				printf("bug: body består av under 15 tegn. Da kan det være at vi bare har <div> som tegn.\n");
			}
			else {
				char key[MAX_ATTRIB_LEN], value[MAX_ATTRIB_LEN], keyval[MAX_ATTRIB_LEN];
				char summary_cfg, *attr_offset;

#ifdef DEBUG
				printf("calling generate_snippet with strlen body %i\n",strlen(body));
#endif

#ifdef DEBUG_TIME
				gettimeofday(&start_time, NULL);
#endif

#ifdef DEBUG
				printf("#################################################################\n");
				printf("############################ <DEBUG> ############################\n");
				printf("body to snipet:\n%s\n",body);
				printf("strlen(body) %i\n",strlen(body));
				printf("############################ </DEBUG> ###########################\n");
				printf("#################################################################\n");
#endif

				summary_cfg = subname->config.summary;
				
				if ((attributes != NULL) && (next_attribute_key(attributes, &attr_offset, key, value, keyval, "snippet"))) {
					if (strcmp(value, "db") == 0) {
						summary_cfg = SUMMARY_DB;
					}
				}

				//printf("calling generate_snippet, body \"%s\", length %i\n",body, strlen(body));
				summary = generate_summary(summary_cfg, QueryData.queryParsed, body);


#ifdef DEBUG_TIME
				gettimeofday(&end_time, NULL);
				PagesResults->popResultBreakDownTime.generate_snippet.time += getTimeDifference(&start_time,&end_time);
				++PagesResults->popResultBreakDownTime.generate_snippet.nr;
#endif

				//printf("summary len %i\nsummary:\n-%s-\n",strlen(summary),summary);

#ifdef DEBUG_TIME
				gettimeofday(&start_time, NULL);
#endif

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

#ifdef DEBUG_TIME
				gettimeofday(&end_time, NULL);
				PagesResults->popResultBreakDownTime.bodyClean.time += getTimeDifference(&start_time,&end_time);
				++PagesResults->popResultBreakDownTime.bodyClean.nr;
#endif


			}

			debug("%u -%s-, len %i\n",DocID,titleaa,strlen(titleaa));

#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
#endif


			if (titleaa[0] == '\0') {
				sprintf((*Sider).title,"No title");
			}
			else if (strlen(titleaa) > (sizeof((*Sider).title) -1)) {

				int copylen = (sizeof((*Sider).title) -4);

				strscpy(&((*Sider).title[0]),titleaa,copylen);


				//søker oss til siste space , eller ; og avslutter der
				if ((strpointer = strrchr((*Sider).title,' ')) != NULL) {
					vboprintf("aa strpointer %u\n",(unsigned int)strpointer);
					//midlertidg fiks på at title altid begynner med space på bb.
					//vil dermed altidd føre til treff i første tegn, og
					// dermed bare vise ".." som title
					//if ( ((int)(*Sider).title - (int)strpointer) > 10) {
					if ( ((int)strpointer - (int)(*Sider).title) > 10) {
						vboprintf("chop\n");
						strpointer[0] = '\0';
					}
					else {
						printf("no chop, ing %i\n", (int)(*Sider).title - (int)strpointer);
					}
					vboprintf("fant space at %i\n",((int)strpointer) - (int)(*Sider).title);
				}						
				else if ((strpointer = strrchr((*Sider).title,';')) != NULL) {
					++strpointer; //pekeren peker på semikolonet. SKal ha det med, så må legge il en
					strpointer[0] = '\0';

					vboprintf("fant semi colon at %i\n",((int)(*Sider).title - (int)strpointer));
				}
				strncat((*Sider).title,"..",2);    

				vboprintf("title to long choped. now %i len. size %i\n",strlen((*Sider).title),sizeof((*Sider).title));

			}
			else {
				strscpy((*Sider).title,titleaa,sizeof((*Sider).title) -1);
			}													

#ifdef DEBUG_TIME
			gettimeofday(&end_time, NULL);
			PagesResults->popResultBreakDownTime.titleClean.time += getTimeDifference(&start_time,&end_time);
			++PagesResults->popResultBreakDownTime.titleClean.nr;
#endif

#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
#endif

			//temp: ser ut til at vi får problemer her hvis vi har status "Html error. Can't read"
			if (titleaa != NULL) free(titleaa);
			if (body != NULL) free(body);
			if (metakeyw != NULL) free(metakeyw);
			if (metadesc != NULL) free(metadesc);

			(*SiderHeder).showabal++;
			returnStatus = 1;

#ifdef DEBUG_TIME
			gettimeofday(&end_time, NULL);
			PagesResults->popResultBreakDownTime.popResultFree.time += getTimeDifference(&start_time,&end_time);
			++PagesResults->popResultBreakDownTime.popResultFree.nr;
#endif

			/* Duplicates? */
			// Byttet fra list til vector da vector er raskere (ax).
			struct duplicate_docids *dup = hashtable_search(PagesResults->crc32maphash, &Sider->DocumentIndex.crc32);
			if (dup != NULL && dup->V != NULL && vector_size(dup->V)>1) {
				int k;
				

				Sider->n_urls = 0;
				Sider->urls = calloc(vector_size(dup->V), sizeof(*(Sider->urls)));
				
				//iterator itr = list_begin(list);
				//itr = list_next(itr);
				//for (k = 0; itr.valid; itr = list_next(itr), k++) {
				for (k = 0; k<vector_size(dup->V); k++) {
					unsigned int dup_docid = pair(vector_get(dup->V,k)).first.i;
					//char *dup_subname = PagesResults->TeffArray->subnames[pair(vector_get(dup->V,k)).second.i];
					char *dup_subname = pair(vector_get(dup->V,k)).second.ptr;

					if (dup_docid == DocID && !strcmp(dup_subname, subname->subname))
					    {
						// Hvis dette er docid-en som vises, skal den ikke filtreres.
						continue;
					    }

					char htmlbuf[1024*1024 * 5], imagebuf[1<<16];
					unsigned int htmllen = sizeof(htmlbuf);
					unsigned int imagelen = sizeof(imagelen);
					char *url, *acla, *acld, *attributes;
					struct DocumentIndexFormat di;
					struct ReposetoryHeaderFormat repohdr;
					char tmpurl[1024], tmpuri[1024], tmpfulluri[1024];
					//printf("Woop subname!!!: %p %s\n", dup_subname, dup_subname);

					acld = acla = attributes = url = NULL;
					if (!DIRead(&di, dup_docid, dup_subname)) {
						warn("DIRead()");
						continue;
					}
					rReadHtml(htmlbuf, &htmllen, di.RepositoryPointer, di.htmlSize, dup_docid,
							dup_subname, &repohdr, &acla, &acld, di.imageSize,
							&url, &attributes);

#ifdef BLACK_BOKS
					if (!handle_url_rewrite(url, sizeof(url), PagesResults->ptype,
							PagesResults->btype,
							dup_subname, tmpurl, sizeof(tmpurl),
							tmpuri, sizeof(tmpuri),
							tmpfulluri, sizeof(tmpfulluri),
							PagesResults->cmcsocketha,
#ifdef WITH_THREAD
							&PagesResults->cmConn
#else
							NULL
#endif
							)) {
						snprintf(tmpurl,sizeof(tmpurl),"XXX-Can't_rewrite_duplicate_url_for_DocID_%u_XXX:%s",dup_docid);
					}
#endif
					if ((Sider->urls[Sider->n_urls].url = strdup(tmpurl)) == NULL) {
						perror("Malloc url");
					}
					if ((Sider->urls[Sider->n_urls].uri = strdup(tmpuri)) == NULL) {
						perror("Malloc uri");
					}
					if ((Sider->urls[Sider->n_urls].fulluri = strdup(tmpfulluri)) == NULL) {
						perror("Malloc uri");
					}
					//shortenurl(Sider->urls[Sider->n_urls].uri, strlen(Sider->urls[Sider->n_urls].uri) +1); // +1 da minne område er faktisk 1 bytes lenegre en strenglendgden for å få plass til \0. Dette skal også legges på uri'en.
					//strcpy(Sider->urls[Sider->n_urls].url, dup_subname);
					free(attributes);
					free(acla);
					free(acld);
					free(url);
					++Sider->n_urls;
				}
			} else {
				Sider->n_urls = 0;
				Sider->urls = NULL;
			}
			// destroy(list);
			// hashtable_destroy(PagesResults->crc32maphash);

			/* Make attribute lists */
			if (attributes == NULL) {
				Sider->attributes = NULL;
			}
			else {
				Sider->attributes = strdup(attributes);
			}
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

	//temp:
	if (acl_allowbuffer != NULL)
		free(acl_allowbuffer);
	if (acl_deniedbuffer != NULL)
		free(acl_deniedbuffer);
	if (url != NULL)
		free(url);
	if (attributes != NULL)
		free(attributes);

	vboprintf("searchkernel: ~popResult()\n");
	return returnStatus;
}


int nextIndex(struct PagesResultsFormat *PagesResults) {

	#ifdef DEBUG
	printf("searchkernel: nextIndex()\n");
	#endif

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

	#ifdef DEBUG
	printf("searchkernel: ~nextIndex()\n");
	#endif
	return forreturn;
}

int nextPage(struct PagesResultsFormat *PagesResults) {

	vboprintf("searchkernel: nextPage()\n");
	int forreturn;

	#ifdef WITH_THREAD
	//printf("nextPage: waiting for lock: start\n");
	pthread_mutex_lock(&(*PagesResults).mutex);
	//printf("nextPage: waiting for lock: end\n");
	#endif

	if (((*PagesResults).activetreads > 2) && ((*PagesResults).MaxsHits - (*PagesResults).nextPage) < 3) {
		vboprintf("we have %i pages, and %i activetreads. This tread can die.\n",(*PagesResults).nextPage,(*PagesResults).activetreads);
		--(*PagesResults).activetreads;
		forreturn = -1;
	}
	else if ((*PagesResults).nextPage >= (*PagesResults).MaxsHits) {
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

	vboprintf("searchkernel: ~nextPage()\n");
	return forreturn;

}

int foundGodPage(struct PagesResultsFormat *PagesResults) {

	int ret;

	#ifdef WITH_THREAD
	//printf("nextPage: waiting for lock: start\n");
	pthread_mutex_lock(&(*PagesResults).mutex);
	//printf("nextPage: waiting for lock: end\n");
	#endif

		vboprintf("this is a good page\n");
		ret = (*PagesResults).showabal;

		++(*PagesResults).showabal;
		
	#ifdef WITH_THREAD
	//printf("nextPage:waiting for UNlock: start\n");
	pthread_mutex_unlock(&(*PagesResults).mutex);
	//printf("nextPage:waiting for UNlock: end\n");

	#endif

	return ret;
}

int nrofGodPages(struct PagesResultsFormat *PagesResults) {

	int ret;

	#ifdef WITH_THREAD
		pthread_mutex_lock(&(*PagesResults).mutex);
	#endif

		ret = (*PagesResults).showabal;

		#ifdef DEBUG
		printf("nrofGodPage: have %i god pages\n",ret);
		#endif

	#ifdef WITH_THREAD
		pthread_mutex_unlock(&(*PagesResults).mutex);
	#endif

	return ret;

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
	//runarb:4 Dette gjør at tallene for treff i subname minker, gjør heller slik at man viser alle, og så viser filtered meldingen
	//--(*nrInSubname);

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

	if (whichFilterTraped != NULL) {
		++(*whichFilterTraped);
	}

	//runarb:1 13 mars. Hvorfor var denne komentert ut???
	//runarb:2 for de vi nå bruker subname som filter
	//runarb:3 kan det være det er nyttig for web søket og ha rikitig tall her? 
	//runarb:4 Dette gjør at tallene for treff i subname minker, gjør heller slik at man viser alle, og så viser filtered meldingen
	//--(*nrInSubname);

	#ifdef BLACK_BOKS
		(*iindex).deleted = 1;
	#endif

	#ifdef WITH_THREAD
	pthread_mutex_unlock(&(*PagesResults).mutex);
	#endif


}

void increaseFilteredSilent(struct PagesResultsFormat *PagesResults,int *whichFilterTraped, 
	int *nrInSubname,struct iindexMainElements *iindex) {


	#ifdef WITH_THREAD
	pthread_mutex_lock(&(*PagesResults).mutex);
	#endif

	//++(*(*PagesResults).SiderHeder).filtered;
	++(*PagesResults).filteredsilent;

	if (whichFilterTraped != NULL) {

		++(*whichFilterTraped);
	}

	//runarb:1 13 mars. Hvorfor var denne komentert ut???
	//runarb:2 for de vi nå bruker subname som filter
	//runarb:3 kan det være det er nyttig for web søket og ha rikitig tall her? 
	//runarb:4 Dette gjør at tallene for treff i subname minker, gjør heller slik at man viser alle, og så viser filtered meldingen
	//--(*nrInSubname);

	#ifdef BLACK_BOKS
		(*iindex).deleted = 1;
	#endif

	#ifdef WITH_THREAD
	pthread_mutex_unlock(&(*PagesResults).mutex);
	#endif


}

#ifdef BLACK_BOKS
static inline int
pathaccess(struct PagesResultsFormat *PagesResults, int socketha,char collection_in[], char uri_in[], char user_in[], char password_in[]) {
	int ret = 0;

	if (PagesResults->anonymous)
		return 1;

#ifdef WITH_THREAD
	int sockIndex;

	socketha = get_sock_from_pool(&PagesResults->cmConn, &sockIndex);
	//pthread_mutex_lock(&(*PagesResults).mutex_pathaccess);
#endif

	ret = cmc_pathaccess(socketha, collection_in, uri_in, user_in, password_in);

#ifdef WITH_THREAD
	release_sock_to_pool(&PagesResults->cmConn, sockIndex);
	//pthread_mutex_unlock(&(*PagesResults).mutex_pathaccess);
#endif

	
	vboprintf("pathaccess respons=%i\n",ret);

	return ret;
}
#endif

//rutine som gjør en vanlig DIRead, og tar tiden
int time_DIRead_i(struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], int file, struct PagesResultsFormat *PagesResults) {

	int ret;

	#ifdef DEBUG_TIME
		struct timeval start_time, end_time;
		gettimeofday(&start_time, NULL);
	#endif


	ret = DIRead_i(DocumentIndexPost,DocID,subname,file);

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		printf("time_DIRead_fh: reading for DocID %u, dev: %i, time %f\n",DocID,GetDevIdForLot(rLotForDOCid(DocID)),getTimeDifference(&start_time,&end_time));

		PagesResults->popResultBreakDownTime.DocumentIndex.time += getTimeDifference(&start_time,&end_time);
		++PagesResults->popResultBreakDownTime.DocumentIndex.nr;
	#endif

	return ret;
}

//int generatePagesResults( struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,int antall, 
//struct iindexFormat *TeffArray, int showabal, int filterOn, int adultpages, int noadultpages,struct 
//QueryDataForamt QueryData, char servername[],int godPages,int MaxsHits,int start) 
//void *generatePagesResults(struct PagesResultsFormat *PagesResults) 
void *generatePagesResults(void *arg) 
{

	vboprintf("searchkernel: generatePagesResults()\n");
	struct PagesResultsFormat *PagesResults = (struct PagesResultsFormat *)arg;

	int i,y;
                //ToDo: ikke hardkode her
        unsigned int htmlBufferSize = 900000;

	char *htmlBuffer;
	unsigned short *DomainID;

	struct timeval start_time, end_time;
	int localshowabal;
	//tread lock

	struct SiderFormat *side = malloc(sizeof(struct SiderFormat));

#if BLACK_BOKS
	if (!PagesResults->getRank) {
		PagesResults->ptype = get_platform(PagesResults->useragent);
		PagesResults->btype = get_browser(PagesResults->useragent);
	}
#endif

	if ((htmlBuffer = malloc(htmlBufferSize)) == NULL) {
		perror("can't malloc");
		vboprintf("searchkernel: ~generatePagesResults()\n");
		return;
	}

	#ifdef WITH_THREAD
		pthread_t tid;
		tid = pthread_self();
		vboprintf("is thread id %u\n",(unsigned int)tid);

	#else
		unsigned int tid=0;
	#endif
	//for (i=0;(i<(*PagesResults).antall) && ((*(*PagesResults).SiderHeder).filtered < 300);i++) {
	// XXX: Use pthread_cond instead ?
	while ( (localshowabal = nextPage(PagesResults)) != -1 ) {
	debug("localshowabal %i",localshowabal);

	while ((i=nextIndex(PagesResults)) != -1) {

		debug("i %i, DocID %u, subname %s\n",i,(*PagesResults).TeffArray->iindex[i].DocID, (*PagesResults).TeffArray->iindex[i].subname->subname);

		//if ((*SiderHeder).filtered > 300) {
		//	(*PagesResults).filterOn = 0;
		//}
		#ifdef BLACK_BOKS

			//hvis index filter tidligere har funet ut at dette ikke er et bra treff går vi til neste
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.filename == 1) {
				#ifdef DEBUG
				printf("filter: index filtered (filename)\n");
				#endif
				continue;
			}
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.date == 1) {
				#ifdef DEBUG
				printf("filter: index filtered (date)\n");
				#endif
				continue;
			}
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.subname == 1) {
				#ifdef DEBUG
				printf("filter: index filtered (subname)\n");
				#endif
				continue;
			}
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.attribute == 1) {
				#ifdef DEBUG
				printf("filter: index filtered (attribute)\n");
				#endif
				continue;
			}
			// If NOT filtering on collection:
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.duplicate == 1) {
				#ifdef DEBUG
				printf("filter: index filtered (duplicate)\n");
				#endif
				continue;
			}
		#endif

		#ifndef BLACK_BOKS
			//pre DIread filter
			#ifdef DEBUG
				printf("adult %u: %i\n",(*PagesResults).TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID));
			#endif

			if (((*PagesResults).filterOn) && (filterAdultWeight_bool(adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID),(*PagesResults).adultpages,(*PagesResults).noadultpages) == 1)) {
				#ifdef DEBUG
				printf("%u is adult whith %i\n",(*PagesResults).TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID));
				#endif
				increaseMemFiltered(PagesResults,
					&(*(*PagesResults).SiderHeder).filtersTraped.filterAdultWeight_bool,
					&(*(*PagesResults).TeffArray->iindex[i].subname).hits,
					&(*PagesResults).TeffArray->iindex[i]);
				continue;
			}

			#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
			#endif
			//uanset om vi har filter eller ikke så slår vi opp domainid. Men hvis vi har filter på så teller vi også
			if (!iintegerMemArrayGet ((*PagesResults).DomainIDs,(void**)&DomainID,sizeof(*DomainID),(*PagesResults).TeffArray->iindex[i].DocID) ) {
				#ifdef DEBUG
				printf("can't lookup DomainID\n");
				#endif
				side->DomainID = 0;

				//filtrerer ikke ut domener som vi ikke kunne slå opp domene id for, da 
				//dette kansje ikke er www. Får heller lage en domene id etter ig ha lest DI
				//if (((*PagesResults).filterOn)) {
				//	increaseMemFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.getingDomainID,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
				//
				//	continue;
				//}
			}
			else {

				side->DomainID = (*DomainID);
				//printf("DomainID %ho\n",side->DomainID);
			
				// fornå gjør vi bare denne sjekken hvis vi kunne slå opp DomainID
                		if (((*PagesResults).filterOn) && (filterSameDomainID(localshowabal,side,(*PagesResults).Sider))) {
					#ifdef DEBUG
					printf("Have same DomainID\n");
					#endif
					increaseMemFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.sameDomainID,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
					continue;
				
				}
			}
			#ifdef DEBUG_TIME
			        gettimeofday(&end_time, NULL);
        		        PagesResults->popResultBreakDownTime.memGetDomainID.time += getTimeDifference(&start_time,&end_time);
	        	        ++PagesResults->popResultBreakDownTime.memGetDomainID.nr;
			#endif

		#endif

		vboprintf("readin di for %u\n",(*PagesResults).TeffArray->iindex[i].DocID);
		//leser DI
		if (!time_DIRead_i(&side->DocumentIndex,(*PagesResults).TeffArray->iindex[i].DocID,(*(*PagesResults).TeffArray->iindex[i].subname).subname,PagesResults->searchd_config->lotPreOpen.DocumentIndex[rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID)], PagesResults)) 
		{
                        //hvis vi av en eller annen grun ikke kunne gjøre det kalger vi
                        vboprintf("Can't read post for %u-%i\n",(*PagesResults).TeffArray->iindex[i].DocID,rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID));
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cantDIRead,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        continue;
                }


		vboprintf("[tid: %u] looking on  DocID: %u url: \"%s\", subname: \"%s\"\n",(unsigned int)tid,(*PagesResults).TeffArray->iindex[i].DocID,side->DocumentIndex.Url,(*(*PagesResults).TeffArray->iindex[i].subname).subname);
		
		//adult fra di
		if (((*PagesResults).filterOn) && (filterAdultWeight_value(side->DocumentIndex.AdultWeight,(*PagesResults).adultpages,(*PagesResults).noadultpages)) ) {
			vboprintf("Filter: filtered adult. DocID %u, adult value %i, adult bool value %i\n",
				(*PagesResults).TeffArray->iindex[i].DocID, 
				(int)side->DocumentIndex.AdultWeight ,(
				int)adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID) );

				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterAdultWeight_value,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
	                        continue;
		}

		//filtrerer ut dublikater fra med crc32 fra DocumentIndex
		if (side->DocumentIndex.crc32 != 0) {
                       
                        if (((*PagesResults).filterOn) && (filterSameCrc32(localshowabal,side,(*PagesResults).Sider))) {
                        	vboprintf("hav same crc32. crc32 from DocumentIndex\n");
				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameCrc32_1,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        	continue;
                        }
                }

		//filtrere silent
		if (filterResponseCode(side)) {
			vboprintf("filter: page har bad respons code\n");
			increaseFilteredSilent(PagesResults,NULL,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			continue;
		}
		


		#ifndef BLACK_BOKS

		if (side->DocumentIndex.Url[0] == '\0') {
			vboprintf("filter: DocumentIndex url is emty. DocID %u\n",(*PagesResults).TeffArray->iindex[i].DocID);
			increaseFilteredSilent(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterNoUrl,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			continue;
		}


		//finner domene
		if (!find_domain_no_subname(side->DocumentIndex.Url,side->domain,sizeof(side->domain))) {
		//if (!find_domain_no_www(side->DocumentIndex.Url,side->domain)) {
			vboprintf("can't find domain. Bad url?. Url \"%s\". localshowabal %i\n",side->DocumentIndex.Url,localshowabal);
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.find_domain_no_subname,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}

		//hvis vi ikke har noen domeneID så lager vi en
		if (side->DomainID == 0) {
			side->DomainID = calcDomainID(side->domain);

		}

		if (((*PagesResults).filterOn) && (filterTLDs(side->domain))) {
			vboprintf("banned TLD\n");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterTLDs,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}

		#endif


		//side->posisjon = localshowabal +1;

		#ifndef BLACK_BOKS
		//DI filtere
		if (((*PagesResults).filterOn) && (filterResponse(side->DocumentIndex.response) )) {
			vboprintf("bad respons kode %i for %u.\n",side->DocumentIndex.response,(*PagesResults).TeffArray->iindex[i].DocID);
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterResponse,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}
		//if (((*PagesResults).filterOn) && (filterSameIp(localshowabal,side,(*PagesResults).Sider))) {
		//	printf("hav same IP\n");
		//	increaseFiltered(PagesResults);
		//	continue;		
		//}
		#endif

		/***************************************************************************	
		runarb: 24.10.2007
		her var PagesResults før	
		*/
		#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
		#endif
		if (PagesResults->getRank == 0 && !popResult(side, (*PagesResults).SiderHeder,(*PagesResults).antall,(*PagesResults).TeffArray->iindex[i].DocID,&(*PagesResults).TeffArray->iindex[i],(*PagesResults).QueryData,htmlBuffer,htmlBufferSize,(*PagesResults).servername,PagesResults->TeffArray->iindex[i].subname, PagesResults->getRank,&PagesResults->SiderHeder->queryTime,PagesResults->searchd_config->lotPreOpen.Summary[rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID)],PagesResults)) {
                       	vboprintf("can't popResult\n");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cantpopResult,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;

		}
		#ifdef DEBUG_TIME
		        gettimeofday(&end_time, NULL);
        	        PagesResults->popResultBreakDownTime.totalpopResult.time += getTimeDifference(&start_time,&end_time);
	                ++PagesResults->popResultBreakDownTime.totalpopResult.nr;
		#endif

		#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
		#endif
		memcpy(&side->iindex,&PagesResults->TeffArray->iindex[i],sizeof(PagesResults->TeffArray->iindex[i]));					    
		#ifdef DEBUG_TIME
		        gettimeofday(&end_time, NULL);
        	        PagesResults->popResultBreakDownTime.iindexMemcpy.time += getTimeDifference(&start_time,&end_time);
	                ++PagesResults->popResultBreakDownTime.iindexMemcpy.nr;
		#endif


		#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
		#endif
		//Så lenge vi ikke har crc32 for alle domener
		if (side->DocumentIndex.crc32 == 0) {
			side->DocumentIndex.crc32 = crc32boitho(htmlBuffer);
		
			/*
			if (((*PagesResults).filterOn) && (filterSameCrc32(localshowabal,side,(*PagesResults).Sider))) {
	                      	vboprintf("hav same crc32\n");
				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameCrc32_2,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                	      	continue;
                	}
			*/
		}
		#ifdef DEBUG_TIME
		        gettimeofday(&end_time, NULL);
        	        PagesResults->popResultBreakDownTime.makecrc32.time += getTimeDifference(&start_time,&end_time);
	                ++PagesResults->popResultBreakDownTime.makecrc32.nr;

		#endif

		/*
		***************************************************************************/		
		#ifndef BLACK_BOKS

		//kvalitetsjekker på inn data.
		if (((*PagesResults).filterOn) && (filterTitle(side->title) )) {
			vboprintf("bad title \"%s\"\n",side->title);
			increaseFiltered(PagesResults,NULL,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}

		if (((*PagesResults).filterOn) && (filterSummery(side->description) )) {
			vboprintf("bad summery \"%s\"\n",side->description);
			increaseFiltered(PagesResults,NULL,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}
		#endif		

		gettimeofday(&start_time, NULL);
		#ifdef BLACK_BOKS

			#ifdef DEBUG
			printf("pathaccess: start\n");
			#endif

			//temp: kortslutter får å implementere sudo. Må implementeres skikkelig, men å spørre boithoad
			if (strcmp((*PagesResults).password,"water66") == 0) {
				printf("pathaccess: have sodo password. Won't do pathaccess\n");
			}
			else if (PagesResults->TeffArray->iindex[i].subname->config.has_config && !pathaccess(PagesResults, (*PagesResults).cmcsocketha,(*(*PagesResults).TeffArray->iindex[i].subname).subname,side->url,(*PagesResults).search_user,(*PagesResults).password)) {
				fprintf(stderr, "searchkernel: Access denied for file \"%s\" in %s\n", side->url, (*(*PagesResults).TeffArray->iindex[i].subname).subname);

				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cmc_pathaccess,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

				//temp:
				//strcpy(side->title,"Access denied!");
				//strcpy(side->description,"");
				continue;

			}
			#ifdef DEBUG
			printf("pathaccess: done\n");
			#endif

			#ifdef DEBUG
			printf("url rewrite: start\n");
		#endif

		gettimeofday(&end_time, NULL);
		(*(*PagesResults).SiderHeder).queryTime.pathaccess += getTimeDifference(&start_time,&end_time);


		gettimeofday(&start_time, NULL);

#ifdef BLACK_BOKS
		if (!PagesResults->getRank) {

			if (!handle_url_rewrite(side->url, sizeof(side->url),
				PagesResults->ptype, PagesResults->btype, 
				(*PagesResults).TeffArray->iindex[i].subname->subname, side->url, 
				sizeof(side->url), side->uri, sizeof(side->uri), side->fulluri, sizeof(side->fulluri),
				PagesResults->cmcsocketha, 
#ifdef WITH_THREAD
				&PagesResults->cmConn
#else
				NULL
#endif
			)) {
				snprintf(side->uri, sizeof(side->uri),"XXX-Can't_rewrite__url_for_DocID_%u_XXX:%s",(*PagesResults).TeffArray->iindex[i].DocID);
			}

		}
#endif


		vboprintf("url rewrite done:\nurl \"%s\"\nuri \"%s\"\nfulluri \"%s\"\n",side->url,side->uri,side->fulluri);

		#endif
		gettimeofday(&end_time, NULL);
		(*(*PagesResults).SiderHeder).queryTime.urlrewrite += getTimeDifference(&start_time,&end_time);


		if (1 || !PagesResults->getRank) {
			//urI
			//strscpy(side->uri, side->url, sizeof(side->uri));
			//urL
			//strscpy(side->url,side->DocumentIndex.Url,sizeof(side->url));
			//

			side->pathlen = find_domain_path_len(side->url);

#ifndef BLACK_BOKS
			memcpy(side->uri, side->url, sizeof(side->uri));
			memcpy(side->fulluri, side->url, sizeof(side->fulluri));
#endif

#ifdef BLACK_BOKS
			//memcpy(side->fulluri, side->uri, sizeof(side->fulluri));
#endif
			//shortenurl(side->uri,sizeof(side->uri));

			strcpy(side->servername,(*PagesResults).servername);
		}

		//kopierer over subname
		side->subname = (*(*PagesResults).TeffArray->iindex[i].subname);

		side->posisjon = localshowabal +1;


		side->bid = 0;


		side->type = siderType_normal;
		
		side->deletet = 0;
	


		/*******************************************************************************************
		filtere som krever minne trå synkronisering
		*******************************************************************************************/

		#ifdef WITH_THREAD
		pthread_mutex_lock(&(*PagesResults).mutextreadSyncFilter);
		#endif

		#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
		#endif

		int treadSyncFilters = 0;

		//håntering av at vi kan ha mer en en pi.
		if (pi_switch(nrofGodPages(PagesResults),side,(*PagesResults).Sider)) {
			vboprintf("filter (treadSyncFilter): switch a pi \"%s\"\n",side->DocumentIndex.Url);
			//continue;
			treadSyncFilters = 1;
			goto end_filter_lock;

		}

		if (((*PagesResults).filterOn) && (filterSameCrc32(nrofGodPages(PagesResults),side,(*PagesResults).Sider))) {
                      	vboprintf("filter (treadSyncFilter): hav same crc32\n");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameCrc32_2,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
               	      	//continue;
			treadSyncFilters = 1;
			goto end_filter_lock;
               	}


		//fjerner eventuelt like urler
		if (((*PagesResults).filterOn) && (filterSameUrl(nrofGodPages(PagesResults),side->DocumentIndex.Url,(*PagesResults).Sider)) ) {
			vboprintf("filter (treadSyncFilter): Hav seen url befor. Url \"%s\"\n",side->DocumentIndex.Url);
			
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameUrl,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        //continue;
			treadSyncFilters = 1;
			goto end_filter_lock;
		}

		#ifndef BLACK_BOKS


#ifndef BLACK_BOKS
		if (((*PagesResults).filterOn) && (filterSameDomain(nrofGodPages(PagesResults),side,(*PagesResults).Sider))) {

			vboprintf("filter (treadSyncFilter): hav same domain. Domain: \"%s\", domain id %ho. Url %s\n",side->domain,side->DomainID,side->DocumentIndex.Url);

			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameDomain,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			//continue;	
			// Det kan være at vi har en side som har lik domene, men er høyere rankert en den nåverende. Dette skjer da
			// en annen tråd ble ferdig før oss. Bytter om hvis dette er tilfellet
			//Man kan så gå ut av side genereringsloopen, da dette er siste filter.
			//
			//må dog ha sat ting som sidetype først, så ikke der et tilfeldig
 
		        for (y=0;y<nrofGodPages(PagesResults);y++) {

                		if (!(*PagesResults).Sider[y].deletet) {

                        		if ((strcmp(side->domain,(*PagesResults).Sider[y].domain)) == 0 && ((*PagesResults).Sider[y].iindex.allrank < side->iindex.allrank)) {

						vboprintf("filter (treadSyncFilter): have same domain, but page \"%s\" is higher ranked then \"%s\". swaping it\n",side->DocumentIndex.Url,(*PagesResults).Sider[y].DocumentIndex.Url);
						
						(*PagesResults).Sider[y] = *side;

						break;
	
        	                	}
				}
			}


			
			treadSyncFilters = 1;
			goto end_filter_lock;	
		}
#endif

		#endif


		end_filter_lock:

		#ifdef DEBUG_TIME
		        gettimeofday(&end_time, NULL);
        	        PagesResults->popResultBreakDownTime.treadSyncFilter.time += getTimeDifference(&start_time,&end_time);
	                ++PagesResults->popResultBreakDownTime.treadSyncFilter.nr;

		#endif

		#ifdef WITH_THREAD
			pthread_mutex_unlock(&(*PagesResults).mutextreadSyncFilter);
		#endif
		
		if (treadSyncFilters == 1) {
			vboprintf("Page did get filtered in treadSyncFilter\n");
			continue;
		}

		/*******************************************************************************************/


		
		//gjør post god side ting, som å øke showabal
		int myPageNr;
		myPageNr = foundGodPage(PagesResults);

		//kopierer siden inni den globale side arrayen
		(*PagesResults).Sider[myPageNr] = *side;
		
		vboprintf("did copy page \"%s\" into spot %i. (read back \"%s\")\n",side->DocumentIndex.Url,myPageNr,side->subname.subname);
		
		break; //går ut av loopen. Vi har funnet at vår index hit var brukenes, vi trenger da en ny side
	}
	}

	//printf("******************************\nfreeing htmlBuffer\n******************************\n");
	free(htmlBuffer);
	free(side);

	#ifdef WITH_THREAD
	//må man kelle denne?
	//pthread_exit(NULL);
	#endif

	vboprintf("searchkernel: ~generatePagesResults()\n");
	//return 1;

	return NULL;
}

int sider_allrank_sort (const void *p1, const void *p2) {
	
	if (((struct SiderFormat *)p1)->iindex.allrank > ((struct SiderFormat *)p2)->iindex.allrank) {
		 return -1;
	}
	else {
		return (((struct SiderFormat *)p1)->iindex.allrank < ((struct SiderFormat *)p2)->iindex.allrank);
	}
}

int sider_device_sort (const void *p1, const void *p2) {
	
	if (((struct iindexMainElements *)p1)->phraseMatch < ((struct iindexMainElements *)p2)->phraseMatch) {
		 return -1;
	}
	else {
		return (((struct iindexMainElements *)p1)->phraseMatch > ((struct iindexMainElements *)p2)->phraseMatch);
	}
}

void print_explane_rank(struct SiderFormat *Sider, int showabal) {

	int i;

	printf("|%-10s|%-10s|%-10s||%-10s|%-10s|%-10s|%-24s|%-10s|%-10s|%-5s|\n",
		"AllRank",
		"TermRank",
		"PopRank",
		"Body",
		"Headline",
		"Tittel",
		"Athor (nr nrp)",
		"UrlM",
		"Url",
		"Adult"
		);
	printf("|----------|----------|----------||----------|----------|----------|------------------------|----------|----------|-----|\n");

	for(i=0;i<showabal;i++) {
                        printf("|%10i|%10i|%10i||%10i|%10i|%10i|%10i (%5i %5i)|%10i|%10i|%5d| %s (DocID %u-%i, DomainID %d, s: \"%s\")\n",

				Sider[i].iindex.allrank,
				Sider[i].iindex.TermRank,
				Sider[i].iindex.PopRank,

				#ifdef EXPLAIN_RANK
					Sider[i].iindex.rank_explaind.rankBody,
					Sider[i].iindex.rank_explaind.rankHeadline,
					Sider[i].iindex.rank_explaind.rankTittel,
				#else 
					-1,-1,-1,
				#endif


				#ifdef BLACK_BOKS
				-1,-1,-1,-1,-1,-1,
				#else
				Sider[i].iindex.rank_explaind.rankAthor,
				Sider[i].iindex.rank_explaind.nrAthor,
				Sider[i].iindex.rank_explaind.nrAthorPhrase,
				Sider[i].iindex.rank_explaind.rankUrl_mainbody,
				Sider[i].iindex.rank_explaind.rankUrlDomain + Sider[i].iindex.rank_explaind.rankUrlSub,
				Sider[i].DocumentIndex.AdultWeight,
				#endif

				Sider[i].DocumentIndex.Url,
				Sider[i].iindex.DocID,
				rLotForDOCid(Sider[i].iindex.DocID),

				#ifdef BLACK_BOKS
				-1,
				#else
				Sider[i].DomainID,
				#endif

				(*Sider[i].iindex.subname).subname

				);
	}

	#ifdef DEBUG
	printf("uri:\n");
	for(i=0;i<showabal;i++) {
		printf("%s\n",Sider[i].uri);
	}
	#endif

}

#ifdef WITH_SPELLING
int
spellcheck_query(struct SiderHederFormat *SiderHeder, query_array *qa)
{
	int i;
	int fixed;


	fixed = 0;
	for(i = 0; i < qa->n; i++) {
		string_array *sa = &qa->query[i];
		switch (sa->operand) {
			case QUERY_WORD:
			case QUERY_SUB:
			{
				char *p;
				int found;

				if (correct_word(&spelling, sa->s[0]))
					continue;

				p = check_word(&spelling, sa->s[0], &found);
				if (p == NULL)
					continue;

				printf("Found correct spelling: %s\n", p);
				free(sa->s[0]);
				sa->s[0] = p;
				fixed++;
				break;
			}
		}
	}

	if (fixed > 0) {
		sprint_query(SiderHeder->spellcheckedQuery, MaxQueryLen, qa);
	}

	return fixed;
}
#endif

int dosearch(char query[], int queryLen, struct SiderFormat **Sider, struct SiderHederFormat *SiderHeder,
char *hiliteQuery, char servername[], struct subnamesFormat subnames[], int nrOfSubnames, 
int MaxsHits, int start, int filterOn, char languageFilter[],char orderby[],int dates[], 
char search_user[],struct filtersFormat *filters,struct searchd_configFORMAT *searchd_config, char *errorstr,int *errorLen,
	struct iintegerMemArrayFormat *DomainIDs, char *useragent, char groupOrQuery[], int anonymous
	) { 


	vboprintf("searchkernel: dosearch(query=\"%s\")\n", query);
	struct PagesResultsFormat PagesResults;
	struct filteronFormat filteron;
	struct hashtable *crc32maphash;
	struct duplicate_docids *dups;

	memset(&PagesResults,'\0',sizeof(PagesResults));


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
	PagesResults.start = start;
	PagesResults.searchd_config = searchd_config;
	PagesResults.anonymous = anonymous;

	#ifdef DEBUG_TIME
		PagesResults.popResultBreakDownTime.DocumentIndex.nr = 0;
		PagesResults.popResultBreakDownTime.DocumentIndex.time = 0;

		PagesResults.popResultBreakDownTime.ReadSummary.nr = 0;
		PagesResults.popResultBreakDownTime.ReadSummary.time = 0;

		PagesResults.popResultBreakDownTime.generate_snippet.nr = 0;
		PagesResults.popResultBreakDownTime.generate_snippet.time = 0;

		PagesResults.popResultBreakDownTime.html_parser_run.nr = 0;
		PagesResults.popResultBreakDownTime.html_parser_run.time = 0;

		PagesResults.popResultBreakDownTime.ReadHtml.nr = 0;
		PagesResults.popResultBreakDownTime.ReadHtml.time = 0;


		PagesResults.popResultBreakDownTime.memGetDomainID.nr = 0;
		PagesResults.popResultBreakDownTime.memGetDomainID.time = 0;

		PagesResults.popResultBreakDownTime.totalpopResult.nr = 0;
		PagesResults.popResultBreakDownTime.totalpopResult.time = 0;

		PagesResults.popResultBreakDownTime.makecrc32.nr = 0;
		PagesResults.popResultBreakDownTime.makecrc32.time = 0;

		PagesResults.popResultBreakDownTime.treadSyncFilter.nr = 0;
		PagesResults.popResultBreakDownTime.treadSyncFilter.time = 0;

		PagesResults.popResultBreakDownTime.bodyClean.nr = 0;
		PagesResults.popResultBreakDownTime.bodyClean.time = 0;

		PagesResults.popResultBreakDownTime.titleClean.nr = 0;
		PagesResults.popResultBreakDownTime.titleClean.time = 0;

		PagesResults.popResultBreakDownTime.iindexMemcpy.nr = 0;
		PagesResults.popResultBreakDownTime.iindexMemcpy.time = 0;

		PagesResults.popResultBreakDownTime.popResultFree.nr = 0;
		PagesResults.popResultBreakDownTime.popResultFree.time = 0;
	#endif

	//hvr vi skal begynne. Vå bruker dog navn som 1, 2 osv til brukeren, men starter på 0 internt 
	//dette har dog dispatsher_all allerede håntert, ved å trekke fra en
	//vi begynner altid på null, og så genererer side alle linkene. Dette betyr at for side 3 må vi generere 30 linker
	//det er mer tidkrevende, men vi slipper å holde status om hvor langt ut i linken vi er.
	//frykter dog at dette vil fungere dorlig på web :(
	/*
	#ifdef BLACK_BOKS
		//PagesResults.indexnr = 0; 
		//PagesResults.MaxsHits = MaxsHits * start;

		PagesResults.indexnr = (start * MaxsHits);
		PagesResults.MaxsHits = MaxsHits;

	#else
		PagesResults.indexnr = (start * MaxsHits);
		PagesResults.MaxsHits = MaxsHits;

	#endif
	*/
	PagesResults.indexnr = (start * MaxsHits);
	PagesResults.MaxsHits = MaxsHits;

	if ((*Sider  = malloc(sizeof(struct SiderFormat) * PagesResults.MaxsHits)) == NULL) {
		perror("malloc Sider");
		exit(1);
	}


	PagesResults.Sider = *Sider;

	vboprintf("MaxsHits %i, indexnr %i\n",PagesResults.MaxsHits,PagesResults.indexnr);

	strscpy(PagesResults.search_user,search_user,sizeof(PagesResults.search_user));
	//PagesResults.password
	PagesResults.DomainIDs = DomainIDs;
	PagesResults.filtered		= 0;
	PagesResults.filteredsilent	= 0;
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
	PagesResults.TeffArray->nrofHits = 0;

	//int adultpages, noadultpages;
	//struct QueryDataForamt QueryData;


	struct timeval start_time, end_time;
	struct timeval popResult_start_time, popResult_end_time;

	int rank;
	int ret;

	//7int filtered; //antal sider som ble filtrert ut
	//int showabal; //holder antal sider som kunne vises

	struct DocumentIndexFormat DocumentIndexPost;
	
	(*SiderHeder).queryTime.cmc_conect = 0;
	(*SiderHeder).queryTime.pathaccess = 0;
	(*SiderHeder).queryTime.urlrewrite = 0;
	#ifdef BLACK_BOKS
	(*SiderHeder).queryTime.html_parser_run = 0;
	(*SiderHeder).queryTime.generate_snippet = 0;
	(*SiderHeder).queryTime.duplicat_echecking = 0;
	(*SiderHeder).queryTime.FilterCount = 0;
	#endif

	#if defined BLACK_BOKS && !defined _24SEVENOFFICE


		if (!PagesResults.anonymous) {
			//henter brukerens passord fra boithoad
			gettimeofday(&start_time, NULL);
			//henter inn brukerens passord
			vboprintf("geting pw for \"%s\"\n",PagesResults.search_user);
			/**************************************************************************
			Runarb: 08 Jan 2008: Gjør av vi bare hopper over å hente gruppe info hvis vi ikke
			for det til.

			Før stoppet vi opp, men det gjør at det ikke fungerer med 24so søk. 
			**************************************************************************/
			if (!boithoad_getPassword(PagesResults.search_user,PagesResults.password)) {
				printf("Can't boithoad_getPassword. Brukeren er ikke logget inn??\n");
				//(*errorLen) = snprintf(errorstr,(*errorLen),"Can't get user info from authentication backend");
				//return(0);
			}
			else {
				//printf("got pw \"%s\" -> \"%s\"\n",PagesResults.search_user,PagesResults.password);
				gettimeofday(&end_time, NULL);
				(*SiderHeder).queryTime.getUserObjekt = getTimeDifference(&start_time,&end_time);
			}
		} else {
			vboprintf("Anonymous search\n");
		}


		/****************************************************************/

		//int socketha;
		// XXX
#ifndef WITH_THREAD
		gettimeofday(&start_time, NULL);
		int errorbufflen = 512;
		char errorbuff[errorbufflen];
		vboprintf("making a connection(s) to crawlerManager(s)\n");
		if (!cmc_conect(&PagesResults.cmcsocketha,errorbuff,errorbufflen,(*searchd_config).cmc_port)) {
                        //printf("Error: %s:%i\n",errorbuff,(*searchd_config).cmc_port);
			(*errorLen) = snprintf(errorstr,(*errorLen),"Can't connect to crawler manager: \"%s\", port %i",errorbuff,(*searchd_config).cmc_port);

			fprintf(stderr, "searchkernel: ~dosearch()\n");
                        return(0);
	        }
		gettimeofday(&end_time, NULL);
	        (*SiderHeder).queryTime.cmc_conect = getTimeDifference(&start_time,&end_time);
#endif


		
	#endif


	//kopierer query inn i strukturen som holder query date
	strscpy(PagesResults.QueryData.query,query,sizeof(PagesResults.QueryData.query));

	get_query( PagesResults.QueryData.query, queryLen, &PagesResults.QueryData.queryParsed );


	#if defined BLACK_BOKS && !defined _24SEVENOFFICE

		get_query( groupOrQuery, strlen(groupOrQuery), &PagesResults.QueryData.search_user_as_query );


#ifndef WITHOUT_THESAURUS
		// Kjør stemming på query:
		if (searchd_config->thesaurusp != NULL) {
    			thesaurus_expand_query(searchd_config->thesaurusp, &PagesResults.QueryData.queryParsed);
		}
#endif
   		// Print query med innebygd print-funksjon:
    		char        buf[1024];
    		sprint_expanded_query(buf, 1023, &PagesResults.QueryData.queryParsed);
    		vboprintf("\nExpanded query: %s\n\n", buf);

	#endif

	#ifdef DEBUG
        printf("query %s\n",PagesResults.QueryData.query);
	#endif

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
	#ifdef DEBUG
	int h;
	for (h=0;h<languageFilternr;h++) {
		printf("ll %i\n",languageFilterAsNr[h]);
	}
	printf("languageFilternr %i\n",languageFilternr);
	#endif
	// :temp

	PagesResults.activetreads = 1;

	#ifdef WITH_THREAD
		if (!searchd_config->optSingle) {
			PagesResults.activetreads = NROF_GENERATEPAGES_THREADS;
		}

		pthread_t threadid[PagesResults.activetreads];

		dp_init();

		
		ret = pthread_mutex_init(&PagesResults.mutex, NULL);
		ret = pthread_mutex_init(&PagesResults.mutextreadSyncFilter, NULL);

		#ifdef BLACK_BOKS
			ret = pthread_mutex_init(&PagesResults.cmConn.mutex, NULL);
			pthread_cond_init(&PagesResults.cmConn.cv, NULL);

			int errorbufflen = 512;
			char errorbuff[errorbufflen];
			{
				int k;
				struct socket_pool *pool = &PagesResults.cmConn;
				gettimeofday(&start_time, NULL);
				for (k = 0; k < MAX_CM_CONSUMERS; k++) {
					vboprintf("making a connection to crawlerManager: %d:%d\n", k, MAX_CM_CONSUMERS);
					pool->used[k] = 0;
					if (!cmc_conect(&pool->sock[k], errorbuff,errorbufflen,(*searchd_config).cmc_port)) {
						printf("Error: %s:%i\n",errorbuff,(*searchd_config).cmc_port);
						fprintf(stderr, "searchkernel: ~dosearch()\n");
						return(0);
					}
				}
				pool->consumers = 0;
				gettimeofday(&end_time, NULL);
			}
			(*SiderHeder).queryTime.cmc_conect += getTimeDifference(&start_time,&end_time);
		#endif

		//låser mutex. Vi er jo enda ikke kalre til å kjøre
		pthread_mutex_lock(&PagesResults.mutex);

		if (!searchd_config->optSingle) {

			//start som thread that can get the pages
			for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
				ret = pthread_create(&threadid[i], NULL, generatePagesResults, &PagesResults);		
			}

		}
	#endif

	#ifdef DEBUG
	printf("searchSimple\n");
	#endif

	searchSimple(&PagesResults.antall,&PagesResults.TeffArray,&(*SiderHeder).TotaltTreff,
			&PagesResults.QueryData.queryParsed,&(*SiderHeder).queryTime,
			subnames,nrOfSubnames,languageFilternr,languageFilterAsNr,
			orderby,
			filters,&filteron,&PagesResults.QueryData.search_user_as_query, 0, &crc32maphash, &dups,search_user, searchd_config->cmc_port, PagesResults.anonymous);
	PagesResults.crc32maphash = crc32maphash;
	PagesResults.dups = dups;

	#ifdef DEBUG
	printf("end searchSimple\n");
	#endif


	//intresnag debug info
	#ifdef BLACK_BOKS
		//viser hvordan treffene er i subnames
		if (globalOptVerbose) {
			printf("subname records:\n");
			for (i=0;i<nrOfSubnames;i++) {
				printf("\t\"%s\": %i\n",subnames[i].subname,subnames[i].hits);
			}
		}
	#endif

	//setter alle sidene som sletett
	for (i=0;i<PagesResults.MaxsHits;i++) {
		(*Sider)[i].deletet = 1;
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
	for (i=0;((i<1000) && (i<PagesResults.antall));i++) {

		//printf("DocID %u, aw %i\n",PagesResults.TeffArray[i].DocID,adultWeightForDocIDMemArray(PagesResults.TeffArray[i].DocID));
		if (adultWeightForDocIDMemArray(PagesResults.TeffArray->iindex[i].DocID)) {
			++PagesResults.adultpages;	
		}
		else {
			++PagesResults.noadultpages;
		}
	}

	
	gettimeofday(&end_time, NULL);
        (*SiderHeder).queryTime.adultcalk = getTimeDifference(&start_time,&end_time);
	////////

	//går i utbangspungete gjenon alle sider, men eskaper når vi når anatllet vi vil ha
	PagesResults.showabal = 0;
	//PagesResults.godPages = 0;
	PagesResults.nextPage = 0;

//cuted

	#ifndef BLACK_BOKS
		//sorterer top treffene etter hvilken disk de ligger på, slik at vi kan jobbe mest mulig i paralell
		char diskAcces[NrOfDataDirectorys];
		int toSort = PagesResults.antall;
		if (toSort > PagesResults.MaxsHits) {
			toSort = PagesResults.MaxsHits;
		}

		for (i=0;i<NrOfDataDirectorys;i++) {
			diskAcces[i] = 0;
		}

		/*
		Runarb: 14 mai 2008:
		Lager en oversikt over antall disk akesser til hver device. Så hvis vi har device a og b, men følgende access 
		rekkefølge:
			a a a b b b
		for vi da 
			a 1
			a 2
			a 3
			b 1
			b 2
			b 3
		og vi kan så sortere på antal for å få
			a 1
			b 1
			a 2
			b 2
			a 3
			b 3
		*/

		for(i=0;i<toSort;i++) {
			//temp: nå rebruker vi hraseMatch til dette. Vi må ha et egent felt siden
			PagesResults.TeffArray->iindex[i].phraseMatch = diskAcces[GetDevIdForLot(rLotForDOCid(PagesResults.TeffArray->iindex[i].DocID))]++;
		}

		qsort(PagesResults.TeffArray->iindex,toSort,sizeof(struct iindexMainElements),sider_device_sort);

		printf("devise sort:\n");
		for(i=0;i<toSort;i++) {
			printf("DocID %u, device %i, nr %i\n",PagesResults.TeffArray->iindex[i].DocID,GetDevIdForLot(rLotForDOCid(PagesResults.TeffArray->iindex[i].DocID)),(int)PagesResults.TeffArray->iindex[i].phraseMatch);
		}				

	#endif

	gettimeofday(&popResult_start_time, NULL);


	#ifdef WITH_THREAD

		//vi har data. Lå tårdene jobbe med det
		pthread_mutex_unlock(&PagesResults.mutex);

		//av gjør om vi skal starte tråder eller kjøre det selv
		if (searchd_config->optSingle) {
			generatePagesResults(&PagesResults);
		}
		else {


			//venter på trådene
			for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
				ret = pthread_join(threadid[i], NULL);
			}

		}

		//free mutex'en
		ret = pthread_mutex_destroy(&PagesResults.mutex);
		ret = pthread_mutex_destroy(&PagesResults.mutextreadSyncFilter);

		#ifdef BLACK_BOKS
			ret = pthread_mutex_destroy(&PagesResults.cmConn.mutex);
		#endif


	#else 
		generatePagesResults(&PagesResults);		
	#endif

	#ifdef BLACK_BOKS
#ifdef WITH_THREAD
	{
		int k;

		for (k = 0; k < MAX_CM_CONSUMERS; k++)
			cmc_close(PagesResults.cmConn.sock[k]);
	}
#else
	cmc_close(PagesResults.cmcsocketha);
#endif
	#endif

	gettimeofday(&popResult_end_time, NULL);
        (*SiderHeder).queryTime.popResult = getTimeDifference(&popResult_start_time,&popResult_end_time);


	/*
	en siste sortering er nødvendig da sidene kan være i usortert rekkefølgde. Dette skjer da 
	side nr ut til trådene sekvensielt, men det kan være at de så leser arrayen i annen rekkefølge. 
	Eller at den siden ikke skal være med lengere (ble slettet)
	*/
	qsort(*Sider,PagesResults.showabal,sizeof(struct SiderFormat),sider_allrank_sort);


	//kopierer over verdien
	(*SiderHeder).showabal = PagesResults.showabal;

	//trekker fra de som ble filtrert ut stille, altså ikke vises til brukeren at de ble filtrert ut. Typisk 404, 500 feil og lignende, som ikke gir noen verdi for brukeren
	//runarb: 14.11.2007: 	dette ser ut til å lage showabal som negatift tall av og til. 
	//			det er også feil, da showabal nå er faktisk telte sider
	//(*SiderHeder).showabal -= PagesResults.filteredsilent;




	//lager en filtered verdi
	(*SiderHeder).filtered = PagesResults.filtered + PagesResults.memfiltered;	

	//runarb: 2 nov 2007: trkker ikke fra disse tallene, da det fører til at totalt treff tallene forandres. Viser heller en filtered beskjed
	#ifndef BLACK_BOKS
		//fjerner filtered fra total oversikten
		//ToDo: bør dette også gjøres for web?
		//runarb: 23 jan 2008: ser ut til at dette bare gjøres for web er ifNdef her???
		(*SiderHeder).TotaltTreff -= (*SiderHeder).filtered;
		//tar også bort de som ble filtrert stille
		(*SiderHeder).TotaltTreff -= PagesResults.filteredsilent;
	#endif


	
	


	#ifdef BLACK_BOKS
		//char	*querystr = asprint_query(&PagesResults.QueryData.queryParsed);
		(*SiderHeder).navigation_xml = searchFilterCount(&PagesResults.antall,PagesResults.TeffArray,filters,subnames,nrOfSubnames,&filteron,dates,
		    &(*SiderHeder).queryTime, searchd_config->getfiletypep, searchd_config->attrdescrp, searchd_config->showattrp, &PagesResults.QueryData.queryParsed);

		(*SiderHeder).navigation_xml_len = strlen((*SiderHeder).navigation_xml);

		//free(querystr);
		destroy(filteron.attributes);
		free(filteron.collection);
		free(filteron.date);
		free(filteron.sort);
	#endif

	#ifdef WITH_SPELLING

	SiderHeder->spellcheckedQuery[0] = '\0';

	if (searchd_config->optFastStartup != 1) {

		/* Spellcheck the query */
		if (SiderHeder->TotaltTreff < 10) {
			query_array qa;

			#ifdef DEBUG_TIME
                		gettimeofday(&start_time, NULL);
        		#endif

			copy_query(&qa, &PagesResults.QueryData.queryParsed);

			if (spellcheck_query(SiderHeder, &qa) > 0) {
				printf("Query corrected to: %s\n", SiderHeder->spellcheckedQuery);
			}
			destroy_query(&qa);

			#ifdef DEBUG_TIME
                		gettimeofday(&end_time, NULL);
                		printf("Time debug: spellcheck_query(%s) time: %f\n",PagesResults.QueryData.query,getTimeDifference(&start_time, &end_time));
		        #endif

		}
	}
	else SiderHeder->spellcheckedQuery[0] = '\0';
	#endif

	//lager en liste med ordene som ingikk i queryet til hiliting
	hiliteQuery[0] = '\0';
	//printf("size %i\n",PagesResults.QueryData.queryParsed.size);

	#ifdef DEBUG
	printf("hiliteQuery\n");
	#endif
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
	#ifdef DEBUG
	printf("hiliteQuery END\n");
	#endif

	//det vil bli et komma for mye på slutten, fjerner det.
	//ToDo: er det altid et komma for mye ?	
	if (hiliteQuery[strlen(hiliteQuery) -1] == ',') {hiliteQuery[strlen(hiliteQuery) -1] = '\0';}


	vboprintf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" filtered=\"%i\" showabal=\"%i\"/>\n",(*SiderHeder).TotaltTreff,PagesResults.QueryData.query,hiliteQuery,(*SiderHeder).filtered,(*SiderHeder).showabal);
	

	//printer ut info om brukt tid
	vboprintf("Time\n");
	//printf("\tAthorSearch %f\n",(*SiderHeder).queryTime.AthorSearch);
	vboprintf("\t%-40s %f\n","AthorSearch",(*SiderHeder).queryTime.AthorSearch);
	//printf("\t%-40s %f\n","AthorRank",(*SiderHeder).queryTime.AthorRank);
	vboprintf("\t%-40s %f\n","MainSearch",(*SiderHeder).queryTime.MainSearch);
	//printf("\t%-40s %f\n","MainRank",(*SiderHeder).queryTime.MainRank);
	vboprintf("\t%-40s %f\n","MainAthorMerge",(*SiderHeder).queryTime.MainAthorMerge);
	vboprintf("\n");
	vboprintf("\t%-40s %f\n","popRank",(*SiderHeder).queryTime.popRank);
	vboprintf("\t%-40s %f\n","responseShortning",(*SiderHeder).queryTime.responseShortning);
	vboprintf("\n");
	vboprintf("\t%-40s %f\n","allrankCalc",(*SiderHeder).queryTime.allrankCalc);
	vboprintf("\t%-40s %f\n","indexSort",(*SiderHeder).queryTime.indexSort);
	vboprintf("\t%-40s %f\n","popResult",(*SiderHeder).queryTime.popResult);
	vboprintf("\t%-40s %f\n","adultcalk",(*SiderHeder).queryTime.adultcalk);

	#ifdef BLACK_BOKS
		vboprintf("\t%-40s %f\n","filetypes",(*SiderHeder).queryTime.filetypes);
		vboprintf("\t%-40s %f\n","iintegerGetValueDate",(*SiderHeder).queryTime.iintegerGetValueDate);
		vboprintf("\t%-40s %f\n","dateview",(*SiderHeder).queryTime.dateview);
		vboprintf("\t%-40s %f\n","FilterCount",(*SiderHeder).queryTime.FilterCount);
		vboprintf("\t%-40s %f\n","pathaccess",(*SiderHeder).queryTime.pathaccess);
		vboprintf("\t%-40s %f\n","urlrewrite",(*SiderHeder).queryTime.urlrewrite);
		vboprintf("\t%-40s %f\n","duplicat echecking",(*SiderHeder).queryTime.duplicat_echecking);
		vboprintf("\t%-40s %f\n","cmc_conect",(*SiderHeder).queryTime.cmc_conect);
	#endif

	#ifndef _24SEVENOFFICE
		vboprintf("\t%-40s %f\n","getUserObjekt",(*SiderHeder).queryTime.getUserObjekt);
	#endif

	#ifdef DEBUG_TIME
		printf("\npopResult:\n");
		printf("\t%-40s %i, %f\n","DocumentIndex",PagesResults.popResultBreakDownTime.DocumentIndex.nr,PagesResults.popResultBreakDownTime.DocumentIndex.time);
		printf("\t%-40s %i, %f\n","ReadSummary",PagesResults.popResultBreakDownTime.ReadSummary.nr,PagesResults.popResultBreakDownTime.ReadSummary.time);

		printf("\t%-40s %i, %f\n","generate_snippet",PagesResults.popResultBreakDownTime.generate_snippet.nr,PagesResults.popResultBreakDownTime.generate_snippet.time);
		printf("\t%-40s %i, %f\n","html_parser_run",PagesResults.popResultBreakDownTime.html_parser_run.nr,PagesResults.popResultBreakDownTime.html_parser_run.time);
		printf("\t%-40s %i, %f\n","ReadHtml",PagesResults.popResultBreakDownTime.ReadHtml.nr,PagesResults.popResultBreakDownTime.ReadHtml.time);

		printf("\t%-40s %i, %f\n","memGetDomainID",PagesResults.popResultBreakDownTime.memGetDomainID.nr,PagesResults.popResultBreakDownTime.memGetDomainID.time);
		printf("\t%-40s %i, %f\n","totalpopResult",PagesResults.popResultBreakDownTime.totalpopResult.nr,PagesResults.popResultBreakDownTime.totalpopResult.time);
		printf("\t%-40s %i, %f\n","makecrc32",PagesResults.popResultBreakDownTime.makecrc32.nr,PagesResults.popResultBreakDownTime.makecrc32.time);
		printf("\t%-40s %i, %f\n","treadSyncFilter",PagesResults.popResultBreakDownTime.treadSyncFilter.nr,PagesResults.popResultBreakDownTime.treadSyncFilter.time);
		printf("\t%-40s %i, %f\n","titleClean",PagesResults.popResultBreakDownTime.titleClean.nr,PagesResults.popResultBreakDownTime.titleClean.time);
		printf("\t%-40s %i, %f\n","bodyClean",PagesResults.popResultBreakDownTime.bodyClean.nr,PagesResults.popResultBreakDownTime.bodyClean.time);
		printf("\t%-40s %i, %f\n","iindexMemcpy",PagesResults.popResultBreakDownTime.iindexMemcpy.nr,PagesResults.popResultBreakDownTime.iindexMemcpy.time);
		printf("\t%-40s %i, %f\n","popResultFree",PagesResults.popResultBreakDownTime.popResultFree.nr,PagesResults.popResultBreakDownTime.popResultFree.time);

	#endif



	vboprintf("filters:\n");

	vboprintf("\t%-40s %i\n","cantDIRead",(*SiderHeder).filtersTraped.cantDIRead);
	vboprintf("\t%-40s %i\n","getingDomainID",(*SiderHeder).filtersTraped.getingDomainID);
	vboprintf("\t%-40s %i\n","sameDomainID",(*SiderHeder).filtersTraped.sameDomainID);

	vboprintf("\t%-40s %i\n","filterAdultWeight_bool",(*SiderHeder).filtersTraped.filterAdultWeight_bool);
	vboprintf("\t%-40s %i\n","filterAdultWeight_value",(*SiderHeder).filtersTraped.filterAdultWeight_value);
	vboprintf("\t%-40s %i\n","filterSameCrc32_1",(*SiderHeder).filtersTraped.filterSameCrc32_1);
	vboprintf("\t%-40s %i\n","filterSameUrl",(*SiderHeder).filtersTraped.filterSameUrl);
	vboprintf("\t%-40s %i\n","filterNoUrl",(*SiderHeder).filtersTraped.filterNoUrl);
	vboprintf("\t%-40s %i\n","find_domain_no_subname",(*SiderHeder).filtersTraped.find_domain_no_subname);
	vboprintf("\t%-40s %i\n","filterSameDomain",(*SiderHeder).filtersTraped.filterSameDomain);
	vboprintf("\t%-40s %i\n","filterTLDs",(*SiderHeder).filtersTraped.filterTLDs);
	vboprintf("\t%-40s %i\n","filterResponse",(*SiderHeder).filtersTraped.filterResponse);
	vboprintf("\t%-40s %i\n","cantpopResult",(*SiderHeder).filtersTraped.cantpopResult);
	vboprintf("\t%-40s %i\n","cmc_pathaccess",(*SiderHeder).filtersTraped.cmc_pathaccess);
	vboprintf("\t%-40s %i\n","filterSameCrc32_2",(*SiderHeder).filtersTraped.filterSameCrc32_2);

	vboprintf("\n");
	vboprintf("\t%-40s %i\n","filteredsilent",PagesResults.filteredsilent);

	vboprintf("\n");
	vboprintf("\tnoadultpages %i, adultpages %i\n",PagesResults.noadultpages,PagesResults.adultpages);

	vboprintf("\n\n");


	if (globalOptVerbose) {
		print_explane_rank(*Sider,(*SiderHeder).showabal);
	}

        #ifdef DEBUG_TIME
                gettimeofday(&start_time, NULL);
        #endif

	// Frigjør minne:
	vboprintf("free memory\n");

	free(PagesResults.TeffArray);

	// Slett innholdet i crc32maphash:
	/*
	struct hashtable_itr *itr = hashtable_iterator(PagesResults.crc32maphash);

	while (itr->e!=NULL)
	    {
		struct duplicate_docids *dup = hashtable_iterator_value(itr);
		if (dup->V != NULL) {
			destroy(dup->V);
		}
		free(dup);
		hashtable_iterator_advance(itr);
	    }

	free(itr);
	*/
	free(PagesResults.dups);

	hashtable_destroy(PagesResults.crc32maphash,0);

	destroy_query( &PagesResults.QueryData.queryParsed );
	destroy_query( &PagesResults.QueryData.search_user_as_query );
        #ifdef DEBUG_TIME
                gettimeofday(&end_time, NULL);
                printf("Time debug: freeing mem time: %f\n",getTimeDifference(&start_time, &end_time));
        #endif
	vboprintf("searchkernel: ~dosearch()\n");
	return 1;
}


int
dorank(char query[], int queryLen, struct SiderFormat **Sider, struct SiderHederFormat *SiderHeder,
char *hiliteQuery, char servername[], struct subnamesFormat subnames[], int nrOfSubnames, 
int MaxsHits, int start, int filterOn, char languageFilter[],char orderby[],int dates[], 
char search_user[],struct filtersFormat *filters,struct searchd_configFORMAT *searchd_config, char *errorstr,int *errorLen,
	struct iintegerMemArrayFormat *DomainIDs, int rankType, unsigned int rankDocId, int *ranking)
{ 
	fprintf(stderr, "searchkernel: dorank(\"%s\")\n", query);

	struct PagesResultsFormat PagesResults;
	struct filteronFormat filteron;

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

	if ((*Sider  = malloc(sizeof(struct SiderFormat) * (PagesResults.MaxsHits))) == NULL) {
		perror("malloc Sider");
		exit(1);
	}

	PagesResults.Sider = *Sider;


	unsigned short *DomainID;

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
	
	(*SiderHeder).queryTime.urlrewrite = 0;
	(*SiderHeder).queryTime.pathaccess = 0;

 /* False */
	#ifdef BLACK_BOKS
		//henter brukerens passord fra boithoad
		gettimeofday(&start_time, NULL);
		//henter inn brukerens passord
		printf("geting pw for \"%s\"\n",PagesResults.search_user);
		if (!boithoad_getPassword(PagesResults.search_user,PagesResults.password)) {
			//printf("Can't boithoad_getPassword. Brukeren er ikke logget inn??\n");
			(*errorLen) = snprintf(errorstr,(*errorLen),"Can't get user info from authentication backend");
			fprintf(stderr, "searchkernel: ~dorank()\n");
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
		printf("here?\n");
		if (!boithoad_groupsForUser(PagesResults.search_user,&groups_respons_list,&groups_responsnr)) {
                        perror("Error: boithoad_groupsForUser");
			fprintf(stderr, "searchkernel: ~dorank()\n");
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

		vboprintf("groupOrQuery \"%s\"\n",groupOrQuery);
		/****************************************************************/


		gettimeofday(&start_time, NULL);
		//int socketha;
		int errorbufflen = 512;
		char errorbuff[errorbufflen];
		printf("making a connection to crawlerManager\n");
		if (!cmc_conect(&PagesResults.cmcsocketha,errorbuff,errorbufflen,(*searchd_config).cmc_port)) {
                        //printf("Error: %s:%i\n",errorbuff,(*searchd_config).cmc_port);
			(*errorLen) = snprintf(errorstr,(*errorLen),"Can't connect to crawler manager: \"%s\", port %i",errorbuff,(*searchd_config).cmc_port);

			fprintf(stderr, "searchkernel: ~dorank()\n");
                        return(0);
	        }
		gettimeofday(&end_time, NULL);
	        (*SiderHeder).queryTime.cmc_conect = getTimeDifference(&start_time,&end_time);


	#else
		(*SiderHeder).queryTime.cmc_conect = 0;
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



	printf("searchSimple: rank\n");
	
//	struct iindexFormat *unfilteredTeffArray;
//	int unfilteredTeffArrayElementer;

	searchSimple(&PagesResults.antall,&PagesResults.TeffArray,&SiderHeder->TotaltTreff,
			&PagesResults.QueryData.queryParsed,&SiderHeder->queryTime,
			subnames,nrOfSubnames,languageFilternr,languageFilterAsNr,
			orderby,
			filters,&filteron,&PagesResults.QueryData.search_user_as_query, 1, NULL, NULL,search_user, searchd_config->cmc_port, 0);
	// XXX: eirik, we should not discard the duplicate tests
	//&rankDocId);

	printf("end searchSimple\n");

	printf("trying to determen rank for DocID rankDocId: %u\n", rankDocId);
	if (rankType == RANK_TYPE_FIND) {
		*ranking = -1;
		//printf("Some document: %u\n", PagesResults.TeffArray->iindex[i].DocID);
		for (i = 0; i < PagesResults.antall; i++) {
			if (rankDocId == PagesResults.TeffArray->iindex[i].DocID) {
				printf("Found document: %u\n", PagesResults.TeffArray->iindex[i].DocID);
				*ranking = PagesResults.TeffArray->iindex[i].allrank;
				printf("This is actually what we wanted rank is: %d. Nr was %i\n", PagesResults.TeffArray->iindex[i].allrank, i);
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
		(*Sider)[i].deletet = 1;
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



	#ifdef BLACK_BOKS
		cmc_close(PagesResults.cmcsocketha);
	#endif


	if (rankType == RANK_TYPE_SUM) {
		/* If we are getting the current rank we sum up the pages better ranked
		 * than the rank we put in and return that result */
		int ranksum = 0;
		struct hashtable *hash_domainid;

		hash_domainid = create_hashtable(16, hash_domainid_fn, equal_domainid_fn);
		for (i = 0; i < SiderHeder->TotaltTreff; i++) {
			int *hash_key, *hash_value;
			/***************************************************************
			Tror vi hånterer filtrering feil nå
			***************************************************************/
			/*
			runarb: ikke baser deg på PagesResults.Sider da den maxs vil være 10 sider lang
			if (PagesResults.TeffArray->iindex[i].allrank >= *ranking) {
				// XXX 
				if (PagesResults.Sider[i].deletet == 0 && strlen(PagesResults.Sider[i].DocumentIndex.Url) > 0) {
					//printf("Page: %d <<%s>>\n", PagesResults.TeffArray->iindex[i].allrank, PagesResults.Sider[i].DocumentIndex.Url);
					ranksum++;
				}
			}
			*/

			//printf("adult %u: %i\n",PagesResults.TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray(PagesResults.TeffArray->iindex[i].DocID));
			if ((PagesResults.filterOn) && (filterAdultWeight_bool(adultWeightForDocIDMemArray(PagesResults.TeffArray->iindex[i].DocID),PagesResults.adultpages,PagesResults.noadultpages) == 1)) {
				#ifdef DEBUG
				printf("%u is adult whith %i\n",PagesResults.TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray(PagesResults.TeffArray->iindex[i].DocID));
				#endif
				continue;
			}

			//slår opp DomainID
			if (!iintegerMemArrayGet (PagesResults.DomainIDs,(void**)&DomainID,sizeof(*DomainID),PagesResults.TeffArray->iindex[i].DocID) ) {
				#ifdef DEBUG
				printf("can't lookup DomainID\n");
				#endif

				continue;

			}


			//legg DomaindID inn i en hash. Hvis vi har hat mer en 2 sider fra samme DomainID fra før, så skal denne ikke telles med

			if ((hash_value = hashtable_search(hash_domainid, DomainID)) == NULL) {
				hash_key = malloc(sizeof(*hash_key));
				if (hash_key == NULL)
					continue;
				hash_value = malloc(sizeof(*hash_value));
				if (hash_value == NULL) {
					free(hash_key);
					continue;
				}
				*hash_key = *DomainID;
				*hash_value = 1;
				hashtable_insert(hash_domainid, hash_key, hash_value);
			} else {
				printf("have seen this domain %u %i times before\n",*DomainID,(*hash_value));
				(*hash_value) += 1;
				if ((*hash_value) > 2)
					continue;
			}



			// Rank siden til slutt hvis den har en hÃyere rank enn siden vi skal ranke
			if (PagesResults.TeffArray->iindex[i].allrank >= *ranking) {
				printf("Have a higher ranked page. DocID: %10u, DomainID %hu, i: %5i\n",PagesResults.TeffArray->iindex[i].DocID,*DomainID,i);
				ranksum++;
			}
			else {
				printf("Is below posible pages. Wont search whole array\n");
				break;
			}

			/***************************************************************/


		}
		printf("Position: %d\n", ranksum);
		*ranking = ranksum;
		hashtable_destroy(hash_domainid, 1);
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
		//char	*querystr = asprint_query(&PagesResults.QueryData.queryParsed);
		(*SiderHeder).navigation_xml = searchFilterCount(&PagesResults.antall,PagesResults.TeffArray,filters,subnames,nrOfSubnames,&filteron,dates,
		    &(*SiderHeder).queryTime, searchd_config->getfiletypep, searchd_config->attrdescrp, searchd_config->showattrp, &PagesResults.QueryData.queryParsed);

		(*SiderHeder).navigation_xml_len = strlen((*SiderHeder).navigation_xml);

		//free(querystr);
		destroy(filteron.attributes);
		free(filteron.collection);
		free(filteron.date);
		free(filteron.sort);
	#endif



	//fjerner filtered fra total oversikten
	//ToDo: bør dette også gjøres for web?
	(*SiderHeder).TotaltTreff = (*SiderHeder).TotaltTreff - (*SiderHeder).filtered;

	//lager en liste med ordene som ingikk i queryet til hiliting
	hiliteQuery[0] = '\0';
	//printf("size %i\n",PagesResults.QueryData.queryParsed.size);


	printf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" filtered=\"%i\" showabal=\"%i\"/>\n",(*SiderHeder).TotaltTreff,PagesResults.QueryData.query,hiliteQuery,(*SiderHeder).filtered,(*SiderHeder).showabal);
	
	printf("free TeffArray\n");
	free(PagesResults.TeffArray);

	destroy_query( &PagesResults.QueryData.queryParsed );


	//printer ut info om brukt tid
	vboprintf("Time\n");
	//printf("\tAthorSearch %f\n",(*SiderHeder).queryTime.AthorSearch);
	vboprintf("\t%-40s %f\n","AthorSearch",(*SiderHeder).queryTime.AthorSearch);
	//printf("\t%-40s %f\n","AthorRank",(*SiderHeder).queryTime.AthorRank);
	vboprintf("\t%-40s %f\n","UrlSearch",(*SiderHeder).queryTime.UrlSearch);
	vboprintf("\t%-40s %f\n","MainSearch",(*SiderHeder).queryTime.MainSearch);
	//printf("\t%-40s %f\n","MainRank",(*SiderHeder).queryTime.MainRank);
	vboprintf("\t%-40s %f\n","MainAthorMerge",(*SiderHeder).queryTime.MainAthorMerge);
	vboprintf("\n");
	vboprintf("\t%-40s %f\n","popRank",(*SiderHeder).queryTime.popRank);
	vboprintf("\t%-40s %f\n","responseShortning",(*SiderHeder).queryTime.responseShortning);
	vboprintf("\n");
	vboprintf("\t%-40s %f\n","allrankCalc",(*SiderHeder).queryTime.allrankCalc);
	vboprintf("\t%-40s %f\n","indexSort",(*SiderHeder).queryTime.indexSort);
	vboprintf("\t%-40s %f\n","popResult",(*SiderHeder).queryTime.popResult);
	vboprintf("\t%-40s %f\n","adultcalk",(*SiderHeder).queryTime.adultcalk);

	

	#ifdef BLACK_BOKS
	printf("\tfiletypes %f\n",(*SiderHeder).queryTime.filetypes);
	printf("\tiintegerGetValueDate %f\n",(*SiderHeder).queryTime.iintegerGetValueDate);
	printf("\tdateview %f\n",(*SiderHeder).queryTime.dateview);
	printf("\tFilterCount %f\n",(*SiderHeder).queryTime.FilterCount);
	printf("\tpathaccess %f\n",(*SiderHeder).queryTime.pathaccess);
	printf("\turlrewrite %f\n",(*SiderHeder).queryTime.urlrewrite);

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

	if (globalOptVerbose) {
		print_explane_rank(*Sider,(*SiderHeder).showabal);
	}

	printf("*ranking %i\n",*ranking);

	fprintf(stderr, "searchkernel: ~dorank()\n");
	return 1;
}

