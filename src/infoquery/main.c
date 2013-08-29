#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <signal.h>
#include <time.h>

#include "../common/define.h"
#include "../common/boithohome.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../acls/acls.h"
#include "../crawlManager2/client.h"
#include "../boithoadClientLib/liboithoaut.h"
#include "../boithoadClientLib/boithoad.h"
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

/* The signal handler exit the program. . */
void
catch_alarm_nolog (int sig)
{
        fprintf(stderr, "Error: Timed out.\n");

        exit(EXIT_FAILURE);
}

int main (int argc, char *argv[]) {

	int i,y;
	char *key = NULL;
	char *value = NULL;
	char *value2 = NULL;
	char *value3 = NULL;
	char *value4 = NULL;
	char *value5 = NULL;
	char *value6 = NULL;
	char **respons_list;
	int responsnr;

	if (argc == 1) {
		printf("./usage key [value]\n\n");
		printf("groupsForUser <user name>\n");
		printf("listUsers <usersystem> [extra]\n");
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
		printf("lotsInCollection <collection name>\n");
		printf("SidToUser <sid>\n");
		printf("SidToGroup <sid>\n");
		printf("AuthUser <username> <password> <usersystem> [extra]\n");
		printf("GetPassword <username>\n");
		printf("collectionLocked <collection>\n");
                printf("killCrawl <pid>\n");
		puts("userGroups <user> <usersystem> [extra]");
		puts("addForeignUser <collection> <user> <group>");
		puts("removeForeignUsers <collection>");
		printf("repositoryRead <nr> <collection> <offset>\n");
		printf("repositoryPageInfo <pointer> <DocID> <htmlSize> <imagesize> <collection> [html]\n");
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

	if (argc >= 6) {
		value5 = argv[6];
	} else {
		value5 = NULL;
	}

	if (argc >= 7) {
		value6 = argv[7];
	} else {
		value6 = NULL;
	}


	// Setter signalhånterer for allarm, da vi bruker alarm som timout. Hvis allarm skjer i straten av programet, er noe seriøst galt. For eks er vi tom for minne.
        //Da fungerer det dårlig å logge, syslog kan kalle malloc og slikt. Vil resette den til en versjon som logger før vi kjører søk.   $
        signal (SIGALRM, catch_alarm_nolog);


	struct config_t maincfg;
        maincfg = maincfgopen();

        int cmc_port = maincfg_get_int(&maincfg,"CMDPORT");

	
	FILE *fp;
	if ((fp = bfopen("logs/infoquery.log","a")) == NULL) {
		#ifdef DEBUG
			perror(bfile("logs/infoquery.log"));
		#endif
		//exit(1);
	}
	else {
		fprintf(fp,"bin/infoquery %s \"%s\" %s %s %s %s %s\n",key,value, value2!=NULL ? value2 : "", value3!=NULL ? value3 : "", value4!=NULL ? value4 : "", value5!=NULL ? value5 : "", value6!=NULL ? value6 : "");	
		fclose(fp);
	}
	// DEPRECATED (24/10/2009)
	/*if (strcmp(key,"listUsers") == 0) { 
		if(!boithoad_listUsers(&respons_list,&responsnr)) {
			perror("Error:Can't conect to boithoad");
		}
		else {
			printf("users: %i\n",responsnr);
                	for (i=0;i<responsnr;i++) {
                	        printf("user: %s\n",respons_list[i]);
                	}
		}
	} */
	if (strcmp(key,"listMailUsers") == 0) {
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
		if ( boithoad_listGroups(&respons_list,&responsnr) == 0) {
			printf("Error: Can't list groups. boithoad_listGroups() returned 0.\n");
		} 
		else {
                	printf("groups: %i\n",responsnr);
                	for (i=0;i<responsnr;i++) {
                	        printf("group: %s\n",respons_list[i]);
                	}
		}
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
		                int r;
                int socketha;
                int errorbufflen = 512;
                char errorbuff[errorbufflen];
                char **groups;
                int i;
                char *group;

		struct userToSubnameDbFormat userToSubnameDb;


                if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        return EXIT_FAILURE;			
                }

		r = cmc_groupsforuserfromusersystem(socketha, value, 0, &groups, value3 == NULL ? "" : value3);

		if (groups == NULL) {
                       printf("Error: Can't get groups from crawlerManager\n");
                       return EXIT_FAILURE;			
		}

                printf("groups: %i\n",r);

                if (!userToSubname_open(&userToSubnameDb,'r')) {
			perror("can't open userToSubname");
			return EXIT_FAILURE;
		}

		group = (char *)groups;
                for (i = 0; i < r; i++) {
                	printf("group: %s\n", group);

			if (userToSubname_getsubnamesAsSaa(&userToSubnameDb,group,&collections, &nrofcollections)) {


				if (nrofcollections != 0) {
					printf("collections: %i\n",nrofcollections);

					for(y=0;y<nrofcollections;y++) {
						printf("collection: %s\n",collections[y]);
					}

					saafree(collections);

				}
			}

                        group += MAX_LDAP_ATTR_LEN;
	        }
		
                userToSubname_close(&userToSubnameDb);

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
	else if (strcmp(key,"lotsInCollection") == 0) {
		if(value == NULL) {printf("no value given.\n");exit(1);}

		printf("Lots: %u\n",rLotForDOCid( bbdocument_nrOfDocuments(value)) );
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
	else if (strcmp(key,"SidToGroup") == 0) {
		char user[64];
		if(value == NULL) {printf("no value given.\n");exit(1);}

		if (boithoad_sidToGroup(value, user))
			printf("Documents: %s\n", user);
		else
			printf("No match\n");
	}
	else if (strcmp(key,"AuthUser") == 0) {
		int r;
		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		if (value == NULL || value2 == NULL || value3 ==  NULL)
			errx(1, "infoquery AuthUser <username> <password> <usersystem> [extra]");

		r = cmc_authuser(socketha, value, value2, atoi(value3), value4 == NULL ? "" : value4);

		if (r == 1)
		    printf("success\n");
		else
		    printf("failure\n");
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
	else if (strcmp(key, "userGroups") == 0) {
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

		r = cmc_groupsforuserfromusersystem(socketha, value, atoi(value2), &groups, value3 == NULL ? "" : value3);
		if (groups != NULL) {
			int i;
			char *group;

			group = (char *)groups;
			for (i = 0; i < r; i++) {
				printf("group: %s\n", group);
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
	else if (strcmp(key, "listUsers") == 0) {
		int r;
		int i;
		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];
		char **users;
		struct cm_listusers_h users_h;

	        /* Set an alarm to go off in a little while. This so we don't run forever if the user system is slow */
	        alarm (90); // Apache timout=120

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		if (value == NULL)
			errx(1, "infoquery collectionsforuser user");

		users_h = cmc_listusersus(socketha, atoi(value), &users, value2 == NULL ? "" : value2);

	        /* cansel alarm */
	        alarm (0);

		if (users_h.num_users < 0) {
			printf("Error: Did not find any users: %s\n", users_h.error);
			exit(1);
		}
		else {
			for (i = 0; i < users_h.num_users; i++) {
				printf("user: %s\n", users[i]);
				free(users[i]);
			}
			free(users);
		}
	}
	else if (strcmp(key, "usersystemlookup") == 0) {
		int i;
		int socketha;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];

		if (!cmc_conect(&socketha,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		if (value == NULL)
			errx(1, "infoquery usersystemlookup collection");

		i = cmc_usersystemfromcollection(socketha, value);
		printf("System: %d\n", i);
	}
	else if (strcmp(key, "addForeignUser") == 0) {
		int s;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];

		if (value == NULL | value2 == NULL || value3 == NULL)
			errx(1, "infoquery addForeignUser collection user group");
		
		if (!cmc_conect(&s,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		printf("%s\n", cmc_addForeignUsers(s, value, value2, value3) ? "ok" : "failed");
	}
	else if (strcmp(key, "removeForeignUsers") == 0) {
		int s;
		int errorbufflen = 512;
                char errorbuff[errorbufflen];

		if (value == NULL)
			errx(1, "infoquery removeForeignUsers collection");

		if (!cmc_conect(&s,errorbuff,errorbufflen,cmc_port)) {
                        printf("Error: %s\n",errorbuff);
                        exit(1);
                }

		printf("%s\n", cmc_removeForeignUsers(s, value) ? "ok" : "failed");
	}
	else if (strcmp(key, "repositoryPageInfo") == 0) {
	    // parse param
            if (value == NULL) { 
                puts("pointer nr not provided"); exit(1); 
                exit(1);
            }
            if (value2 == NULL) { 
                puts("DocID not provided"); exit(1); 
                exit(1);
            }
            if (value3 == NULL) { 
                puts("htmlSize not provided"); exit(1); 
                exit(1);
            }
            if (value4 == NULL) { 
                puts("imagesize not provided"); exit(1); 
                exit(1);
            }
            if (value5 == NULL) { 
                puts("Colection name not provided"); exit(1); 
                exit(1);
            }

		int RepositoryPointer = atoi(value);
		int DocID = atoi(value2);
		int htmlSize = atoi(value3);
		int imageSize = atoi(value4);

                int htmlBufferSize = 3000000;
		char *htmlBuffer, *acl_allowbuffer, *acl_deniedbuffer, *attributes, *url;
            	unsigned int uriindex_DocID;
            	unsigned int uriindex_lastmodified;

                htmlBuffer = malloc(htmlBufferSize);
                struct ReposetoryHeaderFormat ReposetoryHeader;

                if (!rReadHtml(htmlBuffer,&htmlBufferSize,RepositoryPointer,htmlSize,DocID,value5,&ReposetoryHeader,
			&acl_allowbuffer,&acl_deniedbuffer,imageSize, &url, &attributes)) {
                        printf("rReadHtml: did not returne true!\n");
                }

		if (value6 != NULL) {
			printf("%s\n",htmlBuffer);
		}
		else {
	                printf("Url: %s\n", url);

	        	printf("acl_allow: %s\n",acl_allowbuffer);
	                printf("acl_denied: %s\n",acl_deniedbuffer);
			printf("attributes: %s \n", attributes);

			printf("htmlSize: %hu\n", ReposetoryHeader.htmlSize2);
			printf("imageSize: %hu\n", ReposetoryHeader.imageSize);

			printf("DocID: %u\n", ReposetoryHeader.DocID);

			printf("time: %s", ctime(&ReposetoryHeader.time));
			printf("storageTime: %s", ctime(&ReposetoryHeader.storageTime));

			if (uriindex_get(url, &uriindex_DocID, &uriindex_lastmodified, value5) == 0) {
				printf("deleted: 1\n");
	                }

		}


		free(htmlBuffer);
	}
	else if (strcmp(key, "repositoryRead") == 0) {
	    // parse param
            if (value == NULL) { 
                puts("Lot nr not provided"); exit(1); 
                exit(1);
            }
            if (value2 == NULL) { 
                puts("Collection name not provided"); exit(1); 
                exit(1);
            }
            if (value3 == NULL) { 
                puts("Offset nr not provided"); exit(1); 
                exit(1);
            }

	    int cound = 0;

	    int LotNr = atoi(value);
	    int offset = atoi(value3);

	    struct ReposetoryHeaderFormat ReposetoryHeader;
            char htmlbuffer[524288];
            char htmlbuffer_uncom[524288];
            char imagebuffer[524288];
            char *acl_allow;
            char *acl_deny;
            char *url, *attributes;
            unsigned long int radress;
	    char *thumbnale;
	    int deleted;
	    unsigned int uriindex_DocID;
	    unsigned int uriindex_lastmodified;

	    //loppergjenom alle
            while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&radress,0,offset,value2,
		&acl_allow,&acl_deny, &url, &attributes)) {

		if (cound++ > 100) {
			break;
		}
	
		thumbnale = NULL;

		#ifdef BLACK_BOX
			if (ReposetoryHeader.imageSize != 0 && ReposetoryHeader.imageSize != 65535) {

				asprintf(&thumbnale,"/cgi-bin/ShowThumbbb?L=%i&amp;P=%u&amp;S=%i&amp;C=%s",
                                	rLotForDOCid(ReposetoryHeader.DocID),
                                	(unsigned int)getImagepFromRadres(radress,ReposetoryHeader.htmlSize2),
                                	ReposetoryHeader.imageSize,
                                	value2);
			}
		#endif

		deleted = 0;
		if (uriindex_get(ReposetoryHeader.url, &uriindex_DocID, &uriindex_lastmodified, value2) == 0) {
                	fprintf(stderr,"Unable to get uri info. uri=\"%s\",subname=\"%s\".",ReposetoryHeader.url,value2);
                        perror("Unable to get uri info");
			deleted = 1;
                }

                printf("DocId: %i, url: %s, res: %hi, htmlsize: %hi, imagesize: %hi, time: %lu, radress: %lu, deleted: %i",
				ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,
				ReposetoryHeader.htmlSize2,ReposetoryHeader.imageSize,ReposetoryHeader.time,radress, deleted);
		if (thumbnale!=NULL) {printf(", thumbnale: %s", thumbnale);}
		printf("\n");

		free(thumbnale);

	    }

	    if (cound > 100) {
	    	printf("Offset_start: %i\nOffset_last: %i\n",offset,radress);
	    }
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
