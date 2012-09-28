
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
#include <err.h>

#if WITH_PATHACCESS_CACHE
#include <libmemcached/memcached.h>
#endif

#include "../logger/logger.h"

#include "../crawl/crawl.h"
#include "../key/key.h"

#include "../common/collection.h"
#include "../common/config.h"
#include "../common/define.h"
#include "../common/bstr.h"
#include "../common/daemon.h"
#include "../common/error.h"
#include "../common/timediff.h"
#include "../common/boithohome.h"
#include "../common/ht.h"
#include "../common/nice.h"
#include "../common/stdlib.h"
#include "../maincfg/maincfg.h"
#include "../boitho-bbdn/bbdnclient.h"

#include "../common/boithohome.h"
#include "../bbdocument/bbdocument.h"

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../common/pidfile.h"
#include "../boithoadClientLib/boithoad.h"

#include "../acls/acls.h"
#include "perlcrawl.h"
#include "../perlembed/perlembed.h"
#include "usersystem.h"
#include "shortenurl.h"


#define crawl_crawl 1
#define crawl_recrawl 2

/* MYSQL login information */
#define MYSQL_HOST "localhost"
#define MYSQL_USER "boitho"
#define MYSQL_PASS "G7J7v5L5Y7"

#define CRAWL_CRAWLING 1
#define CRAWL_DONE     2
#define CRAWL_ERROR    3
#define CRAWL_NOLAST   4

#define TEST_COLL_NAME "_%s_TestCollection" // %s is connector name.

#define BB_DATA_DIR "/boithoData"
#define MIN_DISK_REQUIRED 4194304 // 4GB in KB

struct hashtable *global_h, *usersystemshash;

struct {
	char *addr;
	int port;
} *memcache_servers;


int global_bbdnport;

#ifdef WITH_PATHACCESS_CACHE
void mc_add_servers(void);
#endif

#ifdef WITH_US_CACHE
	#include "../libcache/libcache.h"

	cache_t *cache;
	cache_t *cache_groups;
#endif



int cm_searchForCollection (MYSQL *db, char cvalue[],struct collectionFormat *collection[],int *nrofcollections);

void my_freevalue(void *p)
{
//        free(p);
}
void groups_freevalue(void *p)
{
	printf("groups_freevalue(%p, %p)\n",p, (*(char *)p));
        free(p);
	printf("~groups_freevalue\n");
}

void sql_connect(MYSQL *db) {
	if (!mysql_init(db)) {
		bblog(ERROR, "mysql_init, out of memory");
		exit(1);
	}

	if (!mysql_real_connect(db, MYSQL_HOST, MYSQL_USER, 
			MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)) {
		bblog(INFO, "%s", mysql_error(db));
		bblog(ERROR, "Mysql connect error: '%s'. Exiting.", mysql_error(db));
		exit(1);
	}
}



usersystem_t *
get_usersystem(MYSQL *db, unsigned int _id, usersystem_data_t *data)
{
	MYSQL_RES *res, *resparam;
	MYSQL_ROW row, rowparam;
	char query[1024], *p;
	size_t querylen;
	int n;
	struct hashtable *collections;
	struct hashtable_itr *itr;
	struct userToSubnameDbFormat userToSubnameDb;
	usersystem_container_t *usc;
	usersystem_t *us;

	// if someone askes for system nr 0, we shal return the primary user system
	if (_id == 0) {
		querylen = snprintf(query, sizeof(query), "SELECT is_primary, connector, id FROM system WHERE is_primary = 1");
	}
	else {
		querylen = snprintf(query, sizeof(query), "SELECT is_primary, connector,id FROM system WHERE id = %d", _id);
	}

	if (mysql_real_query(db, query, querylen)) {
		bblog(ERROR, "Mysql error: %s", mysql_error(db));
		return NULL;
	}

	res = mysql_store_result(db);
	n = mysql_num_rows(res);
	if (n == 0) {
		mysql_free_result(res);
		return NULL;
	}

	row = mysql_fetch_row(res);
	if (row == NULL) {
		bblog(ERROR, "Unable to fetch mysql row ar %s:%d",__FILE__,__LINE__);
		mysql_free_result(res);
		return NULL;
	}

	data->is_primary = atoi(row[0]);
	data->type = atoi(row[1]);
	data->id = atoi(row[2]);;

	usc = hashtable_search(usersystemshash, &data->type);
	if (usc == NULL) {
		bblog(ERROR, "No usersystem module for this type: %d", data->type);
		return NULL;
	}
	switch (usc->moduletype) {
	case USC_TYPE_C:
		us = usc->usersystem.us_c;
		break;
	case USC_TYPE_PERL:
		us = usc->usersystem.us_perl->us;
		break;
	}

	
	data->parameters = create_hashtable(7, ht_stringhash, ht_stringcmp);

	querylen = snprintf(query, sizeof(query), "SELECT param, value FROM systemParamValue WHERE system = %d", data->id);
	if (mysql_real_query(db, query, querylen)) {
		bblog(ERROR, "Mysql error: %s", mysql_error(db));
		return NULL;
	}

	resparam = mysql_store_result(db);
	n = mysql_num_rows(resparam);
	
	while ((rowparam = mysql_fetch_row(resparam)) != NULL) {
		if (!strcmp(rowparam[0], "password")) bblog(INFO, "Param: password => ********");
		else bblog(INFO, "Param: %s => %s",  rowparam[0], rowparam[1]);
		hashtable_insert(data->parameters, strdup(rowparam[0]), strdup(rowparam[1]));
	}

	mysql_free_result(res);
	mysql_free_result(resparam);

	data->usc = usc;

	return us;
}

void
free_usersystem_data(usersystem_data_t *data)
{
	hashtable_destroy(data->parameters, 1);
}

int documentContinue(struct collectionFormat *collection) {

	
	bblog(DEBUGINFO, "documentContinue: start");

	if ((collection->docsRemaining != -1) && (collection->docsCount >= collection->docsRemaining)) {
		snprintf(collection->errormsg, sizeof collection->errormsg, 
			"User specified document limit reached.");
		return 0;
	}

	int recrawl_schedule_start, recrawl_schedule_end;
	struct tm *t;
	time_t now;

        //sjekker om vi har nokk plass lokalt. Sjkker førte gang så hver hundrende gang.
	if ((collection->docsCount == 0) || ((collection->docsCount % 100) == 0)) {
		if (!bbdn_HasSufficientSpace(collection->socketha, collection->collection_name)) {
			snprintf(collection->errormsg, sizeof collection->errormsg,
				"Insufficient disk space. Can't crawl any more documents.");
			return 0;		
		}
	}

	//hvis vi skal crawl oftere en hvert dågn bruker vi ikke schedule time, men tilater å crawl hele tiden.
	if ( (collection->rate != 0) && (collection->rate < 1440)) {
		bblog(INFO, "documentContinue: Collection is set to be recrawled every %i min, ignoring schedule time", collection->rate);
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

	bblog(DEBUGINFO, "now: %i,recrawl_schedule_start %i,recrawl_schedule_end %i",t->tm_hour,recrawl_schedule_start,recrawl_schedule_end);

	//hvis vi ikke har noen begrensning så er det bare å crawler på
	if ((recrawl_schedule_start == 0) || (recrawl_schedule_end == 0)) {
		bblog(DEBUGINFO, "We can crawl att all times.");
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
	int sched_cont = 1;
	if ( recrawl_schedule_start > recrawl_schedule_end ) {

		if ((t->tm_hour < recrawl_schedule_start) && (t->tm_hour >= recrawl_schedule_end)) {
			bblog(INFO, "scenario 1: to early, wont crawl");
			sched_cont = 0;
		}

	}
	else {
		if (t->tm_hour < recrawl_schedule_start) {
			bblog(INFO, "scenario 2: to early, wont crawl");
			sched_cont = 0;
		}
		else if (t->tm_hour >= recrawl_schedule_end) {
			bblog(INFO, "scenario 2: to late, wont crawl");
			sched_cont = 0;
		}
	}
	if (!sched_cont) {
		snprintf(collection->errormsg, sizeof collection->errormsg,
			"Crawl is pending. Waiting for schedule time.");
		return 0;
	}

	bblog(DEBUGINFO, "hour is now %i, will crawl",t->tm_hour);

	bblog(DEBUGINFO, "documentContinue: end");

	return 1;
}

int documentExist(struct collectionFormat *collection, struct crawldocumentExistFormat *crawldocumentExist) {
	int ret;

	#ifdef DEBUG
	bblog(DEBUGINFO, "documentExist: start");
	#endif

	ret = bbdocument_exist(collection->collection_name, crawldocumentExist->documenturi, crawldocumentExist->lastmodified);

	#ifdef DEBUG
	bblog(DEBUGINFO, "documentExist: end");
	#endif

	return ret;
}


char *documentErrorGetlast(struct collectionFormat *collection) {
        return collection->errormsg;
}


int documentError(struct collectionFormat *collection,int level, const char *fmt, ...) {
        va_list     ap;

        va_start(ap, fmt);

	bblog(ERROR, "documentError: level: %d", level);
	bblogv(ERROR, fmt, ap);

	vsprintf(collection->errormsg ,fmt,ap);

        va_end(ap);
}

int documentAdd(struct collectionFormat *collection, struct crawldocumentAddFormat *crawldocumentAdd) {
	#ifdef DEBUG
	bblog(DEBUGINFO, "documentAdd start");
	#endif

	#ifdef DEBUG
	bblog(DEBUGINFO, "documentAdd: uri %s, title %s",(*crawldocumentAdd).documenturi,(*crawldocumentAdd).title);
	#endif

	collection->docsCount++;

        #ifdef DEBUG_TIME
                struct timeval start_time, end_time;
                gettimeofday(&start_time, NULL);
        #endif

	//send it inn
	if (!bbdn_docadd(collection->socketha,
				collection->collection_name,
				crawldocumentAdd->documenturi,
				crawldocumentAdd->documenttype,
				crawldocumentAdd->document,
				crawldocumentAdd->dokument_size,
				crawldocumentAdd->lastmodified,
				crawldocumentAdd->acl_allow,
				crawldocumentAdd->acl_denied,
				crawldocumentAdd->title,
				crawldocumentAdd->doctype,
				crawldocumentAdd->attributes)
	   ) {

		bblog(ERROR, "can't sent to bbdn! Tryed to send doc \"%s\" Will sleep and then reconect. Wont send same doc again.",(*crawldocumentAdd).documenturi);
		
		//ber om å lokke sokketen. Dette er ikke det samme som å steneg kollectionen.
		//bbdn_closecollection((*collection).socketha,(*collection).collection_name);
		bbdn_close((*collection).socketha);

		sleep(10);

		if (!bbdn_conect(&(*collection).socketha,"",global_bbdnport)) {
			bblog(ERROR, "can't connect to bbdn (boitho backend document server)");
			return 0;
		}

		//exit(1);

	}
	else {
		bblog(CLEAN, "crawled url: \"%s\", size: %i b, ACL: allow \"%s\", denied \"%s\"",(*crawldocumentAdd).documenturi,(*crawldocumentAdd).dokument_size,(*crawldocumentAdd).acl_allow,(*crawldocumentAdd).acl_denied);
	}

	printf("** Added url: \"%s\"\n", (*crawldocumentAdd).documenturi);

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		bblog(DEBUGINFO, "Time debug: bbdn_docadd() time: %f",getTimeDifference(&start_time, &end_time));
	#endif

	#ifdef DEBUG
	bblog(DEBUGINFO, "documentAdd end");
	#endif

	return 1;
}

int
collectionsforuser_collection(struct hashtable *collections, char *user, struct userToSubnameDbFormat *usertosubname)
{
	char subnamebuf[maxSubnameLength];
	int n_collections, i;
	char **list;

	#ifdef DEBUG
	bblog(DEBUGINFO, "collectionsforuser_collection:  user=%s", user);
	#endif

	if (!userToSubname_getsubnamesAsString(usertosubname, user, subnamebuf, sizeof(subnamebuf)))
		return 0;

	n_collections = split(subnamebuf, ",", &list);

	#ifdef DEBUG
	bblog(DEBUGINFO, "  n_collections=%i",  n_collections);
	#endif

	for (i = 0; i < n_collections; i++) {
		#ifdef DEBUG
	        bblog(DEBUGINFO, "    collection[%i] = %s",  i, list[i]);
	        #endif

		if (!hashtable_search(collections, list[i]))
			hashtable_insert(collections, strdup(list[i]), (void*)0x1);
	}

	FreeSplitList(list);

	return 0;
}


int _listGroupsForUser(usersystem_t *us, usersystem_data_t *data, const char *user, char ***groups, int *n_groups) {

	int i = 0;
	int j;
	int res;
	char *gstring, *ps;

#ifdef WITH_US_CACHE
	char *ckey;
	asprintf(&ckey,"%s:%p",user,us);

	ps = cache_fetch(cache_groups, "groupsForUser", ckey, 300);
	if (ps != NULL) {

		printf("_listGroupsForUser: Hav groups cached: \"%s\"\n",ps);


		char **Data;
		int Count;
		*n_groups = split(ps, ",", &Data);

		Count = 0;
		(*groups) = malloc(((*n_groups)+1) * sizeof(char*));
			
		while( (Data[Count] != NULL) ) {
			(*groups)[Count] = strdup(Data[Count]);
			Count++;
		}

		res = 1;
	}
	else {
#endif
		printf("_listGroupsForUser(\"%s\"): No cache!\n", user);

		res = (us->us_listGroupsForUser)(data, user, groups, n_groups);


		if ((res == 1) && (n_groups > 0)) {
	
			gstring = malloc( (*n_groups) * MAX_LDAP_ATTR_LEN );
			gstring[0] = '\0';
			ps = gstring;
			for (j = 0; j < *n_groups; j++) {
				if (j!=0) {
					ps += strlcpy(ps, ",", MAX_LDAP_ATTR_LEN);
				}
				ps += strlcpy(ps, (*groups)[j], MAX_LDAP_ATTR_LEN);
			}

			printf("gstring: %s\n",gstring);


		}
#ifdef WITH_US_CACHE
		if ((res == 1) && (n_groups > 0)) {
			if (cache_add(cache_groups, "groupsForUser", ckey, gstring, strlen(gstring) +1) != 1) { // +1 to save \0
				printf("_listGroupsForUser: Can't add to cache");
			}
		}
	}

	free(ckey);
#endif
	return res;

}

int
collectionsforuser(char *user, char **_collections, MYSQL *db)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char query[2][1024], *p;
	size_t querylen[2];
	int n;
	struct hashtable *collections;
	struct hashtable_itr *itr;
	struct userToSubnameDbFormat userToSubnameDb;
	int i, j;

	/* XXX: In lack of union select */
	querylen[0] = snprintf(query[0], sizeof(query[0]), "SELECT secnd_usr, system FROM systemMapping WHERE prim_usr = '%s'", user);
	querylen[1] = snprintf(query[1], sizeof(query[1]), "SELECT '%s' AS secnd_usr, id AS 'system' FROM system WHERE is_primary = 1", user);

	collections = create_hashtable(13, ht_stringhash, ht_stringcmp);

	for (i = 0; i < 2; i++) {
		if (mysql_real_query(db, query[i], querylen[i])) {
			bblog(ERROR, "Mysql error: %s", mysql_error(db));
			continue;
		}

		res = mysql_store_result(db);
		n = mysql_num_rows(res);
		if (n == 0) {
			mysql_free_result(res);
			continue;
		}

		if (!userToSubname_open(&userToSubnameDb,'r')) {
			bblog(WARN, "Warning! Can't open users.db");
			continue;
		}
		
		while ((row = mysql_fetch_row(res)) != NULL) {
			char **groups;
			int nrofcolls, n_groups;
			usersystem_t *us;
			usersystem_data_t data;

			if ((us = get_usersystem(db, atoi(row[1]), &data)) == NULL) {
				bblog(WARN, "No usersystem: %s %s", row[0], row[1]);
				continue;
			}
			if (!_listGroupsForUser(us, &data, row[0], &groups, &n_groups)) {
				bblog(WARN, "Unable to list groups for user: %s", row[0]);
				continue;
			}
			bblog(INFO, "Got %d groups for %s",  n_groups, row[0]);
			bblog(INFO, "Usersystem is %s",  row[1]);

			collectionsforuser_collection(collections, row[0], &userToSubnameDb);
			for (j = 0; j < n_groups; j++)
				collectionsforuser_collection(collections, groups[j], &userToSubnameDb);
			//boithoad_respons_list_free(groups);
			free_usersystem_data(&data);
		}

		mysql_free_result(res);
	}

	userToSubname_close(&userToSubnameDb);

	n = hashtable_count(collections);
	if (n == 0)
		return 0;
	p = *_collections = calloc(n, maxSubnameLength+1/*, and null terminator*/);

	// Get all collections...
	itr = hashtable_iterator(collections);
	do {
		char *name = hashtable_iterator_key(itr);
		size_t len;

		strcpy(p, name);
		len = strlen(name);
		p[len] = ',';
		p += len+1;
		bblog(INFO, "Collection: %s",  hashtable_iterator_key(itr));
	} while (hashtable_iterator_advance(itr));
	free(itr);
	p--;
	*p = '\0';

	hashtable_destroy(collections, 0);

	return n;
}

int sm_collectionfree(struct collectionFormat *collection[],int nrofcollections) {

	int i;

	for (i=0;i<nrofcollections;i++) {
		#ifdef DEBUG
		bblog(DEBUGINFO, "freeing nr %i: start", i);
		#endif
		free((*collection)[i].resource);
                free((*collection)[i].user);
                free((*collection)[i].password);
                free((*collection)[i].connector);
                free((*collection)[i].collection_name);
                free((*collection)[i].query1);
                free((*collection)[i].query2);
		free((*collection)[i].extra);
		free((*collection)[i].userprefix);
		free((*collection)[i].alias);
		hashtable_destroy((*collection)[i].params, 1);
		#ifdef DEBUG
			bblog(DEBUGINFO, "freeing nr %i: end", i);
		#endif
	}

	if ((*collection)) {
		free(*collection);
	}
}

int closecollection(struct collectionFormat *collection) {
	bblog(DEBUGINFO, "closecollection start");
	bbdn_closecollection((*collection).socketha,(*collection).collection_name);
	bblog(DEBUGINFO, "closecollection end");

}

int cmr_crawlcanconect(struct hashtable *h, struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		bblog(INFO, "can't get CrawlLibInfo");
		return 0;
	}

	bblog(DEBUGINFO, "wil crawl \"%s\"",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlcanconect)(collection,documentError)) {
		//overfører error
		bblog(ERROR, "crawlcanconect rountine in cralwer returned 0.");
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
                berror(collection->errormsg);
		return 0;
	}

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		bblog(ERROR, "Error: can't get CrawlLibInfo.");
		//exit(1);
		return 0;
	}

	collection->crawlLibInfo = crawlLibInfo;

	bblog(DEBUGINFO, "wil crawl \"%s\"",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlfirst)(collection,documentExist,documentAdd,documentError,documentContinue)) {
        	bblog(INFO, "problems in crawlfirst_ld");
		//overfører error
                berror( documentErrorGetlast(collection) );
		bblog(ERROR, "Error: Problems in crawlfirst_ld.");

		return 0;
       	}

	if (!documentContinue(collection)) {
                berror(collection->errormsg);
		return 0;
	}
	
	return 1;

}

int cm_crawlupdate(struct hashtable *h,struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!documentContinue(collection)) {
                berror(collection->errormsg);
		return 0;
	}

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		bblog(ERROR, "Error: can't get CrawlLibInfo.");
		exit(1);
	}

	collection->crawlLibInfo = crawlLibInfo;

	bblog(DEBUGINFO, "wil crawl \"%s\"",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlupdate)(collection,documentExist,documentAdd,documentError,documentContinue)) {
        	
		//overfører error
                berror( documentErrorGetlast(collection) );
		bblog(ERROR, "Error: problems in crawlfirst_ld.");

		return 0;
       	}

	if (!documentContinue(collection)) {
                berror(collection->errormsg);
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
	bblog(INFO, "Key: %s",  s);
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
		bblog(INFO, "Unable to add new cache item.");

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

int requires_path_access(struct collectionFormat * coll) {
	return !(coll->user == NULL && coll->password == NULL);
}

int pathAccess(MYSQL *db, struct hashtable *h, char collection[], char uri[], char username_in[], char password[]) {
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

	bblog(INFO, "pathAccess: start");

	//temp:
	//26.0207:quiq fix. Lagger til domene i brukernav
	/*
	char username_t[64];
	strcpy(username_t,"i04\\");
	strcat(username_t,username);
	strcpy(username,username_t);
	*/

	gettimeofday(&start_time, NULL);
	bblog(DEBUGINFO, "cm_searchForCollection");
	if (!cm_searchForCollection(db, collection,&collections,&nrofcollections)) {
		bblog(INFO, "cant't find Collection \"%s\"in db at %s:%d", collection,__FILE__,__LINE__);
		// desverre må vi bare returnere tilgang her, da vi ikke har en collection å mappe dette mot. Kan være på 24so hvor
		// vi ikke har database entrees for alle collectionene.
		return 1;
	}

	//skal returnere 1, og bare 1, hvis ikke har vi fått flere en 1, er det noe feil
	// hvis vi fikk 0 treff, returnerte cm_searchForCollection() 0.
	if (nrofcollections != 1) {
		bblog(INFO, "error looking opp collection \"%s\"", collection);
		return 0;
	}

	if (!requires_path_access(&collections[0])) {
		bblog(INFO, "Collection '%s' requires no path access.", collection);
		return 1;
	}

	gettimeofday(&end_time, NULL);
	pathAccessTimes.searchForCollection = getTimeDifference(&start_time,&end_time);


	gettimeofday(&start_time, NULL);
	bblog(DEBUGINFO, "cm_getCrawlLibInfo");
	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,collections[0].connector)) {
		bblog(INFO, "can't get CrawlLibInfo");
		return 1;
	}
	collections[0].crawlLibInfo = crawlLibInfo;

	gettimeofday(&end_time, NULL);
	pathAccessTimes.getCrawlLibInfo = getTimeDifference(&start_time,&end_time);

	bblog(INFO, "wil pathAccess \"%s\", for user \"%s\"", uri,username_in);

	gettimeofday(&start_time, NULL);

	username = adduserprefix(collections,username_in);

	bblog(INFO, "username after adduserprefix: \"%s\"", username);

	origuri = strdup(uri);
#ifdef WITH_PATHACCESS_CACHE
	if (memcache_servers != NULL)
		mc_add_servers();
#endif
	if ((*crawlLibInfo).crawlpatAcces == NULL) {
		bblog(INFO, "cralwer her ikke crawlpatAcces. returnerer tilgang. Må i fremtiden slå det opp");
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
        	bblog(INFO, "Can't crawlLibInfo. Can be denyed or somthing else");
		//overfører error
                berror("%s", documentErrorGetlast(&collections[0]));

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


	bblog(INFO, "pathAccess: times");	
	bblog(INFO, "\tsearchForCollection: %f", pathAccessTimes.searchForCollection);
	bblog(INFO, "\tgetCrawlLibInfo: %f", pathAccessTimes.getCrawlLibInfo);
	bblog(INFO, "\tcrawlpatAcces: %f", pathAccessTimes.crawlpatAcces);

	bblog(INFO, "pathAccess: end");

	if (forreturn == 1) {
		bblog(INFO,"pathAccess allowed url: \"%s\", user: \"%s\", time used %f s",uri,username,pathAccessTimes.crawlpatAcces);
	}
	else {
		bblog(INFO, "pathAccess denyed url: \"%s\", user: \"%s\", time used %f s, Error: \"%s\"",uri,username,pathAccessTimes.crawlpatAcces, documentErrorGetlast( &collections[0]) );
	}

	free(username);
	sm_collectionfree(&collections,nrofcollections);
	
	return forreturn;

}





/************************************************************
ToDo: dette er HØYST midlertidig. Bruker globale vvariabler her, og ingen free()'ing
************************************************************/

static char *found_share[32];
static char found_sharenr = 0;

int scan_found_share(char share[]) {

	bblog(INFO, "add \"%s\" (nr %i)", share,found_sharenr);
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
		bblog(ERROR, "Error: can't get CrawlLibInfo.");

                exit(1);
        }

	if ((*crawlLibInfo).scan == NULL) {
		bblog(INFO, "can't scan. Crawler dosen't support it.");
		bblog(ERROR, "Error: can't scan. Crawler dosent suport it.");

		return 0;
	}

	bblog(INFO, "scan:\nhost %s\nusername %s\npassword %s", host,username,password);

	scan_found_start();

	if (!(*(*crawlLibInfo).scan)(scan_found_share,host,username,password,documentError)) {
                bblog(INFO, "problems in scan");
		bblog(ERROR, "Error: problems in scan.");

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

void
list_usersystems(struct hashtable *us)
{
	struct hashtable_itr *itr;

	if (hashtable_count(us) > 0) {
		char *name;
		usersystem_container_t *usc;
		usersystem_t *us_c;
		usersystem_perl_t *us_perl;
		usersystem_data_t d;

		itr = hashtable_iterator(us);

		do {
			usc = hashtable_iterator_value(itr);

			switch (usc->moduletype) {
			case USC_TYPE_C:
				us_c = usc->usersystem.us_c;
				name = us_c->us_getName(NULL);
				bblog(INFO, "C usersystem: %s(%d)",  name, us_c->type);
				free(name);
				break;
			case USC_TYPE_PERL:
				us_perl = usc->usersystem.us_perl;
				bblog(INFO, "perl usersysten: %s",  us_perl->us->us_getName(us_perl));
				d.usc = usc;
				break;
			}
		} while (hashtable_iterator_advance(itr));
	}
}

/* Perl default usersystem_t */
usersystem_t perl_usersystem = {
	US_TYPE_INHERIT,
	us_authenticate_perl,
	us_listUsers_perl,
	us_listGroupsForUser_perl,
	us_getName_perl,
};

int
load_usersystem(struct hashtable *us, char *name)
{
	char libpath[PATH_MAX], folderpath[PATH_MAX],
	    perlpath[PATH_MAX], idpath[PATH_MAX];
	usersystem_container_t *usc;
	unsigned int fileid;
	FILE *fpid;

	int insert_us(usersystem_container_t *usc, unsigned int type) {
		if (hashtable_search(us, &type) != NULL) {
			bblog(WARN, "We already have a usersystem with id: %s: %d", name, type);
			return 0;
		}
		hashtable_insert(us, uinttouintp(type), usc);
		bblog(INFO, "Loaded %s(%d)",  name, type);

		return 1;
	}

	sprintf(folderpath, "%s/%s", bfile("usersystems"), name);
	sprintf(libpath, "%s/%s.so", folderpath, name);
	sprintf(perlpath, "%s/main.pm", folderpath, name);
	sprintf(idpath, "%s/id", folderpath, name);
	usc = malloc(sizeof(*usc));

	fileid = 0;
	if ((fpid = fopen(idpath, "r"))) {
		char buf[1024];
		size_t len;

		len = fread(buf, 1, sizeof(buf), fpid);
		if (len > 0) {
			buf[len] = '\0';
			fileid = strtol(buf, NULL, 10);
		}
		
		fclose(fpid);
	}

	if (file_exist(libpath)) {
		void *libhandle;
		usersystem_t *usersystem;
		char *error;

		libhandle = dlopen(libpath, RTLD_LAZY);
		if (libhandle == NULL) {
			bblog(WARN, "Unable to load crawler: %s", name);
			return 0;
		}

		usersystem = dlsym(libhandle, "usersystem_info");
		if ((error = dlerror()) != NULL) {
			bblog(WARN, "Unable to get usersystem_info for %s: %s", name, error);
			dlclose(libhandle);
			return 0;
		}

		usc->usersystem.us_c = usersystem;
		usc->moduletype = USC_TYPE_C;

		if (insert_us(usc, usersystem->type) == 0) {
			bblog(WARN, "C insert failed");
			dlclose(libhandle);
			free(usc);
			return 0;
		}
	} else if (file_exist(perlpath)) {
		usersystem_perl_t *usersystem;
		unsigned int type;
		int retn;

		if (fileid == 0) {
			bblog(ERROR, "Perl usersystem require a file id");
			exit(1);
		}
		
		type = fileid;
		usersystem = malloc(sizeof(*usersystem));
		usersystem->perlpath = strdup(perlpath);
		usersystem->us = &perl_usersystem;
		usersystem->type = type;
		usc->usersystem.us_perl = usersystem;
		usc->moduletype = USC_TYPE_PERL;

		if (insert_us(usc, type) == 0) {
			bblog(WARN, "perl insert failed");
			free(usersystem->perlpath);
			free(usersystem);
			free(usc);
			return 0;
		}
	} else {
		free(usc);
		bblog(WARN, "No usersystem found with name: '%s'", name);
		return 0;
	}

	return 1;
}

void
load_usersystems(struct hashtable **usersystems)
{
	DIR *dirp;
	struct dirent *dp;
	char path[PATH_MAX];

	bblog(INFO, "Looking for users systems...");
	(*usersystems) = create_hashtable(21, ht_integerhash, ht_integercmp);
	if (*usersystems == NULL) {
		bblog(ERROR, "Unable to create usersystem hashtable");
		exit(1);
	}

	strcpy(path, bfile("usersystems"));
	if ((dirp = opendir(path)) == NULL) {
		bblog(ERROR, "No usersystems available: %s", path);
		exit(1);
	}

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.')
			continue;

		load_usersystem((*usersystems), dp->d_name);
	}

	closedir(dirp);
}
//rutine for å laste et crawler biblotek
int cm_loadCrawlLib(struct hashtable **h, char name[]) {

	bblog(DEBUGINFO, "cm_loadCrawlLib(name=%s)",name);


	char libpath[PATH_MAX];
	char perlpath[PATH_MAX];
	char folderpath[PATH_MAX];
	void* lib_handle;       /* handle of the opened library */
	struct crawlLibInfoFormat *crawlLibInfo;
	const char* error_msg;


	sprintf(libpath,"%s/%s/%s.so",bfile("crawlers"),name,name);	
	sprintf(perlpath,"%s/%s/main.pm",bfile("crawlers"),name);	
	sprintf(folderpath,"%s/%s/",bfile("crawlers"),name);	
	


	if (file_exist(libpath)) {
		bblog(INFO, "loading path \"%s\"", libpath);
		lib_handle = dlopen(libpath, RTLD_LAZY);
		if (!lib_handle) {
			bblog(ERROR, "Error: during dlopen(): %s. File %s.",dlerror(),libpath);
		    	//exit(1);
			return 0;
		}

	
		crawlLibInfo = dlsym(lib_handle, "crawlLibInfo");

		/* check that no error occured */
        	error_msg = dlerror();
	        if (error_msg) {
		bblog(ERROR, "Error: Error locating '%s' - %s.",name, error_msg);
        	    exit(1);
        	}

		
		bblog(INFO, "loaded \"%s\"", (*crawlLibInfo).shortname);

		if ((*crawlLibInfo).crawlinit == NULL) {

		}
		else if (!(*crawlLibInfo).crawlinit()) {
			bblog(INFO, "crawlinit dident return 1");
			bblog(ERROR, "Error: crawlinit dident return 1.");
			exit(1);
		}

		//sjekk at den ikke finnes i hashen fra før
		if (hashtable_search((*h),(*crawlLibInfo).shortname) != NULL) {
			bblog(INFO, "all redy have module with shortname \"%s\"", (*crawlLibInfo).shortname);
			return 1;
		}

		if (! hashtable_insert((*h),(*crawlLibInfo).shortname,crawlLibInfo) ) {                        
			bblog(ERROR, "Error: can't hastable insert.");
               		exit(-1);
               	}

		//alt ok. Legg det i en hash

		//dlclose(lib_handle); 
	}		
	else if (file_exist(perlpath)) {

		crawlLibInfo = perlCrawlStart(folderpath, name);

		bblog(INFO, "loaded \"%s\"", (*crawlLibInfo).shortname);

		//sjekk at den ikke finnes i hashen fra før
		if (hashtable_search((*h),(*crawlLibInfo).shortname) != NULL) {
			bblog(INFO, "all redy have module with shortname \"%s\"", (*crawlLibInfo).shortname);
			return 1;
		}

		if (! hashtable_insert((*h),(*crawlLibInfo).shortname,crawlLibInfo) ) {                        
			bblog(ERROR, "Error: can't hastable insert.");
               		exit(-1);
               	}

	}
	else {
		bblog(ERROR, "can't load \"%s\" crawler.",name);
		return 0;
	}

	return 1;

}

int cm_end() {
	perl_embed_clean();
}
int cm_start(struct hashtable **h, struct hashtable **usersystems) {

	bblog(INFO, "cm_start start");

	DIR *dirp;
	struct dirent *dp;

	(*h) = create_hashtable(20, cm_hashfromkey, cm_equalkeys);

	const char *perl_incl[] = { bfile("crawlers/Modules"), NULL };                                                                 
	perl_embed_init(perl_incl, 1);

	if ((dirp = opendir(bfile("crawlers"))) == NULL) {
		perror(bfile("crawlers"));
		bblog(ERROR, "Error: can't open crawlers directory.");

		exit(1);
	}	

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.') {
			continue;
		}
		if (strcmp(dp->d_name,"Modules") == 0) {
			bblog(DEBUGINFO, "Skipping system folder %s",dp->d_name);
			continue;
		}

		cm_loadCrawlLib(h, dp->d_name);

	}

	closedir(dirp);

	load_usersystems(usersystems);
	//list_usersystems(*usersystems);

	bblog(INFO, "cm_start end");


}


/************************************************************/

int cm_getCrawlLibInfo(struct hashtable *h,struct crawlLibInfoFormat **crawlLibInfo,char shortname[]) {
	bblog(DEBUGINFO, "cm_getCrawlLibInfo: start");
	bblog(DEBUGINFO, "wil search for \"%s\"",shortname);
	if (((*crawlLibInfo) = (struct crawlLibInfoFormat *)hashtable_search(h,shortname)) != NULL) {
		return 1;
        }
	else if ((cm_loadCrawlLib(&h, shortname)) && (((*crawlLibInfo) = (struct crawlLibInfoFormat *)hashtable_search(h,shortname)) != NULL)) {
		//hvis vi ikke kunne finne det i hashen prøver vi å laset på ny
		bblog(DEBUGINFO, "Hadde ikke %s fra før, men fikk til å laste den.",shortname);
		return 1;
	}
	else {
		berror("don't have a crawler for \"%s\"\n",shortname);
		bblog(ERROR, "don't have a crawler for \"%s\"",shortname);

		return 0;
	}
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
               	bblog(INFO, "%s", mysql_error(db));
		bblog(ERROR, "MySQL Error: \"%s\".",mysql_error(db));
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
		bblog(DEBUGINFO, "nrofrows %i", numUsers);
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
sql_fetch_params(MYSQL *db, struct hashtable ** h, unsigned int coll_id)  {
    char mysql_query[512];
    MYSQL_ROW row;
    (*h) = create_hashtable(7, cm_hashfromkey, cm_equalkeys);
    if ((*h) == NULL) {
	perror("Fail to creat hash table to hold sql params");
	exit(-1);
    }

    snprintf(mysql_query, sizeof mysql_query, "\
        SELECT shareParam.value, param.param \
        FROM shareParam, param \
        WHERE shareParam.param = param.id \
        AND shareParam.share = %d", coll_id);

    if (mysql_real_query(db, mysql_query, strlen(mysql_query))) {
        bblog(INFO, mysql_error(db));
	bblog(ERROR, "MySQL Error: \"%s\".",mysql_error(db));
        exit(1);
    }

    MYSQL_RES * res = mysql_store_result(db);
    while ((row = mysql_fetch_row(res)) != NULL) {
        if (!hashtable_insert(*h, strdup(row[1]), strdup(row[0]))) {
            bblog(INFO, "can't insert param into ht");
            exit(1);
        }
    }
    mysql_free_result(res);
}

void
sql_set_crawl_pid(MYSQL *db , int *pid, unsigned int coll_id) {
	char query[1024];
	char pid_str[64];
	if (pid == NULL) 
		sprintf(pid_str, "NULL");
	else
		snprintf(pid_str, sizeof pid_str, "'%d'", *pid);

	snprintf(query, sizeof query, "UPDATE shares SET crawl_pid = %s WHERE id = '%d'",
		pid_str, coll_id);

	if (mysql_real_query(db, query, strlen(query)))
		bblog(ERROR, "mysql Error: %s", mysql_error(db));
}

int
sql_coll_by_pid(int crawl_pid) {
	char query[1024];
	int coll_id;
	MYSQL db;

	sql_connect(&db);

	snprintf(query, sizeof query, 
		"SELECT id FROM shares WHERE crawl_pid = '%d'",
		crawl_pid);
    
	MYSQL_ROW row;
	if (mysql_real_query(&db, query, strlen(query))) {
		bblog(ERROR, "mysql error: '%s'", mysql_error(&db));
        exit(1);
    }
    MYSQL_RES * res = mysql_store_result(&db);

	row = mysql_fetch_row(res);
	coll_id = (row == NULL) ? -1 : atoi(row[0]);
	mysql_free_result(res);

	mysql_close(&db);


	return coll_id;
}


int cm_searchForCollection(MYSQL *db, char cvalue[],struct collectionFormat *collection[],int *nrofcollections) {

        char mysql_query [2048];
	MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;

        int i,y;
	
	if (cvalue != NULL) {
            char cvalue_esc[128];
            mysql_real_escape_string(db, cvalue_esc, cvalue, strlen(cvalue));
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
						shares.rate, \
						shares.system, \
						shares.alias, \
						shares.crawl_pid \
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
						shares.rate, \
						shares.system, \
						shares.alias, \
						shares.crawl_pid \
					from \
						shares,connectors \
					where \
						connectors.ID = shares.connector \
					");
	}

	bblog(DEBUGINFO, "mysql_query: %s",mysql_query);

	if(mysql_real_query(db, mysql_query, strlen(mysql_query))){ /* Make query */
               	bblog(INFO, "%s", mysql_error(db));
		bblog(ERROR, "MySQL Error: \"%s\" '''%s'''.",mysql_error(db), mysql_query);
		
               	exit(1);
       	}
	mysqlres=mysql_store_result(db); /* Download result from server */

	(*nrofcollections) = (int)mysql_num_rows(mysqlres);

	if ((*nrofcollections) == 0) {
		bblog(INFO, "dident find any rows");
		(*collection) = NULL;		
		return 0;
	}
	else {

	bblog(DEBUGINFO, "nrofrows %i",*nrofcollections);

	(*collection) = malloc(sizeof(struct collectionFormat) * (*nrofcollections));

	//resetter minne
	for (i=0;i<(*nrofcollections);i++) {
		collectionReset (collection[i]);
	}

        i=0;
        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
        	bblog(DEBUGINFO, "  data resource: %s, connector: %s, collection_name: %s, lastCrawl: %s, userprefix: %s, auth_id: %s",mysqlrow[0],mysqlrow[1],mysqlrow[2],mysqlrow[3],mysqlrow[8],mysqlrow[6]);
		
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

		//runarb: 19 jul 2012 usikker på om dette er riktig. Vi sjekker ikke noen plass om usersystem er -1.
		if (mysqlrow[10] == NULL) {
			(*collection)[i].usersystem = -1;
		}
		else {
			(*collection)[i].usersystem = strtoul(mysqlrow[10], (char **)NULL, 10);
		}

		if (mysqlrow[11] == NULL) {
			(*collection)[i].alias = NULL;
		} else {
			(*collection)[i].alias = strdup(mysqlrow[11]);
		}
		(*collection)[i].extra = NULL;

	
		if (mysqlrow[12] == NULL) {
			(*collection)[i].pid = 0;
		}
		else {
			(*collection)[i].pid = strtoul(mysqlrow[12], (char **)NULL, 10);
		}
		//normaliserer collection navn, ved å fjenre ting som space og - \ / etc
		collection_normalize_name((*collection)[i].collection_name,strlen((*collection)[i].collection_name));


		bblog(DEBUGINFO, "resource \"%s\", connector \"%s\", collection_name \"%s\"",(*collection)[i].resource,(*collection)[i].connector,(*collection)[i].collection_name);

		cm_collectionFetchUsers(collection[i], db);
                sql_fetch_params(db, &(*collection)[i].params, (*collection)[i].id);

		//crawler ny
                ++i;
        }


        mysql_free_result(mysqlres);

	/***********************************************************************/
	// slår opp bruker id hvis vi har det, hvis ikke skal user og password være NULL	
	for (i=0;i<(*nrofcollections);i++) {

		sprintf(mysql_query,"select username,password from collectionAuth where id = %i",(*collection)[i].auth_id);

		bblog(DEBUGINFO, "mysql_query: %s",mysql_query);

		if(mysql_real_query(db, mysql_query, strlen(mysql_query))){ /* Make query */
	               	bblog(INFO, "%s", mysql_error(db));
	               	exit(1);
	       	}
		mysqlres=mysql_store_result(db); /* Download result from server */

		(*collection)[i].user = NULL;
		(*collection)[i].password = NULL;
        
	        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
	        	bblog(DEBUGINFO, "\tUser \"%s\", Password \"%s\"",mysqlrow[0],mysqlrow[1]);
			(*collection)[i].user 			= strdup(mysqlrow[0]);	
			(*collection)[i].password 		= strdup(mysqlrow[1]);	
		
	        }
	}

        mysql_free_result(mysqlres);

	//inaliserer andre ting
	for (i=0;i<(*nrofcollections);i++) {
		(*collection)[i].errormsg[0] = '\0';
		(*collection)[i].docsRemaining = -1;
		(*collection)[i].docsCount = 0;
		key_get((*collection)[i].systemkey);
	}

	/***********************************************************************/

	}


	bblog(DEBUGINFO, "cm_searchForCollection: end");
	return 1;
	
}

int cm_handle_crawlcanconect(MYSQL *db, char cvalue[]) {

	struct collectionFormat *collections = NULL;
	int nrofcollections;
	int i;

	bblog(INFO, "cm_handle_crawlcanconect (%s)", cvalue);

	cm_searchForCollection(db, cvalue,&collections,&nrofcollections);

	bblog(INFO, "crawlcanconect: cm_searchForCollection done. Found %i collection(s)", nrofcollections);
	if (nrofcollections == 1) {
		
		#ifndef DEBUG
		//Temp: funger ikke når vi kompilerer debug. Må også compilere crawlManager debug
		//make a conectina for add to use
		if (!bbdn_conect(&collections[0].socketha,"",global_bbdnport)) {
			berror("can't conect to bbdn (boitho backend document server)\n");
			bblog(ERROR, "can't conect to bbdn (boitho backend document server)");
			return 0;
		}

		if (!cmr_crawlcanconect(global_h,&collections[0])) {
			return 0;
		}

		//ber bbdn om å lukke
		bblog(INFO, "closeing bbdn conn");
                bbdn_close(collections[0].socketha);
		#endif
	}
	else {
		bblog(INFO, "crawlcanconect: Error got back %i coll to crawl", nrofcollections);
	}


	sm_collectionfree(&collections,nrofcollections);

	return 1;
}

void set_crawl_state(int state, unsigned int coll_id, char * msg) {
	
	MYSQL db;
	sql_connect(&db);

	switch (state) {
		case CRAWL_CRAWLING:
			sql_set_crawler_message(&db, 0, "Crawling it now.", coll_id);
			
			int pid = getpid();
			sql_set_crawl_pid(&db, &pid, coll_id);
			break;

		case CRAWL_DONE:
			sql_set_crawler_message(&db, 1, "OK.", coll_id);
			sql_set_crawl_pid(&db, NULL, coll_id);
			break;

		case CRAWL_ERROR:
			sql_set_crawler_message(&db, 0, msg, coll_id);
			sql_set_crawl_pid(&db, NULL, coll_id);
			break;

		case CRAWL_NOLAST:
			sql_set_crawler_message(&db, 0, NULL, coll_id);
			break;

		default:
			bblog(ERROR, "Unknown crawl state %d line: %s file: %s", 
				state, __LINE__, __FILE__);
			exit(1);
	}

	mysql_close(&db);
}

//crawler_success  skal være 0 hvis vi feilet, og 1 hvis vi ikke feilet
int sql_set_crawler_message(MYSQL *db, int crawler_success  , char mrg[], unsigned int id) {

        char mysql_query [2048];
	char messageEscaped[2048];
        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;

	bblog(INFO, "Status: set_crawler_message: mesage: \"%s\", success: %i, id: %i.",mrg,crawler_success,id);


	//printf("mysql_queryEscaped \"%s\"\n",messageEscaped);
	if (mrg == NULL) {
        	sprintf(mysql_query, "UPDATE shares SET last = NULL WHERE id = '%d'",id);
	} 
	else if (crawler_success  == 0) {
		//escaper queryet rikit
		mysql_real_escape_string(db,messageEscaped,mrg,strlen(mrg));
        	sprintf(mysql_query, "UPDATE shares SET crawler_success = \"%i\", crawler_message = \"%s\" WHERE id  = \"%u\"",crawler_success ,messageEscaped,id);
	}
	else {
		//escaper queryet rikit
		mysql_real_escape_string(db,messageEscaped,mrg,strlen(mrg));
        	sprintf(mysql_query, "UPDATE shares SET crawler_success = \"%i\", crawler_message = \"%s\", last = now() WHERE id  = \"%u\"",crawler_success ,messageEscaped,id);

	}

	bblog(INFO, "Status: \"%s\".",mysql_query);

        //debug("mysql_query: %s\n",mysql_query);

        if(mysql_real_query(db, mysql_query, strlen(mysql_query))){ /* Make query */
                //printf(mysql_error(&demo_db));
		bblog(ERROR, "Mysql Error: \"%s\".",mysql_error(db));
                exit(1);
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

	bblog(DEBUGINFO, "locking lock \"%s\"",(*collection_lock).lockfile);

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

	bblog(DEBUGINFO, "locking lock \"%s\"",(*collection_lock).elementlockfile);

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
        bblog(ERROR, "No output file for test coll");
        return 0;
    }

    // Deliting old version
    unlink(file);

    bblog(INFO, "tc: redirecting std[out,err] to %s",  file);
    if ((freopen(file, "a+", stdout)) == NULL
            || freopen(file, "a+", stderr) == NULL) {
        perror(file);
        return 0;
    }
    return 1;
}

int crawl(struct collectionFormat *collection,int nrofcollections, int flag, int docsRemaining, char *extra) {

	int i;
	FILE *LOCK;

	struct collection_lockFormat collection_lock;
	

	for(i=0;i<nrofcollections;i++) {
		if (collection[i].extra != NULL)
			free(collection[i].extra);
		collection[i].extra = extra;
		collection[i].docsRemaining = docsRemaining;

		bblog(ERROR,"Starting crawl of collection \"%s\" (id %u).",collection[i].collection_name,collection[i].id);

#ifdef BLACK_BOKS
		long long left = kbytes_left_in_dir(BB_DATA_DIR);
		if (left == -1)
			bblog(WARN, "Unable to check space left in lots.");
		else if (left < MIN_DISK_REQUIRED) {
			set_crawl_state(CRAWL_ERROR, collection[i].id, "Crawl request ignored. Not enough disk space.");
			bblog(ERROR, "Not enough disk space to craw, space left: %fMB", left / 1024.0);
			continue;
		} 
		else { 
			// printf("Disk space left: %fMB\n", left / 1024.0); 
		}
#endif


        int output_redirected = 0;
        if (collection[i].extra != NULL) {
            if (!redirect_stdoutput(collection[i].extra)) {
                bblog(ERROR, "test collection error, skipping.");
                continue;
            }
            
            // a lock implies that a crawl is still running
            flock(fileno(stdout), LOCK_SH); 
            setvbuf(stdout, NULL, _IOLBF, 0); // line buffered
            setvbuf(stderr, NULL, _IOLBF, 0);
            output_redirected = 1;
        }

		//sletter collection. Gjør dette uavhenging om vi har lock eller ikke, slik at vi altid får slettet, så kan vi gjøre
		// ny crawl etterpå hvis vi ikke hadde lock
		if (flag == crawl_recrawl) {
			bblog(DEBUGINFO, "crawl_recrawl");

			char querybuf[1024];
			int querylen;

			// If we have a pid from befor, we are probobly crawling it. Start by killing the old crawler.
			if (collection[i].pid != 0) {
				kill(collection[i].pid, SIGKILL);
				collection[i].pid = 0;
			}
			set_crawl_state(CRAWL_NOLAST, collection[i].id, NULL);


			//nullsetter lastCrawl, som er den verdien som viser siste crawl. 
			//Tror ikke crawlfirst skal ta hensysn til den, men gjør det for sikkerhets skyld
			(*collection).lastCrawl = 0;

			//beordrer en sletting. Burde kansje vært gjort vi bbdn, ikke direkte, men skit au
			bbdocument_deletecoll(collection[i].collection_name);
		}

		//tester at vi ikke allerede holder på å crawle denne fra før
		if (!crawl_lock(&collection_lock,collection[i].collection_name)) {
			bblog(ERROR, "Error: Can't crawl collection \"%s\". We are already crawling it.",collection[i].collection_name);
			//runarb: 14 jan 2008: ingen grun til å oppdatere beskjeden, da det som står der er med korekt
                        //set_crawler_message(0,"Error: Can't crawl collection. We are all redy crawling it.",collection[i].id);

			continue;
		}

		//tester at vi ikke driver å crawler med den crawleren fra før. Her burde vi kansje 
		//heller satset på å begrense på server. Slik at for eks to smb servere kan crawles samtidig
		if (!crawl_element_lock(&collection_lock,collection[i].connector)) {
			bblog(ERROR, "Error: Can't crawl collection \"%s\". We are already crawling this type/server.",collection[i].collection_name);
			set_crawl_state(CRAWL_ERROR, collection[i].id,
				"Can't crawl collection. We are already crawling this type/server.");

			continue;
		}
		
		//setter at vi har begynt og cralwe
		set_crawl_state(CRAWL_CRAWLING, collection[i].id, NULL);
	
		//make a conectina to bbdn for add to use
		if (!bbdn_conect(&collection[i].socketha,"",global_bbdnport)) {
			berror("can't connect to bbdn (boitho backend document server)");
			set_crawl_state(CRAWL_ERROR, collection[i].id, bstrerror());
		}
		else {
			int success = 0;

			bbdn_opencollection(collection[i].socketha, collection[i].collection_name);

			if (flag == crawl_recrawl || (*collection).lastCrawl == 0) {
				if (cm_crawlfirst(global_h,&collection[i])) { 
					set_crawl_state(CRAWL_DONE, collection[i].id, NULL);
					success = 1;
				} else {
					set_crawl_state(CRAWL_ERROR, collection[i].id, bstrerror());
				}
			}
			else {
				if (cm_crawlupdate(global_h,&collection[i])) {
					set_crawl_state(CRAWL_DONE, collection[i].id, NULL);
					success = 1;
				} else {
					set_crawl_state(CRAWL_ERROR, collection[i].id, bstrerror());
				}
			}

			closecollection(&collection[i]);

			//ber bbdn om å lukke sokket
			bbdn_close(collection[i].socketha);
   
    		}
                
		if (output_redirected) {
			fclose(stderr); 
			fclose(stdout);
        	}

		crawl_unlock(&collection_lock);
		crawl_element_unlock(&collection_lock);

		bblog(CLEAN, "Finished crawling of collection \"%s\" (id %u).",collection[i].collection_name,collection[i].id);

	}

	sm_collectionfree(&collection,nrofcollections);


}

int
rewriteurl(MYSQL *db, char *collection, char *url, size_t urllen, char *uri, size_t urilen, char *fulluri, size_t fullurilen, enum platform_type ptype, enum browser_type btype) {

	struct crawlLibInfoFormat *crawlLibInfo;
	struct collectionFormat *collections;
	int nrofcollections;
	int forret = 1;
	size_t len = urilen;

	bblog(DEBUGINFO, "rewrite1");
	if (!cm_searchForCollection(db, collection,&collections,&nrofcollections)) {
		bblog(DEBUGINFO, "rewrite2");
		bblog(ERROR, "Don't have collection info for this subname.");
		forret = 0;
	}
	else if (!cm_getCrawlLibInfo(global_h,&crawlLibInfo,collections->connector)) {
		bblog(DEBUGINFO, "rewrite3");
		bblog(ERROR, "Error: can't get CrawlLibInfo.");
		//exit(1);
		forret = 0;
	} else if (crawlLibInfo->rewrite_url == NULL) {
		bblog(DEBUGINFO, "Don't have a rewrite url rutine for this connector.");
		forret = 0;
	} else if (!((*crawlLibInfo->rewrite_url)(collections, url, uri, fulluri, len, ptype, btype))) {
		bblog(DEBUGINFO, "rewrite4");
		memcpy(uri, url, urilen);
		shortenurl(uri, urilen);
		memcpy(fulluri, url, fullurilen);
		forret = 0;
	}

	if (forret == 0)
	    {
		memcpy(uri, url, urilen);
		shortenurl(uri, urilen);
		memcpy(fulluri, url, fullurilen);
	    }

	bblog(DEBUGINFO, "rewrite6");

	sm_collectionfree(&collections,nrofcollections);

	//printf("*** [rewriteurl] [fulluri] [%s] ***\n", fulluri);
	return forret;

}

int
addForeignUser(char *collection, char *user, char *group)
{
	int n;
	MYSQL db;
	struct collectionFormat *collections;
	int nrofcollections;
	size_t querylen;
	char query[2048];

	sql_connect(&db);
	cm_searchForCollection(&db, collection,&collections,&nrofcollections);


	querylen = snprintf(query, sizeof(query), "INSERT INTO foreignUserSystem (usersystem, username, groupname)"
			"VALUES(%d, '%s', '%s')", collections[0].usersystem, user, group);
	bblog(INFO, "Query: %s",  query);
	
	n = 1;
	if (mysql_real_query(&db, query, querylen)) {
		bblog(ERROR, "Mysql error: %s", mysql_error(&db));
		n = 0;
	}

	mysql_close(&db);

	return n;
}

int
removeForeignUsers(char *collection)
{
	int n;
	MYSQL db;
	struct collectionFormat *collections;
	int nrofcollections;
	size_t querylen;
	char query[2048];

	sql_connect(&db);
	cm_searchForCollection(&db, collection,&collections,&nrofcollections);

	querylen = snprintf(query, sizeof(query), "DELETE FROM foreignUserSystem WHERE usersystem = %d",
			collections[0].usersystem);

	n = 1;
	if (mysql_real_query(&db, query, querylen)) {
		bblog(ERROR, "Mysql error: %s", mysql_error(&db));
		n = 0;
	}

	mysql_close(&db);		

	return n;
}

void connectHandler(int socket) {

        struct packedHedderFormat packedHedder;
        int intresponse;
        int i,len;
	char collection[64];
	char *berrorbuf;
	struct timeval start_time_all, end_time_all;
	struct timeval start_time, end_time;
	int count = 0;
	
        bblog(INFO, "got new connection");

	while ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {

		gettimeofday(&start_time_all, NULL);

	        bblog(DEBUGINFO, "size is: %i version: %i command: %i ",packedHedder.size,packedHedder.version,packedHedder.command);
	        packedHedder.size = packedHedder.size - sizeof(packedHedder);


		if (packedHedder.command == cm_crawlcollection) {
			char extrabuf[512];
			bblog(INFO, "crawlcollection");
			
			ionice_benice();
			recvall(socket,collection,sizeof(collection));
			recvall(socket,extrabuf,sizeof(extrabuf));
			bblog(INFO, "collection \"%s\"", collection);

			struct collectionFormat *collections;
			int nrofcollections;


			MYSQL db;
			sql_connect(&db);

			cm_searchForCollection(&db, collection,&collections,&nrofcollections);
			mysql_close(&db);


			//ikke mye nyttig som skjer her egentlig. Tvinger klienten bare til 
			//å vente her, slik at vi får satt noen statusmeldinger i databasen om 
			//at ting er på gang
			intresponse=1;
			sendall(socket,&intresponse, sizeof(int));

			crawl(collections, nrofcollections, crawl_crawl, 
				-1, extrabuf[0] != '\0' ? strdup(extrabuf) : NULL);

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
			char extrabuf[512];
			int docsRemaining;
			bblog(INFO, "recrawlcollection");
			
			ionice_benice();
			recvall(socket,collection,sizeof(collection));
			recvall(socket, &docsRemaining, sizeof docsRemaining);
			recvall(socket,extrabuf,sizeof extrabuf);
			bblog(INFO, "collection \"%s\"", collection);

			struct collectionFormat *collections;
			int nrofcollections;

			MYSQL db;
			sql_connect(&db);
			cm_searchForCollection(&db, collection,&collections,&nrofcollections);

			mysql_close(&db);


			//ikke mye nyttig som skjer her egentlig. Tvinger klienten bare til 
			//å vente her, slik at vi får satt noen statusmeldinger i databasen om 
			//at ting er på gang
			intresponse=1;
			sendall(socket,&intresponse, sizeof(int));


			crawl(collections,nrofcollections,crawl_recrawl, docsRemaining, 
				extrabuf[0] != '\0' ? strdup(extrabuf) : NULL);

		}
		else if (packedHedder.command == cm_deleteCollection) {
			bblog(INFO, "cm_deleteCollection");
			
			ionice_benice();
			recvall(socket,collection,sizeof(collection));
			bblog(INFO, "collection \"%s\"", collection);

			struct collectionFormat *collections;
			int nrofcollections;
			struct collection_lockFormat collection_lock;

			//Tester om noen har en lås på collectionen. Hvis de har de må vi la være i slette den, 
			//hvis ikke vil bbdn opprette den på ny
			if (!crawl_lock(&collection_lock,collection)) {
				intresponse=0;
				sendall(socket,&intresponse, sizeof(int));
				char errormsg[] = "Can't delete collection, it's being crawled.";

				bblog(ERROR, "%s", errormsg);
				len = (strlen(errormsg) +1);
				sendall(socket,&len, sizeof(int));			
				sendall(socket,&errormsg, len);		
				

			}
			else {

				//ikke mye nyttig som skjer her egentlig. Tvinger klienten bare til 
				//å vente her, slik at vi får satt noen statusmeldinger i databasen om 
				//at ting er på gang
				intresponse=1;
				sendall(socket,&intresponse, sizeof(int));

				bbdocument_deletecoll(collection);

				crawl_unlock(&collection_lock);

			}
		}
		else if (packedHedder.command == cm_scan) {
			bblog(INFO, "scan");
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

			bblog(INFO, "gor scan job: crawlertype %s host %s username %s password %s", crawlertype,host,username,password);

			if (!scan (global_h,&shares,&nrofshares,crawlertype,host,username,password)) {
				bblog(INFO, "aa can't scan");
				socketsendsaa(socket,&shares,0);
				bblog(INFO, "bb");
				sprintf(errormsg,"Can't scan.");

				len = (strlen(errormsg) +1);
				sendall(socket,&len, sizeof(int));			
				sendall(socket,&errormsg, len);			
				bblog(ERROR, "snedet error msg \"%s\" of len %i", errormsg,len);
					
			}
			else {
				bblog(INFO, "scan ok. Found %i", nrofshares);

				for (i=0;i<nrofshares;++i) {
					bblog(INFO, "share %s", shares[i]);
					
				}

				socketsendsaa(socket,&shares,nrofshares);

			}

		}
		else if (packedHedder.command == cm_pathaccess) {
		
			char uri[512];
			char username[64];
                        char password[64];
			char *pass, *ckey;

			bblog(INFO, "cm_pathaccess: start");

			gettimeofday(&start_time, NULL);
			recvall(socket,collection,sizeof(collection));
			gettimeofday(&end_time, NULL);
			bblog(INFO, "ttt time %f", getTimeDifference(&start_time,&end_time));
			bblog(INFO, "collection: \"%s\", size %i, len %i", collection,sizeof(collection),strlen(collection));

			recvall(socket,uri,sizeof(uri));

			recvall(socket,username,sizeof(username));
			recvall(socket,password,sizeof(password));

                        asprintf(&ckey,"%s",username);
                        pass = cache_fetch(cache, "logedInUser", ckey, 0);

			bblog(INFO, "collection: \"%s\"\nuri: \"%s\"\nusername \"%s\"\npassword \"%s\"", collection,uri,username,password);

			if (pass == NULL) {
				// If we can't loockup password we are probobly dealing with a api request actorised by key. Must skipp pathaccess
				bblog(ERROR, "Can't loockup password! Can't do pathaccess, but must allow anyway.");
				intresponse = 1;
			}
			else {

				MYSQL db;
				sql_connect(&db);

				intresponse = pathAccess(&db, global_h,collection,uri,username,pass);

				mysql_close(&db);
			}
	                sendall(socket,&intresponse, sizeof(int));



			bblog(INFO, "cm_pathaccess: end");
		}
		else if (packedHedder.command == cm_crawlcanconect) {
			bblog(INFO, "crawlcanconect");

			recvall(socket,collection,sizeof(collection));

			bblog(INFO, "got collection name '%s' to crawl", collection);

			MYSQL db;
			sql_connect(&db);
			if (!cm_handle_crawlcanconect(&db, collection)) {

				berrorbuf = bstrerror();	

				bblog(ERROR, "can't connect! error: \"%s\"", berrorbuf);

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
			mysql_close(&db);
			bblog(INFO, "crawlcanconect: done");
			
		}
		else if (packedHedder.command == cm_rewriteurl) {
			struct rewriteFormat rewrite;

			bblog(INFO, "cm_rewriteurl start");

			recvall(socket, &rewrite, sizeof(rewrite));
	
			MYSQL db;
			sql_connect(&db);
			rewriteurl(&db, rewrite.collection, rewrite.url, sizeof(rewrite.url), rewrite.uri, sizeof(rewrite.uri), rewrite.fulluri, sizeof(rewrite.fulluri), rewrite.ptype, rewrite.btype);

			if (sendall(socket, &rewrite, sizeof(rewrite)) == 0) {
				bblog(ERROR, "sendall(uri)");
			}

			mysql_close(&db);

			bblog(INFO, "cm_rewriteurl end");
		}
		else if (packedHedder.command == cm_killcrawl) {
			int pid;
			char errmsg[256];
			recvall(socket, &pid, sizeof pid);

			int coll_id = sql_coll_by_pid(pid);

			int ok = 0;
			if (coll_id != -1) {
				ok = (pid > 0 && kill(pid, SIGKILL) != -1);
				set_crawl_state(CRAWL_ERROR, coll_id, "User stopped crawl.");
			}
			else {
				bblog(ERROR, "cm_killcrawl: no collection has pid %d, ignoring request.", pid);
			}
			sendall(socket, &ok, sizeof ok);

			bblog(INFO, "cm_killcraw: killed pid %d: %s",  pid, ok ? "yes" : "no");
		}
		else if (packedHedder.command == cm_listusersus) {
			char extrabuf[512];
			unsigned int usersystem;
			MYSQL db;
			usersystem_t *us;
			usersystem_data_t data;
			struct cm_listusers_h h_users;
			memset(&h_users.error, '\0', sizeof h_users.error);
			int j, n_users;
			char **users;

			sql_connect(&db);
			recv(socket, &usersystem, sizeof(usersystem), 0);
			recvall(socket,extrabuf,sizeof extrabuf);

			// Redirect output:
			int output_redirected = 0;
			if (extrabuf != NULL && extrabuf[0] != '\0')
			    {
				if (!redirect_stdoutput(extrabuf))
				    {
					bblog(ERROR, "test usersystem error.");
				    }
				else
				    {
			                // a lock implies that a crawl is still running
			                flock(fileno(stdout), LOCK_SH); 
			                setvbuf(stdout, NULL, _IOLBF, 0); // line buffered
			                setvbuf(stderr, NULL, _IOLBF, 0);
			                output_redirected = 1;
				    }
			    }

			if ((us = get_usersystem(&db, usersystem, &data)) == NULL) {
				bblog(ERROR, "Unable to get usersystem");
				strlcpy(h_users.error, "Unable to get usersystem", sizeof h_users.error);
				h_users.num_users = -1;
				n_users = 0;
			} 
			else {
				// asuming call returns 0 on error.
				// TODO: Get error string from lib.
				h_users.num_users = (us->us_listUsers)(&data, &users, &n_users) 
					? n_users : -1;
			}

			sendall(socket, &h_users, sizeof(h_users));
			if (n_users > 0) {
				char *userbuf = malloc(n_users * MAX_LDAP_ATTR_LEN);
				for (j = 0; j < n_users; j++) {
					strlcpy(userbuf + (j * MAX_LDAP_ATTR_LEN), users[j], MAX_LDAP_ATTR_LEN);
				}
				sendall(socket, userbuf, n_users * MAX_LDAP_ATTR_LEN);
				free(userbuf);
				boithoad_respons_list_free(users);
			}
			
			mysql_close(&db);

			if (output_redirected)
			    {
				// Det er dessverre ikke mulig å redirecte std* tilbake til console.
				fclose(stderr); 
				fclose(stdout);
		    	    }
		}
		else if (packedHedder.command == cm_authenticateuser) {
			char user[512], pass[512], extrabuf[512];
			unsigned int usersystem;
			MYSQL db;
			usersystem_t *us;
			usersystem_data_t data;
			unsigned int r = 0;
			unsigned int *pu;
			char *ckey;

			sql_connect(&db);
			recvall(socket,user,sizeof user);
			recvall(socket,pass,sizeof pass);
			recv(socket, &usersystem, sizeof(usersystem), 0);
			recvall(socket,extrabuf,sizeof extrabuf);

			// Redirect output:
			int output_redirected = 0;
			if (extrabuf != NULL && extrabuf[0] != '\0')
			    {
				if (!redirect_stdoutput(extrabuf))
				    {
					bblog(ERROR, "test usersystem error.");
				    }
				else
				    {
			                // a lock implies that a crawl is still running
			                flock(fileno(stdout), LOCK_SH); 
			                setvbuf(stdout, NULL, _IOLBF, 0); // line buffered
			                setvbuf(stderr, NULL, _IOLBF, 0);
			                output_redirected = 1;
				    }
			    }

#ifdef WITH_US_CACHE
			// make a key and looks in the cache.
			asprintf(&ckey,"%s",user);
			pu = cache_fetch(cache, "logedInUser", ckey, 300);
			if ((pu != NULL) && (*pu == 1)) {
				printf("Login was cashed\n");
				r = *pu;
			}
			else {
#endif

				if ((us = get_usersystem(&db, usersystem, &data)) == NULL) {
					bblog(ERROR, "Unable to get usersystem nr %d", usersystem);
				} 
				else {
					r = (us->us_authenticate)(&data, user, pass);
				}

				mysql_close(&db);

				free_usersystem_data(&data);

#ifdef WITH_US_CACHE
				// if login was ok, cache login
				if (r == 1) {
					cache_add(cache, "logedInUser", ckey, strdup(pass), strlen(pass) +1); // +1 to save \0
				}
			}
#endif
			if (output_redirected)
			    {
				// Det er dessverre ikke mulig å redirecte std* tilbake til console.
				fclose(stderr);
				fclose(stdout);
	    		    }

			sendall(socket, &r, sizeof(r));
			free(ckey);			

		}
		else if (packedHedder.command == cm_groupsforuserfromusersystem) {
			char extrabuf[512];
			char user[512], secnduser[512];
			char **groups, group[MAX_LDAP_ATTR_LEN]; //[MAX_LDAP_ATTR_LEN];
			int n_groups, j;
			struct collectionFormat *collections;
			int nrofcollections;
			struct crawlLibInfoFormat *crawlLibInfo;
			unsigned int usersystem;
			MYSQL db;
			MYSQL_ROW row;
			MYSQL_RES *res;
			usersystem_t *us;
			usersystem_data_t data;
			struct timeval ts, te;

			gettimeofday(&ts, NULL);
			bblog(INFO, "groupsforuserfromusersystem");

			recvall(socket, user, sizeof(user));
			recvall(socket, &usersystem, sizeof(usersystem));
			recvall(socket,extrabuf,sizeof extrabuf);

			bblog(INFO, "usersystem \"%d\", user \"%s\"",  usersystem, user);

			// Redirect output:
			int output_redirected = 0;
			if (extrabuf != NULL && extrabuf[0] != '\0')
			    {
				if (!redirect_stdoutput(extrabuf))
				    {
					bblog(ERROR, "test usersystem error.");
				    }
				else
				    {
			                // a lock implies that a crawl is still running
			                flock(fileno(stdout), LOCK_SH); 
			                setvbuf(stdout, NULL, _IOLBF, 0); // line buffered
			                setvbuf(stderr, NULL, _IOLBF, 0);
			                output_redirected = 1;
				    }
			    }

			sql_connect(&db);
			us = get_usersystem(&db, usersystem, &data);

			n_groups = 1;
			if (!data.is_primary)  {
				char query[1024];
				size_t querylen;

				sql_connect(&db);
				querylen = snprintf(query, sizeof(query),
				    "SELECT secnd_usr FROM systemMapping WHERE system = %d AND prim_usr = '%s'",
				    usersystem, user);

				if (mysql_real_query(&db, query, querylen)) {
					bblog(ERROR, "Mysql error: %s", mysql_error(&db));
				} else {
					res = mysql_store_result(&db);
					row = mysql_fetch_row(res);
					if (row == NULL) {
						bblog(ERROR, "Unable to fetch mysql row at %s:%d",__FILE__,__LINE__);
						n_groups = 0;
					} else {
						strlcpy(secnduser, row[0], sizeof(secnduser));
					}
					mysql_free_result(res);
				}
			} else {
				strlcpy(secnduser, user, sizeof(secnduser));
			}

			if (n_groups) {
				if (us != NULL) {
					n_groups = 0;
					_listGroupsForUser(us, &data, secnduser, &groups, &n_groups);
					bblog(INFO, "Got %d groups for user %s",  n_groups, user);
				} else {
					n_groups = 0;
				}

			}

			gettimeofday(&te, NULL);
			bblog(INFO, "GroupsForUser part 1 took %f seconds.",  getTimeDifference(&ts, &te));
			gettimeofday(&ts, NULL);
			if (n_groups > 0) {
				char *groupbuf = malloc(n_groups * sizeof(group));
				for (j = 0; j < n_groups; j++) {
					strlcpy(groupbuf + (j * sizeof(group)), groups[j], sizeof(group));
				}
				sendallPack(socket, 4, &n_groups, sizeof(int), groupbuf, n_groups * sizeof(group));
				free(groupbuf);
				//boithoad_respons_list_free(groups);
			}
			else {
				sendall(socket, &n_groups, sizeof(int));
			}
			gettimeofday(&te, NULL);
			bblog(INFO, "GroupsForUser part 2 took %f seconds.",  getTimeDifference(&ts, &te));
			free_usersystem_data(&data);
			
			mysql_close(&db);

			if (output_redirected)
			    {
				// Det er dessverre ikke mulig å redirecte std* tilbake til console.
				fclose(stderr); 
				fclose(stdout);
		    	    }
		}
		else if (packedHedder.command == cm_usersystemfromcollection) {
			char collection[512];
			MYSQL db;
			int n;
			MYSQL_RES *res;
			MYSQL_ROW row;
			char query[1024];
			size_t querylen;

			sql_connect(&db);
			querylen = snprintf(query, sizeof(query),
			    "SELECT system FROM shares WHERE collection_name = '%s' AND system is NOT NULL",
			    packedHedder.subname);

			if (mysql_real_query(&db, query, querylen)) {
				bblog(ERROR, "Mysql error: %s", mysql_error(&db));
				n = -1;
			} else {
				res = mysql_store_result(&db);
				n = mysql_num_rows(res);

				if (n > 0) {
					row = mysql_fetch_row(res);
					if (row == NULL) {
						bblog(ERROR, "Unable to fetch mysql row at %s:%d",__FILE__,__LINE__);
						mysql_free_result(res);
					}

					n = atoi(row[0]);
					bblog(INFO, "Usersystem: %d",  n);
				} else {
					/* Use the primary usersystem */
					querylen = snprintf(query, sizeof(query),
							"SELECT id FROM system WHERE is_primary = 1");
					if (mysql_real_query(&db, query, querylen)) {
						bblog(ERROR, "Mysql error: %s", mysql_error(&db));
						n = -1;
					} else {
						res = mysql_store_result(&db);
						n = mysql_num_rows(res);

						if (n > 0) {
							row = mysql_fetch_row(res);
							if (row == NULL) {
								bblog(ERROR, "Unable to fetch mysql row at %s:%d",
										__FILE__,__LINE__);
								mysql_free_result(res);
							}

							n = atoi(row[0]);
							bblog(INFO, "Usersystem: %d",  n);
						} else {
							n = -1;
						}
					}
				}

				mysql_free_result(res);
			}
			mysql_close(&db);
			sendall(socket, &n, sizeof(n));
		}
		else if (packedHedder.command == cm_collectionsforuser) {
			char user[512];
			char *collections;
			MYSQL db;
			int n;

			recvall(socket, user, sizeof(user));

			sql_connect(&db);
			bblog(INFO, "Collections for user: %s",  user);
			n = collectionsforuser(user, &collections, &db);
			mysql_close(&db);

			if (n > 0) {
				sendallPack(socket, 4, &n, sizeof(n), collections, (maxSubnameLength+1)*n);
				free(collections);
			}
			else {
				sendall(socket, &n, sizeof(n));
			}
		}
		else if (packedHedder.command == cm_addForeignUser) {
			char user[512];
			char group[512];
			int n;

			bblog(INFO, "addForeignUser");
			
			recvall(socket, collection,sizeof(collection));
			recvall(socket, user, sizeof(user));
			recvall(socket, group, sizeof(group));
			bblog(INFO, "collection \"%s\"",  collection);
			bblog(INFO, "user \"%s\"",  user);
			bblog(INFO, "group \"%s\"",  group);

			n = addForeignUser(collection, user, group);

			sendall(socket, &n, sizeof(n));
		}
		else if (packedHedder.command == cm_removeForeignUsers) {
			char query[2048];
			size_t querylen;
			struct collectionFormat *collections;
			int nrofcollections;
			MYSQL db;
			int n;

			bblog(INFO, "removeForeignUsers");
			
			recvall(socket, collection,sizeof(collection));
			bblog(INFO, "collection \"%s\"",  collection);

			n = removeForeignUsers(collection);

			sendall(socket, &n, sizeof(n));
		}
		else {
			bblog(WARN, "unknown command. %i",  packedHedder.command);
		}

		gettimeofday(&end_time_all, NULL);
		bblog(INFO, "~cm time %f", getTimeDifference(&start_time_all,&end_time_all));

	}
	bblog(INFO, "end of packed");

        ++count;
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

	bblog_init("crawlManager");

	#ifdef DEBUG
		bblog(DEBUGINFO, "Ble kompilert med -DDEBUG");
	#endif

	write_gpidfile("crawlManager");

	bblog(INFO, "crawlManager: in main");

	bblog(INFO, "crawlManager: running maincfgopen");
	maincfg = maincfgopen();

	bblog(INFO, "crawlManager: running maincfg_get_int");
	int crawlport = maincfg_get_int(&maincfg,"CMDPORT");
	global_bbdnport = maincfg_get_int(&maincfg,"BLDPORT");
	bblog(INFO, "crawlManager: runing cm_start");


	/* Initialize the configuration */
        config_init(&cmcfg);

	/* Load the file */
        #ifdef DEBUG
	        bblog(DEBUGINFO, "loading [%s]..", bfile("config/crawlmanager.conf"));
        #endif

        if (!config_read_file(&cmcfg, bfile("config/crawlmanager.conf"))) {
                bblog(ERROR, "[%s]failed: %s at line %i", bfile("config/crawlmanager.conf"),config_error_text(&cmcfg),config_error_line(&cmcfg));
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
		bblog(INFO, "No memcache servers specified, disabling pathaccess caching.");
		memcache_servers = NULL;
	}

	config_destroy(&cmcfg);

	cm_start(&global_h, &usersystemshash);

#ifdef WITH_US_CACHE
	cache_delfiles();

        cache = malloc(sizeof(*cache));
        if (cache_init(cache, my_freevalue) != 1) {
		return (-1);
	}

        cache_groups = malloc(sizeof(*cache_groups));
        if (cache_init(cache_groups, groups_freevalue) != 1) {
		return (-1);
	}
#endif

	sconnect(connectHandler, crawlport);

#ifdef WITH_US_CACHE
	cache_free(cache);
	cache_free(cache_groups);
#endif

	maincfgclose(&maincfg);

	cm_end();


	return 0;
}

