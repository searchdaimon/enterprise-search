#ifdef BLACK_BOX

#include "../common/boithohome.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <db.h>

#include "acls.h"

#include "../common/lot.h"
#include "../common/define.h"
#include "../common/bstr.h"
#include "../common/debug.h"

#define userToSubnameDbFile "config/userToSubname.db"

void aclElementNormalize (char acl[]) {

        strsandr(acl," ","_");
        strsandr(acl,"-","_");

}


int userToSubname_open(struct userToSubnameDbFormat * userToSubnameDb, char mode) {
		unsigned int flags;
		int ret;
		int filemode;
		DB * db;

		debug("Using DB version %s\n", DB_VERSION_STRING);

		switch (mode) {
			case 'w':
				flags = DB_CREATE;    	/* If the database does not exist, * create it.*/
				filemode = 0664;
				break;
			case 'r':
				flags = DB_RDONLY; break;
				filemode = 0;
			default:
				errx(1, "%s %d Unknown open mode '%d'", __FILE__, __LINE__, mode);
		}

		if ((ret = db_create(&db, NULL, 0)) != 0)
			errx(1, "%s db_create: %s\n", __FILE__, db_strerror(ret));

		if ((ret = db->set_flags(db, DB_DUPSORT)) != 0)
			errx(1, "set dupsort flags, %s\n", db_strerror(ret));

		ret = db->open(db, NULL, bfile(userToSubnameDbFile), NULL, DB_BTREE, flags, filemode);
		if (ret != 0)
			errx(1, "db open error %s\n", db_strerror(ret));
	
		(*userToSubnameDb).dbp = db;

		return 1;
}

/***************************************************************************************************
userToSubname som et string array array. Ikke helt god å kalle split slig, kunne heler slåt opp i databasen selv
****************************************************************************************************/
int userToSubname_getsubnamesAsSaa(struct userToSubnameDbFormat *userToSubnameDb,char username[],char ***subnames, int *nr) {

	char buf[512];

	if (!userToSubname_getsubnamesAsString(userToSubnameDb,username,buf,sizeof buf)) {
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

char **userToSubname_getsubnamesList(struct userToSubnameDbFormat *db, char group[], int *num_colls) {
		DBC *cursorp;
		DBT key, data;
		int ret, i = 0;

		*num_colls = 0;

		aclElementNormalize(group);

		//resetter minne
		memset(&key, 0, sizeof(DBT));
		memset(&data, 0, sizeof(DBT));

		// Get a cursor
		db->dbp->cursor(db->dbp, NULL, &cursorp, 0);

		/* Set up our DBTs */
		key.data = group;
		key.size = strlen(group) +1;

		/*
		 * Position the cursor to the first record in the database whose
		 * key and data begin with the correct strings.	
		 */	
		ret = cursorp->c_get(cursorp, &key, &data, DB_SET);


		//fant ingen
		if (ret == DB_NOTFOUND) {
				debug("DB_NOTFOUND for key \"%s\"\n",key.data);
				return NULL;
		}

		char **subnames = NULL;
		while (ret == 0) {
			i++;
			subnames = realloc(subnames, i * sizeof(char *));
			subnames[i-1] = strdup(data.data);
			ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT_DUP);
			(*num_colls)++;
		}
		// vi skal kjøre cursorp->c_get() helt til siste record, og da returnerer den feil not fund.
		//hvis vi er her, og ret ikke er not found, så må vi ha hatt en annen feil...
	        if (ret != DB_NOTFOUND) {
	                perror("Can't get all record by db->c_get");
	        }

		// Close the cursor 
		if (cursorp != NULL)
				cursorp->c_close(cursorp);

		return subnames;
}

void userToSubname_freesubnamesList(char ** subnames, int num_colls) {
	if (subnames == NULL) {
		warn("null var passed to freesubnamesList %d %s\n", 	
			__LINE__, __FILE__);
		return;
	}
	int i = 0;
	for (; i < num_colls; i++) {
		free(subnames[i]);
	}
	free(subnames);
}

int userToSubname_getsubnamesAsString(struct userToSubnameDbFormat *userToSubnameDb,char username[],char subnames[], int subnameslen) {

	subnames[0] = '\0';

	int num_colls, i;
	char **names_list = userToSubname_getsubnamesList(userToSubnameDb, username, &num_colls);
	if (names_list == NULL)
		return 0;

	for (i=0; i < num_colls; i++) {
		strlwcat(subnames, names_list[i], subnameslen);
		strlwcat(subnames,",",subnameslen);
	}
	userToSubname_freesubnamesList(names_list, num_colls);

	//fjernes siste ,
	subnames[strlen(subnames) -1] = '\0';

	return 1;

}


void userToSubname_add (struct userToSubnameDbFormat *userToSubnameDb,char username[], char subname[]) {

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
		printf("can't add \"%s/%s\"\n",(char *)key.data,(char *)data.data);
                (*userToSubnameDb).dbp->err((*userToSubnameDb).dbp, ret, "DB->put");
                //kan ikke returnere her for da blir den aldr lukket.
                //return (EXIT_FAILURE);
        }


}

void userToSubname_close (struct userToSubnameDbFormat *userToSubnameDb) {

        //DB *dbp = (DB *)(*dbpp);

        int ret;

        #ifdef DEBUG
                printf("uriindex_close: closeing\n");
        #endif

        if ((ret = (*userToSubnameDb).dbp->close((*userToSubnameDb).dbp, 0)) != 0) {
                fprintf(stderr, "%s: DB->close: %s\n", "bbdocument", db_strerror(ret));
                return;
        }

        #ifdef DEBUG
                printf("uriindex_close: finished\n");
        #endif

}

int userToSubname_deletecol(struct userToSubnameDbFormat *userToSubnameDb,char subname[]) {
	DBC *cursorp;
	DBT key, data;
	int ret;

	debug("userToSubname_deletecol: deleting \"%s\"", subname);

	/* Get a cursor */
	(*userToSubnameDb).dbp->cursor((*userToSubnameDb).dbp, NULL, &cursorp, 0);

	//resetter minne
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

	/* Iterate over the database, retrieving each record in turn. */
	while ((ret = cursorp->c_get(cursorp, &key, &data, DB_NEXT)) == 0) {
    	    /* Do interesting things with the DBTs here. */
	    if (!strcmp(subname, data.data))
		{
		    debug("Deleting %s->%s (ret=%i)", (char*)key.data, (char*)data.data, cursorp->c_del(cursorp, 0));
		}
	    else
		{
		    debug("Keeping %s->%s", (char*)key.data, (char*)data.data);
		}
	}

	/* Close the cursor */
	if (cursorp != NULL) {
		cursorp->c_close(cursorp);
	}

	return 1;

}


#endif

