#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mysql.h>

int scanMySQL(int (*scan_found_share)(char share[]),char host[],char username[], char password[]) {

        char mysql_query [2048];
        static MYSQL demo_db;
        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;


        mysql_init(&demo_db);

        //koble til mysql
        if(!mysql_real_connect(&demo_db, host, username, password, NULL, 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                return 0;
        }



        //get alle the databases
        sprintf(mysql_query, "show databases");

        printf("mysql_query: %s\n",mysql_query);

        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                printf(mysql_error(&demo_db));
                return 0;
        }

	mysqlres=mysql_store_result(&demo_db); /* Download result from server */

	printf("nr of dbes %i\n",(int)mysql_num_rows(mysqlres));

        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
                printf("\tdata %s\n",mysqlrow[0]);

		//tel cm about it
		(*scan_found_share)(mysqlrow[0]);

        }

	mysql_free_result(mysqlres);

	mysql_close(&demo_db);

	return 1;
}
