#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/boithohome.h"
#include "../acls/acls.h"
#include "../crawlManager/client.h"

#include "../boithoadClientLib/liboithoaut.h"

#include "../bbdocument/bbdocument.h"
#include "../maincfg/maincfg.h"

int main (int argc, char *argv[]) {

	int i,y;
	char *key = NULL;
	char *value = NULL;
	char *value2 = NULL;
	char *value3 = NULL;
	char *value4 = NULL;
	char **respons_list;
	int responsnr;

	if (argc == 1) {
		printf("./usage key [value]\n\n");
		printf("groupsForUser <user name>\n");
		printf("listUsers\n");
		printf("listGroups\n");
		printf("collectionFor <user name> or <group name>\n");
		printf("groupsAndCollectionForUser <user name>\n");

		printf("crawlCollection <collection name> [extra]\n");
		printf("recrawlCollection <collection name>\n");
		printf("deleteCollection <collection name>\n");
		printf("scan <crawlertype> <host> <username> <password>\n");
		printf("documentsInCollection <collection name>\n");
		printf("SidToUser <sid>\n");
		printf("AuthUser <username> <password>\n");
		printf("GetPassword <username>\n");
		printf("\nReturns %i on success and %i on failure\n",EXIT_SUCCESS,EXIT_FAILURE);
		exit(1);
	}

	key = argv[1];


	if (argc >= 3) {
		value = argv[2];
	} else {
		value = NULL;
	}

	if (argc >= 4) {
		value2 = argv[3];
	} else {
		value2 = NULL;
	}

	if (argc >= 5) {
		value3 = argv[4];
	} else {
		value3 = NULL;
	}

	if (argc >= 6) {
		value4 = argv[5];
	} else {
		value4 = NULL;
	}

	struct config_t maincfg;
        maincfg = maincfgopen();

        int cmc_port = maincfg_get_int(&maincfg,"CMDPORT");

	
	FILE *fp;
	if ((fp = bfopen("logs/infoquery.log","a")) == NULL) {
		perror(bfile("logs/infoquery.log"));
		//exit(1);
	}
	else {
		fprintf(fp,"%s: \"%s\"\n",key,value);	
		fclose(fp);
	}
	if (strcmp(key,"listUsers") == 0) {
		if(!boithoad_listUsers(&respons_list,&responsnr)) {
			perror("Error:Can't conect to boithoad");
		}
		else {
			printf("users: %i\n",responsnr);
                	for (i=0;i<responsnr;i++) {
                	        printf("user: %s\n",respons_list[i]);
                	}
		}
	}
	else if (strcmp(key,"listGroups") == 0) {
		boithoad_listGroups(&respons_list,&responsnr);

                printf("groups: %i\n",responsnr);
                for (i=0;i<responsnr;i++) {
                        printf("group: %s\n",respons_list[i]);
                }
		//printf("groups: 2\ngroup: Users\ngroup: Administrators\n");
	}
	else if (strcmp(key,"groupsForUser") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}

		if (!boithoad_groupsForUser(value,&respons_list,&responsnr)) {
			perror("Error: boithoad_groupsForUser");
			return EXIT_FAILURE;

		}
		printf("groups: %i\n",responsnr);
		for (i=0;i<responsnr;i++) {
			printf("group: %s\n",respons_list[i]);
		}
		boithoad_respons_list_free(respons_list);
	}
	else if (strcmp(key,"groupsAndCollectionForUser") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}

                char **collections;
		int nrofcollections;

		struct userToSubnameDbFormat userToSubnameDb;

		if (!boithoad_groupsForUser(value,&respons_list,&responsnr)) {
                        perror("Error: boithoad_groupsForUser");
			return EXIT_FAILURE;
                }

                printf("groups: %i\n",responsnr);
                if (!userToSubname_open(&userToSubnameDb)) {
			perror("can't open userToSubname");
			return EXIT_FAILURE;
		}

                for (i=0;i<responsnr;i++) {
			
			printf("group: %s\n",respons_list[i]);

			if (userToSubname_getsubnamesAsSaa(&userToSubnameDb,respons_list[i],&collections, &nrofcollections)) {


				if (nrofcollections != 0) {
					printf("collections: %i\n",nrofcollections);

					for(y=0;y<nrofcollections;y++) {
						printf("collection: %s\n",collections[y]);
					}

					saafree(collections);

				}
			}

                }

                userToSubname_close(&userToSubnameDb);
                boithoad_respons_list_free(respons_list);

	}
	else if (strcmp(key,"collectionFor") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}
		if (strcmp(value,"Everyone") == 0) {
			printf("collections: 1\n");
			printf("collection: wikipedia\n");
		}
	}
	else if (strcmp(key,"crawlCollection") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}
		int socketha;
		int errorbufflen = 512;
		char errorbuff[errorbufflen];
		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
			printf("Error: %s\n",errorbuff);
			exit(1);
		}

		if (!cmc_crawlcanconect(socketha,value,errorbuff,errorbufflen)) {
			printf("Error: %s\n",errorbuff);
			return EXIT_FAILURE;
		}
		else {
			cmc_crawl(socketha,value, value2 == NULL ? "" : value2);
		}
		cmc_close(socketha);
	}
	else if (strcmp(key,"recrawlCollection") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}
		int socketha;
		int errorbufflen = 512;
		char errorbuff[errorbufflen];
		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
			printf("Error: %s\n",errorbuff);
			exit(1);
		}

		if (!cmc_crawlcanconect(socketha,value,errorbuff,errorbufflen)) {
			printf("Error: %s\n",errorbuff);
			return EXIT_FAILURE;
		}
		else {
			cmc_recrawl(socketha,value);
		}
		cmc_close(socketha);

	}
	else if (strcmp(key,"documentsInCollection") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}

		printf("Documents: %u\n",bbdocument_nrOfDocuments(value));
	}
	else if (strcmp(key,"scan") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}
		if(value2 == NULL) {printf("no value2 given.\n");exit(1);}
		if(value3 == NULL) {printf("no value3 given.\n");exit(1);}
		if(value4 == NULL) {printf("no value4 given.\n");exit(1);}

		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];
		char **respons_list;
		int nrofresponses;
		char *errormsg;

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }
		if (!cmc_scan(socketha,&respons_list,&nrofresponses,&errormsg,value,value2,value3,value4)) {
			printf("Error: %s\n",errormsg);	
			return EXIT_FAILURE;

		}
		else {


			printf("shares: %i\n",nrofresponses);
			for(i=0;i<nrofresponses;i++) {
				printf("share: %s\n",respons_list[i]);
			}
		}

		cmc_close(socketha);


	}
	else if (strcmp(key,"deleteCollection") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}

		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];
		char **respons_list;
		/*
		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		cmc_deleteCollection(socketha,value);

		cmc_close(socketha);
		*/
	}
	else if (strcmp(key,"getPassword") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}

		char password[64];

		//int boithoad_getPassword(char username_in[], char password[])
		boithoad_getPassword(value,password);
		printf("%s -> %s\n",value,password);
	}
	else if (strcmp(key,"SidToUser") == 0) {
		char user[64];
		if(value == NULL) {printf("no value given.\n");exit(1);}

		if (boithoad_sidToUser(value, user))
			printf("Documents: %s\n", user);
		else
			printf("No match\n");
	}
	else if (strcmp(key,"AuthUser") == 0) {
		if (value2 == NULL) {
			printf("AuthUser username password\n");
			exit(1);
		}
		if (boitho_authenticat(value, value2)) {
			printf("%s is authenticated...\n", value);
		} else {
			printf("User not authenticated\n");
		}
	}
	else if (strcmp(key,"GetPassword") == 0) {
		char password[1024];
		if (value == NULL) {
			printf("GetPassword username\n");
			exit(1);
		}
		if (!boithoad_getPassword(value,password)) {
			printf("Unable to get password for %s\n", value);
		} else {
			printf("Password for %s is %s\n", value, password);
		}
	}
	else {
		printf("unknown key %s\n",key);
	}

	return EXIT_SUCCESS;

}
