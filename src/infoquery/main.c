#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include "../common/boithohome.h"
#include "../acls/acls.h"
#include "../crawlManager/client.h"

#include "../boithoadClientLib/liboithoaut.h"
#include "../boithoadClientLib/boithoad.h"

#include "../bbdocument/bbdocument.h"
#include "../maincfg/maincfg.h"

int str_is_nmbr(char * str) {
	int i;
	for (i = 0; str[i] != '\0'; i++) {
		if (i == 0 && (str[i] == '-'))
			continue;
		if (!isdigit(str[i])) 
		return 0;
	}
	return 1;
}


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
		printf("listUsersUS usersystem\n");
		printf("listMailUsers\n");
		printf("listGroups\n");
		printf("collectionFor <user name> or <group name>\n");
		printf("groupsAndCollectionForUser <user name>\n");
		puts("collectionsforuser <user>");

		printf("crawlCollection <collection name> [extra]\n");
		printf("recrawlCollection <collection name> [document to crawl|-1] [extra]\n");
		printf("deleteCollection <collection name>\n");
		printf("scan <crawlertype> <host> <username> <password>\n");
		printf("documentsInCollection <collection name>\n");
		printf("SidToUser <sid>\n");
		printf("AuthUser <username> <password>\n");
		printf("GetPassword <username>\n");
		printf("collectionLocked <collection>\n");
                printf("killCrawl <pid>\n");
		puts("groupsforuserfromusersystem user usersystem");
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
		//perror(bfile("logs/infoquery.log"));
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
	else if (strcmp(key,"listMailUsers") == 0) {
		if(!boithoad_listMailUsers(&respons_list,&responsnr)) {
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
                if (!userToSubname_open(&userToSubnameDb,'r')) {
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
			exit(EXIT_FAILURE);
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
		int docsRemaining = -1;
		if (value2 != NULL) {
			if (!str_is_nmbr(value2) || strcmp(value2, "") == 0)
				errx(1, "documents to crawl should be a number, not '%s'", value2);

			docsRemaining = atoi(value2);
			if (docsRemaining < -1)
				errx(1, "invalid documents to crawl '%d'", docsRemaining);
		}
		char errorbuff[errorbufflen];
		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
			printf("Error: %s\n",errorbuff);
			return EXIT_FAILURE;
		}

		if (!cmc_crawlcanconect(socketha,value,errorbuff,errorbufflen)) {
			printf("Error: %s\n",errorbuff);
			return EXIT_FAILURE;
		}
		else {
			cmc_recrawl(socketha,value, docsRemaining, value3 == NULL ? "" : value3);
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
		char *errormsg;

		//runarb: 01.10.07 hvorfor var denne slåt av
		//ser ut til at vi kan seltte ting vi ikke vil
		//eller stamer det fra dagur problemet????
		
		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		if (!cmc_deleteCollection(socketha,value,&errormsg)) {
			printf("Error: %s\n",errormsg);
			return EXIT_FAILURE;
		}

		cmc_close(socketha);
		
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
	else if (strcmp(key, "collectionLocked") == 0) {
		int r;
		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }


		r = cmc_collectionislocked(socketha, value);
		printf("Collection locked: %s\n", r == 0 ? "no" : "yes");
	}
	else if (strcmp(key, "groupsforuserfromusersystem") == 0) {
		int r;
		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];
		char **groups;

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		if (value == NULL || value2 == NULL)
			errx(1, "infoquery groupsbyuserfromcollection user collection");

		r = cmc_groupsforuserfromusersystem(socketha, value, atoi(value2), &groups);
		if (groups != NULL) {
			int i;
			char *group;

			group = (char *)groups;
			for (i = 0; i < r; i++) {
				printf("%s\n", group);
				group += MAX_LDAP_ATTR_LEN;
			}
		}
		//printf("Collection locked: %s\n", r == 0 ? "no" : "yes");
	}
	else if (strcmp(key, "collectionsforuser") == 0) {
		int r;
		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];
		char *groups;

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		if (value == NULL)
			errx(1, "infoquery collectionsforuser user");

		r = cmc_collectionsforuser(socketha, value, &groups);

		printf("Collections: %s\n", groups);
		free(groups);

		//printf("Collection locked: %s\n", r == 0 ? "no" : "yes");
	}
	else if (strcmp(key, "listUsersUS") == 0) {
		int r;
		int i;
		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];
		char **users;

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		if (value == NULL)
			errx(1, "infoquery collectionsforuser user");

		r = cmc_listusersus(socketha, atoi(value), &users);

		for (i = 0; i < r; i++) {
			printf("%s\n", users[i]);
			free(users[i]);
		}
		free(users);
	}
        else if (strcmp(key, "killCrawl") == 0) {
            // parse param
            if (value == NULL) { 
                puts("pid not provided"); exit(1); 
                exit(1);
            }
            int i = 0;
	    if (!str_is_nmbr(value)) {
		    puts("pid must be a number");
                    exit(1);
	    }
                            
            int pid = atoi(value);
	    if (pid < 0)
		    errx(1, "Invalid pid '%d'", pid);
           
            // send
            int socketha, errorbufflen = 512;
            char errorbuff[errorbufflen];
		
	    if (!cmc_conect(&socketha, errorbuff, errorbufflen, cmc_port)) {
                printf("Connect error: %s\n", errorbuff);
                exit(1);
            }

	    int ok = cmc_killcrawl(socketha, pid);
	    cmc_close(socketha);

        if (ok == 0) {
           puts("Unable to kill crawl.");
           exit(1);
        }
		if (ok != 1) {
			printf("Unexpected response %d\n", ok);
			exit(1);
		}

        puts("Crawl killed.");
    }
	else {
		printf("unknown key %s\n",key);
	}

	return EXIT_SUCCESS;

}
