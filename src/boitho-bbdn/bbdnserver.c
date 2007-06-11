#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>

#include "../common/boithohome.h"
#include "../common/define.h"
#include "../common/daemon.h"
#include "../bbdocument/bbdocument.h"
#include "../maincfg/maincfg.h"

#include "bbdn.h"

#define PROTOCOLVERSION 1

void connectHandler(int socket);



int main (void) {

	struct config_t maincfg;


        printf("crawlManager: running maincfgopen\n");
        maincfg = maincfgopen();

        printf("crawlManager: running maincfg_get_int\n");
        int bbdnport = maincfg_get_int(&maincfg,"BLDPORT");


        sconnect(connectHandler, bbdnport);

        printf("conek ferdig \n");

        exit(0);
}

void connectHandler(int socket) {
        struct packedHedderFormat packedHedder;
	int isAuthenticated = 0;
	char tkeyForTest[32];
	int i,n;
	int intrespons;
	int count = 0;


while ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {
//if ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {

	#ifdef DEBUG
	printf("size is: %i\nversion: %i\ncommand: %i\n",packedHedder.size,packedHedder.version,packedHedder.command);
	#endif
	packedHedder.size = packedHedder.size - sizeof(packedHedder);

	if (packedHedder.command == bbc_askToAuthenticate) {
		if ((i=recv(socket, tkeyForTest, sizeof(tkeyForTest),MSG_WAITALL)) == -1) {
        	    perror("Cant read tkeyForTest");
        	    exit(1);
        	}		
		if (1) {
			printf("authenticated\n");
			intrespons = bbc_authenticate_ok;

			bbdocument_init();

			isAuthenticated = 1;
		}
		else {
			printf("authenticate faild\n");
			intrespons = bbc_authenticate_feiled;

               	}

		if ((n=sendall(socket, &intrespons, sizeof(intrespons))) == -1) {
                               perror("Cant recv filerest");
                               exit(1);
               	}
			
		
	}
	else {
		if (!isAuthenticated) {
			printf("user not autentikated\n");
			exit(1);
		}


		if (packedHedder.command == bbc_docadd) {
			#ifdef DEBUG
			printf("bbc_docadd\n");
			#endif

			char *subname,*documenturi,*documenttype,*document,*acl_allow,*acl_denied,*title,*doctype;
			int dokument_size;
			unsigned int lastmodified;

			//subname
			if ((i=recvall(socket, &intrespons, sizeof(intrespons))) == 0) {
                    		perror("Cant read intrespons");
                    		exit(1);
                	}
			subname = malloc(intrespons +1);
			if ((i=recvall(socket, subname, intrespons)) == 0) {
                                perror("Cant read subname");
                                exit(1);
                        }

			//documenturi
			if ((i=recvall(socket, &intrespons, sizeof(intrespons))) == 0) {
                    		perror("Cant read intrespons");
                    		exit(1);
                	}
			documenturi = malloc(intrespons +1);
			if ((i=recvall(socket, documenturi, intrespons)) == 0) {
                                perror("Cant read documenturi");
                                exit(1);
                        }

			//documenttype
			if ((i=recvall(socket, &intrespons, sizeof(intrespons))) == 0) {
                    		perror("Cant read intrespons");
                    		exit(1);
                	}
			documenttype = malloc(intrespons +1);
			if ((i=recvall(socket, documenttype, intrespons)) == 0) {
                                perror("Cant read documenttype");
                                exit(1);
                        }

			//document
			//dokument_size
			if ((i=recvall(socket, &dokument_size, sizeof(dokument_size))) == 0) {
                    		perror("Cant read dokument_size");
                    		exit(1);
                	}
			if (dokument_size == 0) {
				document = NULL;
			}
			else {
				document = malloc(dokument_size +1);
				if ((i=recvall(socket, document, dokument_size)) == 0) {
                        	        perror("Cant read document");
                        	        exit(1);
                        	}
			}
			//lastmodified
			if ((i=recvall(socket, &lastmodified, sizeof(lastmodified))) == 0) {
                    		perror("Cant read lastmodified");
                    		exit(1);
                	}

			//acl_allow
			if ((i=recvall(socket, &intrespons, sizeof(intrespons))) == 0) {
                    		perror("Cant read intrespons");
                    		exit(1);
                	}
			acl_allow = malloc(intrespons +1);
			if ((i=recvall(socket, acl_allow, intrespons)) == 0) {
                                perror("Cant read acl_allow");
                                exit(1);
                        }

			//acl_denied
			if ((i=recvall(socket, &intrespons, sizeof(intrespons))) == 0) {
                    		perror("Cant read intrespons");
                    		exit(1);
                	}
			acl_denied = malloc(intrespons +1);
			if ((i=recvall(socket, acl_denied, intrespons)) == 0) {
                                perror("Cant read acl_denied");
                                exit(1);
                        }

			//title
			if ((i=recvall(socket, &intrespons, sizeof(intrespons))) == 0) {
                    		perror("Cant read intrespons");
                    		exit(1);
                	}
			title = malloc(intrespons +1);
			if ((i=recvall(socket, title, intrespons)) == 0) {
                                perror("Cant read title");
                                exit(1);
                        }

			//doctype
			if ((i=recvall(socket, &intrespons, sizeof(intrespons))) == 0) {
                    		perror("Cant read intrespons");
                    		exit(1);
                	}
			doctype = malloc(intrespons +1);
			if ((i=recvall(socket, doctype, intrespons)) == 0) {
                                perror("Cant read doctype");
                                exit(1);
                        }

			printf("got subname \"%s\": title \"%s\". Nr %i\n",subname,title,count);

			bbdocument_add(subname,documenturi,documenttype,document,dokument_size,lastmodified,acl_allow,acl_denied,title,doctype);

			free(subname);
			free(documenturi);
			free(documenttype);
			free(document);
			free(acl_allow);
			free(acl_denied);
			free(title);
			free(doctype);
		}
		else if (packedHedder.command == bbc_closecollection) {
			printf("closecollection\n");
			char *subname;
			//subname
                        if ((i=recv(socket, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
                                perror("Cant read intrespons");
                                exit(1);
                        }
                        subname = malloc(intrespons +1);
                        if ((i=recv(socket, subname, intrespons,MSG_WAITALL)) == -1) {
                                perror("Cant read subname");
                                exit(1);
                        }

			bbdocument_close();

			//toDo må bruke subname, og C ikke perl her
			printf("cleanin lots start\n");
			char command[PATH_MAX];
			snprintf(command,sizeof(command),"perl %s",bfile("perl/cleanLots.pl"));
			//system("perl /home/boitho/boitho/websearch/perl/cleanLots.pl");
			//system("perl /home/boitho/boithoTools/perl/cleanLots.pl");

			system(command);
			printf("cleanin lots end\n");

			
		}
		else {
			printf("unnown comand. %i\n", packedHedder.command);
		}
	}

	++count;

}

}
