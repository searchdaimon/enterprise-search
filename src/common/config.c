#ifdef WITH_CONFIG


#include <mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "define.h"

struct _configdataFormat {
	char configkey[255];
	char configvalue[255];
};

struct _configdataFormat *_configdata;
int _configdatanr;

int bconfig_init() {


	//bb har confg i mysql
        #ifdef BLACK_BOKS

	//selecter fra db
	char mysql_query [2048];
        static MYSQL demo_db;
	int i;	

        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;

 	#ifdef WITH_THREAD
                printf("starting whth thread\n");
                my_init();
                if (mysql_thread_safe() == 0) {
                        printf("The MYSQL client is'en compiled at thread safe! This will broboble crash.\n");
                }
        #else
                printf("starting single thread version\n");
        #endif


	mysql_init(&demo_db);

        if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", BOITHO_MYSQL_DB, 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                exit(1);
        }


	sprintf(mysql_query, "select configkey,configvalue from config");

        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                printf(mysql_error(&demo_db));
                //return(1);
                pthread_exit((void *)1); /* exit with status */
        }

        mysqlres=mysql_store_result(&demo_db); /* Download result from server */
	_configdatanr = (int)mysql_num_rows(mysqlres);

	printf("nrofrows %i\n",_configdatanr);
	
	_configdata = malloc(sizeof(struct _configdataFormat) * _configdatanr);

	i=0;
        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
                        printf("\tconfig %s => \"%s\"\n",mysqlrow[0],mysqlrow[1]);
			strcpy(_configdata[i].configkey,mysqlrow[0]);
			strcpy(_configdata[i].configvalue,mysqlrow[1]);
		++i;
	}


	mysql_free_result(mysqlres);
	mysql_close(&demo_db);
        #endif
}


const char *bconfig_getentrystr(char vantkey[]) {

	int i;

	for(i=0;i<_configdatanr;i++) {
		//printf("config %s: %s\n",_configdata[i].configkey,_configdata[i].configvalue);

		if (strcmp(vantkey,_configdata[i].configkey) == 0) {
			return _configdata[i].configvalue;
		}

	}

	return NULL;
}

int bconfig_getentryint(char vantkey[]) {
	return atoi(bconfig_getentrystr(vantkey));
}
#endif

