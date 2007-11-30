#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#include "../common/lot.h"
#include "../common/define.h"
#include "../acls/acls.h"


int main (int argc, char *argv[]) {
	FILE *fp;
	char username[MAX_USER_NAME_LEN];
	DB *dbp = NULL;
	DBT key, data;
	int ret;
	//int *dbpp;

	struct userToSubnameDbFormat userToSubnameDb;

	if (argc != 3) {
		printf("usgae ./mergeUserToSubname lotnr subname\n");
		exit(1);
	}

	int lotNr = atoi(argv[1]);
	char *subname = argv[2];

	if (!userToSubname_open(&userToSubnameDb,'w')) {
		perror("userToSubname_open");
		exit(1);
	}

	if ((fp = lotOpenFileNoCasheByLotNr(lotNr,"acllist","rb", 's',subname) ) == NULL) {
		perror("acllist");
	}
	else {
		while(fgets(username,sizeof(username),fp) != NULL) {
			chomp(username);
			//strsandr(username," ","_NBSP_");
			printf("username \"%s\"\n",username);

			userToSubname_add(&userToSubnameDb,username,subname);
		}

		fclose(fp);

	}


	if ((fp = lotOpenFileNoCasheByLotNr(lotNr,"aclcollectionlist","rb", 's',subname) ) == NULL) {
		perror("aclcollectionlist");
	}
	else {
		printf("looping tru\n");
		while(fgets(username,sizeof(username),fp) != NULL) {
			chomp(username);
			//strsandr(username," ","_NBSP_");
			printf("username \"%s\"\n",username);

			userToSubname_add(&userToSubnameDb,username,subname);
		}

		fclose(fp);

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
