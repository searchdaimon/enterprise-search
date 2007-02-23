
#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dlfcn.h>      /* defines dlopen(), etc.       */
#include <sys/types.h>
#include <dirent.h>

#include "../crawl/crawl.h"
#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/error.h"
#include "../common/timediff.h"

#include "../boitho-bbdn/bbdnclient.h"

#include "../bbdocument/bbdocument.h"

#include "../3pLibs/keyValueHash/hashtable.h"

#define crawlersdir "/home/boitho/boithoTools/crawlers"

#define crawl_crawl 1
#define crawl_recrawl 2

struct hashtable *global_h;

//int crawlfirst(struct collectionFormat *collection)

int cm_searchForCollection (char cvalue[],struct collectionFormat *collection[],int *nrofcollections);

int documentExist(struct collectionFormat *collection, struct crawldocumentExistFormat *crawldocumentExist) {
	printf("documentExist: start\n");


	printf("documentExist: end\n");

	return 0;
}

int documentAdd(struct collectionFormat *collection, struct crawldocumentAddFormat *crawldocumentAdd) {
	printf("documentAdd start\n");

	printf("uri %s, title %s\n",(*crawldocumentAdd).documenturi,(*crawldocumentAdd).title);

	//send it inn
	if (!bbdn_docadd((*collection).socketha,
			(*collection).collection_name,
			(*crawldocumentAdd).documenturi,
			(*crawldocumentAdd).documenttype,
			(*crawldocumentAdd).document,
			(*crawldocumentAdd).dokument_size,
			(*crawldocumentAdd).lastmodified,
			(*crawldocumentAdd).acl,
			(*crawldocumentAdd).title,
			(*crawldocumentAdd).doctype)
	) {
		printf("can't sent to bbdn!\nWill sleep and then reconect. Wont send same doc again.\n");
		
		bbdn_closecollection((*collection).socketha,(*collection).collection_name);

		sleep(10);

		if (!bbdn_conect(&(*collection).socketha,"")) {
			berror("can't conect to bbdn (boitho backend document server)\n");
			return 0;
		}

		//exit(1);

	}


	printf("documentAdd end\n");

	return 1;
}

int closecollection(struct collectionFormat *collection) {
	printf("closecollection start\n");
	bbdn_closecollection((*collection).socketha,(*collection).collection_name);
	printf("closecollection end\n");

}

int cmr_crawlcanconect(struct hashtable *h, struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		printf("cant get CrawlLibInfo\n");
		return 0;
	}

	printf("wil crawl \"%s\"\n",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlcanconect)(collection)) {
		//overfører error
		berror((*crawlLibInfo).strcrawlError());
        	 return 0;
       	}
	else {
		return 1;
	}

}
int crawlfirst(struct hashtable *h,struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		printf("cant get CrawlLibInfo\n");
		exit(1);
	}

	printf("wil crawl \"%s\"\n",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlfirst)(collection,documentExist,documentAdd)) {
        	printf("problems in crawlfirst_ld\n");
		//overfører error
                berror((*crawlLibInfo).strcrawlError());

		return 0;
       	}
	
	return 1;

}


int pathAccess(struct hashtable *h, char collection[], char uri[], char username[], char password[]) {



	struct collectionFormat *collections; // bare en "s" skiller collection og collections her. Ikke bra, bør finne på bedre navn
	int nrofcollections;
	int forreturn;

	struct crawlLibInfoFormat *crawlLibInfo;
	struct timeval start_time, end_time;

	struct pathAccessTimesFormat {
		double searchForCollection;
		double getCrawlLibInfo;
		double crawlpatAcces;
	};

	struct pathAccessTimesFormat pathAccessTimes;

	printf("pathAccess: start\n");

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
		printf("error looking opp collection\n");
		return 0;
	}


	gettimeofday(&start_time, NULL);
	debug("cm_getCrawlLibInfo");
	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,collections[0].connector)) {
		printf("cant get CrawlLibInfo\n");
		return 0;
	}
	gettimeofday(&end_time, NULL);
	pathAccessTimes.getCrawlLibInfo = getTimeDifference(&start_time,&end_time);

	printf("wil pathAccess \"%s\"\n",uri);

	gettimeofday(&start_time, NULL);
	if ((*crawlLibInfo).crawlpatAcces == NULL) {
		printf("cralwer her ikke crawlpatAcces. returnerer tilgang. Må i fremtiden slå det opp\n");
		forreturn = 1;
	}
	else if (!(*(*crawlLibInfo).crawlpatAcces)(uri,username,password)) {
        	printf("Can't crawlLibInfo. Can by denyed or somthing else\n");
		//overfører error
                berror((*crawlLibInfo).strcrawlError());

		forreturn = 0;
       	}
	else {
		forreturn = 1;
	}
	gettimeofday(&end_time, NULL);
	pathAccessTimes.crawlpatAcces = getTimeDifference(&start_time,&end_time);

	printf("pathAccess: times\n");	
	printf("\tsearchForCollection: %f\n",pathAccessTimes.searchForCollection);
	printf("\tgetCrawlLibInfo: %f\n",pathAccessTimes.getCrawlLibInfo);
	printf("\tcrawlpatAcces: %f\n",pathAccessTimes.crawlpatAcces);

	printf("pathAccess: end\n");

	
	return forreturn;

}


int crawlupdate(struct hashtable *h,struct collectionFormat *collection) {

	struct crawlLibInfoFormat *crawlLibInfo;

	if (!cm_getCrawlLibInfo(h,&crawlLibInfo,(*collection).connector)) {
		printf("cant get CrawlLibInfo\n");
		exit(1);
	}

	printf("wil crawl \"%s\"\n",(*collection).resource);

	if (!(*(*crawlLibInfo).crawlupdate)(collection,documentExist,documentAdd)) {
        	printf("problems in crawlfirst_ld\n");
		//overfører error
                berror((*crawlLibInfo).strcrawlError());

		return 0;
       	}

	return 1;
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
                printf("cant get CrawlLibInfo\n");
                exit(1);
        }

	if ((*crawlLibInfo).scan == NULL) {
		printf("cant scan. Crawler dosent suport it.\n");
		return 0;
	}

	printf("scan:\nhost %s\nusername %s\npassword %s\n",host,username,password);

	scan_found_start();

	if (!(*(*crawlLibInfo).scan)(scan_found_share,host,username,password)) {
                printf("problems in scan\n");
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

int cm_start(struct hashtable **h) {

	printf("cm_start start\n");
	void* lib_handle;       /* handle of the opened library */
	const char* error_msg;
	struct crawlLibInfoFormat *crawlLibInfo;

	DIR *dirp;
	struct dirent *dp;
	char libpath[PATH_MAX];

	(*h) = create_hashtable(20, cm_hashfromkey, cm_equalkeys);

	if ((dirp = opendir(crawlersdir)) == NULL) {
		perror(crawlersdir);
		exit(1);
	}	

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.') {
			continue;
		}
		sprintf(libpath,"%s/%s/%s.so",crawlersdir,dp->d_name,dp->d_name);	

		printf("loading path \"%s\"\n",libpath);
		lib_handle = dlopen(libpath, RTLD_LAZY);
		if (!lib_handle) {
		    fprintf(stderr, "Error during dlopen(): %s. File %s\n", dlerror(),libpath);
		    exit(1);
		}

	
		crawlLibInfo = dlsym(lib_handle, "crawlLibInfo");

		/* check that no error occured */
	        error_msg = dlerror();
	        if (error_msg) {
	            fprintf(stderr, "Error locating '%s' - %s\n",dp->d_name, error_msg);
	            exit(1);
	        }

		
		printf("loaded \"%s\"\n",(*crawlLibInfo).shortname);


		//if (!(*(*crawlLibInfo).crawlfirst)(collection,documentExist,documentAdd)) {
	        //        printf("problems in crawlfirst_ld\n");
	        //}

		if ((*crawlLibInfo).crawlinit == NULL) {

		}
		else if (!(*crawlLibInfo).crawlinit()) {
			printf("crawlinit dident return 1\n");
			exit(1);
		}

		//sjekk at den ikke finnes i hashen fra før
		if (hashtable_search((*h),(*crawlLibInfo).shortname) != NULL) {
			printf("all redy have module with shortname \"%s\"\n",(*crawlLibInfo).shortname);
		}

		if (! hashtable_insert((*h),(*crawlLibInfo).shortname,crawlLibInfo) ) {
                        printf("cant insert\n");
                	exit(-1);
                }

		//alt ok. Legg det i en hash

		//dlclose(lib_handle); 
		

	}

	printf("cm_start end\n");


}


/************************************************************/

int cm_getCrawlLibInfo(struct hashtable *h,struct crawlLibInfoFormat **crawlLibInfo,char shortname[]) {
	printf("cm_getCrawlLibInfo: start\n");
	printf("wil search for \"%s\"\n",shortname);
	if (((*crawlLibInfo) = (struct crawlLibInfoFormat *)hashtable_search(h,shortname)) == NULL) {
	//if ((hashtable_search(h,shortname)) == NULL) {
		berror("dont hav a crawler for \"%s\"\n",shortname);
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
		#ifdef DEBUG
			printf("freeing nr %i: end\n",i);
		#endif
	}

	//toDo: hvorfor segfeiler vi her ????
	//free(collection);
}
int cm_searchForCollection (char cvalue[],struct collectionFormat *collection[],int *nrofcollections) {

        char mysql_query [2048];
        static MYSQL demo_db;
	MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;

        int i,y;



	mysql_init(&demo_db);

	//koble til mysql
	if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", BOITHO_MYSQL_DB, 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                exit(1);
        }



	if (cvalue != NULL) {
		sprintf(mysql_query, "\
					select \
						resource, \
						connectors.name, \
						collection_name, \
						UNIX_TIMESTAMP(last), \
						query1, \
						query2, \
						auth_id, \
						shares.id \
					from \
						shares,connectors \
					where \
						connectors.ID = shares.connector \
						AND collection_name='%s' \
					",cvalue);
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
						shares.id \
					from \
						shares,connectors \
					where \
						connectors.ID = shares.connector \
					");
	}

	debug("mysql_query: %s\n",mysql_query);

	if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
               	printf(mysql_error(&demo_db));
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

        i=0;
        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
        	debug("\tdata %s, %s, %s, %s\n",mysqlrow[0],mysqlrow[1],mysqlrow[2],mysqlrow[3]);
		
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

		//fjerner ikke aski tegn, som / og space. De fører til problemer
		for(y=0;y<strlen((*collection)[i].collection_name);y++) {
			if (((*collection)[i].collection_name[y] > 48) && ((*collection)[i].collection_name[y] < 122)) {
				//er arcii
			}
			else {
				debug("removed noen ascii char \"%c\" from collection_name \n",(*collection)[i].collection_name[y]);
				(*collection)[i].collection_name[y] = 'x';
			}
		}

		debug("resource \"%s\", connector \"%s\", collection_name \"%s\"\n",(*collection)[i].resource,(*collection)[i].connector,(*collection)[i].collection_name);

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

	/***********************************************************************/

	}

	mysql_close(&demo_db);

	return 1;

	debug("cm_searchForCollection: end\n");
	
}

int crawlcanconect (char cvalue[]) {

	struct collectionFormat *collection;
	int nrofcollections;
	int i;

	cm_searchForCollection(cvalue,&collection,&nrofcollections);

	if (nrofcollections == 1) {
		//make a conectina for add to use
		if (!bbdn_conect(&collection[0].socketha,"")) {
			//berror("can't conect to bbdn (boitho backend document server)\n");
			return 0;
		}

		if (!cmr_crawlcanconect(global_h,&collection[0])) {
			return 0;
		}
		
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

        mysql_init(&demo_db);

        //koble til mysql
        if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", BOITHO_MYSQL_DB, 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                exit(1);
        }

	//escaper queryet rikit
        mysql_real_escape_string(&demo_db,messageEscaped,mrg,strlen(mrg));

	printf("mysql_queryEscaped \"%s\"\n",messageEscaped);

	if (crawler_success  == 0) {
        	sprintf(mysql_query, "UPDATE shares SET crawler_success = \"%i\", crawler_message = \"%s\" WHERE id  = \"%u\"",crawler_success ,messageEscaped,id);
	}
	else {
        	sprintf(mysql_query, "UPDATE shares SET crawler_success = \"%i\", crawler_message = \"%s\", last = now() WHERE id  = \"%u\"",crawler_success ,messageEscaped,id);

	}



        printf("mysql_query: %s\n",mysql_query);

        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                printf(mysql_error(&demo_db));
                exit(1);
        }


	mysql_close(&demo_db);

}

//setter at vi har begynt og cralwe
cm_setCrawStartMsg(struct collectionFormat *collection,int nrofcollections) {
	int i;

	for(i=0;i<nrofcollections;i++) {
		set_crawler_message(0,"Crawling it now. Click \"Overview\" again to see progression.",collection[i].id);
	}
}

int crawl (struct collectionFormat *collection,int nrofcollections, int flag) {


	int i;

	//if (nrofcollections == 1) {

	for(i=0;i<nrofcollections;i++) {


		//make a conectina for add to use
		if (!bbdn_conect(&collection[i].socketha,"")) {
			berror("can't conect to bbdn (boitho backend document server)");
			set_crawler_message(0,bstrerror(),collection[i].id);
		}
		else {

			if (flag == crawl_recrawl) {
				printf("crawl_recrawl\n");

				//nullsetter lastCrawl, som er den verdien som viser siste crawl. 
				//Tror ikke crawlfirst skal ta hensysn til den, men gjør det for sikkerhets skyld
				(*collection).lastCrawl = 0;

				//beordrer en sletting. Burde kansje vært gjort vi bbdn, ikke direkte, men skit au
				bbdocument_deletecoll(collection[i].collection_name);


				if (!crawlfirst(global_h,&collection[i])) {
                                        set_crawler_message(0,bstrerror(),collection[i].id);
                                }
                                else {
                                        set_crawler_message(1,"Ok",collection[i].id);
                                }

			}
			else if ((*collection).lastCrawl == 0) {
				if (!crawlfirst(global_h,&collection[i])) {
					set_crawler_message(0,bstrerror(),collection[i].id);	
				}
				else {
					set_crawler_message(1,"Ok",collection[i].id);
				}
			}
			else {
                                if (!crawlupdate(global_h,&collection[i])) {
                                        set_crawler_message(0,bstrerror(),collection[i].id);
                                }
                                else {
                                        set_crawler_message(1,"Ok",collection[i].id);
                                }
			}
			closecollection(&collection[i]);

			
			//ber bbdn om å lukke
			bbdn_close(&collection[i].socketha);
		}
	}

	sm_collectionfree(&collection,nrofcollections);


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
			printf("crawlcollection\n");
			
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

			crawl(collections,nrofcollections,crawl_crawl);


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

			crawl(collections,nrofcollections,crawl_recrawl);

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
			char uri[512];
			char username[64];
                        char password[64];

			printf("cm_pathaccess: start\n");

			gettimeofday(&start_time, NULL);
			recvall(socket,collection,sizeof(collection));
			gettimeofday(&end_time, NULL);
			printf("ttt time %f\n",getTimeDifference(&start_time,&end_time));
			printf("collection: \"%s\", size %i, len %i\n",collection,sizeof(collection),strlen(collection));

			recvall(socket,uri,sizeof(uri));

			recvall(socket,username,sizeof(username));
			recvall(socket,password,sizeof(password));

			printf("collection: \"%s\"\nuri: \"%s\"\nusername \"%s\"\npassword \"%s\"\n",collection,uri,username,password);


			//int pathAccess(struct hashtable *h, char connector[], char uri[], char username[], char password[])
			intresponse= pathAccess(global_h,collection,uri,username,password);

                        sendall(socket,&intresponse, sizeof(int));



			printf("cm_pathaccess: end\n");
		}
		else if (packedHedder.command == cm_crawlcanconect) {
			printf("crawlcanconect\n");

			recvall(socket,collection,sizeof(collection));

			printf("got %s name to crawl\n",collection);

			if (!crawlcanconect(collection)) {

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
	        else {
			 printf("unnown comand. %i\n", packedHedder.command);
	        }

		gettimeofday(&end_time_all, NULL);
		printf("cm time %f\n",getTimeDifference(&start_time_all,&end_time_all));

	}
	printf("end of packed\n");
}


int main (int argc, char *argv[]) {

	cm_start(&global_h);


        sconnect(connectHandler, CMDPORT);
}

