#include "../common/define.h"
#include "../common/lot.h"
#include "../common/vid.h"
#include "../common/stdlib.h"
#include "../common/utf8-strings.h"
#include "../UrlToDocID/search_index.h"

#include "../banlists/ban.h"

#include "library.h"

#include <mysql.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h> // gettimeofday
#include <sys/file.h> // flock
#include <unistd.h>
#include <errno.h> 
#include <time.h>
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <zlib.h>
#include <err.h>

#include "../common/boithohome.h"
#include "../common/bstr.h"
#include "../maincfg/maincfg.h"
#include "../common/cgi.h" // escapeHTML
#include "../crawlManager/client.h"
#include "../key/key.h" // key_get_existingconn

#include <libconfig.h>
#define CFG_SEARCHD "config/searchd.conf"


    #include "../ds/dcontainer.h"
    #include "../ds/dset.h"
    #include "../query/query_parser.h"
    #include "../common/bprint.h"
    #include "../common/xml.h"


    
#ifndef BLACK_BOKS
    #include <libconfig.h>
    #define CACHE_STRUCT_VERSION "1.7"
    //#define cashedir "cashedir"
    //#define prequerydir "prequerydir"
#endif

    #define CFG_DISPATCHER "config/dispatcher.conf"

#ifndef BLACK_BOKS
    #include "GeoIP.h"
    #include "GeoIPCity.h"
#endif


    //temp
    //#define NO_LOGING


    #include "../searchFilters/searchFilters.h"
    #include "../common/timediff.h"
    #include "../common/mgsort.h"
    #include "../common/bstr.h"
    #include "../tkey/tkey.h"

    //#define PORT 6500 // the port client will be connecting to 

#ifdef BLACK_BOKS
 	// get user groups
	#include "../acls/acls.h"
	#include "../boithoadClientLib/liboithoaut.h"
#endif

#ifdef DEBUG
#define dprintf(str, args...) printf(str, ##args)
#else
#define dprintf(str, args...) 
#endif
#include "cgihandler.h"
#include "out_sdxml.h"
#include "out_opensearch.h"

void read_collection_cfg(struct subnamesConfigFormat * dst);
void read_dispatcher_cfg(struct config_t * cfg, struct dispconfigFormat * dispconfig, int *cachetimeout);

/* Set if you want to write prequery data */
int prequerywriteFlag = 0;




    int compare_elements (const void *p1, const void *p2);
    int compare_elements_posisjon (const void *p1, const void *p2);



#ifdef BLACK_BOKS

struct subnamesFormat *get_usr_coll(char *usr, int *n_colls, int cmc_port) {
	char **colls;
	char *collections;
	struct subnamesFormat *usr_colls = NULL;
	int sock, i;
	char buf[1024];

	if (cmc_conect(&sock, buf, sizeof(buf), cmc_port) == 0)
		die(1, "", "Unable to connect to crawlManager: %s", buf);
	cmc_collectionsforuser(sock, usr, &collections);
	cmc_close(sock);

	*n_colls = split(collections, ",", &colls);
	free(collections);

	usr_colls = malloc(sizeof(struct subnamesFormat) * *n_colls);
	for (i = 0; i < *n_colls; i++) {
		strncpy(usr_colls[i].subname, colls[i], sizeof(usr_colls[i].subname));
	}

	FreeSplitList(colls);

	return usr_colls;
}


int cfg_parse_array(char * dst, const char * arraystr) {
	char **data;
	int tokens = split(arraystr, ",", &data);

	int len = 0;
	int i;
	for (i = 0; i < tokens; i++) {
		if (i > BMAX_RANKARRAY) {
			warnx("cfg array parse: Array string too large, ignoring");
			continue;
		}

		int score = strtol(data[i], NULL, 10);

		dst[i] = (char) score;
		len++;
	}
	FreeSplitList(data);
	return len;
}


int fetch_coll_cfg(MYSQL *db, char *coll_name, struct subnamesConfigFormat *cfg) {
	const char query_tpl[] = "SELECT  \
		summary, filter_same_url, filter_same_domain, \
		filter_TLDs, filter_response, filter_same_crc32, \
		rank_author_array, rank_title_array, rank_title_first_word, \
		rank_headline_array, rank_body_array, rank_url_array, \
		rank_url_main_word, cache_link, without_aclcheck \
		FROM shareResults, shares \
		WHERE \
			shares.collection_name = '%s' \
			AND shares.id = shareResults.share ";
	char name_esc[maxSubnameLength * 2];
	mysql_real_escape_string(db, name_esc, coll_name, strlen(coll_name));
	
	char query[1024];
	snprintf(query, sizeof query, query_tpl, name_esc);
	
	cfg->has_config = 0;
	if (mysql_real_query(db, query, strlen(query)) != 0) {
		warnx("Mysql error (%s line %d): %s", 
			__FILE__, __LINE__, mysql_error(db));
		return 0;
	}
	
	MYSQL_RES *res = mysql_store_result(db);
	MYSQL_ROW row = mysql_fetch_row(res);
	if (row == NULL) {
		#ifdef DEBUG
			warnx("No config data for collection %s", coll_name);
		#endif

		mysql_free_result(res);
		return 0;
	}

	cfg->has_config = 1;
	if (strcmp(row[0], "start") == 0)
		cfg->summary = SUMMARY_START;
	else if (strcmp(row[0], "snippet") == 0)
		cfg->summary = SUMMARY_SNIPPET;
	else { 
		warnx("Unknown 'summary' format %s", row[0]);
		cfg->summary = SUMMARY_SNIPPET;
	}

	cfg->filterSameUrl    = row[1][0] == '1' ? 1 : 0;
	cfg->filterSameDomain = row[2][0] == '1' ? 1 : 0;
	cfg->filterTLDs       = row[3][0] == '1' ? 1 : 0;
	cfg->filterResponse   = row[4][0] == '1' ? 1 : 0;
	cfg->filterSameCrc32  = row[5][0] == '1' ? 1 : 0;

	cfg->rankAthorArrayLen    = cfg_parse_array(cfg->rankAthorArray, row[6]);
	cfg->rankTittelArrayLen   = cfg_parse_array(cfg->rankTittelArray, row[7]);
	cfg->rankTittelFirstWord = (char) strtol(row[8], NULL, 10);
	cfg->rankHeadlineArrayLen = cfg_parse_array(cfg->rankHeadlineArray, row[9]);
	cfg->rankBodyArrayLen     = cfg_parse_array(cfg->rankBodyArray, row[10]);
	cfg->rankUrlArrayLen      = cfg_parse_array(cfg->rankUrlArray, row[11]);

	cfg->rankUrlMainWord = (char) strtol(row[12], NULL, 10);

	cfg->defaultthumbnail = NULL;
	cfg->isPaidInclusion = 0;
	cfg->sqlImpressionsLogQuery[0] = '\0';

	cfg->cache_link = row[13][0] == '1' ? 1 : 0;
	cfg->without_aclcheck = row[14][0] == '1' ? 1 : 0;

	mysql_free_result(res);
	return 1;
}
#endif



/* Cache helper functions */

// sprintf(cashefile,"%s/%s.%i.%s","/home/boitho/var/cashedir",QueryData.queryhtml,QueryData.start,QueryData.GeoIPcontry);

void
handle_results(int *sockfd, struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,
               struct QueryDataForamt *QueryData,
               struct SiderHederFormat *FinalSiderHeder, int fromCache, struct errorhaFormat *errorha,
               int pageNr, int nrOfServers, int nrOfPiServers, struct filtersTrapedFormat *dispatcherfiltersTraped,
	       int *nrRespondedServers,struct queryNodeHederFormat *queryNodeHeder, MYSQL *demo_db) 
{
	int AdultPages, NonAdultPages;
	int posisjon, i;
	int funnet;
#ifdef DEBUG_TIME
	struct timeval start_time, end_time;
#endif


	*nrRespondedServers = 0;
	if (!fromCache) {

		FinalSiderHeder->TotaltTreff = 0;
		FinalSiderHeder->filtered = 0;

		for (i=0;i<(nrOfServers + nrOfPiServers);i++) {
			//aaaaa
			if (sockfd[i] != 0) {
				FinalSiderHeder->TotaltTreff += SiderHeder[i].TotaltTreff;
				FinalSiderHeder->filtered += SiderHeder[i].filtered;
				dprintf("response from \"%s\": totalt %i, tid %f filtered %i, showabal %i\n",
					SiderHeder[i].servername,
					SiderHeder[i].TotaltTreff,
					SiderHeder[i].total_usecs,
					SiderHeder[i].filtered,
					SiderHeder[i].showabal);
				/*
				runarb: 31.08.07seg feiler her.
				dprintf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" tid=\"%f\" filtered=\"%i\" showabal=\"%i\" servername=\"%s\"/>\n",
					SiderHeder[i].TotaltTreff,
					QueryData->query,
					SiderHeder[i].hiliteQuery,
					SiderHeder[i].total_usecs,
					SiderHeder[i].filtered,
					SiderHeder[i].showabal,
					SiderHeder[i].servername);
				*/
				(*nrRespondedServers)++;
			}
		}

#if 0
#ifdef DEBUG
		dprintf("addservers (have %i):\n",nrOfAddServers);
		for (i=0;i<nrOfAddServers;i++) {
			//aaaaa
			if (addsockfd[i] != 0) {
				dprintf("\t<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" tid=\"%f\" filtered=\"%i\" showabal=\"%i\" servername=\"%s\"/>\n",AddSiderHeder[i].TotaltTreff,QueryData->query,AddSiderHeder[i].hiliteQuery,AddSiderHeder[i].total_usecs,AddSiderHeder[i].filtered,AddSiderHeder[i].
showabal,AddSiderHeder[i].servername);

			}
			else {
				dprintf("addserver nr %i's sockfd is 0\n",i);
			}
		}
#endif
#endif

		//finner en hillitet query
		if (*nrRespondedServers != 0) {
			funnet = 0;
			for(i=0;i<(nrOfServers + nrOfPiServers) && !funnet;i++) {
				if ((sockfd[i] != 0) && (SiderHeder[i].hiliteQuery != '\0')) {
					strcpy(FinalSiderHeder->hiliteQuery,SiderHeder[i].hiliteQuery);
					funnet =1;
				}
			}
		}
		else {
			FinalSiderHeder->hiliteQuery[0] = '\0';
		}

		//hvis vi ikke fikk svar fra noen
		if(*nrRespondedServers == 0) {
			dieLog(demo_db, QueryData, 16,"","Couldn't contact the search system. Please try again later.");
		}
		//genererer feil om at ikke alle server svarte på queryet
		if (*nrRespondedServers != (nrOfServers + nrOfPiServers)) {
			addError(errorha,11,"Not all the search nodes responded to your query. Result quality may have been negatively effected.");
		}
		//hånterer error. Viser den hvis vi hadde noen
		if (*nrRespondedServers != 0) {
			for(i=0;i<(nrOfServers + nrOfPiServers);i++) {
				if (SiderHeder[i].responstype == searchd_responstype_error) {
					addError(errorha,11,SiderHeder[i].errorstr);
				}
			}
		}

		//fjerner eventuelle adult sider
		AdultPages = 0;
		NonAdultPages = 0;
		for(i=0;i<queryNodeHeder->MaxsHits * nrOfServers + nrOfPiServers;i++) {	
			if (!Sider[i].deletet) {

				if (Sider[i].DocumentIndex.AdultWeight > 50) {
					++AdultPages;
				}
				else {
					++NonAdultPages;
				}

			}		
		}

		dprintf("AdultPages %i, NonAdultPages: %i\n",AdultPages,NonAdultPages);
		//hvis vi har adult pages sjekker vi om vi har nokk ikke adult pages å vise, hvis ikke viser vi bare adult

	} // !hascashe && !hasprequery
	else {
		*nrRespondedServers = 1;

	}

#ifdef DEBUG_TIME
		gettimeofday(&start_time, NULL);
#endif
		//sorterer resultatene
		dprintf("mgsort: pageNr %i\n",pageNr);
		//tmp:
		//dette skaper problemer for blaingen på bb. Sikkert samme problmet på web, så vi må se på hva vi kan gjøre
		#ifndef BLACK_BOKS
			mgsort(Sider, pageNr , sizeof(struct SiderFormat), compare_elements);
		#endif

#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		fprintf(stderr,"Time debug: mgsort_1 %f\n",getTimeDifference(&start_time,&end_time));
#endif

#ifdef DEBUG_TIME
		gettimeofday(&start_time, NULL);
#endif		

		filtersTrapedReset(dispatcherfiltersTraped);

		//dette er kansje ikke optimalet, da vi går gjenom alle siden. Ikke bare de som skal være med
		for(i=0;i<queryNodeHeder->MaxsHits * (nrOfServers + nrOfPiServers);i++) {


			if (Sider[i].deletet) {
				dprintf("page is deleted\n");
			}
			else {

				dprintf("looking on url %s, adult %i, allrank %u, i %i, type %i\n",Sider[i].DocumentIndex.Url,Sider[i].DocumentIndex.AdultWeight,Sider[i].iindex.allrank,i,Sider[i].type);

				//setter som slettet
				Sider[i].deletet = 1;


				#ifndef BLACK_BOKS
				// hvis dette er en pi side, må vi håntere at det kan komme versjoner av den som har bedre rank
				// hvis det skjer skal vi bruke pi siden, og forkaste den andre
	
				if (pi_switch(i,&Sider[i],Sider)) {
					dprintf("pi switch'ed url \"%s\"\n",Sider[i].url);
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					continue;
				}
	
				#endif

				if ((QueryData->filterOn) && (Sider[i].subname.config.filterSameUrl) 
						&& (filterSameUrl(i,Sider[i].url,Sider)) ) {
					dprintf("Hav seen url befor. Url '%s', DocID %u\n",Sider[i].url,Sider[i].iindex.DocID);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterSameUrl;					
					continue;
				}

#ifndef BLACK_BOKS

#if 0
				// 19. juni
				//ToDo: fjerner adult vekt filtrering her. Er det trykt. Hvis vi for eks har misket resultater, men ikke noen noder hadde fø sider, og tilot adoult
				// hva er egentlig adoult filter statur på searchd nå?
				if ((QueryData->filterOn) && Sider[i].DocumentIndex.AdultWeight > 50) {
					dprintf("slettet adult side %s ault %i\n",Sider[i].url,Sider[i].DocumentIndex.AdultWeight);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterAdultWeight_value;
					continue;
				}
#endif
				if ((QueryData->filterOn) && (Sider[i].subname.config.filterSameCrc32) 
						&& filterSameCrc32(i,&Sider[i],Sider)) {
					dprintf("hav same crc32. crc32 from DocumentIndex. Will delete \"%s\"\n",Sider[i].DocumentIndex.Url);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterSameCrc32_1;

					continue;
				}

				if ((QueryData->filterOn) && (Sider[i].subname.config.filterSameDomain) 
						&& (filterSameDomain(i,&Sider[i],Sider))) {
					dprintf("hav same domain \"%s\"\n",Sider[i].domain);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterSameDomain;

					continue;
				}

#if 0
				if ((QueryData->filterOn) && filterDescription(i,&Sider[i],Sider)) {
					dprintf("hav same Description. DocID %i\n",Sider[i].iindex.DocID);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					continue;
				}
#endif
#endif
				#ifndef BLACK_BOKS
				if(isDomainBan(Sider[i].domain)) {

					dprintf("hav baned domain \"%s\"\n",Sider[i].domain);
					//(*SiderHeder).filtered++;
					//filtrerer stille
					//FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					//++dispatcherfiltersTraped->filterBannedDomain;

					continue;

				}
				#endif

				//printf("url %s\n",Sider[i].DocumentIndex.Url);

				//hvis siden overlevde helt hit er den ok
				Sider[i].deletet = 0;
			}
		}

//	} // !hascashe && !hasprequery
//	else {
//		*nrRespondedServers = 1;
//
//	}

	//why was sort here???
	posisjon=0;
	for(i=0;i<queryNodeHeder->MaxsHits * (nrOfServers + nrOfPiServers);i++) {
		if (!Sider[i].deletet) {
			Sider[i].posisjon = posisjon++;
			#ifdef DEBUG
			printf("setting pos %i for %s\n",Sider[i].posisjon,Sider[i].url);
			#endif
		}

	}	

	#ifdef DEBUG
		printf("\n");
		printf("Sider etter filtrering:\n");
		for(i=0;i<queryNodeHeder->MaxsHits * (nrOfServers + nrOfPiServers);i++) {
			//if (!Sider[i].deletet) {

				printf("i: %i, url: %s, rank %i, type %i pos %i, server \"%s\"\n",i,Sider[i].url,Sider[i].iindex.allrank,Sider[i].type,Sider[i].posisjon,Sider[i].servername);

			//}
		}
		printf("\n");
	#endif

#ifdef DEBUG_TIME
	gettimeofday(&end_time, NULL);
	fprintf(stderr,"Time debug: filter pages %f\n",getTimeDifference(&start_time,&end_time));
#endif


#ifdef DEBUG_TIME
	gettimeofday(&start_time, NULL);
#endif

	//resorterer query
	//mgsort(Sider, nrOfServers * QueryData->MaxsHits , sizeof(struct SiderFormat), compare_elements_posisjon);
	mgsort(Sider, pageNr , sizeof(struct SiderFormat), compare_elements_posisjon);

#ifdef DEBUG_TIME
	gettimeofday(&end_time, NULL);
	fprintf(stderr,"Time debug: mgsort_2 %f\n",getTimeDifference(&start_time,&end_time));
#endif

	/* tempaa */
#if 0
	FinalSiderHeder->showabal = 0;
	for(i=0;i<queryNodeHeder->MaxsHits * nrOfServers;i++) {
		if (!Sider[i].deletet) {
			++FinalSiderHeder->showabal;
		}
	}
#endif

	FinalSiderHeder->showabal = pageNr;
	/*
	runarb: 23 okt 2007: dette ser ut til å skape problemer med den nye måten og bla serp på. siden vi kan få flere en MaxsHits
	Komenterer ut for nå
	if (FinalSiderHeder->showabal > queryNodeHeder->MaxsHits) {
		FinalSiderHeder->showabal = queryNodeHeder->MaxsHits;
	}
	*/
}

#ifdef EXPLAIN_RANK
void print_explain_rank( struct SiderFormat *Side, char query[]) {

	printf("<![CDATA[Rb=%hu;%hu;%hu&amp;Rh=%hu;%hu;%hu&amp;Rt=%hu;%hu;%hu&amp;Ra=%hu;%hu;%hu&amp;Rum=%hu;%hu;%hu&amp;Rud=%hu;%hu;%hu&amp;Rus=%hu;%hu;%hu&amp;AllRank=%i&amp;TermRank=%i&amp;PopRank=%i&amp;DocID=%i-%i&amp;Url=%s&amp;Query=%s]]>\n",
		Side->iindex.rank_explaind.rankBody,Side->iindex.rank_explaind.nrBody,Side->iindex.rank_explaind.maxBody,
		Side->iindex.rank_explaind.rankHeadline,Side->iindex.rank_explaind.nrHeadline,Side->iindex.rank_explaind.maxHeadline,
		Side->iindex.rank_explaind.rankTittel,Side->iindex.rank_explaind.nrTittel,Side->iindex.rank_explaind.maxTittel,
		Side->iindex.rank_explaind.rankAthor,Side->iindex.rank_explaind.nrAthor,Side->iindex.rank_explaind.maxAthor,
		Side->iindex.rank_explaind.rankUrl_mainbody,Side->iindex.rank_explaind.nrUrl_mainbody,Side->iindex.rank_explaind.maxUrl_mainbody,
		Side->iindex.rank_explaind.rankUrlDomain,Side->iindex.rank_explaind.nrUrlDomain,Side->iindex.rank_explaind.maxUrlDomain,
		Side->iindex.rank_explaind.rankUrlSub,Side->iindex.rank_explaind.nrUrlSub,Side->iindex.rank_explaind.maxUrlSub,
		Side->iindex.allrank,
                Side->iindex.TermRank,
                Side->iindex.PopRank,
		Side->iindex.DocID,rLotForDOCid(Side->iindex.DocID),
		Side->url,
		query

	);

}
#endif

unsigned int getDocIDFromSqlRun (MYSQL *demo_db, char mysql_query[], char rankUrl[]) {

	unsigned int retDocID = 0;


	int numUsers;
	MYSQL_RES *mysqlres; // To be used to fetch information into
	MYSQL_ROW mysqlrow;


	if(mysql_real_query(demo_db, mysql_query, strlen(mysql_query))){ // Make query
       		printf(mysql_error(demo_db));
       		fprintf(stderr,"MySQL Error: \"%s\".\n",mysql_error(demo_db));
	  	numUsers = 0;
	}
	else {
		mysqlres=mysql_store_result(demo_db); // Download result from server 

		numUsers = mysql_num_rows(mysqlres);
	}

	if (numUsers == 1) {

		if ((mysqlrow=mysql_fetch_row(mysqlres)) == NULL) {
        		fprintf(stderr,"MySQL Error: cant download results \"%s\".\n",mysql_error(demo_db));
			//return 0;
		}
		#ifdef DEBUG
			printf("wwwDocID \"%s\"\n",mysqlrow[0]);
		#endif

		retDocID = atou(mysqlrow[0]);

		mysql_free_result(mysqlres);

		return retDocID;
       	}
	else {
		return 0;
	}
}

unsigned int getDocIDFromSql (MYSQL *demo_db, char rankUrl[]) {


	unsigned int retDocID = 0;
		
	////////////////////
	//mysql select for pi og freelistnings
	////////////////////
	char mysql_query[512];

	snprintf(mysql_query, sizeof(mysql_query), "select WWWDocID from submission_url where url='%s' and WWWDocID <> 0 AND WWWDocID is not NULL",
			rankUrl);

	
	retDocID = getDocIDFromSqlRun(demo_db, mysql_query, rankUrl);
	if (retDocID != 0) {
		return retDocID;
	}

	
	//hvis den ikke var i over, å vi søke i neste db
	snprintf(mysql_query, sizeof(mysql_query), "select WWWDocID from pi_sider where url='%s' and WWWDocID <> 0 AND WWWDocID is not NULL",
			rankUrl);

	
	retDocID = getDocIDFromSqlRun(demo_db, mysql_query, rankUrl);
	if (retDocID != 0) {
		return retDocID;
	}
	

	return 0;
}


int main(int argc, char *argv[])
{


	int cmc_port = 0;
        int sockfd[maxServers];
        int addsockfd[maxServers];
	int i,y,x;
	int pageNr;
	char documentlangcode[4];
	int totlaAds;
	//char *strpointer;  
	//int net_status;
	//int res;
	//int nerror;
	//int dataReceived[maxServers];
	//int siderDataReceived[maxServers];
        //char buf[MAXDATASIZE];
        //struct hostent *he[maxServers];
	FILE *LOGFILE;
	//struct SiderFormat Sider[20 * maxServers];
	struct SiderFormat *Sider;
	char colchecked[20];

	int noDoctype = 0;

        char vidbuf[64];
        time_t etime;
        time(&etime);
	
        //struct SiderHederFormat SiderHeder[maxServers];
        //struct SiderHederFormat AddSiderHeder[maxServers];

        struct SiderHederFormat *SiderHeder = malloc(sizeof(struct SiderHederFormat) * maxServers);
        struct SiderHederFormat *AddSiderHeder = malloc(sizeof(struct SiderHederFormat) * maxServers);

	size_t maxSider;
	struct SiderHederFormat FinalSiderHeder;
	//char buff[4096]; //generell buffer
#ifndef BLACK_BOKS
	struct in_addr ipaddr;
#endif
        struct QueryDataForamt QueryData;
	//int connected[maxServers];
	//int NonAdultPages,AdultPages;
	struct timeval search_start_time, search_end_time;
#ifdef DEBUG_TIME
	struct timeval total_start_time, total_end_time;
	struct timeval start_time, end_time;
#endif
	int nrRespondedServers;
	//char errormessage[maxerrorlen];
	struct errorhaFormat errorha;
	errorha.nr = 0;
	//int posisjon;
	//int socketWait;	
	int hascashe;
        int hasprequery;
#ifdef WITH_CASHE	
	char prequeryfile[512];
	char cashefile[512];
#endif
	struct filtersTrapedFormat dispatcherfiltersTraped;

	//char *cpnt;
#ifndef BLACK_BOKS
	char *lastdomain = NULL;
#endif

	unsigned int getRank = 0; /* Set if we are looking for the rank of a specific query on a url */

#ifndef BLACK_BOKS
	unsigned int wantedDocId;
#endif
	struct queryNodeHederFormat queryNodeHeder;

	struct dispconfigFormat dispconfig;
	int cachetimeout;

	#ifdef DEBUG_TIME
		gettimeofday(&total_start_time, NULL);
	#endif

	//starter å ta tiden
	gettimeofday(&search_start_time, NULL);


	#ifdef DEBUG_TIME
	gettimeofday(&start_time, NULL);
	#endif
	dprintf("struct SiderFormat size %i\n",sizeof(struct SiderFormat));



	struct config_t maincfg;

	maincfg = maincfgopen();

	#ifndef BLACK_BOKS
		domainLoad();
	#endif

	int searchport = 0;

	struct config_t cfg;
	// TODO: Also read servers witin function (so we can remove cfg from main() )
	read_dispatcher_cfg(&cfg, &dispconfig, &cachetimeout); 

	//char query [2048];

	//MYSQL_RES *mysqlres; /* To be used to fetch information into */
	static MYSQL demo_db;

	mysql_init(&demo_db);

#ifndef BLACK_BOKS
	//if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", "boithoweb", 3306, NULL, 0)){
	if(!mysql_real_connect(&demo_db, dispconfig.webdb_host, dispconfig.webdb_user, dispconfig.webdb_password, dispconfig.webdb_db, 3306, NULL, 0)){
		fprintf(stderr,"Can't connect to mysqldb: %s",mysql_error(&demo_db));
		//exit(1);
	}
#else
	if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", BOITHO_MYSQL_DB, 3306, NULL, 0)){
		fprintf(stderr,"Can't connect to mysqldb: %s",mysql_error(&demo_db));
		//exit(1);
	}
#endif



	//////////////////
	//for nå angir vi bare servere slik. Må skilles u i egen fil siden


	int nrOfServers;
	int nrOfPiServers;
	int nrOfAddServers;


	#ifdef BLACK_BOKS

		char *servers[] = { "localhost" };
		char *addservers[] = { };
		char *piservers[] = { };


		nrOfServers = (sizeof(servers) / sizeof(char *));
		nrOfPiServers = (sizeof(piservers) / sizeof(char *));
	        nrOfAddServers = (sizeof(addservers) / sizeof(char *));

	#else 

		config_setting_t *cfgarray;
		char **servers;
		char **piservers;
		char **addservers;

		//load server array from config    
	    	if ( (cfgarray = config_lookup(&cfg, "servers") ) == NULL) {
			printf("can't load \"servers\" from config\n");
			exit(1);
	  	}



		nrOfServers = config_setting_length(cfgarray);

		//ToDO: vi har nrOfPiServers med her, men uten at jeg kan se at vi fakrisk 
		//legger inn de. tar bort
		//servers = malloc(sizeof(char *) * (nrOfServers + nrOfPiServers));
		servers = malloc(sizeof(char *) * (nrOfServers));

		for(i=0;i<nrOfServers;i++) {
			servers[i] = strdup(config_setting_get_string_elem(cfgarray,i));
		}


		//load server array from config    
	    	if ( (cfgarray = config_lookup(&cfg, "piservers") ) == NULL) {
			printf("can't load \"piservers\" from config\n");
			exit(1);
	  	}

		nrOfPiServers = config_setting_length(cfgarray);

		piservers = malloc(sizeof(char *) * nrOfPiServers);

		for(i=0;i<nrOfPiServers;i++) {
			piservers[i] = strdup(config_setting_get_string_elem(cfgarray,i));
		}



		//load addserver array from config    
	    	if ( (cfgarray = config_lookup(&cfg, "addservers") ) == NULL) {
			printf("can't load \"nrOfAddServers\" from config\n");
			exit(1);
	  	}

		nrOfAddServers = config_setting_length(cfgarray);

		addservers = malloc(sizeof(char *) * nrOfAddServers);

		for(i=0;i<nrOfAddServers;i++) {
			addservers[i] = strdup(config_setting_get_string_elem(cfgarray,i));
		}


		//char *servers[] = { "bbs-002.boitho.com" , "bbs-003.boitho.com", "bbs-004.boitho.com", "bbs-005.boitho.com", "bbs-006.boitho.com" , "bbs-007.boitho.com" };
		//char *servers[] = { "bbs-002.boitho.com" , "bbs-003.boitho.com", "bbs-004.boitho.com", "bbs-005.boitho.com", "bbs-006.boitho.com" };
		//char *servers[] = { "bbs-002.boitho.com" };

	
		//char *addservers[] = { "bbh-001.boitho.com" };
		//char *addservers[] = { "www2.boitho.com" };

		//int nrOfServers = (sizeof(servers) / sizeof(char *));
		//int nrOfAddServers = (sizeof(addservers) / sizeof(char *));

	#endif



	//he = (struct hostent *) malloc(nrOfServers * sizeof(struct hostent));
	//////////////////


	memset(&QueryData,'\0',sizeof(QueryData));
	memset(&queryNodeHeder,'\0',sizeof(queryNodeHeder));
	QueryData.navmenucfg[0] = '\0';

	cgi_set_defaults(&QueryData);

        //hvis vi har argumeneter er det første et query
        if (getenv("QUERY_STRING") == NULL) {

	        char *optRank = NULL;
		int optStart = 1;
		int optMaxsHits = DefultMaxsHits;
		int anonymous = 0;

        	extern char *optarg;
        	extern int optind, opterr, optopt;
        	char c;
        	while ((c=getopt(argc,argv,"apr:s:m:o:f:"))!=-1) {
        	        switch (c) {
				case 'a':
					anonymous = 1;
					break;
				case 'p':
					prequerywriteFlag = 1;
					dispconfig.writeprequery = 1;
					#ifdef DEBUG
					printf("Forcing prequery write\n");
					#endif
					break;
        	                case 'r':
        	                        optRank = optarg;
                	                printf("will look up rank for \"%s\"\n",optRank);
                	                break;
        	                case 's':
        	                        optStart = atoi(optarg);
                	                printf("will start at page %i\n",optStart);
                	                break;
        	                case 'o':
        	                        searchport = atoi(optarg);
                	                printf("will use prot %i\n",searchport);
                	                break;
        	                case 'm':
        	                        optMaxsHits = atoi(optarg);
                	                printf("will show max %i pages\n",optStart);
                	                break;
        	                case 'f':
        	                        if (strcmp(optarg,"opensearch") == 0) {
						QueryData.outformat = _OUT_FOMRAT_OPENSEARCH;
					}
					else {
						printf("Unknown out format %s\n",optRank);
						exit(-1);
					}
                	                printf("will output data in \"%s\" format\n",optRank);
                	                break;

                        	default:
					printf("ukjent option\n");
                                	exit(1);
                	}

        	}
        	--optind;

		#ifdef DEBUG
        	printf("argc %i, optind %i\n",argc,optind);
		#endif



                if (argc < 3 ) {
                        printf("Error ingen query spesifisert eller subname .\n\nEksempel på bruk for å søke på boitho:\n");
			#ifdef BLACK_BOKS
				printf("\tdispatcher_all boitho www bruker\n\n\n");
			#else
				printf("\tdispatcher_all boitho www\n\n\n");
			#endif
                }
                else {
			strcpy(QueryData.userip,"213.179.58.99");

                        strcpy(QueryData.query,argv[1 +optind]);
			strcpy(QueryData.subname,argv[2 +optind]);
			#ifdef BLACK_BOKS
				strcpy(QueryData.search_user,argv[3 +optind]);

			#else
				QueryData.search_user[0] = '\0';
			#endif

			if (optRank == NULL) {
				getRank = 0;
				QueryData.rankUrl[0] = '\0';
			}
			else {
				getRank = 1;
				strscpy(QueryData.rankUrl,optRank,sizeof(QueryData.rankUrl));
				printf("will rank \"%s\"\n",QueryData.rankUrl);
			}

			QueryData.MaxsHits = optMaxsHits;
			QueryData.start = optStart;
			QueryData.filterOn = 1;
			QueryData.anonymous = anonymous;
			QueryData.HTTP_ACCEPT_LANGUAGE[0] = '\0';
        		QueryData.HTTP_USER_AGENT[0] = '\0';
        		QueryData.HTTP_REFERER[0] = '\0';
			QueryData.AmazonAssociateTag[0] = '\0';
			QueryData.AmazonSubscriptionId[0] = '\0';
			//v3 QueryData.languageFilter[0] = '\0';
			QueryData.orderby[0] = '\0';
			QueryData.version = 2.1;

			if (!dispconfig.writeprequery) {
                        	printf("query %s, subname %s\n",QueryData.query,QueryData.subname);

				//printer ut oversikt over serverne vi skal koble til
				printf("server(s):\n");
				for(i=0;i<nrOfServers;i++) {
					printf("%i %s\n",i,servers[i]);
				}

				printf("piserver(s):\n");
				for(i=0;i<nrOfPiServers;i++) {
					printf("%i %s\n",i,piservers[i]);
				}

				printf("adserver(s):\n");
				for(i=0;i<nrOfAddServers;i++) {
					printf("%i %s\n",i,addservers[i]);
				}


				printf("\n");
			}
                }
        }
        else {
        	//send out an HTTP header:
		#ifdef DEBUG
	        	printf("Content-type: text/plain\n\n");
		#else
	        	printf("Content-type: text/xml\n\n");
		#endif
		
		char *remoteaddr = getenv("REMOTE_ADDR");
		if (remoteaddr == NULL)
			errx(1, "env variable REMOTE_ADDR missing");

		dispatcher_cgi_init();

		char correct_key[512];
		key_get_existingconn(&demo_db, correct_key);
		
		int access = cgi_access_type(remoteaddr, correct_key);
		if (access == ACCESS_TYPE_NONE) 
			die(1,"", "Access key missing, or wrong access key.");
		
		cgi_fetch_common(&QueryData, &noDoctype);
		if (access == ACCESS_TYPE_LIMITED)
			cgi_fetch_limited(&QueryData, remoteaddr);
		else if (access == ACCESS_TYPE_FULL) 
			cgi_fetch_full(&QueryData);
		else errx(1, "invalid access type");


		getRank = (QueryData.rankUrl[0] == '\0') ? 0 : 1;
        }
	if (QueryData.start > MAX_RESULT_OFFSET) {
		warnx("'start' larger than MAX_RESULT_OFFSET. Setting to %d.", MAX_RESULT_OFFSET);
		QueryData.start = MAX_RESULT_OFFSET;
	}


	struct subnamesConfigFormat default_cfg;
	read_collection_cfg(&default_cfg);
	default_cfg.has_config = 0;

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		fprintf(stderr,"Time debug: init %f\n",getTimeDifference(&start_time,&end_time));
	#endif
		
#if BLACK_BOKS

	if (cmc_port == 0) {
		cmc_port = maincfg_get_int(&maincfg,"CMDPORT");
	}

	#ifdef DEBUG_TIME
        gettimeofday(&start_time, NULL);
	#endif

	int num_colls = 0;
	struct subnamesFormat *collections = NULL;
	char **wantcolls = NULL;
	int n_wantcolls = 0;
	if (QueryData.subname[0] != '\0') {
		n_wantcolls = split(QueryData.subname, ",", &wantcolls);
	}
	/* Use local function so we get the lexical scope we want */
	int want_collection(char *coll) {
		int i;

		if (n_wantcolls == 0) {
			//fprintf(stderr, "Go ahead...\n");
			return 1;
		}
		
		for (i = 0; i < n_wantcolls; i++) {
			//fprintf(stderr, "Checking %s <-> %s\n", coll, wantcolls[i]);
			if (strcmp(coll, wantcolls[i]) == 0) {
				fprintf(stderr, "Wants it!");
				return 1;
			}
		}

		//fprintf(stderr, "No way, don't want that!\n");
		return 0;
	}
	if (QueryData.anonymous) {
		MYSQL_RES *res;
		MYSQL_ROW row;
		char query[1024];
		size_t querylen;

		querylen = snprintf(query, sizeof(query), "SELECT collection_name FROM shares WHERE without_aclcheck = 1");

		if (mysql_real_query(&demo_db, query, querylen) != 0) {
			warnx("Mysql error (%s line %d): %s\n",
					__FILE__, __LINE__, mysql_error(&demo_db));
		} else if ((res = mysql_store_result(&demo_db)) == NULL) {
			warnx("Mysql error (%s line %d): %s\n",
					__FILE__, __LINE__, mysql_error(&demo_db));
		} else {
			num_colls = mysql_num_rows(res);
			if (num_colls != 0) {
				int i = 0;
				
				collections = calloc(num_colls, sizeof(*collections));
				while ((row = mysql_fetch_row(res)) != NULL) {
					//fprintf(stderr, "Yay: %s\n", row[0]);
					if (want_collection(row[0])) {
						strlcpy(collections[i].subname, row[0], sizeof(collections[i].subname));
						i++;
					}
				}
				/* Set num_colls to the actual amount of collection we got */
				num_colls = i;
				mysql_free_result(res);
			}
		}
	} else {
		collections = get_usr_coll(QueryData.search_user, &num_colls, cmc_port);

		struct subnamesFormat *filtered_collections = calloc(num_colls, sizeof(*filtered_collections));
		int i, j;

		j = 0; /* New num_colls */
		for (i = 0; i < num_colls; i++) {
			if (want_collection(collections[i].subname)) {
				strlcpy(filtered_collections[j].subname, collections[i].subname,
				        sizeof(filtered_collections[j].subname));
				j++;
			}
		}
		free(collections);
		collections = filtered_collections;
		num_colls = j;
	}


	#ifdef DEBUG_TIME
        	gettimeofday(&end_time, NULL);
        	fprintf(stderr,"Time debug: get_usr_coll() %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG_TIME
        gettimeofday(&start_time, NULL);
	#endif

	for (i = 0; i < num_colls; i++) {
		if (!fetch_coll_cfg(&demo_db, collections[i].subname, &collections[i].config)) {
			collections[i].config = default_cfg;
		}
		collections[i].hits = -1;
	}

	#ifdef DEBUG_TIME
        	gettimeofday(&end_time, NULL);
        	fprintf(stderr,"Time debug: fetch_coll_cfg() %f\n",getTimeDifference(&start_time,&end_time));
	#endif

#else
	int num_colls = 1;
	struct subnamesFormat *collections = malloc(sizeof(struct subnamesFormat));
	snprintf(collections[0].subname, sizeof collections[0].subname, QueryData.subname);
	collections[0].config = default_cfg;
	collections[0].hits = - 1;
#endif

	#ifdef DEBUG
		warnx("Numer of colls: %d\n", num_colls);
	#endif


	#ifdef DEBUG_TIME
		gettimeofday(&start_time, NULL);
	#endif

	if (searchport == 0) {
		searchport  = maincfg_get_int(&maincfg,"BSDPORT");
	}


        if (strlen(QueryData.query) > MaxQueryLen -1) {
                die(3,QueryData.query,"Query too long.");
        }

	for(i=0;i<dispconfig.bannedwordsnr;i++) {
		if (strstr(QueryData.query,dispconfig.bannedwords[i]) != NULL) {
			die(3,QueryData.query,"Sorry, the word '%s' is banned.",dispconfig.bannedwords[i]);
		}
	}

	#ifdef BLACK_BOKS

	#else
		//hvis vi har : i seg må vi avslutte da vi ikke støtter dette
		if(strchr(QueryData.query,':') != NULL) {
			die(4,QueryData.query,"Command not yet implemented.");
		}
	#endif
	//hvis vi har et for kort query
	if(strlen(QueryData.query) < 2) {
		die(5,QueryData.query,"Query too short.");
	}

        //gjør om til liten case
	#ifdef BLACK_BOKS
		//21 feb 2007: collection er case sensetiv. Bare søkeord skal gjøres om. Må gjøre dette en annen plass
	#else
		//må gjøres for web da både prequery og cashe er lagret på disk som lovercase. Hvis ikke vil ikke søk på Msn treffe msn
        	//for(i=0;i<strlen(QueryData.query);i++) {
                //	QueryData.query[i] = btolower(QueryData.query[i]);
        	//}
		convert_to_lowercase_n(QueryData.query,sizeof(QueryData.query));
	#endif

	//nårmalisere query. 
	//strcasesandr(QueryData.query,sizeof(QueryData.query),"."," ");

	
	#ifndef BLACK_BOKS

	if (getRank) {

		//normaliserer url. Setter for eks / på slutten
		url_normalization(QueryData.rankUrl,sizeof(QueryData.rankUrl));



        	char            db_index[strlen(dispconfig.UrlToDocID)+7];
        	sprintf(db_index, "%s.index", dispconfig.UrlToDocID);
	        urldocid_data   *data;

		if ((data = urldocid_search_init(db_index, dispconfig.UrlToDocID)) == NULL) {
			die(100, QueryData.query,"Unable to open index file \"%s\".",dispconfig.UrlToDocID);
		}


                if (getDocIDFromUrl(data, QueryData.rankUrl, &wantedDocId)) {
			getRank = wantedDocId;
			queryNodeHeder.getRank = wantedDocId;
		
			#ifdef DEBUG
				printf("getRank: found DocID %u ( for url \"%s\" )\n",wantedDocId,QueryData.rankUrl);
			#endif	
		}
		else {

    	        	getRank = getDocIDFromSql(&demo_db, QueryData.rankUrl);
                     	queryNodeHeder.getRank = getRank;

			if (getRank == 0) {
				die(100,QueryData.query, "Ranking information is not yet available for this URL. It takes 24 hours for new site submissions to be crawled, indexed and ranked. Check back later.");
			}

		}

			

		fprintf(stderr,"getRank: queryNodeHeder.getRank %u\n",queryNodeHeder.getRank);


	}
	else {
		queryNodeHeder.getRank = 0;
	}
	#else
		queryNodeHeder.getRank = 0;
	#endif

	for(i=0;i<strlen(QueryData.query);i++) {

		// 92: \, 32: space, 34: ", 43: +

		if (QueryData.query[i] == ';') {
			//sprintf(errormessage,"Illegal character in query. \"%c\" ascii value: %u",QueryData.query[i],(unsigned int)QueryData.query[i]);
			die(15,QueryData.query,"Illegal character in query. \"%c\" ascii value: %u",QueryData.query[i],(unsigned int)QueryData.query[i]);
		}
		/*
		if(
		(isalnum(QueryData.query[i])) 
                || (43 == (unsigned int)QueryData.query[i])
                || (34 == (unsigned int)QueryData.query[i])
                || (32 == (unsigned int)QueryData.query[i])
		|| (45 == (unsigned int)QueryData.query[i])
		|| (58 == (unsigned int)QueryData.query[i])
		|| (64 == (unsigned int)QueryData.query[i]) // @
		|| (46 == (unsigned int)QueryData.query[i]) // .
		|| (128 < (unsigned int)QueryData.query[i])

		) {
			//gjø ingenting for nå
		}
		else {
			sprintf(errormessage,"Illegal character in query. \"%c\" ascii value: %u",QueryData.query[i],(unsigned int)QueryData.query[i]);
			die(15,QueryData.query,errormessage);

		}
		*/

	}

        //fjerner tegn. " blir til &quot;

	//strcpy(QueryData.queryhtml,QueryData.query);
	escapeHTML(QueryData.queryhtml, sizeof QueryData.queryhtml, QueryData.query);


	//printf("query behandlet %s\n",QueryData.queryhtml);
	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		fprintf(stderr,"Time debug: query normalizeing %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG_TIME
		gettimeofday(&start_time, NULL);
	#endif

	#ifndef BLACK_BOKS
	if (!dispconfig.writeprequery) {
	//prøver å finne ut hvilket land ut fra ip
	GeoIP *gi;
	GeoIPRecord * gir;

        //gi = GeoIP_open(bfile("data/GeoLiteCity.dat"), GEOIP_MEMORY_CACHE);
        gi = GeoIP_open(bfile("data/GeoLiteCity.dat"), GEOIP_STANDARD);
        if (gi == NULL) {
                fprintf(stderr, "Error opening ip database\n");
                //exit(1);
        }
	else {
        	//iparesse
        	if ((gir = GeoIP_record_by_name (gi, (const char *)&QueryData.userip)) == NULL) {
			strscpy(QueryData.GeoIPcontry,"na",sizeof(QueryData.GeoIPcontry));
        	        fprintf(stderr, "Error looking up ip for \"%s\". Wil use \"%s\" as country code\n",QueryData.userip,QueryData.GeoIPcontry);

        	}
		else {
			strcpy(QueryData.GeoIPcontry, gir->country_code);

        		dprintf("GeoIP: %s\t%s\t%s\t%s\t%s\t%f\t%f\t%d\t%d\n", queryNodeHeder.userip,
                                         gir->country_code,
                                         gir->region,
                                         gir->city,
                                         gir->postal_code,
                                         gir->latitude,
                                         gir->longitude,
                                         gir->dma_code,
                                         gir->area_code);
			dprintf("GeoIPcontry: %s\n",QueryData.GeoIPcontry);
        		GeoIPRecord_delete(gir);
		}

		GeoIP_delete(gi);
	}
	} //if(!dispconfig.writeprequery)
	#else
		sprintf(QueryData.GeoIPcontry,"na");
	#endif

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		fprintf(stderr,"Time debug: geoip %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG_TIME
		gettimeofday(&start_time, NULL);
	#endif

	//kopierer inn query	
	strscpy(queryNodeHeder.query,QueryData.query,sizeof(queryNodeHeder.query) -1);
	strscpy(queryNodeHeder.subname,QueryData.subname,sizeof(queryNodeHeder.subname) -1);
	strscpy(queryNodeHeder.userip,QueryData.userip,sizeof(queryNodeHeder.userip) -1);

	//hvorfår får vi broblemer når vi bruker -02 og -1 her ? Ser ut til at strcpy ikke terminerer hvis 
	//den når max lengde.
	//strscpy(queryNodeHeder.GeoIPcontry,QueryData.GeoIPcontry,sizeof(queryNodeHeder.GeoIPcontry) -1);
	strcpy(queryNodeHeder.GeoIPcontry,QueryData.GeoIPcontry);

	strscpy(queryNodeHeder.search_user,QueryData.search_user,sizeof(queryNodeHeder.search_user) -1);

	strscpy(queryNodeHeder.HTTP_ACCEPT_LANGUAGE,QueryData.HTTP_ACCEPT_LANGUAGE,sizeof(queryNodeHeder.HTTP_ACCEPT_LANGUAGE));
	strscpy(queryNodeHeder.HTTP_USER_AGENT,QueryData.HTTP_USER_AGENT,sizeof(queryNodeHeder.HTTP_USER_AGENT));
	strscpy(queryNodeHeder.HTTP_REFERER,QueryData.HTTP_REFERER,sizeof(queryNodeHeder.HTTP_REFERER));
	//v13 strscpy(queryNodeHeder.languageFilter,QueryData.languageFilter,sizeof(queryNodeHeder.languageFilter) -1);
	strscpy(queryNodeHeder.orderby,QueryData.orderby,sizeof(queryNodeHeder.orderby) -1);


	strscpy(queryNodeHeder.AmazonAssociateTag,QueryData.AmazonAssociateTag,sizeof(queryNodeHeder.AmazonAssociateTag) -1);
	strscpy(queryNodeHeder.AmazonSubscriptionId,QueryData.AmazonSubscriptionId,sizeof(queryNodeHeder.AmazonSubscriptionId) -1);

	strscpy(queryNodeHeder.navmenucfg, QueryData.navmenucfg, sizeof queryNodeHeder.navmenucfg);
	
	queryNodeHeder.lang = QueryData.lang;


	//--QueryData.start; //maskinen begynner på 1, meneske på 0
	//queryNodeHeder.start = QueryData.start;
	queryNodeHeder.start = 0;

	//queryNodeHeder.MaxsHits = QueryData.MaxsHits;
	queryNodeHeder.MaxsHits = QueryData.MaxsHits * QueryData.start;

	//på første side kan vi la være og hente alle treff siden dataene er fordelt, men for de neste 
	//sidene trenger vi de. 
	if (QueryData.start == 1) {
		if (nrOfServers >= 3) {
			queryNodeHeder.MaxsHits = (queryNodeHeder.MaxsHits / 2); // datane er fordelt, så hver server trenger ikke å generere mer en xx deler av den
		}
	}
	queryNodeHeder.filterOn = QueryData.filterOn;
	queryNodeHeder.anonymous = QueryData.anonymous;




	//Sider = malloc(QueryData.MaxsHits * maxServers * sizeof(struct SiderFormat));
	maxSider = queryNodeHeder.MaxsHits * maxServers * sizeof(struct SiderFormat);

	Sider = malloc(maxSider);

	//inaliserer side arrayen
	for(i=0;i<(nrOfServers + nrOfPiServers) * queryNodeHeder.MaxsHits;i++) {
        	Sider[i].iindex.allrank = 0;
        	Sider[i].deletet = 1;
	}

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		fprintf(stderr,"Time debug: query copying %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	hasprequery = 0;
	hascashe = 0;
	pageNr = 0;

	#ifdef WITH_CASHE

	char cachepath[1024];

	cache_path(cachepath, sizeof(cachepath), CACHE_SEARCH, QueryData.queryhtml, QueryData.start, QueryData.GeoIPcontry);
	cache_path(prequeryfile, sizeof(cachepath), CACHE_PREQUERY, QueryData.queryhtml, QueryData.start, QueryData.GeoIPcontry);


	if (!prequerywriteFlag && getRank == 0 && (dispconfig.useprequery) && (QueryData.filterOn) &&
	    cache_read(prequeryfile, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, 0, maxSider)) {
		hasprequery = 1;

		debug("can open prequeryfile file \"%s\"",prequeryfile);

	}
	else if (!prequerywriteFlag && getRank == 0 && (dispconfig.usecashe) && (QueryData.filterOn) &&
	         cache_read(cachepath, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, cachetimeout, maxSider)) {
		hascashe = 1;

		debug("can open cashe file \"%s\"",cachepath);
	}

	#ifdef DEBUG
		printf("cache and prequery info:\n");
		printf("\tcachepath: \"%s\"\n",cachepath);
		printf("\tprequeryfile: \"%s\"\n",prequeryfile);

		printf("\tuseprequery: %i\n",dispconfig.useprequery);
		printf("\tuseprequery: %i\n",dispconfig.useprequery);
		printf("\tfilterOn: %i\n",QueryData.filterOn);

		printf("\thasprequery: %i\n",hasprequery);
		printf("\thascashe: %i\n",hascashe);
	#endif

	#else
	#endif


	#ifdef DEBUG_TIME
		gettimeofday(&start_time, NULL);
	#endif

	//Paid inclusion
	//bsConectAndQuery(sockfd,nrOfPiServers,piservers,&queryNodeHeder,0,searchport);
	bsConnectAndQuery(sockfd, nrOfPiServers, piservers,
		&queryNodeHeder, collections, num_colls, 0, searchport);

	//kobler til vanlige servere
	if (!(hascashe || hasprequery)) {
		bsConnectAndQuery(sockfd, nrOfServers, servers,
			&queryNodeHeder, collections, num_colls, nrOfPiServers, searchport);
	}


	//addservere
	//bsConectAndQuery(addsockfd,nrOfAddServers,addservers,&queryNodeHeder,0,searchport);
	bsConnectAndQuery(addsockfd, nrOfAddServers, addservers,
		&queryNodeHeder, collections, num_colls, 0, searchport);

	//Paid inclusion
	//bsConectAndQuery(sockfd,nrOfPiServers,piservers,&queryNodeHeder,nrOfServers,searchport);
	
	#ifndef BLACK_BOKS

		if (getRank) {
			int endranking = 0;
			int ranking = -1;
			int net_status;
			int n;

			/* XXX: Need to handle the paid inclusion servers as well? */
			for (i = 0; i < nrOfServers + nrOfPiServers; i++) {
				if (sockfd[i] != 0) {
					int status;
					//motter hedderen for svaret
					if (bsread (&sockfd[i],sizeof(net_status), (char *)&net_status, maxSocketWait_CanDo)) {
						if (net_status != net_CanDo) {
							fprintf(stderr, "net_status wasn't net_CanDo but %i\n",net_status);
							sockfd[i] = 0;
							continue;
						}
					} else {
						perror("initial protocol read failed");
						sockfd[i] = 0;
						continue;
					}
 
					if (!bsread(&sockfd[i],sizeof(status), (char *)&status, 1000)) //maxSocketWait_CanDo))
						die(2,QueryData.query, "Unable to get rank status. Server %i is not responding.",i);
					else if (status == net_match) {
						if (!bsread(&sockfd[i],sizeof(ranking), (char *)&ranking, 1000))//maxSocketWait_CanDo))
							perror("recv rank");
					} else if (status == net_nomatch) {
						//return 1;
					} else {
						//die(1,QueryData.query, "searchd does not support ranking?");
					}

				
					dprintf("getRank: server %i, ranking %i\n",i,ranking);
				}
			}
			if (ranking != -1) {
				for (i = 0; i < nrOfServers + nrOfPiServers; i++) {
					if (sockfd[i] != 0) {
						if (send(sockfd[i], &ranking, sizeof(ranking), 0) != sizeof(ranking))
							perror("send...");
					}
				}

				for (i = 0; i < nrOfServers + nrOfPiServers; i++) {
					if (sockfd[i] != 0) {
						if (!bsread(&sockfd[i], sizeof(ranking), (char *)&ranking, 10000))
							perror("endranking");
						endranking += ranking; 
					}
				}
			}
/*
			else {
				die(1,QueryData.query, "No rank found");
			}
*/
			dprintf("getRank: endranking %i, queryNodeHeder.MaxsHits %i\n",endranking,queryNodeHeder.MaxsHits);

			if ((endranking < queryNodeHeder.MaxsHits) || (endranking == 0)) {
				queryNodeHeder.getRank = 0;

				//enten skal vi være i top, og dermed være med, hvis ikke skal vi ikke være telt med. Uanset er tidligere verdi potensielt feil.
				endranking = 0;

				for (i=0;i<nrOfServers + nrOfPiServers;i++) {
					if (sockfd[i] != 0) {
						bsQuery(&sockfd[i], &queryNodeHeder);
					}
				}

				for (i=0;i<nrOfServers + nrOfPiServers;i++) {
					if (sockfd[i] != 0) {
						close(sockfd[i]);
					}
				}
//
//				brGetPages(sockfd,nrOfPiServers,SiderHeder,Sider,&pageNr,nrOfServers);
//				if ((!hascashe) && (!hasprequery)) {
//					brGetPages(sockfd,nrOfServers,SiderHeder,Sider,&pageNr,0);
//				}
//


				//kobler til vanlige servere
				if ((!hascashe) && (!hasprequery)) {
					//bsConectAndQuery(sockfd,nrOfServers,servers,&queryNodeHeder,0,searchport);
					bsConnectAndQuery(sockfd, nrOfServers, servers,
						&queryNodeHeder, collections, num_colls, 0, searchport);
				}

				//addservere
				//bsConectAndQuery(addsockfd,nrOfAddServers,addservers,&queryNodeHeder,0,searchport);
				bsConnectAndQuery(addsockfd, nrOfAddServers, addservers,
						&queryNodeHeder, collections, num_colls, 0, searchport);

				//Paid inclusion
				//bsConectAndQuery(sockfd,nrOfPiServers,piservers,&queryNodeHeder,nrOfServers,searchport);
				bsConnectAndQuery(sockfd, nrOfPiServers, piservers,
					&queryNodeHeder, collections, num_colls, nrOfServers, searchport);

				//Paid inclusion
				brGetPages(sockfd,nrOfPiServers,SiderHeder,Sider,&pageNr,0);

				if ((!hascashe) && (!hasprequery)) {
					brGetPages(sockfd,nrOfServers,SiderHeder,Sider,&pageNr,nrOfPiServers);
				}

				//addservere
				//addserver bruker som regel mest tid, så tar den sist slik at vi ikke trenger å vente unødvendig
				brGetPages(addsockfd,nrOfAddServers,AddSiderHeder,Sider,&pageNr,0);

				handle_results(sockfd, Sider, SiderHeder, &QueryData, &FinalSiderHeder, (hascashe || hasprequery), &errorha, pageNr,
					       nrOfServers, nrOfPiServers, &dispatcherfiltersTraped, &nrRespondedServers, &queryNodeHeder, &demo_db);

				//for((x<FinalSiderHeder.showabal) && (i < (QueryData.MaxsHits * (nrOfServers + nrOfPiServers)))) {


				x = 0;
				int printed = 0;
				for(i=0;x < FinalSiderHeder.showabal && i < (queryNodeHeder.MaxsHits * (nrOfServers + nrOfPiServers)); i++) {
					if (Sider[i].deletet)
						continue;
					if ((Sider[i].iindex.DocID == wantedDocId) || (strcmp(Sider[i].url,QueryData.rankUrl) == 0)) {
						dprintf("did find pi pages as nr %i, url %s\n",i,Sider[i].url);
						endranking = printed+1;
						break;
					}
					if (Sider[i].type == siderType_normal) {
						x++;
					}
					printed++;
				}
			}


			if (endranking == 0) {
				//printf("\t<noresult />\n");
				//die(1,QueryData.query, "No rank found");
				die(1,QueryData.query, "That site does not rank in the top 60,000 sites for that search term.");
			} else {
				printf("<ranking>\n");
				printf("\t<rank>%d</rank>\n", endranking);
				printf("\t<url>%s</url>\n", QueryData.rankUrl);
				printf("\t<docid>%d</docid>\n", wantedDocId);
				printf("</ranking>\n");

			}

			/* Free the configuration */
			config_destroy(&cfg);
			maincfgclose(&maincfg);

			free(SiderHeder);
			free(AddSiderHeder);
			free(Sider);

			for (i=0;i<nrOfServers + nrOfPiServers;i++) {
				if (sockfd[i] != 0) {
					close(sockfd[i]);
				}
			}

#ifndef BLACK_BOKS
			for(i=0;i<nrOfServers;i++) {
				free(servers[i]);
			}
			for(i=0;i<nrOfPiServers;i++) {
				free(piservers[i]);
			}
			for(i=0;i<nrOfAddServers;i++) {
				free(addservers[i]);
			}

			free(servers);
			free(piservers);
			free(addservers);
#endif

			return 0;
		}
	#endif
	free(collections);

	//Paid inclusion
	dprintf("starting to get pi\n");
	brGetPages(sockfd,nrOfPiServers,SiderHeder,Sider,&pageNr,0);
	dprintf("end get pi\n");

	if ((!hascashe) && (!hasprequery)) {
		brGetPages(sockfd,nrOfServers,SiderHeder,Sider,&pageNr,nrOfPiServers);
	}

	//addservere
	//addserver bruker som regel mest tid, så tar den sist slik at vi ikke trenger å vente unødvendig
	brGetPages(addsockfd,nrOfAddServers,AddSiderHeder,Sider,&pageNr,0);

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		fprintf(stderr,"Time debug: geting pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG_TIME
        	gettimeofday(&start_time, NULL);
	#endif
	
	handle_results(sockfd, Sider, SiderHeder, &QueryData, &FinalSiderHeder, (hascashe || hasprequery), &errorha, pageNr,
	               nrOfServers, nrOfPiServers, &dispatcherfiltersTraped, &nrRespondedServers,&queryNodeHeder, &demo_db);

	#ifdef DEBUG_TIME
        	gettimeofday(&end_time, NULL);
        	fprintf(stderr,"Time debug: handle_results %f\n",getTimeDifference(&start_time,&end_time));
	#endif



	#ifdef BLACK_BOKS

	#else 
		//Sier ikke noe om filtrerte treff hvis vi hadde mange nokk
		if (FinalSiderHeder.TotaltTreff > 100) {

			dprintf("have more then 100 filtered results. Has %i\n",FinalSiderHeder.TotaltTreff);

			FinalSiderHeder.filtered = 0;;
		}
	#endif

        #ifdef DEBUG_TIME
                gettimeofday(&start_time, NULL);
        #endif

	//stopper å ta tidn og kalkulerer hvor lang tid vi brukte
	gettimeofday(&search_end_time, NULL);
	FinalSiderHeder.total_usecs = getTimeDifference(&search_start_time,&search_end_time);

	#ifdef DEBUG_TIME
		fprintf(stderr,"Time debug: Total time %f\n",FinalSiderHeder.total_usecs);
	#endif


	totlaAds = 0;

	//skriver ikke ut masse data hvis vi lager prequery
	if (dispconfig.writeprequery) {
		printf("query \"%s\", total %i,showabal %i, nodes %i, time %f\n",QueryData.queryhtml,FinalSiderHeder.TotaltTreff,FinalSiderHeder.showabal,nrRespondedServers,FinalSiderHeder.total_usecs);
	}
	else if (QueryData.outformat == _OUT_FOMRAT_OPENSEARCH) {
		disp_out_opensearch(
			FinalSiderHeder.showabal, 
			Sider, &queryNodeHeder, 
			(nrOfServers + nrOfPiServers), 
			QueryData.start, QueryData.MaxsHits,
			QueryData.queryhtml
		
		);
	}
	else if ((QueryData.outformat == _OUT_FOMRAT_SD) && (QueryData.version == 2.0)) {
		noDoctype = 1; //det var ikke doctype orginalt i xml'en. Og sende den bryter 24so.
    		disp_out_sd_v2_0(FinalSiderHeder, QueryData, noDoctype, SiderHeder, hascashe, hasprequery, nrRespondedServers, (nrOfServers + nrOfPiServers), nrOfAddServers, dispatcherfiltersTraped,
		sockfd, addsockfd, AddSiderHeder, errorha, Sider, queryNodeHeder, etime
		);
	} 
	else if ((QueryData.outformat == _OUT_FOMRAT_SD) && (QueryData.version == 2.1)) { 
    		disp_out_sd_v2_1(FinalSiderHeder, QueryData, noDoctype, SiderHeder, hascashe, hasprequery, nrRespondedServers, (nrOfServers + nrOfPiServers), nrOfAddServers, dispatcherfiltersTraped,
		sockfd, addsockfd, AddSiderHeder, errorha, Sider, queryNodeHeder, etime
		);
	} 
	else {
		warnx("Error - unknown version %f, no output available.\n", QueryData.version);
	}

	#ifdef DEBUG_TIME
	        gettimeofday(&end_time, NULL);
	        fprintf(stderr,"Time debug: xml gen %f\n",getTimeDifference(&start_time,&end_time));
	#endif
	
	#ifdef DEBUG_TIME
		gettimeofday(&start_time, NULL);
	#endif

	//stenger ned stdout da det ikke skal sendes ut mer data, og dertfor er det ikke noen vits at klienten venter mer
	//frøykter apache dreper den den da. Kan ikke gjøre
	//fclose(stdout);

	//30 mai 2007
	//ser ut til å skape problemer når vi har cashed verdier her
	if ((!hascashe) && (!hasprequery)) {
	//kalkulerer dette på ny, men uten pi servere
	nrRespondedServers = 0;
        for (i=0;i<nrOfServers;i++) {

                if (sockfd[i] != 0) {
                        ++nrRespondedServers;
                }
        }

	//stenger ned tilkoblingen til nodene
	for (i=0;i<nrOfServers + nrOfPiServers;i++) {
		if (sockfd[i] != 0) {
        		close(sockfd[i]);
		}
	}
	}

	#ifndef BLACK_BOKS
		if ((LOGFILE = fopen(QUERY_LOG_FILE,"a")) == NULL) {
			perror(QUERY_LOG_FILE);
		}
		else {
			flock(fileno(LOGFILE),LOCK_EX);
	        	fprintf(LOGFILE,"%s %i %f nodes: %i\n",queryNodeHeder.query,FinalSiderHeder.TotaltTreff,FinalSiderHeder.total_usecs,nrRespondedServers);
	        	fclose(LOGFILE);
		}
	#endif

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		fprintf(stderr,"Time debug: end clean up %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	#ifdef WITH_CASHE
	//FILE *CACHE;
	if (hascashe) {
		//touch
		//CACHE = open(cashefile,"r+b");
		//fputc('1',CACHE); //fremprovoserer en oppdatering av akksestiden		
		//fclose(CACHE);
	}
	else if(hasprequery) {
		//har prequery
	}
	else if (!QueryData.filterOn) {

	}
	//skriver bare cashe hvis vi fikk svar fra all servere, og vi var ikke ute etter ranking
	else if (getRank == 0 && pageNr > 0 && nrRespondedServers == nrOfServers) {
		if (dispconfig.writeprequery) {
			if (!cache_write(prequeryfile, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, pageNr)) {
				//fprintf(stderr, "Prequery file: %s\n",prequeryfile);
				perror("cache_write");
			}
		}
		else if (!cache_write(cachepath, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, pageNr)) {
			//fprintf(stderr, "Cache file: %s\n", cachepath);
			perror("cache_write");
		}
	}
	#endif

	#ifdef DEBUG_TIME
        	gettimeofday(&start_time, NULL);
	#endif

	/********************************************************************************************/
	//mysql logging
	/********************************************************************************************/
	if (!dispconfig.writeprequery) {
		mysql_search_logg(&demo_db, &QueryData, &FinalSiderHeder, totlaAds, &queryNodeHeder, nrOfServers, Sider, nrOfPiServers);
	}
	/********************************************************************************************/

	mysql_close(&demo_db);

	#ifdef DEBUG_TIME
	        gettimeofday(&end_time, NULL);
	        fprintf(stderr,"Time debug: sql loging %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG_TIME
	        gettimeofday(&start_time, NULL);
	#endif

	free(Sider);

  	/* Free the configuration */
	config_destroy(&cfg);
	maincfgclose(&maincfg);


	#ifdef ATTRIBUTES
	free(SiderHeder[0].navigation_xml);
	#endif
	free(SiderHeder);
	free(AddSiderHeder);
	//hvis vi har web system kan vi ha flere servere, og de er da som en char **
	#ifndef BLACK_BOKS
		for(i=0;i<nrOfServers;i++) {
			free(servers[i]);
		}
		for(i=0;i<nrOfPiServers;i++) {
			free(piservers[i]);
		}
		for(i=0;i<nrOfAddServers;i++) {
			free(addservers[i]);
		}


		free(servers);
		free(piservers);
		free(addservers);
	#endif

	//må vi tvinge en buffer tømming ???
	if (!dispconfig.writeprequery) {
		printf("\n\n");	
	}

	#ifdef DEBUG_TIME
        	gettimeofday(&end_time, NULL);
        	fprintf(stderr,"Time debug: freeing at the end %f\n",getTimeDifference(&start_time,&end_time));


		// skriver total tiden
        	gettimeofday(&total_end_time, NULL);
        	fprintf(stderr,"Time debug: Total time %f\n",getTimeDifference(&total_start_time,&total_end_time));
	#endif

        return EXIT_SUCCESS;
} 


int compare_elements (const void *_p1, const void *_p2) {
	const struct SiderFormat *p1, *p2;

	p1 = _p1;
	p2 = _p2;

	if (p1->type != p2->type) {
                //sortering på type, slik at ppc kommer sammen, og først
                //printf("type %i != %i\n",p1->type,p2->type);
                if (p1->type > p2->type)
                        return -1;
                else
                        return p1->type < p2->type;
        }
        else if (p1->iindex.allrank > p2->iindex.allrank)
                return -1;
        else
                return p1->iindex.allrank < p2->iindex.allrank;

}
int compare_elements_posisjon (const void *_p1, const void *_p2) {
	const struct SiderFormat *p1, *p2;

	p1 = _p1;
	p2 = _p2;

	if (p1->type != p2->type) {
		//sortering på type, slik at ppc kommer sammen, og først. Må så sorteres etter pris og relevans
		//printf("type %i != %i\n",p1->type,p2->type);
		if (p1->type > p2->type)
			return -1;
		else
			return p1->type < p2->type;
	}
	//hvis vi har en normal side, og har forskjelig path lengde
	else if (p1->type == siderType_normal) {
		if (p1->posisjon == p2->posisjon ){

			/*
			//printf("a: %s (%i) - %s (%i)\n",p1->DocumentIndex.Url,p1->pathlen,p2->DocumentIndex.Url,p2->pathlen);
			if (p1->pathlen == p2->pathlen) {
				if (p1->iindex.allrank > p2->iindex.allrank) {
                        		return -1;
                		}
                		else {
                	        	return p1->iindex.allrank < p2->iindex.allrank;
        	        	}			
	
			}
			else if (p1->pathlen < 
				p2->pathlen ) {
				return -1;
			}
			else {
				return 1;
			}	
			*/	
			if (p1->iindex.allrank > p2->iindex.allrank) {
                        	return -1;
                	}
                	else {
                        	return p1->iindex.allrank < p2->iindex.allrank;
                	}			

		}
        	else if (p1->posisjon < p2->posisjon)
        	        return -1;
        	else {
        	        return p1->posisjon > p2->posisjon;
		}
	}
	else {
	//for ppc og ppc_side sider. Sorterer først på bud, så pr relevans
		if (p1->bid == p2->bid) {
			
			if (p1->iindex.allrank > p2->iindex.allrank) {
                        	return -1;
                	}
                	else {
                        	return p1->iindex.allrank < p2->iindex.allrank;
                	}			
		}
		else if (p1->bid > p2->bid) {
                        return -1;
		}
                else {
                        return p1->bid < p2->bid;
                }
	}
}

void read_collection_cfg(struct subnamesConfigFormat * dst) {
	
	struct config_t cfg;

  	config_init(&cfg);
  	if (!config_read_file(&cfg, bfile(CFG_SEARCHD))) {
		die(1, "", "Failed to load config '%s', '%s' line %d.\n",
			bfile(CFG_SEARCHD),
			config_error_text(&cfg),
			config_error_line(&cfg));
	}

	struct subnamesConfigFormat subnamesDefaultsConfig;
	subnamesDefaultsConfig.cache_link = 1;
	subnamesDefaultsConfig.without_aclcheck = 0;

	config_setting_t *cfgstring;
	config_setting_t *cfgcollections;
	config_setting_t *cfgcollection;
	if ((cfgcollections = config_lookup(&cfg, "collections")) == NULL) {
		
		fprintf(stderr, "searchd_child: Error! Can't load \"collections\" from config\n");
		exit(1);
	}

	if ((cfgcollection = config_setting_get_member(cfgcollections, "defaults")) == NULL ) {
		fprintf(stderr, "searchd_child: Error! Can't load \"collections defaults\" from config\n");
		exit(1);

	}


	/****************/
	if ( (cfgstring = config_setting_get_member(cfgcollection, "summary") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"summary\" from config\n");
                exit(1);
        }
	
	const char *summarystr = config_setting_get_string(cfgstring);
	if (strcmp(summarystr, "start") == 0) {
		subnamesDefaultsConfig.summary = SUMMARY_START;
	}
	else if (strcmp(summarystr, "snippet") == 0) {
		subnamesDefaultsConfig.summary = SUMMARY_SNIPPET;
	}
	else {
		die(1, "", "Invalid summary in config: %s\n", summarystr);
	}
	free((void *) summarystr);

	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameUrl") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"filterSameUrl\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameUrl = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameUrl") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"filterSameUrl\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameUrl = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameDomain") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"filterSameDomain\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameDomain = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterTLDs") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"filterTLDs\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterTLDs = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterResponse") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"filterResponse\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterResponse = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameCrc32") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"filterSameCrc32\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameCrc32 = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankAthorArray") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"rankAthorArray\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankAthorArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankAthorArrayLen) {
		subnamesDefaultsConfig.rankAthorArrayLen = BMAX_RANKARRAY;
	}
	int i;
	for(i=0;i<subnamesDefaultsConfig.rankAthorArrayLen;i++) {
		subnamesDefaultsConfig.rankAthorArray[i] = config_setting_get_int_elem(cfgstring,i);
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankTittelArray") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"rankTittelArray\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankTittelArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankTittelArrayLen) {
		subnamesDefaultsConfig.rankTittelArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankTittelArrayLen;i++) {
		subnamesDefaultsConfig.rankTittelArray[i] = config_setting_get_int_elem(cfgstring,i);
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankHeadlineArray") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"rankHeadlineArray\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankHeadlineArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankHeadlineArrayLen) {
		subnamesDefaultsConfig.rankHeadlineArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankHeadlineArrayLen;i++) {
		subnamesDefaultsConfig.rankHeadlineArray[i] = config_setting_get_int_elem(cfgstring,i);
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankBodyArray") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"rankBodyArray\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankBodyArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankBodyArrayLen) {
		subnamesDefaultsConfig.rankBodyArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankBodyArrayLen;i++) {


		subnamesDefaultsConfig.rankBodyArray[i] = config_setting_get_int_elem(cfgstring,i);
	}

	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankUrlArray") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"rankUrlArray\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankUrlArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankUrlArrayLen) {
		subnamesDefaultsConfig.rankUrlArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankUrlArrayLen;i++) {
		subnamesDefaultsConfig.rankUrlArray[i] = config_setting_get_int_elem(cfgstring,i);
	}

	//rankTittelFirstWord
	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankTittelFirstWord") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"rankTittelFirstWord\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.rankTittelFirstWord = config_setting_get_int(cfgstring);

	//rankUrlMainWord
	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankUrlMainWord") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"rankUrlMainWord\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.rankUrlMainWord = config_setting_get_int(cfgstring);

	if ( (cfgstring = config_setting_get_member(cfgcollection, "defaultthumbnail") ) == NULL) {
		#ifdef DEBUG
			warnx("can't load \"defaultthumbnail\" from config");
		#endif
		subnamesDefaultsConfig.defaultthumbnail = NULL;
        }
	else {
		subnamesDefaultsConfig.defaultthumbnail = config_setting_get_string(cfgstring);
	}

	if ( (cfgstring = config_setting_get_member(cfgcollection, "sqlImpressionsLogQuery") ) == NULL) {
		#ifdef DEBUG
                	warnx("can't load \"sqlImpressionsLogQuery\" from config");
		#endif
		subnamesDefaultsConfig.sqlImpressionsLogQuery[0] = '\0';
        }
	else {
		strscpy(subnamesDefaultsConfig.sqlImpressionsLogQuery,config_setting_get_string(cfgstring),sizeof(subnamesDefaultsConfig.sqlImpressionsLogQuery));
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "isPaidInclusion") ) == NULL) {
                fprintf(stderr, "searchd_child: Error! Can't load \"isPaidInclusion\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.isPaidInclusion = config_setting_get_bool(cfgstring);

	(*dst) = subnamesDefaultsConfig;
}


void read_dispatcher_cfg(struct config_t * cfg, struct dispconfigFormat * dispconfig, int *cachetimeout) {
	/* Initialize the configuration */
	config_setting_t *cfgstring;
	config_init(cfg);

	/* Load the file */
	dprintf("loading [%s]..\n", bfile(CFG_DISPATCHER) );

	if (!config_read_file(cfg, bfile(CFG_DISPATCHER) )) {
		printf("[%s]failed: %s at line %i\n",bfile(CFG_DISPATCHER),config_error_text(cfg),config_error_line(cfg));
		exit(1);
	}

	if ((cfgstring = config_lookup(cfg, "cachetimeout")) == NULL) {
		*cachetimeout = 0;
	} else {
		*cachetimeout = config_setting_get_int(cfgstring);
	}

	dispconfig->bannedwordsnr = 0;

  	#ifdef BLACK_BOKS
		dispconfig->writeprequery = 0;
	#else
	
	config_setting_t *cfgarray;

	    	if ( (cfgarray = config_lookup(cfg, "usecashe") ) == NULL) {
			printf("can't load \"usecashe\" from config\n");
			exit(1);
	  	}

		dispconfig->usecashe = config_setting_get_bool(cfgarray);

	    	if ( (cfgarray = config_lookup(cfg, "useprequery") ) == NULL) {
			printf("can't load \"useprequery\" from config\n");
			exit(1);
	  	}

		dispconfig->useprequery = config_setting_get_bool(cfgarray);

	    	if ( (cfgarray = config_lookup(cfg, "writeprequery") ) == NULL) {
			printf("can't load \"writeprequery\" from config\n");
			exit(1);
	  	}

		dispconfig->writeprequery = config_setting_get_bool(cfgarray);

	    	if ( (cfgarray = config_lookup(cfg, "UrlToDocID") ) == NULL) {
			printf("can't load \"UrlToDocID\" from config\n");
			exit(1);
	  	}

		dispconfig->UrlToDocID = config_setting_get_string(cfgarray);


		// mysql web db config
	        if ((cfgarray = config_lookup(cfg, "mysql_webdb")) == NULL) {
	
        	        printf("can't load \"mysql_webdb\" from config\n");
                	exit(1);
        	}


        	if ( (cfgstring = config_setting_get_member(cfgarray, "host") ) == NULL) {
                	printf("can't load \"host\" from config\n");
                	exit(1);
        	}

        	dispconfig->webdb_host = config_setting_get_string(cfgstring);


        	if ( (cfgstring = config_setting_get_member(cfgarray, "user") ) == NULL) {
                	printf("can't load \"user\" from config\n");
                	exit(1);
        	}

        	dispconfig->webdb_user = config_setting_get_string(cfgstring);


        	if ( (cfgstring = config_setting_get_member(cfgarray, "password") ) == NULL) {
                	printf("can't load \"password\" from config\n");
                	exit(1);
        	}

        	dispconfig->webdb_password = config_setting_get_string(cfgstring);


        	if ( (cfgstring = config_setting_get_member(cfgarray, "db") ) == NULL) {
                	printf("can't load \"db\" from config\n");
                	exit(1);
        	}

        	dispconfig->webdb_db = config_setting_get_string(cfgstring);


		//load banned words array from config    
	    	if ( (cfgarray = config_lookup(cfg, "bannedwords") ) == NULL) {
			fprintf(stderr,"can't load \"bannedwords\" from config\n");
	  	}
		else {
			//finner antall ord
			dispconfig->bannedwordsnr = config_setting_length(cfgarray);

			if ((dispconfig->bannedwords = malloc(sizeof(char *) * (dispconfig->bannedwordsnr))) == NULL) {
				perror("malloc dispconfig.bannedwords");
				exit(-1);
			}

			int i;
			for(i=0;i<dispconfig->bannedwordsnr;i++) {
				dispconfig->bannedwords[i] = strdup(config_setting_get_string_elem(cfgarray,i));
				dprintf("banned: %s\n",dispconfig->bannedwords[i]);
			}
		}

	#endif
	



}

