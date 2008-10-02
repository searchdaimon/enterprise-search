#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#include "../common/lot.h"
#include "../common/define.h"
#include "../acls/acls.h"


int main (int argc, char *argv[]) {
	FILE *fp;
	DB *dbp = NULL;
	DBT key, data;
	int ret;
	DBC *cursorp;

	//int *dbpp;

	struct userToSubnameDbFormat userToSubnameDb;


	if (!userToSubname_open(&userToSubnameDb,'w')) {
		perror("userToSubname_open");
		exit(1);
	}


        /* Get a cursor */
        userToSubnameDb.dbp->cursor(userToSubnameDb.dbp, NULL, &cursorp, 0);

        //resetter minne
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

        /* Iterate over the database, retrieving each record in turn. */
        while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
            /* Do interesting things with the DBTs here. */
            printf("%s -> %s\n", (char*)key.data, (char*)data.data);                
        }

        /* Close the cursor */
        if (cursorp != NULL) {
                cursorp->c_close(cursorp);
        }



	userToSubname_close(&userToSubnameDb);

	/*
	//temp
	userToSubname_open(&dbpp);
	char buf[128] = "*****";
	userToSubname_getsubnamesAsString(&dbpp,"Everyone",buf);
	printf("aa  subnames \"%s\"\n",buf);
	userToSubname_close(&dbpp);
	*/

}
