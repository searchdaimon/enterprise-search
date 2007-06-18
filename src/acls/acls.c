#ifdef BLACK_BOKS

#include "../common/boithohome.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#include "acls.h"

#include "../common/lot.h"
#include "../common/define.h"


#define userToSubnameDbFile "config/userToSubname.db"


int userToSubname_open(struct userToSubnameDbFormat *userToSubnameDb) {

        //DB *dbp = (DB *)(*dbpp);


        char fileName[512];
        int ret;

	#ifdef DEBUG
		printf("opening db\n");
	#endif


        /********************************************************************
        * Opening nrOfFiles to stor the data in
        ********************************************************************/
                /* Create and initialize database object */
                if ((ret = db_create(&(*userToSubnameDb).dbp, NULL, 0)) != 0) {
                        fprintf(stderr,
                            "%s: db_create: %s\n", "bbdocument", db_strerror(ret));
                        return (0);
                }


                /*
                 * Configure the database for sorted duplicates
                 */


                ret = (*userToSubnameDb).dbp->set_flags((*userToSubnameDb).dbp, DB_DUPSORT);
                if (ret != 0) {
                    (*userToSubnameDb).dbp->err((*userToSubnameDb).dbp, ret, "Attempt to set DUPSORT flag failed.");
                    (*userToSubnameDb).dbp->close((*userToSubnameDb).dbp, 0);
                    return(ret);
                }


                /* open the database. */
                if ((ret = (*userToSubnameDb).dbp->open((*userToSubnameDb).dbp, NULL, bfile(userToSubnameDbFile), NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
                        (*userToSubnameDb).dbp->err((*userToSubnameDb).dbp, ret, "%s: open", userToSubnameDb);
                        //goto err1;
                        return (0);

                }

        /********************************************************************/

        //(*dbpp) = (int *)dbp;
	return 1;
}

/***************************************************************************************************
userToSubname som et string array array. Ikke helt god å kalle split slig, kunne heler slåt opp i databasen selv
****************************************************************************************************/
int userToSubname_getsubnamesAsSaa(struct userToSubnameDbFormat *userToSubnameDb,char username[],char ***subnames, int *nr) {

	char buf[512];

	if (!userToSubname_getsubnamesAsString(userToSubnameDb,username,buf,sizeof(buf))) {
		#ifdef DEBUG
		fprintf(stderr,"cant run userToSubname_getsubnamesAsString\n");
		#endif
		return 0;
	}

	if (buf[0] == '\0') {
		(*nr) = 0;
		return 0; //toDo skal vi returnere 1 eller 0 her. Vi klarte jo og slå opp gruppe, men det var ingen å vise
	}

	(*nr) = split(buf, ",", subnames);
	
	return 1;
}

int userToSubname_getsubnamesAsString(struct userToSubnameDbFormat *userToSubnameDb,char username[],char subnames[], int subnameslen) {

	//DB *dbp = (DB *)(*dbpp);
	DBC *cursorp;
	DBT key, data;
	int ret;


	//resetter minne
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

	/* Get a cursor */
	(*userToSubnameDb).dbp->cursor((*userToSubnameDb).dbp, NULL, &cursorp, 0);

	/* Set up our DBTs */
	key.data = username;
	key.size = strlen(username) +1;

	subnames[0] = '\0';



	/*
	 * Position the cursor to the first record in the database whose
	 * key and data begin with the correct strings.	
	 */	
	ret = cursorp->c_get(cursorp, &key, &data, DB_SET);


	//fant ingen
	if (ret == DB_NOTFOUND) {
		printf("DB_NOTFOUND for key \"%s\"\n",key.data);
		return 0;
	}

	while (ret != DB_NOTFOUND) {
		//printf("found collection \"%c%c%c%c\", size %i\n",(char *)data.data,data.size);
		//printf("key: %s, data: %s\n", (char *)key.data, (char *)data.data);
		strlwcat(subnames,(char *)data.data,subnameslen);
		strlwcat(subnames,",",subnameslen);

		ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT_DUP);
	}

	/* Close the cursor */
	if (cursorp != NULL) {
		cursorp->c_close(cursorp);
	}

	//fjernes siste ,
	subnames[strlen(subnames) -1] = '\0';


	//(*dbpp) = (int *)dbp;

	return 1;

}

int userToSubname_add (struct userToSubnameDbFormat *userToSubnameDb,char username[], char subname[]) {
        DB dbpArray;

	//DB *dbp = (DB *)(*dbpp);

        DBT key, data;
        int ret;

	//resetter minne
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	key.data = username;
	key.size = strlen(username) +1;

	data.data = subname;
	data.size = (strlen(subname) +1);

	//legger til i databasen
        if  ((ret = (*userToSubnameDb).dbp->put((*userToSubnameDb).dbp, NULL, &key, &data, 0)) != 0) {
                (*userToSubnameDb).dbp->err((*userToSubnameDb).dbp, ret, "DB->put");
                //kan ikke returnere her for da blir den aldr lukket.
                //return (EXIT_FAILURE);
        }

        //(*dbpp) = (int *)dbp;

}

int userToSubname_close (struct userToSubnameDbFormat *userToSubnameDb) {

        //DB *dbp = (DB *)(*dbpp);

        int ret;

        #ifdef DEBUG
                printf("uriindex_close: closeing\n");
        #endif
        if ((ret = (*userToSubnameDb).dbp->close((*userToSubnameDb).dbp, 0)) != 0) {
                fprintf(stderr, "%s: DB->close: %s\n", "bbdocument", db_strerror(ret));
                return (EXIT_FAILURE);
        }

        #ifdef DEBUG
                printf("uriindex_close: finished\n");
        #endif

        //(*dbpp) = (int *)dbp;
}

#endif

