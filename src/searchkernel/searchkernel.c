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

#include "shortenurl.h"
#include "searchkernel.h"
#include "htmlstriper.h"
#include "search.h"

#include "../common/adultWeight.h"
#include "../common/DocumentIndex.h"
#include "../parser2/html_parser.h"
#include "../generateSnippet/snippet.parser.h"
#include "../common/ir.h"
#include "../common/timediff.h"
#include "../common/attributes.h"
#include "../common/bstr.h"
#include "../query/query_parser.h"
#include "../query/stemmer.h"
#include "../common/integerindex.h"
#include "../ds/dcontainer.h"
#include "../ds/dset.h"
#include "../logger/logger.h"
#include "../common/utf8-strings.h"
#include "../searchFilters/searchFilters.h"
#include "../crawlManager2/client.h"
#include "../common/reposetory.h"
#include "../common/reposetoryNET.h"
#include "../ds/dcontainer.h"
#include "../ds/dvector.h"
#include "../ds/dpair.h"
#include "../common/ht.h"
#include "../common/doc_cache.h"
#include "../common/strlcpy.h"
#include "../common/langToNr.h"
#include "../common/url.h"
#include "../common/dp.h"

#ifdef WITH_SPELLING
	#include "../newspelling/spelling.h"
#endif

#ifdef BLACK_BOX
	#include "../getdate/getdate.h"
#endif

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"


#include "../utf8-filter/utf8-filter.h"

#ifdef WITH_THREAD
        #include <pthread.h>
	#ifdef BLACK_BOX
		#define NROF_GENERATEPAGES_THREADS 5
	#else
		#define NROF_GENERATEPAGES_THREADS 5
	#endif
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
#define MAX_CM_CONSUMERS_URLREWRITE 2
#define SUMMARY_LEN 160

struct socket_pool {
	int sock[MAX_CM_CONSUMERS];
	int used[MAX_CM_CONSUMERS];
	int consumers;
	pthread_mutex_t mutex;
	pthread_cond_t cv;
	int max_consumers;
};


struct PagesResultsFormat {
		struct SiderFormat *Sider;
		struct SiderHederFormat *SiderHeder;
		int antall;
		struct iindexFormat *TeffArray;
		int showabal;
		int nextPage;
		int filterOn;
		#ifndef BLACK_BOX
			int adultpages;
			int noadultpages;
		#endif
		struct QueryDataForamt QueryData;
		char *servername;
		int MaxsHits;
		int start;
		int indexnr;
		char search_user[64];
		char password[64];
		char useragent[64];
		struct iintegerMemArrayFormat *DomainIDs;

		int filtered;
		int filteredsilent;

		int memfiltered;


		#ifdef WITH_THREAD
			pthread_mutex_t mutex;
			pthread_mutex_t mutextreadSyncFilter;
		#endif

		struct socket_pool cmConn;
		struct socket_pool cmConnUrlrewrite;

		int activetreads;

		struct searchd_configFORMAT *searchd_config;
		#ifdef DEBUG_TIME
			struct popResultBreakDownTimeFormat popResultBreakDownTime;
		#endif
		struct hashtable *crc32maphash;
		struct duplicate_docids *dups;
		enum platform_type ptype; 
		enum browser_type btype; 
		int anonymous;
		int nrOfSubnames;
		struct subnamesFormat *subnames;
		container **groups_per_usersystem;
		int *usersystem_per_subname;
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
	int has_hits;

	if (summary_cfg == SUMMARY_DB) {
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , db_snippet, SUMMARY_LEN, 4, 60, &has_hits);
	}
	else if (summary_cfg == SUMMARY_SNIPPET) {
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , plain_snippet, SUMMARY_LEN, 4, 80, &has_hits);
	}
	else if (summary_cfg == SUMMARY_START) {
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , first_snippet, SUMMARY_LEN, 4, 80, &has_hits);
	}
	/*
	  ++Ax:
	    Snippet for database-records:
		generate_snippet(query_parsed, body, body_len, &summary, "<b>", "</b>" , db_snippet, SUMMARY_LEN, 4, 80);
	    De to siste tallene er maks antall rader og maks antall tegn per kolonne.
	*/
	else { 
		bblog(WARN, "Unknown snippet/summery cfg: %d", summary_cfg); 
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

#ifdef BLACK_BOX

static inline int
get_sock_from_pool(struct socket_pool *pool, int *index)
{
	int i;

	#ifdef WITH_THREAD
		if (pthread_mutex_lock(&pool->mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
			return 0;
		}
	#endif

	while (pool->consumers == pool->max_consumers) {
		pthread_cond_wait(&pool->cv, &pool->mutex);
	}
	pool->consumers++;

	for (i = 0; i < pool->max_consumers; i++) {
		if (pool->used[i] == 0) {
			pool->used[i] = 1;
			#ifdef WITH_THREAD
				if (pthread_mutex_unlock(&pool->mutex) != 0) {
					bblog(ERROR, "Can't unlock mutex!");
				}
			#endif
			*index = i;

			bblog(INFO, "~get_sock_from_pool(pool index=%i)",i); 
			return pool->sock[i];
		}
	}
	
	assert(1 == 0);
}

static inline void release_sock_to_pool(struct socket_pool *pool, int index) {
	#ifdef WITH_THREAD
		if (pthread_mutex_lock(&pool->mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
	#endif
	pool->used[index] = 0;
	
	pool->consumers--;
	#ifdef WITH_THREAD
		pthread_cond_signal(&pool->cv);
		if (pthread_mutex_unlock(&pool->mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}
	#endif

}

#endif



#ifdef BLACK_BOX
static inline int handle_url_rewrite(const char *url_in, enum platform_type ptype, enum browser_type btype, char *collection,
           char *url_out, size_t len, char *uri_out, size_t uri_out_len, char *fulluri_out, size_t fulluri_out_len, struct socket_pool *pool) {

	int ret = 1;

#ifndef _24SEVENOFFICE

	int index;
	int sock;

	sock = get_sock_from_pool(pool, &index);

	ret = cmc_rewrite_url(sock, collection, url_in, ptype, btype, url_out, len, uri_out, uri_out_len, fulluri_out, fulluri_out_len);
	if (ret == 0) {
		bblog(ERROR, "Cant rewrite url \"%s\"", url_in);
	}
	else {
		bblog(INFO, "handle_url_rewrite: Did rewrite \"%s\" to: url_out: \"%s\" fulluri_out: \"%s\"",url_in,url_out,fulluri_out);
	}

	release_sock_to_pool(pool, index);
#endif

	return ret;
}
#endif


int popResult(struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,int antall,unsigned int DocID,
	struct iindexMainElements *TeffArray,struct QueryDataForamt QueryData, char *htmlBuffer,
	unsigned int htmlBufferSize, char servername[], struct subnamesFormat *subname, 
	struct queryTimeFormat *queryTime, int summaryFH, struct PagesResultsFormat *PagesResults,
	char **acl_allow, char **acl_denied)
{

	char *url = NULL, *attributes = NULL;
	int y;
	char        *titleaa, *body, *metakeyw, *metadesc;
	struct ReposetoryHeaderFormat *ReposetoryHeader;
	off_t imagep;
	struct timeval start_time, end_time;
	titleaa = body = metakeyw = metadesc = NULL;

	char *strpointer;

	int returnStatus = 0;	


	htmlBuffer[0] = '\0';

	if ((ReposetoryHeader = malloc( sizeof(struct ReposetoryHeaderFormat) )) == NULL) {
		bblog_errno(ERROR, "Malloc ReposetoryHeader");
		return 0;
	}


	bblog(INFO, "searchkernel: popResult(antall=%i, DocID=%i, subname=%s)",  antall, DocID, subname->subname);

	//ser om vi har bilde, og at det ikke er på 65535 bytes. (65535  er maks støresle 
	//for unsigned short. Er bilde så stort skyldes det en feil)
	if (((*Sider).DocumentIndex.imageSize != 0) && ((*Sider).DocumentIndex.imageSize != 65535)) {

		imagep =  getImagepFromRadres((*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize2);
		bblog(DEBUGINFO, "imakep %u", (unsigned int)imagep);
		#ifdef BLACK_BOX
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

		(*Sider).thumbnailwidth = 100;
		(*Sider).thumbnailheight = 100;
	}
	else {
		(*Sider).thumbnale[0] = '\0';
	}

	//tester om vi har en url
	if ((*Sider).DocumentIndex.Url[0] == '\0') {
		bblog(WARN, "Cant read url for DocID=%i-%i, Subname=\"%s\"", DocID,rLotForDOCid(DocID),subname->subname);
		returnStatus = 0;
	}
	//tester om vi har noe innhold
	else if ((*Sider).DocumentIndex.htmlSize2 == 0) {
		//har ingen reposetroy data, ikke kravlet anda?

		//setter utlen som title
		strscpy((*Sider).title,(*Sider).DocumentIndex.Url,sizeof((*Sider).title));
		(*Sider).DocumentIndex.AdultWeight = 0;

		(*Sider).description[0] = '\0'; 


		(*SiderHeder).showabal++;
		returnStatus = 1;
	}
	else {
		
		htmlBuffer[0] = '\0';

		if ((*Sider).DocumentIndex.response == 404) {
			//404 sider kan dukke opp i anchor indeksen, da den som kjent er basert på data som ikke ser på om siden er crawlet
			bblog(WARN, "Page has 404 status %i-%i", DocID,rLotForDOCid(DocID));
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
					bblog(ERROR, "Unable to get resource for %d", DocID);
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
					#ifdef BLACK_BOX
						queryTime->generate_snippet += getTimeDifference(&start_time,&end_time);
					#endif

					strcpy(Sider->title, titleaa);
					strcpy(Sider->description, snippet);

					(*SiderHeder).showabal++;
					returnStatus = 1;

					if (titleaa != NULL) free(titleaa);
					if (body != NULL) free(body);
					free(snippet);
				}
				free(resbuf);
			} else if (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize2,DocID,subname->subname,ReposetoryHeader,acl_allow,acl_denied,(*Sider).DocumentIndex.imageSize, &url, &attributes) != 1) {
				//kune ikke lese html. Pointer owerflow ?
				bblog(ERROR, "error reding html for %s", (*Sider).DocumentIndex.Url);
				sprintf((*Sider).description,"Html error. Can't read html");
				(*Sider).title[0] = '\0';
				(*SiderHeder).showabal++;
				returnStatus = 0;
			}
			else {

				for(y=0;y<htmlBufferSize;y++) {

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
					(*Sider).description[0] = '\0';
				}


				(*SiderHeder).showabal++;
				returnStatus = 1;
			}
		}
		else if ((*Sider).DocumentIndex.response == 200) {

			#ifdef BLACK_BOX
			       // Include time and sign the parameters.
				time_t u_time = time(NULL);
				unsigned int signature;
				signature = sign_cache_params(DocID, subname->subname, u_time);
				
				Sider->cache_params.doc_id = DocID;
				Sider->cache_params.time = u_time;
				Sider->cache_params.signature = signature;
				strscpy(Sider->cache_params.subname, subname->subname, sizeof Sider->cache_params.subname);
				strscpy(Sider->cache_params.cache_host, servername, sizeof Sider->cache_params.cache_host);
			#else
				Sider->cache_params.doc_id = DocID;
				Sider->cache_params.time = 0;
				Sider->cache_params.signature = 0;
				strscpy(Sider->cache_params.subname, subname->subname, sizeof Sider->cache_params.subname);
				strscpy(Sider->cache_params.cache_host, servername, sizeof Sider->cache_params.cache_host);
			#endif


			gettimeofday(&start_time, NULL);						


			if (((*Sider).DocumentIndex.SummaryPointer != 0) && 
					((rReadSummary_l(DocID,&metadesc, &titleaa,&body,(*Sider).DocumentIndex.SummaryPointer,(*Sider).DocumentIndex.SummarySize,subname->subname,summaryFH) != 0))) {

				bblog(INFO, "hav Summary on disk");

				(*Sider).HtmlPreparsed = 1;

				gettimeofday(&end_time, NULL);				
				#ifdef DEBUG_TIME
					PagesResults->popResultBreakDownTime.ReadSummary.time += getTimeDifference(&start_time,&end_time);
					++PagesResults->popResultBreakDownTime.ReadSummary.nr;					
				#endif

			}
			else {
				bblog(DEBUGINFO, "don't hav Summary on disk. Will hav to read html");	
				if (rReadHtml(htmlBuffer,&htmlBufferSize,(*Sider).DocumentIndex.RepositoryPointer,(*Sider).DocumentIndex.htmlSize2,DocID,subname->subname,ReposetoryHeader,acl_allow,acl_denied,(*Sider).DocumentIndex.imageSize, &url, &attributes) != 1) {
					//printf("Fii faa foo: %s\n", url);
					//kune ikke lese html. Pointer owerflow ?
					bblog(ERROR, "error reding html for %s", (*Sider).DocumentIndex.Url);
					sprintf((*Sider).description,"Html error. Can't read html");
					(*Sider).title[0] = '\0';
					(*SiderHeder).showabal++;
					returnStatus = 1;
					bblog(INFO, "searchkernel: ~popResult()");
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
			//temp: bug generate_snippet er ikke ut til å takle og ha en tom body
			if (strlen(body) == 0) {
				(*Sider).description[0] = '\0';
			}
			else if (strlen(body) < 15) {
				bblog(WARN, "bug: body består av under 15 tegn. Da kan det være at vi bare har <div> som tegn.");
			}
			else {
				char key[MAX_ATTRIB_LEN], value[MAX_ATTRIB_LEN], keyval[MAX_ATTRIB_LEN];
				char summary_cfg, *attr_offset=NULL;

				#ifdef DEBUG
					bblog(DEBUGINFO, "calling generate_snippet with strlen body %i", strlen(body));
				#endif

				#ifdef DEBUG_TIME
					gettimeofday(&start_time, NULL);
				#endif

				#ifdef DEBUG
					bblog(DEBUGINFO, "#################################################################");
					bblog(DEBUGINFO, "############################ <DEBUG> ############################");
					bblog(DEBUGINFO, "body to snipet:");
					bblog(DEBUGINFO, "%s", body);
					bblog(DEBUGINFO, "strlen(body) %i", strlen(body));
					bblog(DEBUGINFO, "############################ </DEBUG> ###########################");
					bblog(DEBUGINFO, "#################################################################");
				#endif

				summary_cfg = subname->config.summary;
				
				if ((attributes != NULL) && (next_attribute_key(attributes, &attr_offset, key, value, keyval, "snippet"))) {
					if (strcmp(value, "db") == 0) {
						summary_cfg = SUMMARY_DB;
					}
				}

				summary = generate_summary(summary_cfg, QueryData.queryParsed, body);


				#ifdef DEBUG_TIME
					gettimeofday(&end_time, NULL);
					PagesResults->popResultBreakDownTime.generate_snippet.time += getTimeDifference(&start_time,&end_time);
					++PagesResults->popResultBreakDownTime.generate_snippet.nr;
				#endif


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

			bblog(DEBUGINFO, "%u -%s-, len %i",DocID,titleaa,strlen(titleaa));

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
					bblog(INFO, "aa strpointer %u", (unsigned int)strpointer);
					//midlertidg fiks på at title altid begynner med space på bb.
					//vil dermed altidd føre til treff i første tegn, og
					// dermed bare vise ".." som title
					//if ( ((int)(*Sider).title - (int)strpointer) > 10) {
					if ( ((int)strpointer - (int)(*Sider).title) > 10) {
						bblog(DEBUGINFO, "chop");
						strpointer[0] = '\0';
					}
					else {
						bblog(DEBUGINFO, "no chop, ing %i",  (int)(*Sider).title - (int)strpointer);
					}
					bblog(INFO, "fant space at %i", ((int)strpointer) - (int)(*Sider).title);
				}						
				else if ((strpointer = strrchr((*Sider).title,';')) != NULL) {
					++strpointer; //pekeren peker på semikolonet. SKal ha det med, så må legge il en
					strpointer[0] = '\0';

					bblog(INFO, "fant semi colon at %i", ((int)(*Sider).title - (int)strpointer));
				}
				strncat((*Sider).title,"..",2);    

				bblog(INFO, "title to long choped. now %i len. size %i", strlen((*Sider).title),sizeof((*Sider).title));

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
				
				for (k = 0; k<vector_size(dup->V); k++) {
					unsigned int dup_docid = pair(vector_get(dup->V,k)).first.i;

					char *dup_subname = pair(vector_get(dup->V,k)).second.ptr;

					if (dup_docid == DocID && !strcmp(dup_subname, subname->subname))
					    {
						// Hvis dette er docid-en som vises, skal den ikke filtreres.
						continue;
					    }

					char *htmlbuf;
					unsigned int htmllen = 1024*1024 * 5;
					//unsigned int imagelen = sizeof(imagelen);
					char *url, *acla, *acld, *attributes;
					struct DocumentIndexFormat di;
					struct ReposetoryHeaderFormat repohdr;
					char *tmpurl, *tmpuri, *tmpfulluri;

					tmpurl = malloc(1024);
					tmpuri = malloc(1024);
					tmpfulluri = malloc(1024);
					htmlbuf = malloc(htmllen);

					if(tmpurl == NULL || tmpuri == NULL || tmpfulluri == NULL || htmlbuf == NULL) {
						free(tmpurl); free(tmpuri); free(tmpfulluri); free(htmlbuf);
						bblog_errno(ERROR, "Malloc tmpurl, tmpuri, tmpfulluri or htmlbuf");
						continue;
					}

					acld = acla = attributes = url = NULL;
					if (!DIRead(&di, dup_docid, dup_subname)) {
						bblog_errno(WARN, "DIRead()");
						continue;
					}

					if (DIS_isDeleted(&di)) {
						bblog(WARN, "Duplicate document is deleted (DocID=%u, subname=\"%s\"). Skipping.", dup_docid, dup_subname);
						continue;
					}


					rReadHtml(htmlbuf, &htmllen, di.RepositoryPointer, di.htmlSize2, dup_docid,
							dup_subname, &repohdr, &acla, &acld, di.imageSize,
							&url, &attributes);


					if (url == NULL) {
						bblog(WARN, "Duplicate documents url is NULL (DocID=%u, subname=\"%s\"). Skipping", dup_docid, dup_subname);
						continue;
					}


#ifdef BLACK_BOX
					if (!handle_url_rewrite(url, PagesResults->ptype,
							PagesResults->btype,
							dup_subname, tmpurl, sizeof(tmpurl),
							tmpuri, sizeof(tmpuri),
							tmpfulluri, sizeof(tmpfulluri),
#ifdef WITH_THREAD
							&PagesResults->cmConnUrlrewrite
#else
							NULL
#endif
							)) {
						snprintf(tmpurl,sizeof(tmpurl),"XXX-Can't_rewrite_duplicate_url_for_DocID_%u",dup_docid);
					}
#endif
					if ((Sider->urls[Sider->n_urls].url = strdup(tmpurl)) == NULL) {
						bblog_errno(ERROR, "Malloc url");
					}
					if ((Sider->urls[Sider->n_urls].uri = strdup(tmpuri)) == NULL) {
						bblog_errno(ERROR, "Malloc uri");
					}
					if ((Sider->urls[Sider->n_urls].fulluri = strdup(tmpfulluri)) == NULL) {
						bblog_errno(ERROR, "Malloc uri");
					}

					free(tmpurl);
					free(tmpuri);
					free(tmpfulluri);
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
			bblog(WARN, "Status error for page %i-%i. Status %i", DocID,rLotForDOCid(DocID),(*Sider).DocumentIndex.response);
			sprintf((*Sider).title,"%i error.",(*Sider).DocumentIndex.response);
			returnStatus = 0;
		}

	}


	if (url != NULL)
		free(url);
	if (attributes != NULL)
		free(attributes);
	if (ReposetoryHeader != NULL)
		free(ReposetoryHeader);

	bblog(INFO, "searchkernel: ~popResult()");
	return returnStatus;
}


int nextIndex(struct PagesResultsFormat *PagesResults) {

	#ifdef DEBUG
		bblog(DEBUGINFO, "searchkernel: nextIndex()");
	#endif

	int forreturn;

	#ifdef WITH_THREAD
		//debug: printf("nextIndex: waiting for lock: start\n");
		if (pthread_mutex_lock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
		//debug: printf("nextIndex: waiting for lock: end\n");
	#endif

	if ((*PagesResults).filtered > 300) {
		bblog(DEBUGINFO, "nextIndex: filtered (%i) > 300",(*PagesResults).filtered);
		forreturn = -1;
	}
	//mister vi en her? var før >, bytet til >=
	else if ((*PagesResults).indexnr >= (*PagesResults).antall) {
		bblog(DEBUGINFO, "nextIndex: indexnr (%i) > antall (%i)",(*PagesResults).indexnr,(*PagesResults).antall);

		forreturn = -1;
	}
	else {

		forreturn = (*PagesResults).indexnr++;
	}	

	#ifdef DEBUG
		bblog(DEBUGINFO, "nextIndex: returning %i as index nr. Nr of hist is %i",forreturn,(*PagesResults).antall);
	#endif

	#ifdef WITH_THREAD
		//debug: printf("nextIndex: waiting for UNlock: start\n");
		if (pthread_mutex_unlock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}
		//debug: printf("nextIndex: waiting for UNlock: end\n");
	#endif

	#ifdef DEBUG
		bblog(DEBUGINFO, "searchkernel: ~nextIndex()");
	#endif
	return forreturn;
}

int nextPage(struct PagesResultsFormat *PagesResults) {

	bblog(INFO, "searchkernel: nextPage()");
	int forreturn;

	#ifdef WITH_THREAD
		//debug: printf("nextPage: waiting for lock: start\n");
		if (pthread_mutex_lock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
		//debug: printf("nextPage: waiting for lock: end\n");
	#endif

	if (((*PagesResults).activetreads > 2) && ((*PagesResults).MaxsHits - (*PagesResults).nextPage) < 3) {
		bblog(INFO, "we have %i pages, and %i activetreads. This tread can die.", (*PagesResults).nextPage,(*PagesResults).activetreads);
		--(*PagesResults).activetreads;
		forreturn = -1;
	}
	else if ((*PagesResults).nextPage >= (*PagesResults).MaxsHits) {
		bblog(DEBUGINFO, "nextPage: nextPage (%i) >= MaxsHits (%i)",(*PagesResults).nextPage,(*PagesResults).MaxsHits);

		forreturn = -1;
	}
	//mister vi en her? Var før >, men måte elgge til >= får å håntere at vi kan ha 0. Da er 
	//indexnr og antall, like, men ikke støre en. Hvis dette er 0 så skal vi da ikke gå vire. Men hva hvis det
	//er 1?? Vil vi ha 0 først, og det er elemenet 1 ????
	else if ((*PagesResults).indexnr >= (*PagesResults).antall) {
		bblog(DEBUGINFO, "nextPage: indexnr (%i) > antall (%i)",(*PagesResults).indexnr,(*PagesResults).antall);

		forreturn = -1;
	}
	else {
		forreturn = (*PagesResults).nextPage++;
	}
	
	bblog(DEBUGINFO, "nextPage: returning %i as next page.",forreturn);

	#ifdef WITH_THREAD
		//debug: printf("nextPage:waiting for UNlock: start\n");
		if (pthread_mutex_unlock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}
		//debug: printf("nextPage:waiting for UNlock: end\n");
	#endif

	bblog(INFO, "searchkernel: ~nextPage()");
	return forreturn;

}

int foundGodPage(struct PagesResultsFormat *PagesResults) {

	int ret;

	#ifdef WITH_THREAD
		//debug: printf("nextPage: waiting for lock: start\n");
		if (pthread_mutex_lock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
		//debug: printf("nextPage: waiting for lock: end\n");
	#endif

	bblog(INFO, "this is a good page");
	ret = (*PagesResults).showabal;

	++(*PagesResults).showabal;
		
	#ifdef WITH_THREAD
		//debug: printf("nextPage:waiting for UNlock: start\n");
		if (pthread_mutex_unlock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}
		//debug: printf("nextPage:waiting for UNlock: end\n");
	#endif

	return ret;
}

int nrofGodPages(struct PagesResultsFormat *PagesResults) {

	int ret;

	#ifdef WITH_THREAD
		if (pthread_mutex_lock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
	#endif

	ret = (*PagesResults).showabal;

	#ifdef DEBUG
		bblog(DEBUGINFO, "nrofGodPage: have %i god pages", ret);
	#endif

	#ifdef WITH_THREAD
		if (pthread_mutex_unlock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}
	#endif

	return ret;

}

void increaseMemFiltered(struct PagesResultsFormat *PagesResults,int *whichFilterTraped, 
	int *nrInSubname,struct iindexMainElements *iindex) {


	#ifdef WITH_THREAD
		if (pthread_mutex_lock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
	#endif

	++(*PagesResults).memfiltered;

	++(*whichFilterTraped);

	//runarb:1 13 mars. Hvorfor var denne komentert ut???
	//runarb:2 for de vi nå bruker subname som filter
	//runarb:3 kan det være det er nyttig for web søket og ha rikitig tall her? 
	//runarb:4 Dette gjør at tallene for treff i subname minker, gjør heller slik at man viser alle, og så viser filtered meldingen
	//--(*nrInSubname);

	#ifdef BLACK_BOX
		(*iindex).deleted = 1;
	#endif

	#ifdef WITH_THREAD
		if (pthread_mutex_unlock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}
	#endif


}
void increaseFiltered(struct PagesResultsFormat *PagesResults,int *whichFilterTraped, 
	int *nrInSubname,struct iindexMainElements *iindex) {


	#ifdef WITH_THREAD
		if (pthread_mutex_lock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
	#endif

	++(*PagesResults).filtered;

	if (whichFilterTraped != NULL) {
		++(*whichFilterTraped);
	}

	//runarb:1 13 mars. Hvorfor var denne komentert ut???
	//runarb:2 for de vi nå bruker subname som filter
	//runarb:3 kan det være det er nyttig for web søket og ha rikitig tall her? 
	//runarb:4 Dette gjør at tallene for treff i subname minker, gjør heller slik at man viser alle, og så viser filtered meldingen
	//--(*nrInSubname);

	#ifdef BLACK_BOX
		(*iindex).deleted = 1;
	#endif

	#ifdef WITH_THREAD
		if (pthread_mutex_unlock(&(*PagesResults).mutex)) {
			bblog(ERROR, "Can't unlock mutex!");
		}
	#endif


}

void increaseFilteredSilent(struct PagesResultsFormat *PagesResults,int *whichFilterTraped, 
	int *nrInSubname,struct iindexMainElements *iindex) {


	#ifdef WITH_THREAD
		if (pthread_mutex_lock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}
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

	#ifdef BLACK_BOX
		(*iindex).deleted = 1;
	#endif

	#ifdef WITH_THREAD
		if (pthread_mutex_unlock(&(*PagesResults).mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}
	#endif


}

#ifdef BLACK_BOX
static inline int pathaccess(struct PagesResultsFormat *PagesResults, struct subnamesFormat *subname, char uri_in[], char user_in[], char password_in[]) {
	int ret = 0;

	if (PagesResults->anonymous)
		return 1;

	if ((subname->config.accesslevel == CAL_GROUP) || (subname->config.accesslevel == CAL_USER)) return 1;

	int sockIndex;
	int socketha;

	socketha = get_sock_from_pool(&PagesResults->cmConn, &sockIndex);

	ret = cmc_pathaccess(socketha, subname->subname, uri_in, user_in, password_in);

	release_sock_to_pool(&PagesResults->cmConn, sockIndex);

	
	bblog(INFO, "pathaccess respons=%i", ret);

	return ret;
}

static inline int repositoryaccess(struct PagesResultsFormat *PagesResults, int DocID, struct subnamesFormat *subname, char *acl_allow, char *acl_denied) {

    int error = 0; // if error, then return no access

    bblog(DEBUGINFO, "repositoryaccess %s-%i\n", subname->subname, DocID);

    if (PagesResults->anonymous) return 1;

    if (PagesResults->usersystem_per_subname == NULL
	|| PagesResults->groups_per_usersystem == NULL) return error;

    if (acl_allow==NULL || acl_denied==NULL) return error;

    if ((subname->config.accesslevel == CAL_GROUP) || (subname->config.accesslevel == CAL_USER)) return 1;
    
    bblog(DEBUGINFO,"repositoryaccess: acl_allowed = %s",acl_allow);
    bblog(DEBUGINFO,"repositoryaccess: acl_denied = %s", acl_denied);

    container	*groups = NULL;
    int		i;
    for (i=0; i<PagesResults->nrOfSubnames; i++)
	{
	    if (!strcmp(PagesResults->subnames[i].subname, subname->subname))
		{
		    int		index = PagesResults->usersystem_per_subname[i];

		    if (index == -1) return error; // Collection has no usersystem

		    groups = PagesResults->groups_per_usersystem[index];
		    break;
		}
	}

    if (groups == NULL) return error;

    #ifdef DEBUG
	    bblog(DEBUGINFO,"repositoryaccess: groups = "); println(groups);
    #endif

    char **Data;
    int Count;

    if (split(acl_denied, ",", &Data) != 0)
	{
	    int	has_been_denied = 0;

	    for (Count=0; Data[Count] != NULL; Count++)
		{
		    iterator	it = set_find(groups, Data[Count]);
		    if (it.valid)
			{
			    has_been_denied = 1;
			    break;
			}

		    free(Data[Count]);
    		}

	    for (; Data[Count] != NULL; Count++) free(Data[Count]);
	    free(Data);

	    if (has_been_denied) return 0;
	}

    if (split(acl_allow, ",", &Data) != 0)
	{
	    int has_been_allowed = 0;

	    for (Count=0; Data[Count] != NULL; Count++)
		{
	    	    if (Data[Count][0] == '\0') continue;

		    iterator	it = set_find(groups, Data[Count]);

		    if (it.valid)
			{
			    has_been_allowed = 1;
			    break;
			}

		    free(Data[Count]);
    		}

	    for (; Data[Count] != NULL; Count++) free(Data[Count]);
	    free(Data);

	    if (has_been_allowed) return 1;
	}

    return 0;
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
		bblog(DEBUGINFO, "time_DIRead_fh: reading for DocID %u, dev: %i, time %f", DocID,GetDevIdForLot(rLotForDOCid(DocID)),getTimeDifference(&start_time,&end_time));

		PagesResults->popResultBreakDownTime.DocumentIndex.time += getTimeDifference(&start_time,&end_time);
		++PagesResults->popResultBreakDownTime.DocumentIndex.nr;
	#endif

	return ret;
}

void *generatePagesResults(void *arg) 
{

	bblog(INFO, "searchkernel: generatePagesResults()");
	struct PagesResultsFormat *PagesResults = (struct PagesResultsFormat *)arg;

	int i;
	//ToDo: ikke hardkode her
        unsigned int htmlBufferSize = 900000;

	char *htmlBuffer;

	double ltime;
	struct timeval start_time, end_time;
	int localshowabal;
	//tread lock

	struct SiderFormat *side = malloc(sizeof(struct SiderFormat));

	#if BLACK_BOX
		PagesResults->ptype = get_platform(PagesResults->useragent);
		PagesResults->btype = get_browser(PagesResults->useragent);
	#else
		unsigned short *DomainID;
		int y;
	#endif

	if ((htmlBuffer = malloc(htmlBufferSize)) == NULL) {
		bblog_errno(ERROR, "can't malloc");
		bblog(INFO, "searchkernel: ~generatePagesResults()");
		return NULL;
	}

	#ifdef WITH_THREAD
		pthread_t tid;
		tid = pthread_self();
		bblog(INFO, "is thread id %u", (unsigned int)tid);
	#else
		unsigned int tid=0;
	#endif

	// XXX: Use pthread_cond instead ?
	while ( (localshowabal = nextPage(PagesResults)) != -1 ) {
	bblog(DEBUGINFO, "localshowabal %i",localshowabal);

	while ((i=nextIndex(PagesResults)) != -1) {

		#ifdef DEBUG
			bblog(DEBUGINFO, "i %i, DocID %u, subname %s",i,(*PagesResults).TeffArray->iindex[i].DocID, (*PagesResults).TeffArray->iindex[i].subname->subname);
		#endif


		#ifdef BLACK_BOX

			//hvis index filter tidligere har funet ut at dette ikke er et bra treff går vi til neste
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.filename == 1) {
				#ifdef DEBUG
					bblog(DEBUGINFO, "filter: index filtered (filename)");
				#endif
				continue;
			}
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.date == 1) {
				#ifdef DEBUG
					bblog(DEBUGINFO, "filter: index filtered (date)");
				#endif
				continue;
			}
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.subname == 1) {
				#ifdef DEBUG
					bblog(DEBUGINFO, "filter: index filtered (subname)");
				#endif
				continue;
			}
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.attribute == 1) {
				#ifdef DEBUG
					bblog(DEBUGINFO, "filter: index filtered (attribute)");
				#endif
				continue;
			}
			// If NOT filtering on collection:
			if ((*PagesResults).TeffArray->iindex[i].indexFiltered.duplicate == 1) {
				#ifdef DEBUG
					bblog(DEBUGINFO, "filter: index filtered (duplicate)");
				#endif
				continue;
			}
		#endif

		#ifndef BLACK_BOX
			//pre DIread filter
			#ifdef DEBUG
				bblog(DEBUGINFO, "adult %u: %i", (*PagesResults).TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID));
			#endif

			if (((*PagesResults).filterOn) && (filterAdultWeight_bool(adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID),(*PagesResults).adultpages,(*PagesResults).noadultpages) == 1)) {
				#ifdef DEBUG
					bblog(DEBUGINFO, "%u is adult whith %i", (*PagesResults).TeffArray->iindex[i].DocID,adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID));
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
					bblog(DEBUGINFO, "can't lookup DomainID");
				#endif
				
				side->DomainID = 0;

			}
			else {

				side->DomainID = (*DomainID);
			
				// fornå gjør vi bare denne sjekken hvis vi kunne slå opp DomainID
                		if (((*PagesResults).filterOn) && (filterSameDomainID(localshowabal,side,(*PagesResults).Sider))) {
					#ifdef DEBUG
						bblog(DEBUGINFO, "Have same DomainID");
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

		bblog(INFO, "readin di for %u", (*PagesResults).TeffArray->iindex[i].DocID);
		//leser DI
		if (!time_DIRead_i(&side->DocumentIndex,(*PagesResults).TeffArray->iindex[i].DocID,(*(*PagesResults).TeffArray->iindex[i].subname).subname,PagesResults->searchd_config->lotPreOpen.DocumentIndex[rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID)], PagesResults)) 
		{
                        //hvis vi av en eller annen grun ikke kunne gjøre det kalger vi
                        bblog(INFO, "Can't read post for %u-%i", (*PagesResults).TeffArray->iindex[i].DocID,rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID));
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cantDIRead,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        continue;
                }


		bblog(INFO, "[tid: %u] looking on  DocID: %u url: \"%s\", subname: \"%s\"", (unsigned int)tid,(*PagesResults).TeffArray->iindex[i].DocID,side->DocumentIndex.Url,(*(*PagesResults).TeffArray->iindex[i].subname).subname);

		#ifndef BLACK_BOX
		//adult fra di
		if (((*PagesResults).filterOn) && (filterAdultWeight_value(side->DocumentIndex.AdultWeight,(*PagesResults).adultpages,(*PagesResults).noadultpages)) ) {
			bblog(INFO, "Filter: filtered adult. DocID %u, adult value %i, adult bool value %i", 
				(*PagesResults).TeffArray->iindex[i].DocID, 
				(int)side->DocumentIndex.AdultWeight ,(
				int)adultWeightForDocIDMemArray((*PagesResults).TeffArray->iindex[i].DocID) );

				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterAdultWeight_value,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
	                        continue;
		}
		#endif


		//filtrerer ut dublikater fra med crc32 fra DocumentIndex
		if (side->DocumentIndex.crc32 != 0) {
                       
                        if (((*PagesResults).filterOn) && (filterSameCrc32(localshowabal,side,(*PagesResults).Sider))) {
                        	bblog(INFO, "hav same crc32. crc32 from DocumentIndex");
				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameCrc32_1,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
                        	continue;
                        }
                }

		//filtrere silent
		if (filterResponseCode(side)) {
			bblog(INFO, "filter: page har bad respons code");
			increaseFilteredSilent(PagesResults,NULL,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			continue;
		}
		


		#ifndef BLACK_BOX

		if (side->DocumentIndex.Url[0] == '\0') {
			bblog(INFO, "filter: DocumentIndex url is emty. DocID %u", (*PagesResults).TeffArray->iindex[i].DocID);
			increaseFilteredSilent(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterNoUrl,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			continue;
		}


		//finner domene
		if (!find_domain_no_subname(side->DocumentIndex.Url,side->domain,sizeof(side->domain))) {
			bblog(INFO, "can't find domain. Bad url?. Url \"%s\". localshowabal %i", side->DocumentIndex.Url,localshowabal);
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.find_domain_no_subname,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}

		//hvis vi ikke har noen domeneID så lager vi en
		if (side->DomainID == 0) {
			side->DomainID = calcDomainID(side->domain);

		}

		if (((*PagesResults).filterOn) && (filterTLDs(side->domain))) {
			bblog(INFO, "banned TLD");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterTLDs,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			continue;
		}

		#endif



		#ifndef BLACK_BOX
			//DI filtere
			if (((*PagesResults).filterOn) && (filterResponse(side->DocumentIndex.response) )) {
				bblog(INFO, "bad respons kode %i for %u.", side->DocumentIndex.response,(*PagesResults).TeffArray->iindex[i].DocID);
				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterResponse,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
				continue;
			}
		#endif


		#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
		#endif
		char	*acl_allow=NULL, *acl_denied=NULL;

		if (!popResult(side, (*PagesResults).SiderHeder,(*PagesResults).antall,(*PagesResults).TeffArray->iindex[i].DocID,&(*PagesResults).TeffArray->iindex[i],(*PagesResults).QueryData,htmlBuffer,htmlBufferSize,(*PagesResults).servername,PagesResults->TeffArray->iindex[i].subname, &PagesResults->SiderHeder->queryTime,PagesResults->searchd_config->lotPreOpen.Summary[rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID)],PagesResults, &acl_allow, &acl_denied)) {
                       	bblog(INFO, "can't popResult");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cantpopResult,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			if (acl_allow!=NULL) free(acl_allow);
			if (acl_denied!=NULL) free(acl_denied);
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
		}
		#ifdef DEBUG_TIME
		        gettimeofday(&end_time, NULL);
        	        PagesResults->popResultBreakDownTime.makecrc32.time += getTimeDifference(&start_time,&end_time);
	                ++PagesResults->popResultBreakDownTime.makecrc32.nr;

		#endif

		/****************************************************************************/		
		#ifndef BLACK_BOX

		//kvalitetsjekker på inn data.
		if (((*PagesResults).filterOn) && (filterTitle(side->title) )) {
			bblog(INFO, "bad title \"%s\"", side->title);
			increaseFiltered(PagesResults,NULL,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			if (acl_allow!=NULL) free(acl_allow);
			if (acl_denied!=NULL) free(acl_denied);
			continue;
		}

		if (((*PagesResults).filterOn) && (filterSummery(side->description) )) {
			bblog(INFO, "bad summery \"%s\"", side->description);
			increaseFiltered(PagesResults,NULL,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
			if (acl_allow!=NULL) free(acl_allow);
			if (acl_denied!=NULL) free(acl_denied);
			continue;
		}
		#endif		

		gettimeofday(&start_time, NULL);
		#ifdef BLACK_BOX

			#ifdef DEBUG
			bblog(DEBUGINFO, "pathaccess: start");
			#endif

			//temp: kortslutter får å implementere sudo. Må implementeres skikkelig, men å spørre boithoad
			// Runarb 7 jan 2010: Tar dette bort for nå, intil vi begynner å bruke verdien fra config tabellen.
			// nå kan en bruker sette passordet sitt til water66 og slippe pathaccess.
			//if (strcmp((*PagesResults).password,"water66") == 0) {
			//	bblog(WARN, "pathaccess: have sodo password. Won't do pathaccess");
			//}
			//else 
			if ((*PagesResults).usersystem_per_subname!=NULL && !repositoryaccess(PagesResults, (*PagesResults).TeffArray->iindex[i].DocID, (*PagesResults).TeffArray->iindex[i].subname, acl_allow, acl_denied)) {
				bblog(ERROR, "searchkernel: Access denied for file \"%s\" in %s (repository fail)", side->url, (*(*PagesResults).TeffArray->iindex[i].subname).subname);
				
				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cmc_pathaccess,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);
				if (acl_allow!=NULL) free(acl_allow);
				if (acl_denied!=NULL) free(acl_denied);
				continue;
			}
			else if (!pathaccess(PagesResults,(*PagesResults).TeffArray->iindex[i].subname,side->url,(*PagesResults).search_user,(*PagesResults).password)) {
				bblog(ERROR, "searchkernel: Access denied for file \"%s\" in %s", side->url, (*(*PagesResults).TeffArray->iindex[i].subname).subname);

				increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.cmc_pathaccess,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

				if (acl_allow!=NULL) free(acl_allow);
				if (acl_denied!=NULL) free(acl_denied);
				continue;

			}
			#ifdef DEBUG
				bblog(DEBUGINFO, "pathaccess: done");
			#endif

		if (acl_allow!=NULL) free(acl_allow);
		if (acl_denied!=NULL) free(acl_denied);

		gettimeofday(&end_time, NULL);
		(*(*PagesResults).SiderHeder).queryTime.pathaccess += getTimeDifference(&start_time,&end_time);


		#ifdef DEBUG
		bblog(DEBUGINFO, "url rewrite: start");
		#endif
		gettimeofday(&start_time, NULL);

#ifdef BLACK_BOX

		if (!handle_url_rewrite(side->url,
			PagesResults->ptype, PagesResults->btype, 
			(*PagesResults).TeffArray->iindex[i].subname->subname, side->url, 
			sizeof(side->url), side->uri, sizeof(side->uri), side->fulluri, sizeof(side->fulluri),
#ifdef WITH_THREAD
			&PagesResults->cmConnUrlrewrite
#else
			NULL
#endif
		)) {
			snprintf(side->uri, sizeof(side->uri),"XXX-Can't_rewrite_url_for_DocID_%u",(*PagesResults).TeffArray->iindex[i].DocID);
			snprintf(side->fulluri, sizeof(side->fulluri),"XXX-Can't_rewrite_fulluri_for_DocID_%u",(*PagesResults).TeffArray->iindex[i].DocID);
		}

#endif


		bblog(INFO, "url rewrite done:");
		bblog(INFO, "url \"%s\"", side->url);
		bblog(INFO, "uri \"%s\"", side->uri);
		bblog(INFO, "fulluri \"%s\"", side->fulluri);

		#endif
		gettimeofday(&end_time, NULL);
		ltime = getTimeDifference(&start_time,&end_time);
		bblog(DEBUGINFO, "url rewrite: time %f for url \"%s\"", ltime, side->uri);

		(*(*PagesResults).SiderHeder).queryTime.urlrewrite += ltime;


		side->pathlen = find_domain_path_len(side->url);

		#ifndef BLACK_BOX
			memcpy(side->uri, side->url, sizeof(side->uri));
			memcpy(side->fulluri, side->url, sizeof(side->fulluri));
		#endif

		strcpy(side->servername,(*PagesResults).servername);

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
			if (pthread_mutex_lock(&(*PagesResults).mutextreadSyncFilter) != 0) {
				bblog(ERROR, "Can't lock mutex!");
			}
		#endif

		#ifdef DEBUG_TIME
			gettimeofday(&start_time, NULL);
		#endif

		int treadSyncFilters = 0;

		//håntering av at vi kan ha mer en en pi.
		if (pi_switch(nrofGodPages(PagesResults),side,(*PagesResults).Sider)) {
			bblog(INFO, "filter (treadSyncFilter): switch a pi \"%s\"", side->DocumentIndex.Url);
			
			treadSyncFilters = 1;
			goto end_filter_lock;

		}

		if (((*PagesResults).filterOn) && (filterSameCrc32(nrofGodPages(PagesResults),side,(*PagesResults).Sider))) {
                      	bblog(INFO, "filter (treadSyncFilter): hav same crc32");
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameCrc32_2,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			treadSyncFilters = 1;
			goto end_filter_lock;
               	}


		//fjerner eventuelt like urler
		if (((*PagesResults).filterOn) && (filterSameUrl(nrofGodPages(PagesResults),side->DocumentIndex.Url,(*PagesResults).Sider)) ) {
			bblog(INFO, "filter (treadSyncFilter): Hav seen url befor. Url \"%s\"", side->DocumentIndex.Url);
			
			increaseFiltered(PagesResults,&(*(*PagesResults).SiderHeder).filtersTraped.filterSameUrl,&(*(*PagesResults).TeffArray->iindex[i].subname).hits,&(*PagesResults).TeffArray->iindex[i]);

			treadSyncFilters = 1;
			goto end_filter_lock;
		}

		#ifndef BLACK_BOX


#ifndef BLACK_BOX
		if (((*PagesResults).filterOn) && (filterSameDomain(nrofGodPages(PagesResults),side,(*PagesResults).Sider))) {

			bblog(INFO, "filter (treadSyncFilter): hav same domain. Domain: \"%s\", domain id %ho. Url %s", side->domain,side->DomainID,side->DocumentIndex.Url);

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

						bblog(INFO, "filter (treadSyncFilter): have same domain, but page \"%s\" is higher ranked then \"%s\". swaping it", side->DocumentIndex.Url,(*PagesResults).Sider[y].DocumentIndex.Url);
						
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
			if (pthread_mutex_unlock(&(*PagesResults).mutextreadSyncFilter) != 0) {
				bblog(ERROR, "Can't unlock mutex!");
			}
		#endif
		
		if (treadSyncFilters == 1) {
			bblog(INFO, "Page did get filtered in treadSyncFilter");
			continue;
		}

		/*******************************************************************************************/


		
		//gjør post god side ting, som å øke showabal
		int myPageNr;
		myPageNr = foundGodPage(PagesResults);

		//kopierer siden inni den globale side arrayen
		(*PagesResults).Sider[myPageNr] = *side;
		
		bblog(INFO, "did copy page \"%s\" into spot %i. (read back \"%s\")", side->DocumentIndex.Url,myPageNr,side->subname.subname);
		
		break; //går ut av loopen. Vi har funnet at vår index hit var brukenes, vi trenger da en ny side
	}
	}

	//printf("******************************\nfreeing htmlBuffer\n******************************\n");
	free(htmlBuffer);
	free(side);


	bblog(INFO, "searchkernel: ~generatePagesResults()");

	return NULL;
}

int sider_allrank_sort (const void *p1, const void *p2) {

	// en mer stabil sortering der vi også sorterer på DocID slik at resultatene ikke håpper på serp'ene.
	if (((struct SiderFormat *)p1)->iindex.allrank == ((struct SiderFormat *)p2)->iindex.allrank) {

		if (((struct SiderFormat *)p1)->iindex.originalPosition < ((struct SiderFormat *)p2)->iindex.originalPosition) {
			 return -1;
		}
		else {
			return (((struct SiderFormat *)p1)->iindex.originalPosition > ((struct SiderFormat *)p2)->iindex.originalPosition);
		}

	}
	else if (((struct SiderFormat *)p1)->iindex.allrank > ((struct SiderFormat *)p2)->iindex.allrank) {
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

	bblog(WARN, "|%-10s|%-10s|%-10s||%-10s|%-10s|%-10s|%-24s|%-10s|%-10s|%-5s|", 
		"AllRank",
		"TermRank",
		"PopRank",
		"Body",
		"Headline",
		"Tittel",
		"Anchor (nr nrp)",
		"UrlM",
		"Url",
		"Adult"
		);
	bblog(WARN, "|----------|----------|----------||----------|----------|----------|------------------------|----------|----------|-----|");

	for(i=0;i<showabal;i++) {
                        bblog(WARN, "|%10i|%10i|%10i||%10i|%10i|%10i|%10i (%5i %5i)|%10i|%10i|%5d| %s (DocID %u-%i, DomainID %d, s: \"%s\"), O. Pos %u", 

				Sider[i].iindex.allrank,
				Sider[i].iindex.TermRank,
				#ifdef BLACK_BOX
				-1,
				#else
				Sider[i].iindex.PopRank,
				#endif

				#ifdef EXPLAIN_RANK
					Sider[i].iindex.rank_explaind.rankBody,
					Sider[i].iindex.rank_explaind.rankHeadline,
					Sider[i].iindex.rank_explaind.rankTittel,
				#else 
					-1,-1,-1,
				#endif


				#ifdef BLACK_BOX
				-1,-1,-1,-1,-1,-1,
				#else
				Sider[i].iindex.rank_explaind.rankAnchor,
				Sider[i].iindex.rank_explaind.nrAnchor,
				Sider[i].iindex.rank_explaind.nrAnchorPhrase,
				Sider[i].iindex.rank_explaind.rankUrl_mainbody,
				Sider[i].iindex.rank_explaind.rankUrlDomain + Sider[i].iindex.rank_explaind.rankUrlSub,
				Sider[i].DocumentIndex.AdultWeight,
				#endif

				Sider[i].DocumentIndex.Url,
				Sider[i].iindex.DocID,
				rLotForDOCid(Sider[i].iindex.DocID),

				#ifdef BLACK_BOX
				-1,
				#else
				Sider[i].DomainID,
				#endif

				(*Sider[i].iindex.subname).subname,
				Sider[i].iindex.originalPosition

				);
	}

	#ifdef DEBUG
		bblog(DEBUGINFO, "uri:");
		for(i=0;i<showabal;i++) {
			bblog(DEBUGINFO, "%s", Sider[i].uri);
		}
	#endif

}

#ifdef WITH_SPELLING
int
spellcheck_query(struct SiderHederFormat *SiderHeder, query_array *qa, spelling_t *spelling, container *groups, container *subnames)
{
	int i;
	int fixed;

	if (spelling == NULL) {
		return 0;
	}

	fixed = 0;
	for(i = 0; i < qa->n; i++) {
		string_array *sa = &qa->query[i];
		switch (sa->operand) {
			case QUERY_WORD:
			case QUERY_SUB:
			{
				char *p;
				int found;

				if (correct_word(spelling, sa->s[0], groups, subnames)) {
					bblog(INFO, "correct_word sees: %s is correct.",sa->s[0]);
					continue;
				}

				p = check_word(spelling, sa->s[0], &found, groups, subnames);
				if (p == NULL) {
					bblog(INFO, "check_word sees: Didi not find correct spelling.");
					continue;
				}
				bblog(INFO, "Found correct spelling: %s",  p);
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


int cmConect_and_pool (struct socket_pool *pool, char *errorbuff, int errorbufflen, int max_consumers, struct searchd_configFORMAT *searchd_config){

	int k;
	pool->max_consumers = max_consumers;
	if (searchd_config->optSingle) {
		pool->max_consumers = 1;
	}

	for (k = 0; k < pool->max_consumers; k++) {
		bblog(INFO, "making a connection to crawlerManager: %d:%d", k, pool->max_consumers);
		pool->used[k] = 0;
		if (!cmc_conect(&pool->sock[k], errorbuff,errorbufflen,searchd_config->cmc_port)) {
			bblog(ERROR, "%s:%i",errorbuff,(*searchd_config).cmc_port);
			bblog(INFO, "searchkernel: ~dosearch()");
			return(0);
		}
	}
	pool->consumers = 0;

	return 1;
}




int dosearch(char query[], int queryLen, struct SiderFormat **Sider, struct SiderHederFormat *SiderHeder,
char *hiliteQuery, char servername[], struct subnamesFormat subnames[], int nrOfSubnames, 
int MaxsHits, int start, int filterOn, char languageFilter[],char orderby[],int dates[], 
char search_user[],struct filtersFormat *filters,struct searchd_configFORMAT *searchd_config, char *errorstr,int errorLen,
	struct iintegerMemArrayFormat *DomainIDs, char *useragent, char groupOrQuery[], int anonymous, attr_conf *navmenu_cfg,
	spelling_t *spelling
	) { 

	bblog(INFO, "searchkernel: dosearch(query=\"%s\")",  query);
	struct PagesResultsFormat PagesResults;
	struct filteronFormat filteron;
	struct hashtable *crc32maphash;
	struct duplicate_docids *dups;

	memset(&PagesResults,'\0',sizeof(PagesResults));


	PagesResults.SiderHeder = SiderHeder;
	PagesResults.antall = 0;
	PagesResults.filterOn = filterOn;
	PagesResults.servername = servername;
	PagesResults.start = start;
	PagesResults.searchd_config = searchd_config;
	PagesResults.anonymous = anonymous;
	PagesResults.nrOfSubnames = nrOfSubnames;
	PagesResults.subnames = subnames;
	PagesResults.groups_per_usersystem = NULL;
	PagesResults.usersystem_per_subname = NULL;

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


	PagesResults.indexnr = (start * MaxsHits);
	PagesResults.MaxsHits = MaxsHits;

	if ((*Sider  = malloc(sizeof(struct SiderFormat) * PagesResults.MaxsHits)) == NULL) {
		bblog_errno(ERROR, "malloc Sider");
		exit(1);
	}


	PagesResults.Sider = *Sider;

	bblog(INFO, "MaxsHits %i, indexnr %i", PagesResults.MaxsHits,PagesResults.indexnr);

	strscpy(PagesResults.search_user,search_user,sizeof(PagesResults.search_user));
	//PagesResults.password
	PagesResults.DomainIDs = DomainIDs;
	PagesResults.filtered					= 0;
	PagesResults.filteredsilent				= 0;
	PagesResults.memfiltered				= 0;	
	strcpy(PagesResults.useragent, useragent);


         (*SiderHeder).filtersTraped.cantDIRead 		= 0;
         (*SiderHeder).filtersTraped.getingDomainID 		= 0;
         (*SiderHeder).filtersTraped.sameDomainID 		= 0;

         (*SiderHeder).filtersTraped.filterAdultWeight_bool 	= 0;
         (*SiderHeder).filtersTraped.filterAdultWeight_value 	= 0;
         (*SiderHeder).filtersTraped.filterSameCrc32_1 		= 0;
         (*SiderHeder).filtersTraped.filterSameUrl 		= 0;
         (*SiderHeder).filtersTraped.find_domain_no_subname 	= 0;
         (*SiderHeder).filtersTraped.filterSameDomain 		= 0;
         (*SiderHeder).filtersTraped.filterTLDs 		= 0;
         (*SiderHeder).filtersTraped.filterResponse 		= 0;
         (*SiderHeder).filtersTraped.cantpopResult 		= 0;
         (*SiderHeder).filtersTraped.cmc_pathaccess 		= 0;
         (*SiderHeder).filtersTraped.filterSameCrc32_2 		= 0;
         (*SiderHeder).filtersTraped.filterNoUrl 		= 0;


	#ifdef BLACK_BOX
		searchFilterInit(filters,dates);
	#endif

	int readedFromIndex;
	int i, y, j;

	if ((PagesResults.TeffArray = malloc(sizeof(struct iindexFormat))) == NULL) {
		perror("Cant malloc PagesResults.TeffArray");
		exit(-1);
	}
	PagesResults.TeffArray->nrofHits = 0;


	struct timeval start_time, end_time;
	struct timeval popResult_start_time, popResult_end_time;


	
	// inaliserer alle queryTime elementene
	memset(&(*SiderHeder).queryTime, 0, sizeof((*SiderHeder).queryTime));

	(*SiderHeder).queryTime.cmc_conect 			= 0;
	(*SiderHeder).queryTime.pathaccess 			= 0;
	(*SiderHeder).queryTime.urlrewrite 			= 0;
	#ifdef BLACK_BOX
		(*SiderHeder).queryTime.html_parser_run 	= 0;
		(*SiderHeder).queryTime.generate_snippet 	= 0;
		(*SiderHeder).queryTime.duplicat_echecking 	= 0;
		(*SiderHeder).queryTime.FilterCount 		= 0;
	#endif


	#if defined BLACK_BOX && !defined _24SEVENOFFICE


		gettimeofday(&start_time, NULL);

		if (!PagesResults.anonymous) {
			//henter brukerens passord fra boithoad
			//henter inn brukerens passord
			bblog(INFO, "geting pw for \"%s\"", PagesResults.search_user);
			/**************************************************************************
			Runarb: 08 Jan 2008: Gjør av vi bare hopper over å hente gruppe info hvis vi ikke
			for det til.

			Før stoppet vi opp, men det gjør at det ikke fungerer med 24so søk. 
			**************************************************************************/
			if (!cmc_getPassword(PagesResults.search_user,PagesResults.password)) {
				bblog(ERROR, "Can't boithoad_getPassword. Brukeren er ikke logget inn??");
			}
			else {
				//debug: printf("got pw \"%s\" -> \"%s\"\n",PagesResults.search_user,PagesResults.password);
			}
		} else {
			bblog(INFO, "Anonymous search");
		}


		/****************************************************************/


		gettimeofday(&end_time, NULL);
		(*SiderHeder).queryTime.getUserObjekt = getTimeDifference(&start_time,&end_time);
		bblog(INFO, "geting pw tme boithoad_getPassword() %f", (*SiderHeder).queryTime.getUserObjekt);		
		
	#endif


	//kopierer query inn i strukturen som holder query date
	strscpy(PagesResults.QueryData.query,query,sizeof(PagesResults.QueryData.query));

	get_query( PagesResults.QueryData.query, queryLen, &PagesResults.QueryData.queryParsed );


	#if defined BLACK_BOX && !defined _24SEVENOFFICE
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
    		bblog(INFO, "Expanded query: %s",  buf);

	#endif

	#ifdef DEBUG
		bblog(DEBUGINFO, "query %s", PagesResults.QueryData.query);
	#endif

	int languageFilterAsNr[5];

	int languageFilternr;

	if (languageFilter[0] != '\0') {
		bblog(INFO, "languageFilter: %s", languageFilter);
		languageFilterAsNr[0] = getLangNr(languageFilter);
		languageFilternr = 1;
	}
	else {
		languageFilternr = 0;
	}


	#ifdef DEBUG
		int h;
		for (h=0;h<languageFilternr;h++) {
			bblog(DEBUGINFO, "ll %i", languageFilterAsNr[h]);
		}
		bblog(DEBUGINFO, "languageFilternr %i", languageFilternr);
	#endif

	// inaliseres til 1. Hvis vi har tråstøtte, og ikke kjører i single mode settes den så til NROF_GENERATEPAGES_THREADS lengere nede.
	PagesResults.activetreads = 1;

	#ifdef WITH_THREAD
		if (!searchd_config->optSingle) {
			PagesResults.activetreads = NROF_GENERATEPAGES_THREADS;
		}

		pthread_t threadid[PagesResults.activetreads];

		dp_init();

		
		if (pthread_mutex_init(&PagesResults.mutex, NULL) != 0) {
			bblog(ERROR, "Can't init mutex!");
		}
		if (pthread_mutex_init(&PagesResults.mutextreadSyncFilter, NULL) != 0) {
			bblog(ERROR, "Can't init mutex!");
		}

		//låser mutex. Vi er jo enda ikke kalre til å kjøre
		if (pthread_mutex_lock(&PagesResults.mutex) != 0) {
			bblog(ERROR, "Can't lock mutex!");
		}

		if (!searchd_config->optSingle) {

			//start som thread that can get the pages
			for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
				if (pthread_create(&threadid[i], NULL, generatePagesResults, &PagesResults) != 0) {
					bblog(ERROR, "Can't create thread for page geberation!");
				}
			}

		}
	#endif

	#ifdef DEBUG
		bblog(DEBUGINFO, "searchSimple");
	#endif

	searchSimple(&PagesResults.antall,&PagesResults.TeffArray,&(*SiderHeder).TotaltTreff,
			&PagesResults.QueryData.queryParsed,&(*SiderHeder).queryTime,
			subnames,nrOfSubnames,languageFilternr,languageFilterAsNr,
			orderby, filters,&filteron,&PagesResults.QueryData.search_user_as_query, 0, 
			&crc32maphash, &dups,search_user, searchd_config->cmc_port, PagesResults.anonymous,
			&PagesResults.groups_per_usersystem, &PagesResults.usersystem_per_subname);

	PagesResults.crc32maphash = crc32maphash;
	PagesResults.dups = dups;

	#ifdef DEBUG
		bblog(DEBUGINFO, "end searchSimple");
	#endif


	#ifdef BLACK_BOX
		#ifdef WITH_THREAD
			if (pthread_mutex_init(&PagesResults.cmConn.mutex, NULL) != 0) {
				bblog(ERROR, "Can't init mutex!");
			}
			if (pthread_mutex_init(&PagesResults.cmConnUrlrewrite.mutex, NULL) != 0) {
				bblog(ERROR, "Can't init mutex!");				
			}

			pthread_cond_init(&PagesResults.cmConn.cv, NULL);
			pthread_cond_init(&PagesResults.cmConnUrlrewrite.cv, NULL);
		#endif

		gettimeofday(&start_time, NULL);



		bblog(INFO, "making a connection(s) to crawlerManager(s)\n");

		// lager tilkobling for pathaccess
		if (!cmConect_and_pool(&PagesResults.cmConn, errorstr, errorLen, MAX_CM_CONSUMERS, searchd_config) ) {
			return 0;
		}
	
		// lager tilkobling for Urlrewrite
		if (!cmConect_and_pool(&PagesResults.cmConnUrlrewrite, errorstr, errorLen, MAX_CM_CONSUMERS_URLREWRITE, searchd_config) ) {
			return 0;
		}


		gettimeofday(&end_time, NULL);
		(*SiderHeder).queryTime.cmc_conect = getTimeDifference(&start_time,&end_time);
	#endif

	//intresang debug info som viser antall treff pr subname
	#ifdef BLACK_BOX
		//viser hvordan treffene er i subnames
		if (globalOptVerbose) {
			bblog(INFO, "subname records:");
			for (i=0;i<nrOfSubnames;i++) {
				bblog(INFO, "\t\"%s\": %i", subnames[i].subname,subnames[i].hits);
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

	#ifndef BLACK_BOX
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

	#else
		(*SiderHeder).queryTime.adultcalk = 0;
	#endif
	

	//går i utgangspungete gjenon alle sider, men eskaper når vi når anatllet vi vil ha
	PagesResults.showabal = 0;
	PagesResults.nextPage = 0;


	#ifndef BLACK_BOX
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

		bblog(DEBUGINFO, "devise sort:");
		for(i=0;i<toSort;i++) {
			bblog(DEBUGINFO, "DocID %u, device %i, nr %i", PagesResults.TeffArray->iindex[i].DocID,GetDevIdForLot(rLotForDOCid(PagesResults.TeffArray->iindex[i].DocID)),(int)PagesResults.TeffArray->iindex[i].phraseMatch);
		}				

	#endif

	gettimeofday(&popResult_start_time, NULL);


	#ifdef WITH_THREAD

		//vi har data. Lå tårdene jobbe med det
		if (pthread_mutex_unlock(&PagesResults.mutex) != 0) {
			bblog(ERROR, "Can't unlock mutex!");
		}

		//av gjør om vi skal starte tråder eller kjøre det selv
		if (searchd_config->optSingle) {
			generatePagesResults(&PagesResults);
		}
		else {
			//venter på trådene
			for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
				if (pthread_join(threadid[i], NULL) != 0) {
					bblog(ERROR, "Can't join thread");
				}
			}

		}

		//free mutex'en
		if (pthread_mutex_destroy(&PagesResults.mutex) != 0) {
			bblog(ERROR, "Can't destroy mutex!");
		}
		if(pthread_mutex_destroy(&PagesResults.mutextreadSyncFilter) != 0) {
			bblog(ERROR, "Can't destroy mutex!");
		}

		#ifdef BLACK_BOX
			if (pthread_mutex_destroy(&PagesResults.cmConn.mutex) != 0) {
				bblog(ERROR, "Can't destroy mutex!");
			}
			if (pthread_mutex_destroy(&PagesResults.cmConnUrlrewrite.mutex) != 0) {
				bblog(ERROR, "Can't destroy mutex!");
			}
		#endif


	#else 
		generatePagesResults(&PagesResults);		
	#endif

	#ifdef BLACK_BOX
		#ifdef WITH_THREAD
			{
				int k;

				for (k = 0; k < PagesResults.cmConn.max_consumers; k++) {
					cmc_close(PagesResults.cmConn.sock[k]);
				}

				for (k = 0; k < PagesResults.cmConnUrlrewrite.max_consumers; k++) {
					cmc_close(PagesResults.cmConnUrlrewrite.sock[k]);
				}
			}
		#else
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


	//lager en filtered verdi
	(*SiderHeder).filtered = PagesResults.filtered + PagesResults.memfiltered;	

	//runarb: 2 nov 2007: trkker ikke fra disse tallene, da det fører til at totalt treff tallene forandres. Viser heller en filtered beskjed
	#ifndef BLACK_BOX
		//fjerner filtered fra total oversikten
		//ToDo: bør dette også gjøres for web?
		//runarb: 23 jan 2008: ser ut til at dette bare gjøres for web er ifNdef her???
		(*SiderHeder).TotaltTreff -= (*SiderHeder).filtered;
		//tar også bort de som ble filtrert stille
		(*SiderHeder).TotaltTreff -= PagesResults.filteredsilent;
	#endif


	
	


	#ifdef BLACK_BOX
		(*SiderHeder).navigation_xml = searchFilterCount(&PagesResults.antall,PagesResults.TeffArray,filters,subnames,nrOfSubnames,&filteron,dates,
		    &(*SiderHeder).queryTime, searchd_config->getfiletypep, searchd_config->attrdescrp, navmenu_cfg, &PagesResults.QueryData.queryParsed, PagesResults.QueryData.outformat);

		(*SiderHeder).navigation_xml_len = strlen((*SiderHeder).navigation_xml);

		destroy(filteron.attributes);
		free(filteron.collection);
		free(filteron.date);
		free(filteron.sort);
	#endif

	// må altid inaliseres
	SiderHeder->spellcheckedQuery[0] = '\0';

	#ifdef WITH_SPELLING

	if (searchd_config->optFastStartup != 1) {

		/* Spellcheck the query */
		if (SiderHeder->TotaltTreff < 10) {
			query_array qa;

			#ifdef DEBUG_TIME
                		gettimeofday(&start_time, NULL);
        		#endif

			copy_query(&qa, &PagesResults.QueryData.queryParsed);

			container	*all_groups = set_container( string_container() );
			container	*all_subnames = set_container( string_container() );
			iterator2	it_grp;

			// Add groups
			for (i=0; i<256; i++)
			    if (PagesResults.groups_per_usersystem[i]!=NULL)
				for (it_grp=set_begin2(PagesResults.groups_per_usersystem[i]); it_grp.valid; ds_next(it_grp))
				    set_insert(all_groups, ds_key(it_grp).str);

			// Add subnames
			for (i=0; i<qa.n; i++)
			    if (qa.query[i].operand == QUERY_COLLECTION)
				{
				    int		j, len=0;
				    for (j=0; j<qa.query[i].n; j++)
					len+= strlen(qa.query[i].s[j]);

				    char	subname[len+1];
				    subname[len] = '\0';

				    len = 0;
				    for (j=0; j<qa.query[i].n; j++)
					{
					    strcpy(subname+len, qa.query[i].s[j]);
					    len+= strlen(qa.query[i].s[j]);
					}

				    utf8_strtolower((utf8_byte*)subname);
				    set_insert(all_subnames, subname);
			        }

			if (set_size(all_subnames)==0)
			    {
				for (i=0; i<nrOfSubnames; i++)
				    {
					char	subname[strlen(subnames[i].subname)+1];
					strcpy(subname, subnames[i].subname);
					utf8_strtolower((utf8_byte*)subname);
					set_insert(all_subnames, subname);
				    }
			    }

			printf("groups = "); println(all_groups);
			printf("subnames = "); println(all_subnames);

			if (spellcheck_query(SiderHeder, &qa, spelling, PagesResults.anonymous ? NULL : all_groups, all_subnames) > 0) {
				bblog(INFO, "Spelling: Query corrected to: %s",  SiderHeder->spellcheckedQuery);
			}
			else {
				bblog(DEBUGINFO, "Spelling: no match");
			}

			destroy(all_groups);
			destroy(all_subnames);
			destroy_query(&qa);

			#ifdef DEBUG_TIME
                		gettimeofday(&end_time, NULL);
                		bblog(DEBUGINFO, "Time debug: spellcheck_query(%s) time: %f", PagesResults.QueryData.query,getTimeDifference(&start_time, &end_time));
		        #endif

		}
	}
	#endif

	//lager en liste med ordene som ingikk i queryet til hiliting
	hiliteQuery[0] = '\0';


	#ifdef DEBUG
		bblog(DEBUGINFO, "hiliteQuery");
	#endif

	for (i=0; i<PagesResults.QueryData.queryParsed.n; i++) {

		for (j=0; j<PagesResults.QueryData.queryParsed.query[i].n; j++) {

			strcat(hiliteQuery,PagesResults.QueryData.queryParsed.query[i].s[j]);      	

			//appender et komma, slik at vi får en komma separert liste med ord
			strncat(hiliteQuery,",",sizeof(*hiliteQuery) -1);		
			strcat(hiliteQuery,",");

		}

	}
	#ifdef DEBUG
		bblog(DEBUGINFO, "hiliteQuery END");
	#endif

	//det vil bli et komma for mye på slutten, fjerner det.
	//ToDo: er det altid et komma for mye ?	
	if (hiliteQuery[strlen(hiliteQuery) -1] == ',') {hiliteQuery[strlen(hiliteQuery) -1] = '\0';}


	bblog(INFO, "<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" filtered=\"%i\" showabal=\"%i\"/>", (*SiderHeder).TotaltTreff,PagesResults.QueryData.query,hiliteQuery,(*SiderHeder).filtered,(*SiderHeder).showabal);
	

	//printer ut info om brukt tid
	bblog(INFO, "Time");
	bblog(INFO, "\t%-40s %f", "AnchorSearch",(*SiderHeder).queryTime.AnchorSearch);
	bblog(INFO, "\t%-40s %f", "MainSearch",(*SiderHeder).queryTime.MainSearch);
	bblog(INFO, "\t%-40s %f", "MainAnchorMerge",(*SiderHeder).queryTime.MainAnchorMerge);
	bblog(INFO, "");
	bblog(INFO, "\t%-40s %f", "popRank",(*SiderHeder).queryTime.popRank);
	bblog(INFO, "\t%-40s %f", "responseShortning",(*SiderHeder).queryTime.responseShortning);
	bblog(INFO, "");
	bblog(INFO, "\t%-40s %f", "allrankCalc",(*SiderHeder).queryTime.allrankCalc);
	bblog(INFO, "\t%-40s %f", "indexSort",(*SiderHeder).queryTime.indexSort);
	bblog(INFO, "\t%-40s %f", "popResult",(*SiderHeder).queryTime.popResult);
	bblog(INFO, "\t%-40s %f", "adultcalk",(*SiderHeder).queryTime.adultcalk);

	#ifdef BLACK_BOX
		bblog(INFO, "\t%-40s %f", "filetypes",(*SiderHeder).queryTime.filetypes);
		bblog(INFO, "\t%-40s %f", "iintegerGetValueDate",(*SiderHeder).queryTime.iintegerGetValueDate);
		bblog(INFO, "\t%-40s %f", "dateview",(*SiderHeder).queryTime.dateview);
		bblog(INFO, "\t%-40s %f", "FilterCount",(*SiderHeder).queryTime.FilterCount);
		bblog(INFO, "\t%-40s %f", "pathaccess",(*SiderHeder).queryTime.pathaccess);
		bblog(INFO, "\t%-40s %f", "urlrewrite",(*SiderHeder).queryTime.urlrewrite);
		bblog(INFO, "\t%-40s %f", "duplicat echecking",(*SiderHeder).queryTime.duplicat_echecking);
		bblog(INFO, "\t%-40s %f", "cmc_conect",(*SiderHeder).queryTime.cmc_conect);
	#endif

	#ifndef _24SEVENOFFICE
		bblog(INFO, "\t%-40s %f", "getUserObjekt",(*SiderHeder).queryTime.getUserObjekt);
	#endif

	#ifdef DEBUG_TIME
		bblog(DEBUGINFO, "popResult:");
		bblog(DEBUGINFO, "\t%-40s %i, %f", "DocumentIndex",PagesResults.popResultBreakDownTime.DocumentIndex.nr,PagesResults.popResultBreakDownTime.DocumentIndex.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "ReadSummary",PagesResults.popResultBreakDownTime.ReadSummary.nr,PagesResults.popResultBreakDownTime.ReadSummary.time);

		bblog(DEBUGINFO, "\t%-40s %i, %f", "generate_snippet",PagesResults.popResultBreakDownTime.generate_snippet.nr,PagesResults.popResultBreakDownTime.generate_snippet.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "html_parser_run",PagesResults.popResultBreakDownTime.html_parser_run.nr,PagesResults.popResultBreakDownTime.html_parser_run.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "ReadHtml",PagesResults.popResultBreakDownTime.ReadHtml.nr,PagesResults.popResultBreakDownTime.ReadHtml.time);

		bblog(DEBUGINFO, "\t%-40s %i, %f", "memGetDomainID",PagesResults.popResultBreakDownTime.memGetDomainID.nr,PagesResults.popResultBreakDownTime.memGetDomainID.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "totalpopResult",PagesResults.popResultBreakDownTime.totalpopResult.nr,PagesResults.popResultBreakDownTime.totalpopResult.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "makecrc32",PagesResults.popResultBreakDownTime.makecrc32.nr,PagesResults.popResultBreakDownTime.makecrc32.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "treadSyncFilter",PagesResults.popResultBreakDownTime.treadSyncFilter.nr,PagesResults.popResultBreakDownTime.treadSyncFilter.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "titleClean",PagesResults.popResultBreakDownTime.titleClean.nr,PagesResults.popResultBreakDownTime.titleClean.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "bodyClean",PagesResults.popResultBreakDownTime.bodyClean.nr,PagesResults.popResultBreakDownTime.bodyClean.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "iindexMemcpy",PagesResults.popResultBreakDownTime.iindexMemcpy.nr,PagesResults.popResultBreakDownTime.iindexMemcpy.time);
		bblog(DEBUGINFO, "\t%-40s %i, %f", "popResultFree",PagesResults.popResultBreakDownTime.popResultFree.nr,PagesResults.popResultBreakDownTime.popResultFree.time);

	#endif



	bblog(INFO, "filters:");

	bblog(INFO, "\t%-40s %i", "cantDIRead",(*SiderHeder).filtersTraped.cantDIRead);
	bblog(INFO, "\t%-40s %i", "getingDomainID",(*SiderHeder).filtersTraped.getingDomainID);
	bblog(INFO, "\t%-40s %i", "sameDomainID",(*SiderHeder).filtersTraped.sameDomainID);

	bblog(INFO, "\t%-40s %i", "filterAdultWeight_bool",(*SiderHeder).filtersTraped.filterAdultWeight_bool);
	bblog(INFO, "\t%-40s %i", "filterAdultWeight_value",(*SiderHeder).filtersTraped.filterAdultWeight_value);
	bblog(INFO, "\t%-40s %i", "filterSameCrc32_1",(*SiderHeder).filtersTraped.filterSameCrc32_1);
	bblog(INFO, "\t%-40s %i", "filterSameUrl",(*SiderHeder).filtersTraped.filterSameUrl);
	bblog(INFO, "\t%-40s %i", "filterNoUrl",(*SiderHeder).filtersTraped.filterNoUrl);
	bblog(INFO, "\t%-40s %i", "find_domain_no_subname",(*SiderHeder).filtersTraped.find_domain_no_subname);
	bblog(INFO, "\t%-40s %i", "filterSameDomain",(*SiderHeder).filtersTraped.filterSameDomain);
	bblog(INFO, "\t%-40s %i", "filterTLDs",(*SiderHeder).filtersTraped.filterTLDs);
	bblog(INFO, "\t%-40s %i", "filterResponse",(*SiderHeder).filtersTraped.filterResponse);
	bblog(INFO, "\t%-40s %i", "cantpopResult",(*SiderHeder).filtersTraped.cantpopResult);
	bblog(INFO, "\t%-40s %i", "cmc_pathaccess",(*SiderHeder).filtersTraped.cmc_pathaccess);
	bblog(INFO, "\t%-40s %i", "filterSameCrc32_2",(*SiderHeder).filtersTraped.filterSameCrc32_2);

	bblog(INFO, "");
	bblog(INFO, "\t%-40s %i", "filteredsilent",PagesResults.filteredsilent);

	bblog(INFO, "");

	#ifndef BLACK_BOX
		bblog(INFO, "\tnoadultpages %i, adultpages %i", PagesResults.noadultpages,PagesResults.adultpages);
	#endif

	bblog(INFO, "");


	if (globalOptVerbose) {
		print_explane_rank(*Sider,(*SiderHeder).showabal);
	}

        #ifdef DEBUG_TIME
                gettimeofday(&start_time, NULL);
        #endif

	// Frigjør minne:
	bblog(INFO, "free memory");

	free(PagesResults.TeffArray);

	// Slett innholdet i crc32maphash:
	
	struct hashtable_itr *itr = hashtable_iterator(PagesResults.crc32maphash);

	while (itr->e!=NULL)
	    {
		struct duplicate_docids *dup = hashtable_iterator_value(itr);
		if (dup->V != NULL) {
			destroy(dup->V);
		}
		hashtable_iterator_advance(itr);
	    }

	free(itr);
	
	free(PagesResults.dups);

	hashtable_destroy(PagesResults.crc32maphash,0);

	destroy_query( &PagesResults.QueryData.queryParsed );
	destroy_query( &PagesResults.QueryData.search_user_as_query );

	if (PagesResults.groups_per_usersystem != NULL)
	    {
		for (i=0; i<256; i++)
		    if (PagesResults.groups_per_usersystem[i] != NULL)
			destroy(PagesResults.groups_per_usersystem[i]);

		free(PagesResults.groups_per_usersystem);
	    }

	free(PagesResults.usersystem_per_subname);

        #ifdef DEBUG_TIME
                gettimeofday(&end_time, NULL);
                bblog(DEBUGINFO, "Time debug: freeing mem time: %f", getTimeDifference(&start_time, &end_time));
        #endif
	
	bblog(INFO, "searchkernel: ~dosearch()");
	return 1;
}

