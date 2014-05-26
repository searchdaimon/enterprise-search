
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql.h>

#include "../crawl/crawl.h"
#include "../common/bstr.h"

//usikker på om vi skal ha med denne. Er det kansje bedre og lagge inn strcasestr her?
//#define _GNU_SOURCE //for strcasestr

#define hedderTemplate "<table width='75%' border='1'>\n"
#define fotterTemplate "</table>\n"
#define rowtrTemplate "<tr valign='top'><td width='19%'><p><b>\n%s\n</b></p></td><td width='81%'><p>\n%s\n</p></td></tr>\n\n"

int istitlecandidate(char **titlecandidates, char name[]) {
	int i = 0;
	while (titlecandidates[i] != '\0') {
		printf("istitlecandidate: search \"%s\"\n",titlecandidates[i]);
		if ((char *)strcasestr(name,titlecandidates[i]) != NULL) {
			return 1;
		}
		++i;
	}
	return 0;
}

int gettitlecollon(char **titlecandidates,char **collons,int nrof) {
	int i;

	for (i=0;i<nrof;i++) {
		if (istitlecandidate(titlecandidates,collons[i])) {
			printf("gettitlecollon: found \"%s\"\n",collons[i]);
			return i;
		}
	}
	return -1;
}

int mysql_recurse (char host[], char user[], char password[], char dbname[],
	struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {

	struct crawldocumentExistFormat crawldocumentExist;
	struct crawldocumentAddFormat crawldocumentAdd;


        char mysql_query [2048];
        static MYSQL demo_db;
        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;
	int i,y,x,len,count;
	int nrOfTables,nrOfColumns;
	char **tables;
	char **columns;
	unsigned int *lengths;
	unsigned int num_fields;
	unsigned int totlengths,columnstotlengths;
	char *document;
	char *cp;
	char uri[512];
	int titlecollon;
	char *fortitle;

	printf("mysql_recurse: start\n");

	char *titlecandidates[] = { "title", "subject", '\0' };


        mysql_init(&demo_db);

        //koble til mysql
        if(!mysql_real_connect(&demo_db, host, user, password, dbname, 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                exit(1);
        }



	//get alle the tables
        sprintf(mysql_query, "show tables");

        printf("mysql_query: %s\n",mysql_query);

        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                printf(mysql_error(&demo_db));
                exit(1);
        }

        mysqlres=mysql_store_result(&demo_db); /* Download result from server */
        nrOfTables = (int)mysql_num_rows(mysqlres);
        printf("nrofrows %i\n",nrOfTables);

	tables = malloc(sizeof(char *) * (nrOfTables +1));

        i=0;
        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
                printf("\tdata %s\n",mysqlrow[0]);
		
		tables[i] = strdup(mysqlrow[0]);

		++i;
	}
	tables[i] = NULL;

        mysql_free_result(mysqlres);


	//for every table
	i=0;
	while(tables[i] != NULL) {

		printf("table: %s\n",tables[i]);

		//hent ut alle colonne navnene
        	sprintf(mysql_query, "SHOW COLUMNS FROM %s",tables[i]);

        	printf("mysql_query: %s\n",mysql_query);

        	if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
        	        printf(mysql_error(&demo_db));
        	        exit(1);
        	}

        	mysqlres=mysql_store_result(&demo_db); /* Download result from server */
        	nrOfColumns = (int)mysql_num_rows(mysqlres);
        	printf("nrofrows %i\n",nrOfTables);

		columns = malloc(sizeof(char *) * (nrOfColumns +1));

        	y=0;
		columnstotlengths = 0;
        	while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */


			columnstotlengths += strlen(mysqlrow[0]);
        	        //printf("\tdata %s\n",mysqlrow[0]);
		
			columns[y] = strdup(mysqlrow[0]);

			++y;
		}

		titlecollon =  gettitlecollon(titlecandidates,columns,y);
		printf("found titlecollon %i\n",titlecollon);


		//hent ut all dataen
        	sprintf(mysql_query, "select * FROM %s",tables[i]);

        	printf("mysql_query: %s\n",mysql_query);

        	if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
        	        printf(mysql_error(&demo_db));
        	        exit(1);
        	}

        	mysqlres=mysql_store_result(&demo_db); /* Download result from server */

        	printf("nrofrows %i\n",(int)mysql_num_rows(mysqlres));

		printf("%s\n",tables[i]);
		count = 0;
        	while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */

			lengths = mysql_fetch_lengths(mysqlres);
			num_fields = mysql_num_fields(mysqlres);

			totlengths = 0;
			for(y=0;y<num_fields;y++) {
				printf("lengths: %i\n",lengths[y]);
				totlengths += lengths[y];
			}
			printf("totlengths: %u\n",totlengths);

			printf("rowtrTemplate %i, hedderTemplate %i, fotterTemplate %i\n",strlen(rowtrTemplate),strlen(hedderTemplate),strlen(fotterTemplate));

			len = ((columnstotlengths + totlengths + (strlen(rowtrTemplate) * y)) + strlen(hedderTemplate) + strlen(fotterTemplate) +1);

			printf("len %i\n",len);
			
			document = malloc(len);

			cp = document;

			cp += sprintf(cp,hedderTemplate);

			for(y=0;y<num_fields;y++) {
			//	printf("\t%s: %s\n",columns[y],mysqlrow[y]);
				cp += sprintf(cp,rowtrTemplate,columns[y],mysqlrow[y]);

			}
		
			cp += sprintf(cp,fotterTemplate);
			
			printf("doc strlen %i, len mallocet %i, cp-document %i\n",strlen(document),len,(int)(cp  - document));


			sprintf(uri,"mysql:%s-%i",tables[i],count);

			if (titlecollon == -1) {
				//strncpy(fortitle,"mysql no title",sizeof(fortitle)); 
				fortitle = strdup("mysql no title");
			}
			else if (mysqlrow[titlecollon] == NULL) {
				fortitle = strdup("mysql title NULL");
			}
			else {
				//strncpy(fortitle,mysqlrow[i],sizeof(fortitle));
				printf("titlecollon %i\n",titlecollon);
				fortitle = strdup(mysqlrow[titlecollon]);
			}


			//printf("%s\n",document);
			crawldocumentAdd.documenturi    = uri;
                        crawldocumentAdd.documenttype   = "hnxt";
                        crawldocumentAdd.document       = document;
                        crawldocumentAdd.dokument_size  = (cp - document);
                        crawldocumentAdd.lastmodified   = 0;
                        crawldocumentAdd.acl            = "";
                        crawldocumentAdd.title          = fortitle;
                        crawldocumentAdd.doctype        = "db";

                        (*documentAdd)(collection ,&crawldocumentAdd);


			//treneger vi ikke denne ? mysql_fetch_lengths lager vel ikke en ny, men returnerer en peker til sin interne lengths
			//free(lengths);

			free(document);
			free(fortitle);

			++count;
		}
		



		++i;
	}






	free(tables);

        mysql_close(&demo_db);













	printf("mysql_recurse: end\n");
}

