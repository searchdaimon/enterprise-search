#include <sys/file.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
#include <signal.h>
#include <err.h>

#include "../common/boithohome.h"
#include "../common/define.h"
#include "../common/re.h"
#include "../common/daemon.h"
#include "../bbdocument/bbdocument.h"
#include "../maincfg/maincfg.h"
#include "../common/timediff.h"
#include "../common/lot.h"
#include "../common/re.h"
#include "../common/gcwhisper.h"
#include "../common/reposetory.h"
#include "../common/nice.h"
#include "../common/DocumentIndex.h"

#include "../perlembed/perlembed.h"

#include "bbdn.h"

#define PROTOCOLVERSION 1

void connectHandler(int socket);
/* perlembed has some perlxs that requires global_bbdnport to exist */
int global_bbdnport = 0;



int main (int argc, char *argv[]) {

	struct config_t maincfg;
	char c;

	int noFork = 0;
	int breakAfter = 0;


        while ((c=getopt(argc,argv,"m:s"))!=-1) {
                switch (c) {
                         case 'm':
                                breakAfter = atoi(optarg);
                                break;
                         case 's':
                                noFork = 1;
                         	break;
                         default:
                                fprintf(stderr, "Unknown argument: %c", c);
                                errx(1, "Unknown argument: %c", c);
                }
        }


        printf("boitho-bbdn: running maincfgopen\n");
        maincfg = maincfgopen();

        printf("boitho-bbdn: running maincfg_get_int\n");
        int bbdnport = maincfg_get_int(&maincfg,"BLDPORT");


	bbdocument_init(NULL);

        sconnect(connectHandler, bbdnport, noFork, breakAfter);

        printf("bbdnserver:Main() sconnect ferdig\n");

	maincfgclose(&maincfg);

	bbdocument_clean();

	printf("boitho-bbdn: ending\n");
        return 0;
}

void connectHandler(int socket) {
        struct packedHedderFormat packedHedder;
	int isAuthenticated = 0;
	char tkeyForTest[32];
	int i,n;
	int intrespons;
	int count = 0;
	container *attrkeys = NULL;

        #ifdef DEBUG_TIME
      		struct timeval start_time, end_time;
                struct timeval tot_start_time, tot_end_time;
                gettimeofday(&tot_start_time, NULL);
        #endif

	ionice_benice();

while ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {

	#ifdef DEBUG
	printf("size is: %i\nversion: %i\ncommand: %i\n",packedHedder.size,packedHedder.version,packedHedder.command);
	#endif
	packedHedder.size = packedHedder.size - sizeof(packedHedder);

	if (attrkeys == NULL) {
		attrkeys = ropen();
	}

	if (packedHedder.command == bbc_askToAuthenticate) {
		if ((i=recv(socket, tkeyForTest, sizeof(tkeyForTest),MSG_WAITALL)) == -1) {
        	    perror("Cant read tkeyForTest");
        	    exit(1);
        	}		
		if (1) {
			printf("authenticated\n");
			intrespons = bbc_authenticate_ok;

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
			char *attributes;
			int dokument_size;
			unsigned int lastmodified;

			#ifdef DEBUG_TIME
                		gettimeofday(&start_time, NULL);
        		#endif

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

			document = malloc(dokument_size +1);

			if (dokument_size == 0) {
				document[0] = '\0';
			}
			else {
				if ((i=recvall(socket, document, dokument_size)) == 0) {
                        	        fprintf(stderr,"Can't read document of size %i\n",dokument_size);
					perror("recvall");
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

			// Attribute list
			if ((i = recvall(socket, &intrespons, sizeof(intrespons))) == 0)
				err(1, "Can't receive attribute list len");
			attributes = malloc(intrespons +1);
			if ((i=recvall(socket, attributes, intrespons)) == 0)
				err(1, "Can't receive attribute list");

			#ifdef DEBUG_TIME
                		gettimeofday(&end_time, NULL);
                		printf("Time debug: bbdn_docadd recv data time: %f\n",getTimeDifference(&start_time, &end_time));
        		#endif

			printf("\n");
			printf("########################################################\n");
			printf("Url: %s\n",documenturi);
			printf("got subname \"%s\": title \"%s\". Nr %i, dokument_size %i attrib: %s\n",subname,title,count,dokument_size, attributes);
			printf("########################################################\n");
			printf("calling bbdocument_add():\n");
        		#ifdef DEBUG_TIME
        		        gettimeofday(&start_time, NULL);
		        #endif

			intrespons = bbdocument_add(subname,documenturi,documenttype,document,dokument_size,lastmodified,acl_allow,acl_denied,title,doctype, attributes, attrkeys);

			printf(":bbdocument_add end\n");
			printf("########################################################\n");

			#ifdef DEBUG_TIME
                		gettimeofday(&end_time, NULL);
                		printf("Time debug: bbdn_docadd runing bbdocument_add() time: %f\n",getTimeDifference(&start_time, &end_time));
        		#endif
			free(subname);
			free(documenturi);
			free(documenttype);
			free(document);
			free(acl_allow);
			free(acl_denied);
			free(title);
			free(doctype);
			free(attributes);

			// send status
	                if ((n=sendall(socket, &intrespons, sizeof(intrespons))) == -1) {
                               perror("Cant recv filerest");
                               exit(1);
	                }

		}
		else if (packedHedder.command == bbc_opencollection) {
			char *subname;
			char path[PATH_MAX];

			printf("open collection\n");

                        if ((i=recv(socket, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1)
                                err(1, "Cant read intrespons");
                        subname = malloc(intrespons +1);
                        if ((i=recv(socket, subname, intrespons,MSG_WAITALL)) == -1)
                                err(1, "Cant read subname");

			GetFilPathForLot(path, 1, subname);
			strcat(path, "fullyCrawled");

			unlink(path);

			free(subname);
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

			bbdocument_close(attrkeys);
			attrkeys = NULL;

			//toDo må bruke subname, og C ikke perl her
			printf("cleanin lots start\n");
			char command[PATH_MAX];
			snprintf(command,sizeof(command),"perl %s -l -s \"%s\"",bfile("perl/cleanLots.pl"),subname);

			printf("running \"%s\"\n",command);
			intrespons = system(command);
			printf("cleanin lots end\n");

			// legger subnamet til listen over ventene subnavn, og huper searchd.
			lot_recache_collection(subname);


			/* We are done crawling  */
			{
				int fd = lotOpenFileNoCasheByLotNrl(1, "fullyCrawled", ">>", '\0', subname);

				if (fd == -1) {
					warn("Unable to write fullyCrawled file");
				} else {
					close(fd);
				}
			}

			free(subname);

                        if ((n=sendall(socket, &intrespons, sizeof(intrespons))) == -1) {
                                       perror("Cant recv filerest");
                                       exit(1);
                        }
			
		}
		else if (packedHedder.command == bbc_deleteuri) {
			printf("deleteuri\n");
			char *subname, *uri;
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
			subname[intrespons] = '\0';
                        if ((i=recv(socket, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
                                perror("Cant read intrespons");
                                exit(1);
                        }
                        uri = malloc(intrespons +1);
                        if ((i=recv(socket, uri, intrespons,MSG_WAITALL)) == -1) {
                                perror("Cant read uri");
                                exit(1);
                        }
			uri[intrespons] = '\0';

			printf("going to delete: %s from %s\n", uri, subname);

			/* Add docid to the gced file */
			{
				FILE *fh;
				unsigned int DocID, lastmodified;
				unsigned int lotNr;
				int err = 0;

				if (uriindex_get(uri, &DocID, &lastmodified, subname) == 0) {
					fprintf(stderr,"Unable to get uri info. uri=\"%s\",subname=\"%s\".",uri,subname);
					perror("Unable to get uri info");
					err++;
				}
				if (!err) {
					lotNr = rLotForDOCid(DocID);

					if ((fh = lotOpenFileNoCasheByLotNr(lotNr,"gced","a", 'e',subname)) == NULL) {
						perror("can't open gced file");
						err++;
					} else {
						fwrite(&DocID, sizeof(DocID), 1, fh);
						fclose(fh);
					}
				}
				if (!err) {
					struct reformat *re;

					if((re = reopen(rLotForDOCid(DocID), sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, RE_HAVE_4_BYTES_VERSION_PREFIX)) == NULL) {
						perror("can't reopen()");
						err++;
					} else {
						DIS_delete(RE_DocumentIndex(re, DocID));
						reclose(re);
					}
				}
				//markerer at den er skitten
				if (!err) {
					FILE *dirtfh;
					dirtfh = lotOpenFileNoCashe(DocID,"dirty","ab",'e',subname);
					fwrite("1",1,1,dirtfh);
					fclose(dirtfh);
				}
				if (err == 0) 
					bbdocument_delete(uri, subname);
			}
			free(subname);

			intrespons = 1; // Always return ok for now
                        if ((n=sendall(socket, &intrespons, sizeof(intrespons))) == -1) {
                                       perror("Cant recv filerest");
                                       exit(1);
                        }

		}
		else if (packedHedder.command == bbc_deletecollection) {
			printf("deletecollection\n");
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
			subname[intrespons] = '\0';


			printf("going to delete collection: %s\n", subname);

			intrespons = bbdocument_deletecoll(subname);

			if ((n=sendall(socket, &intrespons, sizeof(intrespons))) == -1) {
                	               perror("Cant recv filerest");
        	                       exit(1);
	               	}


			free(subname);
		}
		else if (packedHedder.command == bbc_addwhisper) {
			whisper_t add;
			char *subname;

			if ((i=recv(socket, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) 
				err(1, "Cant read intrespons");
			subname = malloc(intrespons+1);
			if ((i=recv(socket, subname, intrespons,MSG_WAITALL)) == -1) {
				perror("Cant read subname");
				exit(1);
			}
			subname[intrespons] = '\0';
			if ((i=recv(socket, &add, sizeof(add),MSG_WAITALL)) == -1) 
				err(1, "Cant read add whisper");

			gcwhisper_write(subname, add);
			free(subname);

		}
		else if (packedHedder.command == bbc_HasSufficientSpace) {

			char *subname;
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

			// tester bare i lot 1 her. Må også sjekke andre loter når vi begynner å støtte frlere disker på ES.
			intrespons = lotHasSufficientSpace(1, 4096, subname);

			if ((n=sendall(socket, &intrespons, sizeof(intrespons))) == -1) {
                	               perror("Cant recv filerest");
        	                       exit(1);
	               	}

			printf("~Asked for HasSufficientSpace for subname \"%s\". Returnerer %d\n",subname, intrespons);

			free(subname);
		}
		else {
			printf("unnown comand. %i\n", packedHedder.command);
		}
	}

	++count;


}

        #ifdef DEBUG_TIME
                gettimeofday(&tot_end_time, NULL);
                printf("Time debug: bbdn total time time: %f\n",getTimeDifference(&tot_start_time, &tot_end_time));
        #endif

}
