#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/define.h"

#include <db.h>
#include "../common/crc32.h"

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif


/*************************************************************************************
* slår opp i databasen for å finne DoCID for en url
*************************************************************************************/
int getDocIDFromUrl(char bdbfiledir[],char url[],unsigned int *DocID) {

        unsigned int crc32Value;
        int dbFileForUrl;
        int ret;
        DB *dbp;
	static char inited;
	static DB *dbp_store[nrOfUrlToDocIDFiles];

        DBT key, data;
        char fileName[256];
	
        crc32Value = crc32boitho(url);
        dbFileForUrl = (crc32Value % nrOfUrlToDocIDFiles);

	if (inited == 0) {
		int i;

		for(i = 0; i < nrOfUrlToDocIDFiles; i++) {
			sprintf(fileName,"%s%i.db",bdbfiledir,i);
			/* Create and initialize database object */
			if ((ret = db_create(&dbp, NULL, 0)) != 0) {
				fprintf(stderr, "%s: db_create: %s\n", "getDocIDFromUrl", db_strerror(ret));
				return (EXIT_FAILURE);
			}
			/* open the database. */
			//if ((ret = dbp->open(dbp, NULL, fileName, NULL, DB_BTREE, DB_CREATE, 0444)) != 0) {
			if ((ret = dbp->open(dbp, NULL, fileName, NULL, DB_BTREE, DB_RDONLY, 0444)) != 0) {
				dbp->err(dbp, ret, "%s: open", fileName);
				//goto err1;
			}
			dbp_store[i] = dbp;
		}

		inited = 1;
        }

	dbp = dbp_store[dbFileForUrl];


        //finner ut hvilken database vi skal opne
        //lager en has verdi slik at vi kan velge en av filene


        #ifdef DEBUG
                printf("Openig db %s\n",fileName);
        #endif


        /* Initialize the key/data pair so the flags aren't set. */
        memset(&key, 0, sizeof(key));
        memset(&data, 0, sizeof(data));


        key.data = url;
        key.size = strlen(url);


        /* Walk through the database and print out the key/data pairs. */
        if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
                //printf("%s : %u-%i \n", key.data, *(int *)data.data,rLotForDOCid(*(int *)data.data));
                *DocID = *(int *)data.data;
                return 1;
        }
        else if (ret == DB_NOTFOUND) {
		#ifdef DEBUG
                dbp->err(dbp, ret, "DBcursor->get");
		#endif
                return 0;
        }
        else {
                dbp->err(dbp, ret, "DBcursor->get");
                return 0;
        }
}

