
#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/file.h> //flock
#include <dlfcn.h>      /* defines dlopen(), etc.       */
#include <sys/types.h>
#include <dirent.h>
#include <time.h> // time(), localtime()
#include <unistd.h> // getpid()
#include <signal.h> // kill()

#if WITH_PATHACCESS_CACHE
#include <libmemcached/memcached.h>
#endif

#include "../crawl/crawl.h"

#include "../common/collection.h"
#include "../common/config.h"
#include "../common/define.h"
#include "../common/bstr.h"
#include "../common/daemon.h"
#include "../common/error.h"
#include "../common/timediff.h"
#include "../common/boithohome.h"
#include "../common/logs.h"
#include "../maincfg/maincfg.h"
#include "../boitho-bbdn/bbdnclient.h"

#include "../common/boithohome.h"

#include "../bbdocument/bbdocument.h"

#include "../3pLibs/keyValueHash/hashtable.h"


#define crawl_crawl 1
#define crawl_recrawl 2

/* MYSQL login information */
#define MYSQL_HOST "localhost"
#define MYSQL_USER "boitho"
#define MYSQL_PASS "G7J7v5L5Y7"

#define TEST_COLL_NAME "_%s_TestCollection" // %s is connector name.

struct hashtable *global_h;

struct {
	char *addr;
	int port;
} *memcache_servers;


FILE *LOGACCESS, *LOGERROR;
int global_bbdnport;

#ifdef WITH_PATHACCESS_CACHE
void mc_add_servers(void);
#endif



int cm_searchForCollection (char cvalue[],struct collectionFormat *collection[],int *nrofcollections);

int documentContinue(struct collectionFormat *collection) {

	debug("documentContinue: start");

	int recrawl_schedule_start, recrawl_schedule_end;
	struct tm *t;
	time_t now;

	//hvis vi skal crawl oftere en hvert dågn bruker vi ikke schedule time, men tilater å crawl hele tiden.
	if ( (collection->rate != 0) && (collection->rate < 1440)) {
		printf("documentContinue: Collection is set to be recrawled every %i min, ignoring schedule time\n",collection->rate);
		return 1;
	}


	bconfig_flush(CONFIG_CACHE_IS_OK);

	if (!bconfig_getentryint("recrawl_schedule_start",&recrawl_schedule_start)) {
		recrawl_schedule_start = 0;
	}
	if (!bconfig_getentryint("recrawl_schedule_end",&recrawl_schedule_end)) {
		recrawl_schedule_end = 0;
	}


	now = time(NULL);
	t = localtime(&now);	

	debug("now: %i,recrawl_schedule_start %i,recrawl_schedule_end %i\n",t->tm_hour,recrawl_schedule_start,recrawl_schedule_end);

	//hvis vi ikke har noen begrensning så er det bare å crawler på
	if ((recrawl_schedule_start == 0) || (recrawl_schedule_end == 0)) {
		return 1;
	}

	//tar en avgjørelse om vi skal fortsette å crawle
	//vi har to scenarioer, 
	// 1: start er større en slutt tidspungete, og er dermed i fremtiden
	// 	start 10, end 07
	//	kl nå 14
	//	kl nå 06
	//
	// 2: start og slutt tidspungtet er etter hverandre, og derfor innen samme dag
	//	start 10, end 12
	//	kl nå 14
	//
	if ( recrawl_schedule_start > recrawl_schedule_end ) {

		if ((t->tm_hour < recrawl_schedule_start) && (t->tm_hour > recrawl_schedule_end)) {
			printf("scenario 1: to early, wont crawl\n");
			return 0;
		}

	}
	else {
		if (t->tm_hour < recrawl_schedule_start) {
			printf("scenario 2: to early, wont crawl\n");
			return 0;
		}
		else if (t->tm_hour > recrawl_schedule_end) {
			printf("scenario 2: to late, wont crawl\n");
			return 0;
		}

	}

	debug("hour is now %i, will crawl\n",t->tm_hour);

	debug("documentContinue: end\n");

	return 1;
}

int documentExist(struct collectionFormat *collection, struct crawldocumentExistFormat *crawldocumentExist) {
	int ret;

	#ifdef DEBUG
	printf("documentExist: start\n");
	#endif

	ret = bbdocument_exist(collection->collection_name, crawldocumentExist->documenturi, crawldocumentExist->lastmodified);

	#ifdef DEBUG
	printf("documentExist: end\n");
	#endif

	return ret;
}


char *documentErrorGetlast(struct collectionFormat *collection) {
        return collection->errormsg;
}


int documentError(struct collectionFormat *collection,int level, const char *fmt, ...) {



        va_list     ap;

        va_start(ap, fmt);

	printf("documentError: ");
        vprintf(fmt,ap);

	bvlog(LOGERROR,level,fmt,ap);

	vsprintf(collection->errormsg ,fmt,ap);

        va_end(ap);

}

int documentAdd(struct collectionFormat *collection, struct crawldocumentAddFormat *crawldocumentAdd) {
	#ifdef DEBUG
	printf("documentAdd start\n");
	#endif

	#ifdef DEBUG
	printf("documentAdd: uri %s, title %s\n",(*crawldocumentAdd).documenturi,(*crawldocumentAdd).title);
	#endif

	//send it inn
	if (!bbdn_docadd((*collection).socketha,
			(*collection).collection_name,
			(*crawldocumentAdd).documenturi,
			(*crawldocumentAdd).documenttype,
			(*crawldocumentAdd).document,
			(*crawldocumentAdd).dokument_size,
			(*crawldocumentAdd).lastmodified,
			(*crawldocumentAdd).acl_allow,
			(*crawldocumentAdd).acl_denied,
			(*crawldocumentAdd).title,
			(*crawldocumentAdd).doctype)
	) {

		blog(LOGERROR,1,"can't sent to bbdn! Tryed to send doc \"%s\" Will sleep and then reconect. Wont send same doc again.",(*crawldocumentAdd).documenturi);
		
		//ber om å lokke sokketen. Dette er ikke det samme som å steneg kollectionen.
		//bbdn_closecollection((*collection).socketha,(*collection).collection_name);
		bbdn_close((*collection).socketha);

		sleep(10);

		if (!bbdn_conect(&(*collection).socketha,"",global_bbdnport)) {
			blog(LOGERROR,1,"can't conect to bbdn (boitho backend document server)");
			return 0;
		}

		//exit(1);

	}
	else {
		blog(LOGACCESS,1,"crawled url: \"%s\", size: %i b, ACL: allow \"%s\", denied \"%s\"",(*crawldocumentAdd).documenturi,(*crawldocumentAdd).dokument_size,(*crawldocumentAdd).acl_allow,(*crawldocumentAdd).acl_denied);
	}

	#ifdef DEBUG
	printf("documentAdd end\n");
	#endif

	return 1;
}

int closecollection(struct collectionFormat *collection) {
	debug("closecollection start\n");
	bbdn_closecollection((*collection).socketha,(*collection).collection_name);
	debug("closecollection end\n");

}

int cmr_crawlcanconect(struct hashtable *h, struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		printf("cant get CrawlLibInfo\n");
		return 0;
	}

	debug("wil crawl \"%s\"",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlcanconect)(collection,documentError)) {
		//overfører error
		berror(documentErrorGetlast(collection));
        	 return 0;
       	}
	else {
		return 1;
	}

}
int cm_crawlfirst(struct hashtable *h,struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!documentContinue(collection)) {
                berror("Crawl is pending. Waiting for schedule time.");
		return 0;
	}

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		blog(LOGERROR,1,"Error: can't get CrawlLibInfo.");
		//exit(1);
		return 0;
	}

	collection->crawlLibInfo = crawlLibInfo;

	debug("wil crawl \"%s\"",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlfirst)(collection,documentExist,documentAdd,documentError,documentContinue)) {
        	printf("problems in crawlfirst_ld\n");
		//overfører error
                berror( documentErrorGetlast(collection) );
		blog(LOGERROR,1,"Error: Problems in crawlfirst_ld.");

		return 0;
       	}

	if (!documentContinue(collection)) {
                berror("Crawl is pending. Waiting for schedule time.");
		return 0;
	}
	
	return 1;

}

int cm_crawlupdate(struct hashtable *h,struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!documentContinue(collection)) {
                berror("Crawl is pending. Waiting for schedule time.");
		return 0;
	}

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		blog(LOGERROR,1,"Error: can't get CrawlLibInfo.");
		exit(1);
	}

	collection->crawlLibInfo = crawlLibInfo;

	debug("wil crawl \"%s\"",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlupdate)(collection,documentExist,documentAdd,documentError,documentContinue)) {
        	
		//overfører error
                berror( documentErrorGetlast(collection) );
		blog(LOGERROR,1,"Error: problems in crawlfirst_ld.");

		return 0;
       	}

	if (!documentContinue(collection)) {
                berror("Crawl is pending. Waiting for schedule time.");
		return 0;
	}

	return 1;
}

char *adduserprefix(struct collectionFormat *collections,const char username[]) {

	char *newusername;

	if ((*collections).userprefix != NULL) {
		newusername = malloc(strlen((*collections).userprefix) + strlen(username) +1);
		strcpy(newusername,(*collections).userprefix);
		strcat(newusername,username);
	}
	else {
		newusername = strdup(username);
	}

	return newusername;
}

#ifdef WITH_PATHACCESS_CACHE
char *
pathaccess_makekey(char *collection, char *username, char *password, char *uri, size_t *outlen)
{
	char *s;
	size_t len;
	int i;

	if (password == NULL)
		password = "";

	len = 11 + strlen(collection) + strlen(username) + strlen(password) + strlen(uri) + 4;
	*outlen = len-1;
	s = malloc(len);
	//sprintf(s, "pathaccess_%s\1%s\1%s\1%s", collection, username, password, uri);
	sprintf(s, "pathaccess_%s_%s_%s_%s", collection, username, password, uri);
	printf("Key: %s\n", s);
	for (i = 0; s[i] != '\0'; i++) {
		if (isspace(s[i]))
			//s[i] = '\2';
			s[i] = '!';
	}

	return s;
}

memcached_st *mc;

void
pathaccess_savecache(char *collection, char *username, char *password, char *uri, char res, char *newuri)
{
	char *s;
	size_t len;
	int ret;
	char *add;
	size_t alen;

	s = pathaccess_makekey(collection, username, password, uri, &len);

	alen = strlen(newuri) + 2;
	add = malloc(alen);
	add[0] = res;
	strcpy(add+1, newuri);

	ret = memcached_add(mc, s, len, add, alen, 300, 0);
	if (ret != MEMCACHED_SUCCESS)
		printf("Unable to add new cache item.\n");

	free(s);
}

int
pathaccess_cachelookup(char *collection, char *username, char *password, char *uri, char *newuri)
{
	char *s;
	int error;
	size_t len;
	char *ret;
	char r;

	memcached_return err;
	uint16_t flags;
	size_t retsize;

	s = pathaccess_makekey(collection, username, password, uri, &len);

	ret = memcached_get(mc, s, len, &retsize, &flags, &err);
	free(s);

	if (ret == NULL) {
		return 0;	
	}

	r = ret[0];
	if (r == 1)
		strcpy(newuri, ret+1);
	free(ret);

	return r;
}
#endif


int pathAccess(struct hashtable *h, char collection[], char uri[], char username_in[], char password[]) {
	char *origuri;

	int cacheret;

	struct collectionFormat *collections; // bare en "s" skiller collection og collections her. Ikke bra, bør finne på bedre navn
	int nrofcollections;
	int forreturn;

	char *username;

	struct crawlLibInfoFormat *crawlLibInfo;
	struct timeval start_time, end_time;

	struct pathAccessTimesFormat {
		double searchForCollection;
		double getCrawlLibInfo;
		double crawlpatAcces;
	};

	struct pathAccessTimesFormat pathAccessTimes;

	printf("pathAccess: start\n");

	//temp:
	//26.0207:quiq fix. Lagger til domene i brukernav
	/*
	char username_t[64];
	strcpy(username_t,"i04\\");
	strcat(username_t,username);
	strcpy(username,username_t);
	*/

	gettimeofday(&start_time, NULL);
	debug("cm_searchForCollection");
	if (!cm_searchForCollection(collection,&collections,&nrofcollections)) {
		printf("cant't find Collection \"%s\"in db at %s:%d\n",collection,__FILE__,__LINE__);
		return 0;
	}
	gettimeofday(&end_time, NULL);
	pathAccessTimes.searchForCollection = getTimeDifference(&start_time,&end_time);

	//skal returnere 1, og bare 1, hvis ikke er det noe feil
	if (nrofcollections != 1) {
		printf("error looking opp collection \"%s\"\n",collection);
		return 0;
	}


	gettimeofday(&start_time, NULL);
	debug("cm_getCrawlLibInfo");
	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,collections[0].connector)) {
		printf("cant get CrawlLibInfo\n");
		return 0;
	}
	collections[0].crawlLibInfo = crawlLibInfo;

	gettimeofday(&end_time, NULL);
	pathAccessTimes.getCrawlLibInfo = getTimeDifference(&start_time,&end_time);

	printf("wil pathAccess \"%s\", for user \"%s\"\n",uri,username_in);

	gettimeofday(&start_time, NULL);

	username = adduserprefix(collections,username_in);

	printf("username after adduserprefix: \"%s\"\n",username);

	origuri = strdup(uri);
#ifdef WITH_PATHACCESS_CACHE
	if (memcache_servers != NULL)
		mc_add_servers();
#endif
	if ((*crawlLibInfo).crawlpatAcces == NULL) {
		printf("cralwer her ikke crawlpatAcces. returnerer tilgang. Må i fremtiden slå det opp\n");
		forreturn = 1;
	}
#ifdef WITH_PATHACCESS_CACHE
	else if (memcache_servers != NULL && (cacheret = pathaccess_cachelookup(collection, username, password, origuri, uri)) > 0) {
		if (cacheret == 1)
			forreturn = 1;
		else if (cacheret == 2)
			forreturn = 0;
		else
			forreturn = 0;
	}
#endif
	else if (!(*(*crawlLibInfo).crawlpatAcces)(uri,username,password,documentError,&collections[0])) {
        	printf("Can't crawlLibInfo. Can be denyed or somthing else\n");
		//overfører error
                berror( documentErrorGetlast( &collections[0]) );

#ifdef WITH_PATHACCESS_CACHE
		if (memcache_servers != NULL)
			pathaccess_savecache(collection, username, password, origuri, 2, "");
#endif
		forreturn = 0;
       	}
	else {
#ifdef WITH_PATHACCESS_CACHE
		if (memcache_servers != NULL)
			pathaccess_savecache(collection, username, password, origuri, 1, uri);
#endif
		forreturn = 1;
	}
	gettimeofday(&end_time, NULL);
	free(origuri);
	pathAccessTimes.crawlpatAcces = getTimeDifference(&start_time,&end_time);


	printf("pathAccess: times\n");	
	printf("\tsearchForCollection: %f\n",pathAccessTimes.searchForCollection);
	printf("\tgetCrawlLibInfo: %f\n",pathAccessTimes.getCrawlLibInfo);
	printf("\tcrawlpatAcces: %f\n",pathAccessTimes.crawlpatAcces);

	printf("pathAccess: end\n");

	if (forreturn == 1) {
		blog(LOGACCESS,2,"pathAccess allowed url: \"%s\", user: \"%s\", time used %f s",uri,username,pathAccessTimes.crawlpatAcces);
	}
	else {
		blog(LOGERROR,2,"pathAccess denyed url: \"%s\", user: \"%s\", time used %f s, Error: \"%s\"",uri,username,pathAccessTimes.crawlpatAcces, documentErrorGetlast( &collections[0]) );
	}

	free(username);
	
	return forreturn;

}





/************************************************************
ToDo: dette er HØYST midlertidig. Bruker globale vvariabler her, og ingen free()'ing
************************************************************/

static char *found_share[32];
static char found_sharenr = 0;

int scan_found_share(char share[]) {

	printf("add \"%s\" (nr %i)\n",share,found_sharenr);
	found_share[found_sharenr] = malloc(strlen(share) +1);
	strcpy(found_share[found_sharenr],share);

	++found_sharenr;
}

int scan_found_start() {
	found_sharenr = 0;
}

int scan_found_get (char ***scares,int *nr) {
	found_share[found_sharenr] = '\0';

	(*scares) = found_share;
	(*nr) = found_sharenr;

}
/************************************************************/
/************************************************************/
int scan (struct hashtable *h,char ***shares,int *nrofshares,char crawlertype[],char host[],char username[],char password[]) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,crawlertype)) {
		blog(LOGERROR,1,"Error: can't get CrawlLibInfo.");

                exit(1);
        }

	if ((*crawlLibInfo).scan == NULL) {
		printf("cant scan. Crawler dosent suport it.\n");
		blog(LOGERROR,1,"Error: cant scan. Crawler dosent suport it.");

		return 0;
	}

	printf("scan:\nhost %s\nusername %s\npassword %s\n",host,username,password);

	scan_found_start();

	if (!(*(*crawlLibInfo).scan)(scan_found_share,host,username,password,documentError)) {
                printf("problems in scan\n");
		blog(LOGERROR,1,"Error: problems in scan.");

		return 0;
        }

	scan_found_get(shares,nrofshares);
}



static unsigned int cm_hashfromkey(void *k)
{
	
    return ((int) ((char *)k)[0]);
}

static int cm_equalkeys(void *k1, void *k2)
{
    return (0 == strcmp(k1,k2));
}

int file_exist (char file[]) {

	FILE *FH;
	if ((FH = fopen(file,"rb")) == NULL) {
		return 0;
	}

	fclose(FH);

	return 1;
}

int cm_start(struct hashtable **h) {

	printf("cm_start start\n");
	void* lib_handle;       /* handle of the opened library */
	const char* error_msg;
	struct crawlLibInfoFormat *crawlLibInfo;

	DIR *dirp;
	struct dirent *dp;
	char libpath[PATH_MAX];
	char perlpath[PATH_MAX];
	char folderpath[PATH_MAX];

	(*h) = create_hashtable(20, cm_hashfromkey, cm_equalkeys);

	if ((dirp = opendir(bfile("crawlers"))) == NULL) {
		perror(bfile("crawlers"));
		blog(LOGERROR,1,"Error: cant open crawlers directory.");

		exit(1);
	}	

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.') {
			continue;
		}
		sprintf(libpath,"%s/%s/%s.so",bfile("crawlers"),dp->d_name,dp->d_name);	
		sprintf(perlpath,"%s/%s/main.pm",bfile("crawlers"),dp->d_name);	
		sprintf(folderpath,"%s/%s/",bfile("crawlers"),dp->d_name);	


		if (file_exist(libpath)) {
			printf("loading path \"%s\"\n",libpath);
			lib_handle = dlopen(libpath, RTLD_LAZY);
			if (!lib_handle) {
				blog(LOGERROR,1,"Error: during dlopen(): %s. File %s.",dlerror(),libpath);
			    	//exit(1);
				continue;
			}

	
			crawlLibInfo = dlsym(lib_handle, "crawlLibInfo");

			/* check that no error occured */
	        	error_msg = dlerror();
		        if (error_msg) {
				blog(LOGERROR,1,"Error: Error locating '%s' - %s.",dp->d_name, error_msg);
	        	    exit(1);
	        	}

		
			printf("loaded \"%s\"\n",(*crawlLibInfo).shortname);



			if ((*crawlLibInfo).crawlinit == NULL) {

			}
			else if (!(*crawlLibInfo).crawlinit()) {
				printf("crawlinit dident return 1\n");
				blog(LOGERROR,1,"Error: crawlinit dident return 1.");

				exit(1);
			}

			//sjekk at den ikke finnes i hashen fra før
			if (hashtable_search((*h),(*crawlLibInfo).shortname) != NULL) {
				printf("all redy have module with shortname \"%s\"\n",(*crawlLibInfo).shortname);
				continue;
			}

			if (! hashtable_insert((*h),(*crawlLibInfo).shortname,crawlLibInfo) ) {                        
				blog(LOGERROR,1,"Error: can't hastable insert.");
                		exit(-1);
                	}

			//alt ok. Legg det i en hash

			//dlclose(lib_handle); 
		}		
		else if (file_exist(perlpath)) {

			crawlLibInfo = perlCrawlStart(folderpath,dp->d_name);

			printf("loaded \"%s\"\n",(*crawlLibInfo).shortname);

			//sjekk at den ikke finnes i hashen fra før
			if (hashtable_search((*h),(*crawlLibInfo).shortname) != NULL) {
				printf("all redy have module with shortname \"%s\"\n",(*crawlLibInfo).shortname);
				continue;
			}

			if (! hashtable_insert((*h),(*crawlLibInfo).shortname,crawlLibInfo) ) {                        
				blog(LOGERROR,1,"Error: can't hastable insert.");
                		exit(-1);
                	}

		}
		else {
			fprintf(stderr,"can't load \"%s\" crawler.\n",dp->d_name);
		}
	}

	closedir(dirp);

	printf("cm_start end\n");


}


/************************************************************/

int cm_getCrawlLibInfo(struct hashtable *h,struct crawlLibInfoFormat **crawlLibInfo,char shortname[]) {
	debug("cm_getCrawlLibInfo: start");
	debug("wil search for \"%s\"",shortname);
	if (((*crawlLibInfo) = (struct crawlLibInfoFormat *)hashtable_search(h,shortname)) == NULL) {
		berror("don't have a crawler for \"%s\"\n",shortname);
		return 0;
        }
	else {
		return 1;
	}
}



int sm_collectionfree(struct collectionFormat *collection[],int nrofcollections) {

	int i;

	for (i=0;i<nrofcollections;i++) {
		#ifdef DEBUG
		printf("freeing nr %i: start\n",i);
		#endif
		free((*collection)[i].resource);
                free((*collection)[i].user);
                free((*collection)[i].password);
                free((*collection)[i].connector);
                free((*collection)[i].collection_name);
                free((*collection)[i].query1);
                free((*collection)[i].query2);
		free((*collection)[i].extra);
		#ifdef DEBUG
			printf("freeing nr %i: end\n",i);
		#endif

                hashtable_destroy((*collection)[i].params, 1);
	}

	//toDo: hvorfor segfeiler vi her ????
	//free(collection);
}

int
cm_collectionFetchUsers(struct collectionFormat *collection, MYSQL *db)
{
        char mysql_query [2048];
	MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;
	int i, numUsers;

	sprintf(mysql_query, "SELECT name FROM shareUsers WHERE share = %d",
	       collection->id);

	if(mysql_real_query(db, mysql_query, strlen(mysql_query))){ /* Make query */
               	printf(mysql_error(db));
		blog(LOGERROR,1,"MySQL Error: \"%s\".",mysql_error(db));
               	return 0;
       	}
	mysqlres=mysql_store_result(db); /* Download result from server */

	numUsers = mysql_num_rows(mysqlres);

	if (numUsers == 0) {
		collection->users = NULL;
		//returner 1 lengder nede, så for vi også frigjort resursen
		//return 1;
	}
	else {
		debug("nrofrows %i\n", numUsers);
		collection->users = malloc((numUsers+1) * sizeof(char *));
		if (collection->users == NULL)
			return 0;
		i=0;
		while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
			collection->users[i] = strdup(mysqlrow[0]);
			i++;
		}
		collection->users[i] = NULL;
	}

	//mysql freeing
	mysql_free_result(mysqlres);
	
	return 1;
}

void 
sql_fetch_params(struct hashtable ** h, MYSQL *db, unsigned int coll_id)  {
    char mysql_query[512];
    MYSQL_ROW row;
    (*h) = create_hashtable(7, cm_hashfromkey, cm_equalkeys);

    snprintf(mysql_query, sizeof mysql_query, "\
        SELECT shareParam.value, param.param \
        FROM shareParam, param \
        WHERE shareParam.param = param.id \
        AND shareParam.share = %d", coll_id);

    if (mysql_real_query(db, mysql_query, strlen(mysql_query))) {
        printf(mysql_error(db));
	blog(LOGERROR,1,"MySQL Error: \"%s\".",mysql_error(db));
        exit(1);
    }

    MYSQL_RES * res = mysql_store_result(db);
    while ((row = mysql_fetch_row(res)) != NULL) {
        if (!hashtable_insert(*h, strdup(row[1]), strdup(row[0]))) {
            printf("can't insert param into ht");
            exit(1);
        }
    }
    mysql_free_result(res);
}


int cm_searchForCollection (char cvalue[],struct collectionFormat *collection[],int *nrofcollections) {

        char mysql_query [2048];
        static MYSQL demo_db;
	MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;

        int i,y;



	mysql_init(&demo_db);

	//koble til mysql
	if(!mysql_real_connect(&demo_db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)){
		printf(mysql_error(&demo_db));
		blog(LOGERROR,1,"MySQL Error: \"%s\".",mysql_error(&demo_db));
		exit(1);
	}

	if (cvalue != NULL) {
            char cvalue_esc[128];
            mysql_real_escape_string(&demo_db, cvalue_esc, cvalue, strlen(cvalue));
	    sprintf(mysql_query, "\
					select \
						resource, \
						connectors.name, \
						collection_name, \
						UNIX_TIMESTAMP(last), \
						query1, \
						query2, \
						auth_id, \
						shares.id, \
						shares.userprefix, \
						shares.rate \
					from \
						shares,connectors \
					where \
						connectors.ID = shares.connector \
						AND collection_name='%s' \
					",cvalue_esc);
	}
	else {
		sprintf(mysql_query, "\
					select \
						resource, \
						connectors.name, \
						collection_name, \
						UNIX_TIMESTAMP(last), \
						query1, \
						query2, \
						auth_id, \
						shares.id, \
						shares.userprefix, \
						shares.rate \
					from \
						shares,connectors \
					where \
						connectors.ID = shares.connector \
					");
	}

	debug("mysql_query: %s\n",mysql_query);

	if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
               	printf(mysql_error(&demo_db));
		blog(LOGERROR,1,"MySQL Error: \"%s\".",mysql_error(&demo_db));
		
               	exit(1);
       	}
	mysqlres=mysql_store_result(&demo_db); /* Download result from server */

	(*nrofcollections) = (int)mysql_num_rows(mysqlres);

	if ((*nrofcollections) == 0) {
		printf("dident find any rows\n");
		
	}
	else {

	debug("nrofrows %i\n",*nrofcollections);

	(*collection) = malloc(sizeof(struct collectionFormat) * (*nrofcollections));

	//resetter minne
	for (i=0;i<(*nrofcollections);i++) {
		collectionReset (collection[i]);
	}

        i=0;
        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
        	debug("\tdata resource: %s, connector: %s, collection_name: %s, lastCrawl: %s, userprefix: %s\n",mysqlrow[0],mysqlrow[1],mysqlrow[2],mysqlrow[3],mysqlrow[8]);
		
		//lagger inn i info struct
		(*collection)[i].resource  		= strdup(mysqlrow[0]);	
		(*collection)[i].connector		= strdup(mysqlrow[1]);	
		(*collection)[i].collection_name 	= strdup(mysqlrow[2]);


		if (mysqlrow[3] == NULL) {
			(*collection)[i].lastCrawl = 0;
		}
		else {
			(*collection)[i].lastCrawl = strtoul(mysqlrow[3], (char **)NULL, 10);
		}

		(*collection)[i].query1 		= strdup(mysqlrow[4]);
		(*collection)[i].query2		 	= strdup(mysqlrow[5]);
		
		if (mysqlrow[6] == NULL) {
			(*collection)[i].auth_id = 0;
		}
		else {
			(*collection)[i].auth_id = strtoul(mysqlrow[6], (char **)NULL, 10);
		}

		(*collection)[i].id = strtoul(mysqlrow[7], (char **)NULL, 10);
		(*collection)[i].userprefix = strdupnul(mysqlrow[8]);
		(*collection)[i].rate = strtoul(mysqlrow[9], (char **)NULL, 10);

		(*collection)[i].extra = NULL;

		//normaliserer collection navn, ved å fjenre ting som space og - \ / etc
		collection_normalize_name((*collection)[i].collection_name,strlen((*collection)[i].collection_name));


		debug("resource \"%s\", connector \"%s\", collection_name \"%s\"\n",(*collection)[i].resource,(*collection)[i].connector,(*collection)[i].collection_name);

		cm_collectionFetchUsers(collection[i], &demo_db);
                sql_fetch_params(&(*collection)[i].params, &demo_db, (*collection)[i].id);

		//crawler ny
                ++i;
        }


        mysql_free_result(mysqlres);

	/***********************************************************************/
	// slår opp bruker id hvis vi har det, hvis ikke skal user og password være NULL	
	for (i=0;i<(*nrofcollections);i++) {

		sprintf(mysql_query,"select username,password from collectionAuth where id = %i",(*collection)[i].auth_id);

		debug("mysql_query: %s\n",mysql_query);

		if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
	               	printf(mysql_error(&demo_db));
	               	exit(1);
	       	}
		mysqlres=mysql_store_result(&demo_db); /* Download result from server */

		(*collection)[i].user = NULL;
		(*collection)[i].password = NULL;
        
	        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
	        	debug("\tUser \"%s\", Password \"%s\"\n",mysqlrow[0],mysqlrow[1]);
			(*collection)[i].user 			= strdup(mysqlrow[0]);	
			(*collection)[i].password 		= strdup(mysqlrow[1]);	
		
	        }
	}

        mysql_free_result(mysqlres);

	//inaliserer andre ting
	for (i=0;i<(*nrofcollections);i++) {
		(*collection)[i].errormsg[0] = '\0';
	}

	/***********************************************************************/

	}

	mysql_close(&demo_db);

	return 1;

	debug("cm_searchForCollection: end\n");
	
}

int cm_handle_crawlcanconect(char cvalue[]) {

	struct collectionFormat *collection;
	int nrofcollections;
	int i;

	printf("cm_handle_crawlcanconect (%s)\n",cvalue);

	cm_searchForCollection(cvalue,&collection,&nrofcollections);
	printf("crawlcanconect: cm_searchForCollection done\n");
	if (nrofcollections == 1) {
		
		#ifndef DEBUG
		//Temp: funger ikke når vi kompilerer debug. Må også compilere crawlManager debug
		//make a conectina for add to use
		if (!bbdn_conect(&collection[0].socketha,"",global_bbdnport)) {
			//berror("can't conect to bbdn (boitho backend document server)\n");
			return 0;
		}

		if (!cmr_crawlcanconect(global_h,&collection[0])) {
			return 0;
		}

		//ber bbdn om å lukke
		printf("closeing bbdn con\n");
                bbdn_close(&collection[0].socketha);
		#endif
	}
	else {
		printf("crawlcanconect: Error got back %i coll to crawl\n",nrofcollections);
	}


	sm_collectionfree(&collection,nrofcollections);

	return 1;
}

//crawler_success  skal være 0 hvis vi feilet, og 1 hvis vi ikke feilet
int set_crawler_message(int crawler_success  , char mrg[], unsigned int id) {

        char mysql_query [2048];
	char messageEscaped[2048];
        static MYSQL demo_db;
        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;

	blog(LOGACCESS, 2, "Status: set_crawler_message: mesage: \"%s\", success: %i, id: %i.",mrg,crawler_success,id);

        mysql_init(&demo_db);

        //koble til mysql
        if(!mysql_real_connect(&demo_db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
		blog(LOGERROR,1,"Mysql Error: \"%s\".",mysql_error(&demo_db));
                exit(1);
        }

	//escaper queryet rikit
        mysql_real_escape_string(&demo_db,messageEscaped,mrg,strlen(mrg));

	//printf("mysql_queryEscaped \"%s\"\n",messageEscaped);

	if (crawler_success  == 0) {
        	sprintf(mysql_query, "UPDATE shares SET crawler_success = \"%i\", crawler_message = \"%s\" WHERE id  = \"%u\"",crawler_success ,messageEscaped,id);
	}
	else {
        	sprintf(mysql_query, "UPDATE shares SET crawler_success = \"%i\", crawler_message = \"%s\", last = now() WHERE id  = \"%u\"",crawler_success ,messageEscaped,id);

	}

	blog(LOGACCESS,2,"Status: \"%s\".",mysql_query);

        //debug("mysql_query: %s\n",mysql_query);

        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                //printf(mysql_error(&demo_db));
		blog(LOGERROR,1,"Mysql Error: \"%s\".",mysql_error(&demo_db));
                exit(1);
        }


	mysql_close(&demo_db);

}

//setter at vi har begynt og cralwe
cm_setCrawStartMsg(struct collectionFormat *collection,int nrofcollections) {
	int i;

	for(i=0;i<nrofcollections;i++) {
		set_crawler_message(0,"Crawling it now.",collection[i].id);
	}
}


struct collection_lockFormat {
	char lockfile[512];
	FILE *LOCK;	

	char elementlockfile[512];
	FILE *ELEMENTLOCK;
};

//dette bør nokk på litt sikt flyttes ut i dokument manageren
int crawl_lock(struct collection_lockFormat *collection_lock, char collection[]) {

	//oppretter var mappen hvis den ikke finnes. Dette slik at vi slipper og gjøre dette under instalsjonen
	bmkdir_p(bfile("var/"),0755);

	sprintf((*collection_lock).lockfile,"var/boitho-collections-%s.lock",collection);

	debug("locking lock \"%s\"",(*collection_lock).lockfile);

	if (((*collection_lock).LOCK = bfopen((*collection_lock).lockfile,"w+")) == NULL) {
		perror((*collection_lock).lockfile);
		return 0;
	}

	//trying to get a lock. If we can we vil keep it as long we are crawling to rewnet dublicat crawling
	if (flock(fileno((*collection_lock).LOCK),LOCK_EX | LOCK_NB) != 0) {
		fclose((*collection_lock).LOCK);
		return 0;
	}	
	else {
		return 1;
	}

}


int crawl_element_lock(struct collection_lockFormat *collection_lock, char connector[]) {

	//oppretter var mappen hvis den ikke finnes. Dette slik at vi slipper og gjøre dette under instalsjonen
	bmkdir_p(bfile("var/"),0755);

	sprintf((*collection_lock).elementlockfile,"var/boitho-element-%s.lock",connector);

	debug("locking lock \"%s\"",(*collection_lock).elementlockfile);

	if (((*collection_lock).ELEMENTLOCK = bfopen((*collection_lock).elementlockfile,"w+")) == NULL) {
		perror((*collection_lock).elementlockfile);
		return 0;
	}

	//trying to get a lock. If we can we vil keep it as long we are crawling to rewnet dublicat crawling
	if (flock(fileno((*collection_lock).ELEMENTLOCK),LOCK_EX | LOCK_NB) != 0) {
		fclose((*collection_lock).ELEMENTLOCK);
		return 0;
	}	
	else {
		return 1;
	}

}

int crawl_unlock(struct collection_lockFormat *collection_lock) {
	fclose((*collection_lock).LOCK);
	unlink((*collection_lock).lockfile);
	return 1;
}
int crawl_element_unlock(struct collection_lockFormat *collection_lock) {
	fclose((*collection_lock).ELEMENTLOCK);
	unlink((*collection_lock).elementlockfile);
	return 1;
}

int is_test_collection(struct collectionFormat * coll) {
    
    char test_coll[512];
    snprintf(test_coll, sizeof test_coll, TEST_COLL_NAME, coll->connector);
    collection_normalize_name(test_coll, strlen(test_coll));

    return strcmp(coll->collection_name, test_coll) == 0;
}

/**
* Redirect stderr, stdout to file */
int redirect_stdoutput(char * file) {
    if (file == NULL) {
        blog(LOGERROR, 1, "No output file for test coll");
        return 0;
    }

    if (file_exist(file)) {
        blog(LOGERROR, 1, "Test output file already exists '%s' exists.", file);
        return 0;
    }

    printf("tc: redirecting std[out,err] to %s.", file);
    if ((freopen(file, "a+", stdout)) == NULL
            || freopen(file, "a+", stderr) == NULL) {
        perror(file);
        return 0;
    }
    return 1;
}

int crawl (struct collectionFormat *collection,int nrofcollections, int flag, char *extra) {

	int i;
	FILE *LOCK;

	struct collection_lockFormat collection_lock;

	for(i=0;i<nrofcollections;i++) {
		if (collection[i].extra != NULL)
			free(collection[i].extra);
		collection[i].extra = extra;

		blog(LOGACCESS,1,"Starting crawl of collection \"%s\" (id %u).",collection[i].collection_name,collection[i].id);

                int output_redirected = 0;
                if (is_test_collection(&collection[i])) {
                    if (!redirect_stdoutput(collection[i].extra)) {
                        blog(LOGERROR, 1, "test collection error, skipping.");
                        continue;
                    }
                    
                    // a lock implies that a crawl is still running
                    flock(fileno(stdout), LOCK_SH); 
                    setvbuf(stdout, NULL, _IOLBF, 0); // line buffered
                    setvbuf(stderr, NULL, _IOLBF, 0);
                    output_redirected = 1;
                    
                    printf("pid:%d\n", getpid());
                }

		//sletter collection. Gjør dette uavhenging om vi har lock eller ikke, slik at vi altid får slettet, så kan vi gjøre
		// ny crawl etterpå hvis vi ikke hadde lock
		if (flag == crawl_recrawl) {

				static MYSQL demo_db;

				printf("crawl_recrawl\n");

				mysql_init(&demo_db);

				//koble til mysql
				if(mysql_real_connect(&demo_db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)){
					char querybuf[1024];
					int querylen;

					querylen = snprintf(querybuf, sizeof(querybuf), "UPDATE shares SET last = 0 WHERE id = '%d'",
					    collection[i].id);
					if (mysql_real_query(&demo_db, querybuf, querylen)) {
						printf("Mysql error: %s", mysql_error(&demo_db));
						blog(LOGERROR,1,"MySQL Error: %s: \"%s\".", querybuf, mysql_error(&demo_db));
					}
				} else {
					printf(mysql_error(&demo_db));
					blog(LOGERROR,1,"MySQL Error: \"%s\".",mysql_error(&demo_db));
				}



				//nullsetter lastCrawl, som er den verdien som viser siste crawl. 
				//Tror ikke crawlfirst skal ta hensysn til den, men gjør det for sikkerhets skyld
				(*collection).lastCrawl = 0;

				//beordrer en sletting. Burde kansje vært gjort vi bbdn, ikke direkte, men skit au
				bbdocument_deletecoll(collection[i].collection_name);


		}

		//tester at vi ikke allerede holder på å crawle denne fra før
		if (!crawl_lock(&collection_lock,collection[i].collection_name)) {
			blog(LOGERROR,1,"Error: Can't crawl collection \"%s\". We are all redy crawling it.",collection[i].collection_name);
			//runarb: 14 jan 2008: ingen grun til å oppdatere beskjeden, da det som står der er med korekt
                        //set_crawler_message(0,"Error: Can't crawl collection. We are all redy crawling it.",collection[i].id);

			continue;
		}

		//tester at vi ikke driver å crawler med den crawleren fra før. Her burde vi kansje 
		//heller satset på å begrense på server. Slik at for eks to smb servere kan crawles samtidig
		if (!crawl_element_lock(&collection_lock,collection[i].connector)) {
			blog(LOGERROR,1,"Error: Can't crawl collection \"%s\". We are all redy crawling this type/server.",collection[i].collection_name);
                        set_crawler_message(0,"Error: Can't crawl collection. We are all redy crawling this type/server.",collection[i].id);

			continue;
		}
		
	
		//make a conectina to bbdn for add to use
		if (!bbdn_conect(&collection[i].socketha,"",global_bbdnport)) {
			berror("can't conect to bbdn (boitho backend document server)");
			set_crawler_message(0,bstrerror(),collection[i].id);
		}
		else {

			if (flag == crawl_recrawl) {


				if (!cm_crawlfirst(global_h,&collection[i])) {
                                        set_crawler_message(0,bstrerror(),collection[i].id);
                                }
                                else {
                                        set_crawler_message(1,"Ok",collection[i].id);
                                }

			}
			else if ((*collection).lastCrawl == 0) {
				if (!cm_crawlfirst(global_h,&collection[i])) {
					set_crawler_message(0,bstrerror(),collection[i].id);	
				}
				else {
					set_crawler_message(1,"Ok",collection[i].id);
				}
			}
			else {
                                if (!cm_crawlupdate(global_h,&collection[i])) {
                                        set_crawler_message(0,bstrerror(),collection[i].id);
                                }
                                else {
                                        set_crawler_message(1,"Ok",collection[i].id);
                                }
			}

			closecollection(&collection[i]);

			
			//ber bbdn om å lukke sokket
			bbdn_close(&collection[i].socketha);
   
    		}
                
                if (output_redirected) {
                    fclose(stderr); 
                    fclose(stdout);
                }

		crawl_unlock(&collection_lock);
		crawl_element_unlock(&collection_lock);

		blog(LOGACCESS,1,"Finished crawling of collection \"%s\" (id %u).",collection[i].collection_name,collection[i].id);

	}

	sm_collectionfree(&collection,nrofcollections);


}

int
rewriteurl(char *collection, char *uri, size_t len, enum platform_type ptype, enum browser_type btype) {

	struct crawlLibInfoFormat *crawlLibInfo;
	struct collectionFormat *collections;
	int nrofcollections;

	cm_searchForCollection(collection,&collections,&nrofcollections);

	if (!cm_getCrawlLibInfo(global_h,&crawlLibInfo,collections->connector)) {
		blog(LOGERROR,1,"Error: can't get CrawlLibInfo.");
		//exit(1);
		return 0;
	}

	if (crawlLibInfo->rewrite_url == NULL || !((*crawlLibInfo->rewrite_url)(uri, ptype, btype)))
		return 0;


	return 1;

}

void connectHandler(int socket) {

        struct packedHedderFormat packedHedder;
        int intresponse;
        int i,len;
	char collection[64];
	char *berrorbuf;
	struct timeval start_time_all, end_time_all;
	struct timeval start_time, end_time;
	
        printf("got new connection\n");

	while ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {

		gettimeofday(&start_time_all, NULL);

	        debug("size is: %i\nversion: %i\ncommand: %i\n",packedHedder.size,packedHedder.version,packedHedder.command);
	        packedHedder.size = packedHedder.size - sizeof(packedHedder);


		if (packedHedder.command == cm_crawlcollection) {
			char extrabuf[512];
			printf("crawlcollection\n");
			
			recvall(socket,collection,sizeof(collection));
			recvall(socket,extrabuf,sizeof(extrabuf));
			printf("collection \"%s\"\n",collection);

			struct collectionFormat *collections;
			int nrofcollections;

			cm_searchForCollection(collection,&collections,&nrofcollections);

			cm_setCrawStartMsg(collections,nrofcollections);

			//ikke mye nyttig som skjer her egentlig. Tvinger klienten bare til 
			//å vente her, slik at vi får satt noen statusmeldinger i databasen om 
			//at ting er på gang
			intresponse=1;
			sendall(socket,&intresponse, sizeof(int));

			crawl(collections,nrofcollections,crawl_crawl, extrabuf[0] != '\0' ? strdup(extrabuf) : NULL);


	        }
		else if (packedHedder.command == cm_collectionislocked) {
			int response;
			struct collection_lockFormat cl;
			recvall(socket,collection,sizeof(collection));

			if (!crawl_lock(&cl,collection)) {
				response = 1;
			} else {
				response = 0;
				crawl_unlock(&cl);
			}
			sendall(socket,&response, sizeof(response));
		}
		else if (packedHedder.command == cm_recrawlcollection) {
			printf("recrawlcollection\n");
			
			recvall(socket,collection,sizeof(collection));
			printf("collection \"%s\"\n",collection);

			struct collectionFormat *collections;
			int nrofcollections;

			cm_searchForCollection(collection,&collections,&nrofcollections);

			cm_setCrawStartMsg(collections,nrofcollections);

			//ikke mye nyttig som skjer her egentlig. Tvinger klienten bare til 
			//å vente her, slik at vi får satt noen statusmeldinger i databasen om 
			//at ting er på gang
			intresponse=1;
			sendall(socket,&intresponse, sizeof(int));

			crawl(collections,nrofcollections,crawl_recrawl, NULL);

		}
		else if (packedHedder.command == cm_deleteCollection) {
			printf("cm_deleteCollection\n");
			
			recvall(socket,collection,sizeof(collection));
			printf("collection \"%s\"\n",collection);

			struct collectionFormat *collections;
			int nrofcollections;


			//ikke mye nyttig som skjer her egentlig. Tvinger klienten bare til 
			//å vente her, slik at vi får satt noen statusmeldinger i databasen om 
			//at ting er på gang
			intresponse=1;
			sendall(socket,&intresponse, sizeof(int));

			bbdocument_deletecoll(collection);


		}
		else if (packedHedder.command == cm_scan) {
			printf("scan\n");
			char crawlertype[64];
			char host[64];
			char username[64];
			char password[64];
			char **shares;
			int nrofshares;
			char errormsg[512];

			recvall(socket,crawlertype,sizeof(crawlertype));
			recvall(socket,host,sizeof(host));
			recvall(socket,username,sizeof(username));
			recvall(socket,password,sizeof(password));

			printf("gor scan job:\ncrawlertype %s\nhost %s\nusername %s\npassword %s\n",crawlertype,host,username,password);

			if (!scan (global_h,&shares,&nrofshares,crawlertype,host,username,password)) {
				printf("aa cant scan\n");
				socketsendsaa(socket,&shares,0);
				printf("bb\n");
				sprintf(errormsg,"Can't scan.");

				len = (strlen(errormsg) +1);
				sendall(socket,&len, sizeof(int));			
				sendall(socket,&errormsg, len);			
				printf("snedet error msg \"%s\" of len %i\n",errormsg,len);
					
			}
			else {
				printf("scan ok. Found %i\n",nrofshares);

				for (i=0;i<nrofshares;++i) {
					printf("share %s\n",shares[i]);
					
				}

				socketsendsaa(socket,&shares,nrofshares);

			}

		}
		else if (packedHedder.command == cm_pathaccess) {
			//char all[512+64+64+64];
			char uri[512];
			char username[64];
                        char password[64];

			printf("cm_pathaccess: start\n");

#if 1
			gettimeofday(&start_time, NULL);
			recvall(socket,collection,sizeof(collection));
			gettimeofday(&end_time, NULL);
			printf("ttt time %f\n",getTimeDifference(&start_time,&end_time));
			printf("collection: \"%s\", size %i, len %i\n",collection,sizeof(collection),strlen(collection));

			recvall(socket,uri,sizeof(uri));

			recvall(socket,username,sizeof(username));
			recvall(socket,password,sizeof(password));
#else
			recvall(socket,all,sizeof(all));
#endif

			printf("collection: \"%s\"\nuri: \"%s\"\nusername \"%s\"\npassword \"%s\"\n",collection,uri,username,password);
			//printf("collection: \"%s\"\nuri: \"%s\"\nusername \"%s\"\npassword \"%s\"\n",all,all+64,all+64+512,all+64+64+512);


			//int pathAccess(struct hashtable *h, char connector[], char uri[], char username[], char password[])
			intresponse = pathAccess(global_h,collection,uri,username,password);
			//intresponse = pathAccess(global_h,all,all+64,all+64+512,all+64+64+512);

                        sendall(socket,&intresponse, sizeof(int));



			printf("cm_pathaccess: end\n");
		}
		else if (packedHedder.command == cm_crawlcanconect) {
			printf("crawlcanconect\n");

			recvall(socket,collection,sizeof(collection));

			printf("got collection name \"%s\" to crawl\n",collection);

			if (!cm_handle_crawlcanconect(collection)) {

				berrorbuf = bstrerror();	

				printf("cant conect! berrorbuf \"%s\"\n",berrorbuf);


				len = strlen(berrorbuf) +1;
        			sendall(socket,&len, sizeof(int));
			        sendall(socket,berrorbuf, len);

			}			
			else {
				berrorbuf = strdup("ok");

				len = strlen(berrorbuf) +1;
                                sendall(socket,&len, sizeof(int));
                                sendall(socket,berrorbuf, len);

				free(berrorbuf);
			}
			printf("crawlcanconect: done");
			
		}
		else if (packedHedder.command == cm_rewriteurl) {
			struct rewriteFormat rewrite;
			struct timeval start_time2, end_time2;

			puts("cm_rewriteurl start");

			//gettimeofday(&start_time2, NULL);
			recvall(socket, &rewrite, sizeof(rewrite));
			//gettimeofday(&end_time2, NULL);
			//printf("# 222 ### Time debug: sending url rewrite data %f\n",getTimeDifference(&start_time2,&end_time2));
	
			rewriteurl(rewrite.collection, rewrite.uri, sizeof(rewrite.uri), rewrite.ptype, rewrite.btype);

			if (sendall(socket, rewrite.uri, sizeof(rewrite.uri)) == 0) {
				perror("sendall(uri)");
			}

			puts("cm_rewriteurl end");
	        }
                else if (packedHedder.command == cm_killcrawl) {
                    int pid, ok;
                    char errmsg[256];
                    recvall(socket, &pid, sizeof pid);

                    // TODO: check if pid is a crawlerManager process?
                    ok = (pid > 0 && kill(pid, SIGKILL) != -1);
        	    sendall(socket, &ok, sizeof ok);

                    printf("cm_killcraw: killed pid %d: %s\n", pid, ok ? "yes" : "no");
                }
		else {
			 printf("unknown command. %i\n", packedHedder.command);
	        }

		gettimeofday(&end_time_all, NULL);
		printf("cm time %f\n",getTimeDifference(&start_time_all,&end_time_all));

	}
	printf("end of packed\n");
}

#ifdef WITH_PATHACCESS_CACHE
void
mc_add_servers(void)
{
	int i;

	mc = memcached_create(NULL);
	memcached_behavior_set(mc, MEMCACHED_BEHAVIOR_NO_BLOCK, NULL);
	for (i = 0; memcache_servers[i].addr != NULL; i++) {
		memcached_server_add(mc, memcache_servers[i].addr, memcache_servers[i].port);
	}
}
#endif

int main (int argc, char *argv[]) {
	struct config_t maincfg;
	struct config_t cmcfg;
	config_setting_t *cfgarr;

	#ifdef DEBUG
		printf("Ble kompilert med -DDEBUG\n");
	#endif

	if (!openlogs(&LOGACCESS,&LOGERROR,"crawlManager")) {
		perror("logs");
		exit(1);
	}

	printf("crawlManager: in main\n");

	printf("crawlManager: running maincfgopen\n");
	maincfg = maincfgopen();

	printf("crawlManager: running maincfg_get_int\n");
	int crawlport = maincfg_get_int(&maincfg,"CMDPORT");
	global_bbdnport = maincfg_get_int(&maincfg,"BLDPORT");
	printf("crawlManager: runing cm_start\n");


	/* Initialize the configuration */
        config_init(&cmcfg);

	/* Load the file */
        #ifdef DEBUG
	        printf("loading [%s]..\n",bfile("config/crawlmanager.conf"));
        #endif

        if (!config_read_file(&cmcfg, bfile("config/crawlmanager.conf"))) {
                printf("[%s]failed: %s at line %i\n",bfile("config/crawlmanager.conf"),config_error_text(&cmcfg),config_error_line(&cmcfg));
        	exit(1);
        }

	cfgarr = config_lookup(&cmcfg, "memcacheservers");
	int n_server = config_setting_length(cfgarr);
	
	if (n_server > 0) {
		int i;
		memcache_servers = calloc(n_server+1, sizeof(*memcache_servers));
		for (i = 0; i < n_server; i++) {
			memcache_servers[i].addr = strdup(config_setting_get_string_elem(cfgarr, i));
			memcache_servers[i].port = 11211;
		}
		memcache_servers[i].addr = NULL;
	} else {
		printf("No memcache servers specified, disabling pathaccess caching.\n");
		memcache_servers = NULL;
	}

	config_destroy(&cmcfg);

	cm_start(&global_h);

	sconnect(connectHandler, crawlport);
}
