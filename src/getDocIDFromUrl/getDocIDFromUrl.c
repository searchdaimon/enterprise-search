#include "define.h"

#include <db.h>
#include <stdlib.h>
#include <string.h>
#include "crc32.h"

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif


/*************************************************************************************
* slår opp i databasen for å finne DoCID for en url
*************************************************************************************/
int getDocIDFromUrl(char url[],unsigned int *DocID) {

        unsigned long crc32Value;
        int dbFileForUrl;
        int ret;
        DB *dbp;

        DBT key, data;
        char fileName[256];

        //finner ut hvilken database vi skal opne
        //lager en has verdi slik at vi kan velge en av filene
        crc32Value = crc32(url);
        dbFileForUrl = (crc32Value % nrOfUrlToDocIDFiles);

        sprintf(fileName,"%s%i.db",URLTODOCIDINDEX,dbFileForUrl);


        #ifdef DEBUG
                printf("Openig db %s\n",fileName);
        #endif
        /* Create and initialize database object */
        if ((ret = db_create(&dbp, NULL, 0)) != 0) {
                fprintf(stderr,
                    "%s: db_create: %s\n", "getDocIDFromUrl", db_strerror(ret));
                return (EXIT_FAILURE);
        }

        /* open the database. */
        if ((ret = dbp->open(dbp, fileName, NULL, DB_BTREE, DB_CREATE, 0444)) != 0) {
                dbp->err(dbp, ret, "%s: open", fileName);
                //goto err1;
        }



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
                dbp->err(dbp, ret, "DBcursor->get");
                return 0;
        }
        else {
                dbp->err(dbp, ret, "DBcursor->get");
                return 0;
        }



        /* Close everything down. */
        if ((ret = dbp->close(dbp, 0)) != 0) {
                fprintf(stderr,
                    "%s: DB->close: %s\n", getDocIDFromUrl, db_strerror(ret));
                return (EXIT_FAILURE);
        }

}

