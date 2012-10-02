
/**
 *	CHAANGELOG:	3. oktober 2007 - Magnus: Forandret UrlToDocID-database til å bruke den nye versjonen.
 */

#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../common/define.h"
#include "../common/bstr.h"
#include "../UrlToDocID/search_index.h"

#define PIStartDOCID 2000000000

struct IdDocIDmapFormat {
	unsigned int id;
	unsigned int DocID;
};

struct _configdataFormat *_configdata;
int _configdatanr;
time_t lastConfigRead = 0;

int main(int argc, char *argv[]) {

	struct IdDocIDmapFormat *IdDocIDmap;


	time_t now = time(NULL);

	char query[512];
	char url[201];
	//selecter fra db
	char mysql_query [2048];
        static MYSQL demo_db;
	int i, pages;	
	unsigned int DocID;
	int total;

        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;

      	if (argc < 3) {
//                printf("Dette programet slår opp ekte DocID for pi. \n\n\tUsage: ./PiToWWWDocID UrlToDocID_folder sql_table");
                printf("Dette programet slår opp ekte DocID for pi. \n\n\tUsage: ./PiToWWWDocID UrlToDocID_db sql_table\n");
		printf("\nMerk: Navn på index-fil er det samme som UrlToDocID_db + '.index'\n");
               exit(0);
        }

        char *UrlToDocIDdb = argv[1];
        char *table = argv[2];


 	#ifdef WITH_THREAD
                my_init();
                if (mysql_thread_safe() == 0) {
                        printf("The MYSQL client is'en compiled at thread safe! This will broboble crash.\n");
                }
        #endif


	mysql_init(&demo_db);

        if(!mysql_real_connect(&demo_db, "web1.jayde.boitho.com", "boitho", "G7J7v5L5Y7", "boithoweb", 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                exit(1);
        }


	char		db_index[strlen(UrlToDocIDdb)+7];
	sprintf(db_index, "%s.index", UrlToDocIDdb);
	urldocid_data	*data = urldocid_search_init(db_index, UrlToDocIDdb);


	sprintf(mysql_query, "select id,url from %s WHERE WWWDocID is NULL OR WWWDocID='0' limit 10",table);

	total = 0;
	while (1) {

        	if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
        	        printf(mysql_error(&demo_db));
        	        //return(1);
        	        pthread_exit((void *)1); /* exit with status */
        	}

        	mysqlres=mysql_store_result(&demo_db); /* Download result from server */
		_configdatanr = (int)mysql_num_rows(mysqlres);

		if (_configdatanr == 0) {
			break;
		}

		#ifdef DEBUG
		printf("nrofrows %i\n",_configdatanr);
		#endif

		if ((IdDocIDmap = malloc(sizeof(struct IdDocIDmapFormat) * _configdatanr)) == NULL) {
			perror("malloc IdDocIDmap");
			exit(1);
		}	
		pages=0;



        	while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */

			strscpy(url,mysqlrow[1],sizeof(url));
			url_normalization(url,sizeof(url));

			#ifdef DEBUG
			printf("url \"%s\"\n",url);
			#endif

               		if (!getDocIDFromUrl(data, url, &DocID)) {
                	        printf("Unable to find docId.\n\tMain \"%s\"\n\tIndex \"%s\"\n",UrlToDocIDdb, db_index);              
				

				//DocID = 0;
				DocID = PIStartDOCID + atou(mysqlrow[0]);


	                } else {
	                        //look up rank for docid
	                }

			IdDocIDmap[pages].id = atou(mysqlrow[0]);
			IdDocIDmap[pages].DocID = DocID;
 	
			#ifdef DEBUG
        	        printf("\tid %s, WWWDocID %u,url \"%s\"\n",mysqlrow[0],DocID,url);
			#endif

			if ((total % 1000) == 0) {
				printf("total %i\n",total);
			}

			++pages;
			++total;
		}


		for (i=0;i<pages;i++) {
			#ifdef DEBUG
			printf("id %u => DocID %u\n",IdDocIDmap[i].id,IdDocIDmap[i].DocID);
			#endif
			snprintf(query,sizeof(query),"update LOW_PRIORITY %s set WWWDocID=%u where id=%u",table,IdDocIDmap[i].DocID,IdDocIDmap[i].id);

			#ifdef DEBUG
			printf("query \"%s\"\n",query);
			#endif
			mysql_real_query(&demo_db, query, strlen(query));
		}	

		free(IdDocIDmap);

		mysql_free_result(mysqlres);

	}

	mysql_close(&demo_db);
	urldocid_search_exit(data);

	printf("did lookup %i\n",total);
}


