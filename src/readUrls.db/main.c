#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#include "../common/lot.h"
#include "../common/define.h"
#include "../acls/acls.h"
#include "../bbdocument/bbdocument.h"


int main (int argc, char *argv[]) {
	FILE *fp;
	DB *dbp = NULL;
	DBT key, data;
	int ret;
	DBC *cursorp;

        if (argc < 2) {
                printf("Dette programet leser urls.db for en subname.\n");
                exit(0);
        }


	char *subname = argv[1];


	if (!uriindex_open(&dbp,subname)) {
		perror("Can't open db.");
		exit(-1);
	}



        /* Get a cursor */
        dbp->cursor(dbp, NULL, &cursorp, 0);

        //resetter minne
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

        /* Iterate over the database, retrieving each record in turn. */
        while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
            /* Do interesting things with the DBTs here. */
            printf("\"%.*s\" -> %u\n", key.size, (char*)key.data, (*(unsigned int *)data.data));                

        }

        /* Close the cursor */
        if (cursorp != NULL) {
                cursorp->c_close(cursorp);
        }



	uriindex_close(&dbp);


}
