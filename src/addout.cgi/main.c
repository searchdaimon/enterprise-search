#include <mysql.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../cgi-util/cgi-util.h"

struct issuedaddFormat {
	char query[65];
	char uri[1024];
	float bid;
	char ppcuser[21];
	char affuser[21];
	char ipadress[16];
	time_t issuetime;
	time_t nowtime;
	int clickfrequency;
	char HTTP_REFERER[255];
	int betaler_keyword_id;
	unsigned int DocID;
};
int main(int argc, char *argv[]) {

        static MYSQL demo_db;
        char mysql_query [2048];
        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;
	int res;
	struct issuedaddFormat issuedadd;
	char clickStatus[33];

	unsigned int addid;
	char addurl[251];
	char remote_addr[32];

	#ifdef DEBUG
	printf("Content-type: text/html\n\n");
	#endif

	
	if (getenv("QUERY_STRING") == NULL) {
		if (argc != 4 ) {
			fprintf(stderr,"no QUERY_STRING and no command lin input.\n\n\tUsage addout.cgi addid http://www.test.com\n");
			exit(1);
		}
		else {
			addid = strtoul(argv[1], (char **)NULL, 10);
			strcpy(addurl,argv[2]);	
			strncpy(remote_addr,argv[3],sizeof(remote_addr) -1);
		}

	}
	else {
		//leser inn cgi variabler
		// Initialize the CGI lib
        	res = cgi_init();

        	// Was there an error initializing the CGI???
        	if (res != CGIERR_NONE) {
        	        fprintf(stderr,"Error # %d: %s\n", res, cgilib_strerror(res));
        		exit(1);
        	}

	
		if (cgi_getentrystr("addurl") == NULL) {
        		fprintf(stderr,"Did'n receive any addurl.\n");
			exit(1);
        	}
        	else {
        		strncpy(addurl,cgi_getentrystr("addurl"),sizeof(addurl) -1);
        	}

		if (cgi_getentrystr("addid") == NULL) {
        		fprintf(stderr,"Did'n receive any id.\n");
			exit(1);
        	}
        	else {
        		addid = strtoul(cgi_getentrystr("addid"), (char **)NULL, 10);
        	}

		if (getenv("REMOTE_ADDR") == NULL) {
        		fprintf(stderr,"Did'n receive any REMOTE_ADDR.\n");
			exit(1);
        	}
        	else {
        		strncpy(remote_addr,getenv("REMOTE_ADDR"),sizeof(remote_addr) -1);
        	}
		

	}
	
        mysql_init(&demo_db);

        //if(!mysql_real_connect(&demo_db, "www2.boitho.com", "boitho_remote", "G7J7v5L5Y7", "boitho", 3306, NULL, 0)){
        if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", "boithoweb", 3306, NULL, 0)){
                fprintf(stderr,mysql_error(&demo_db));
                exit(1);
        }

	#ifdef DEBUG
	printf("add id %u\n",addid);
	#endif
        sprintf(mysql_query, "select keyword,bid,uri,clickfrequency,ppcuser,affuser,ipadress,UNIX_TIMESTAMP(issuetime),UNIX_TIMESTAMP(NOW()),HTTP_ACCEPT_LANGUAGE,HTTP_USER_AGENT,HTTP_REFERER,betaler_keyword_id,betaler_side_id from issuedadds where id='%u'",addid); 

        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
             fprintf(stderr,mysql_error(&demo_db));
             exit(1);
        }


        //henter svaret
        mysqlres=mysql_store_result(&demo_db); /* Download result from server */
        if  ((mysqlrow=mysql_fetch_row(mysqlres)) == NULL) { /* Get a row from the results */
		fprintf(stderr,"cnat't fint the add in db.\n");
		strcpy(clickStatus,"CANT_FIND_ADD");
	}
	else {
                
                #ifdef DEBUG
		printf("%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n%s<br>\n",mysqlrow[0],mysqlrow[1],mysqlrow[2],mysqlrow[3],mysqlrow[4],mysqlrow[5],mysqlrow[6],mysqlrow[7],mysqlrow[8],mysqlrow[9],mysqlrow[10],mysqlrow[11]);
		#endif
		strncpy(issuedadd.query,mysqlrow[0],sizeof(issuedadd.query)-1);
		issuedadd.bid = atof(mysqlrow[1]);
		strncpy(issuedadd.uri,mysqlrow[2],sizeof(issuedadd.uri) -1);
		issuedadd.clickfrequency = atoi(mysqlrow[3]);
		strncpy(issuedadd.ppcuser,mysqlrow[4],sizeof(issuedadd.ppcuser)-1);
		strncpy(issuedadd.affuser,mysqlrow[5],sizeof(issuedadd.ppcuser)-1);
		strncpy(issuedadd.ipadress,mysqlrow[6],sizeof(issuedadd.ipadress)-1);
		issuedadd.issuetime = atoi(mysqlrow[7]);
		issuedadd.nowtime = atoi(mysqlrow[8]);
        	strncpy(issuedadd.HTTP_REFERER,mysqlrow[11],sizeof(issuedadd.HTTP_REFERER)-1);
		issuedadd.betaler_keyword_id = atoi(mysqlrow[12]);
		issuedadd.DocID = strtoul(mysqlrow[13], (char **)NULL, 10);

		
		//sjekker om vi har noe klikk på denne ipen til samme side fra før
		
	        sprintf(mysql_query, "select * from short_out_logg where betaler_side_id='%u' AND ip_adresse='%s' AND tid > (NOW() - INTERVAL 24 HOUR)",issuedadd.DocID,issuedadd.ipadress); 

	        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
        	     	printf(mysql_error(&demo_db));
             		exit(1);
        	}

		int limit24hour;;
		mysqlres=mysql_store_result(&demo_db); /* Download result from server */
        	if  ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
			//printf("we have a record in short_out_logg\n");
			//bør kansje oppdatere klikk frekvens her
			limit24hour = 1;
		}
		else {
			//printf("we dont have a record in short_out_logg.\n %s\n",mysql_query);
			//legger den inn
			sprintf(mysql_query, "insert into short_out_logg values('%u','%s','%i',%s)",
				issuedadd.DocID,
				issuedadd.ipadress,
				0,
				"NOW()");
			if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                        	printf(mysql_error(&demo_db));
                        	exit(1);
                	}		
			limit24hour = 0;
		}

		//sjekker om dette er et gyldig klikk 
		if (strcmp(remote_addr,issuedadd.ipadress) != 0){
			strcpy(clickStatus,"IP_MISS_MATCH");
		}
		//Sjekker om annonsen er ung nokk. 
		// bruker now time fra mysql server isteden for time() da vi ikke nødvendivis kjører på 
		//samme server, og klokkene ikke er synkronisert
		else if (issuedadd.nowtime > (issuedadd.issuetime + 3600)) { 

			strcpy(clickStatus,"ADD_TO_OLD");
		} 
		else if (issuedadd.clickfrequency > 0) {
			strcpy(clickStatus,"MORE_THEN_ONE_CLICK");
		}
		else if (limit24hour) {
			strcpy(clickStatus,"24_HOUR_LIMIT");
		}
		else {
			//vi har en ok status
			strcpy(clickStatus,"OK");
			
		}

		//redirekter brukeren
		if (strcmp(clickStatus,"OK") == 0) {
			printf("Location:%s\n\n",issuedadd.uri);			
		}
		else {
			printf("Location:%s\n\n",addurl);
		}

		//kan vel egentlig stenge ned standat utputt nå?

		//oppdaterer klikk frekvens
		sprintf(mysql_query, "update issuedadds set clickfrequency = clickfrequency+1 where id='%u'",addid);

        	if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
             		fprintf(stderr,mysql_error(&demo_db));
             		exit(1);
        	}

		//trekker penger hvis vi har ok status
		if (strcmp(clickStatus,"OK") == 0) {

			//trekker penger fra ppc brukene
			sprintf(mysql_query, "update brukere set penger=penger - %f where bruker_navn='%s'",issuedadd.bid,issuedadd.ppcuser);

			if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
        	                fprintf(stderr,mysql_error(&demo_db));
        	                exit(1);
        	        }

			//betaler search/aff brukeren
			sprintf(mysql_query, "update brukere set penger=penger + %f where bruker_navn='%s'",issuedadd.bid,issuedadd.affuser);
		
			if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
        	                fprintf(stderr,mysql_error(&demo_db));
        	                exit(1);
	                }
		}



		//logger
		sprintf(mysql_query, "insert DELAYED into out_logg values(%s,%s,'%s','%s','%s','%i','%s','%s','%f','%s')",
			"NULL",
			"NOW()",
			issuedadd.query,
			issuedadd.affuser,
			issuedadd.ppcuser,
			issuedadd.betaler_keyword_id,
			issuedadd.ipadress,
			issuedadd.HTTP_REFERER,
			issuedadd.bid,
			clickStatus);
	  
		if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                        fprintf(stderr,mysql_error(&demo_db));
                        exit(1);
                }


	}


        mysql_free_result(mysqlres);
        mysql_close(&demo_db);


}
