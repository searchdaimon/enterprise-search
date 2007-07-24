
/******************************/
#include "searchkernel.h"
#include "../common/boithohome.h"
#include "../common/bstr.h"

#include "../common/poprank.h"
#include "../common/integerindex.h"
#include "../common/adultWeight.h"
#include "../common/daemon.h"
#include "../acls/acls.h"
#include "../boithoadClientLib/liboithoaut.h"
#include "../common/timediff.h"
#include <sys/time.h>
#include "../parser/html_parser.h"
#include "../maincfg/maincfg.h"

#define _REENTRANT

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>


#define cfg_searchd "config/searchd.conf"

#ifdef WITH_THREAD
	#include <pthread.h>
#endif
/* the TCP port that is used for this example */
//#define TCP_PORT   6500

/* function prototypes and global variables */
void *do_chld(void *);
//mutex_t lock;
int	service_count;

//gloal variabel for å holde servernavn
char servername[32];

//#ifndef BLACK_BOKS
	#include <libconfig.h>
	struct config_t cfg;
//#endif

#ifdef WITH_PROFILING
	#define profiling_maxruncount 3
	static int profiling_runcount = 0;
#endif

struct iintegerMemArrayFormat global_DomainIDs;

int isInSubname(struct subnamesFormat *subnames,int nrOfSubnames,char s[]) {

	int i;

	for (i=0;i<nrOfSubnames;i++) {
		if (strcmp(subnames[i].subname,s) == 0) {
			return 1;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{


	int 	sockfd;
	//int newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;
	FILE *LOGFILE;
	FILE *LOCK;
	struct searchd_configFORMAT searchd_config;

	struct config_t maincfg;

        int searchport = 0;
	int optLog = 0;
        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"lp:"))!=-1) {
                switch (c) {
                        case 'p':
                                searchport = atoi(optarg);
				printf("will use port %i\n",searchport);
                                break;
                        case 'l':
				optLog = 1;
                                break;

			default:
                        	exit(1);
                }
        
	}
        --optind;

        printf("argc %i, optind %i\n",argc,optind);


	strncpy(servername,argv[1 +optind],sizeof(servername) -1);


	#ifdef BLACK_BOKS
	time_t starttime;

	time(&starttime);
	
	FILE *FH;

	#define stdoutlog "logs/searchdbb_stdout"
	#define stderrlog "logs/searchdbb_stderr"

	if (optLog) {
		printf("opening log \"%s\"\n",bfile(stdoutlog));
	
		if ((FH = freopen (bfile(stdoutlog), "a+", stdout)) == NULL) {
			perror(bfile(stdoutlog));

		}

		printf("opening log \"%s\"\n",bfile(stderrlog));
	
		if ((FH = freopen (bfile(stderrlog), "a+", stderr)) == NULL) {
			perror(bfile(stderrlog));

		}

		//setter filene til å være linjebuferet, akurat slik en terminal er, ikke block bufferede slik en fil er
		//hvis ikke får vi ikke med oss siste del hvis vi får en seg feil
		setvbuf(stdout, NULL, _IOLBF, 0); 
		setvbuf(stderr, NULL, _IOLBF, 0); 
	}
	
	printf("starting. Time %s",ctime(&starttime));
	fprintf(stderr,"Error test\n");
	#endif

        maincfg = maincfgopen();

	if (searchport == 0) {
        	searchport = maincfg_get_int(&maincfg,"BSDPORT");
	}

	searchd_config. cmc_port = maincfg_get_int(&maincfg,"CMDPORT");

	/***********************************************************************************/
	//prøver å få fil lock. Bare en deamon kan kjøre avgangen

	/*
	#ifndef ALLOW_MULTIPLE_SEARCHD
	if ((LOCK = fopen("/tmp/searchd.loc","w")) == NULL) {
		perror("lock file");
		exit(1);
	}

	if (flock(fileno(LOCK),LOCK_EX | LOCK_NB) != 0) {
		if (errno == EWOULDBLOCK) {
			printf("En annen prosses kjører allerede. Steng denne først.\n");
		}
		else {
			perror("cant get lock file");
		}
		exit(1);
	}
	#endif
	*/
	/***********************************************************************************/

	//#ifndef BLACK_BOKS
	config_setting_t *cfgarray;

  	/* Initialize the configuration */
  	config_init(&cfg);


  	/* Load the file */
	#ifdef DEBUG
  	printf("loading [%s]..\n",bfile(cfg_searchd));
	#endif

  	if (!config_read_file(&cfg, bfile(cfg_searchd))) {
    		printf("[%s]failed: %s at line %i\n",bfile(cfg_searchd),config_error_text(&cfg),config_error_line(&cfg));
		exit(1);
	}
	//#endif	

	html_parser_init();

	#ifdef WITH_THREAD
		pthread_t chld_thr;

		printf("starting whth thread\n");
	#else
		printf("starting single thread version\n");
	#endif

        if (argc < 2) {
                printf("Inget servernavn gitt.\n");
                exit(0);
        }


	printf("servername %s\n",servername);

	//ToDo: må ha låsing her
        if ((LOGFILE = bfopen("config/query.log","a")) == NULL) {
                perror(bfile("config/query.log"));
        }
        else {
                fprintf(LOGFILE,"starting server %s\n",servername);
                fclose(LOGFILE);
        }


	//starter opp
        //laster inn alle poprankene
        printf("loding pop MemArray\n");
        popopenMemArray(servername,"www"); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får et inn enda
        printf("done\n");

	printf("loding adultWeight MemArray\n");
	adultWeightopenMemArray(servername,"www"); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får et inn enda
	printf("done\n");


	iintegerLoadMemArray(&global_DomainIDs,"domainid",sizeof(unsigned short),servername, "www");

	IIndexInaliser();

	#ifdef WITH_MEMINDEX
		IIndexLoad();
	#endif

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fprintf(stderr,"server: can't open stream socket\n"), exit(0);




	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(searchport);
	printf("will bind to port %i\n",searchport);
	//seter at sokket kan rebrukes
        int yes=1;
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr,"server: can't bind local address. Port %i\n",searchport);
		exit(0);
	}	

	/* set the level of thread concurrency we desire */
	//thr_setconcurrency(5);

	listen(sockfd, 5);

	for(;;)
	{
		clilen = sizeof(cli_addr);
		searchd_config.newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if(searchd_config.newsockfd < 0) {
			//fprintf(stderr,"server: a

/***************************************/
			//fprintf(stderr,"server: accept error\n"), exit(0);
			fprintf(stderr,"server: accept error\n");
		}
		else {

			#ifdef WITH_THREAD
			/* create a new thread to process the incomming request */
				//thr_create(NULL, 0, do_chld, (void *) searchd_config, THR_DETACHED, &chld_thr);
				pthread_create(&chld_thr, NULL, do_chld, (void *) &searchd_config);
				/* the server is now free to accept another socket request */
			#else
				do_chld((void *) &searchd_config);	
			#endif
		}

	}

	html_parser_exit();
	
	return(0);
}


/* 
	This is the routine that is executed from a new thread 
*/
void *do_chld(void *arg)
{
	//int 	mysocfd = (int) arg;
	struct searchd_configFORMAT *searchd_config = arg;
	int   mysocfd = (*searchd_config).newsockfd;

	struct timeval globalstart_time, globalend_time;


	FILE *LOGFILE;
	char 	data[100];
	int 	i,n;
	struct queryNodeHederFormat queryNodeHeder;
	struct SiderFormat *Sider;
	int net_status;
	int ranking;

	//struct SiderFormat Sid

	int nrOfSubnames;
        struct subnamesFormat *subnames;

	config_setting_t *cfgstring;
	config_setting_t *cfgcollection;
	config_setting_t *cfgcollections;

	struct subnamesConfigFormat subnamesDefaultsConfig;
/***************************************/
	char **Data;
        int Count;

	//struct SiderFormat Sider[MaxsHits * 2];

	struct SiderHederFormat SiderHeder;

	gettimeofday(&globalstart_time, NULL);

	
	#ifdef WITH_THREAD
		printf("Child thread [%d]: Socket number = %d\n", pthread_self(), mysocfd);
	#else
		printf("Socket number = %d\n",mysocfd);
	#endif

	#ifdef DEBUG
        	struct timeval start_time, end_time;
	#endif

	/* read from the given socket */

	if ((i=recv(mysocfd, &queryNodeHeder, sizeof(queryNodeHeder),MSG_WAITALL)) == -1) {
		perror("recv");
	}

	//sender svar med en gang at vi kan gjøre dette
	net_status = net_CanDo;
	//if ((n=sendall(mysocfd,&net_status, sizeof(net_status))) != sizeof(net_status)) {
	if ((n=send(mysocfd,&net_status, sizeof(net_status),MSG_NOSIGNAL)) != sizeof(net_status)) {
		printf("send only %i of %i\n",n,sizeof(net_status));
		perror("sendall net_status");
	}


	printf("MaxsHits %i\n",queryNodeHeder.MaxsHits);
	Sider  = (struct SiderFormat *)malloc(sizeof(struct SiderFormat) * (queryNodeHeder.MaxsHits));

	printf("Ranking search?\n");


	//ToDo: må ha låsing her
        if ((LOGFILE = bfopen("config/query.log","a")) == NULL) {
                perror("logfile");
        }
        else {
                fprintf(LOGFILE,"%s\n",queryNodeHeder.query);
                fclose(LOGFILE);
        }

	printf("query:%s\n",queryNodeHeder.query);

	strcpy(SiderHeder.servername,servername);
	#ifdef BLACK_BOKS


	printf("username is \"%s\"\n",queryNodeHeder.search_user);
	struct userToSubnameDbFormat userToSubnameDb;
	char **respons_list;
	int responsnr;

	if (!userToSubname_open(&userToSubnameDb)) {
		printf("cant open users.db\n");
		//strcpy(queryNodeHeder.subname,"wikipedia,boithodocs");
	}
	else {
		char subnamebuf[maxSubnameLength];
		queryNodeHeder.subname[0] = '\0';
		boithoad_groupsForUser(queryNodeHeder.search_user,&respons_list,&responsnr);
	        printf("groups: %i\n",responsnr);
	        for (i=0;i<responsnr;i++) {
			printf("i= %i, responsnr = %i\n",i,responsnr);
	        	printf("group: \"%s\"\n",respons_list[i]);

			if (!userToSubname_getsubnamesAsString(&userToSubnameDb,respons_list[i],subnamebuf,sizeof(subnamebuf))) {
				printf("dosent apare to hav a subname for \"%s\"\n",respons_list[i]);
			}
			else {
				printf("godt subname from getsubnamesAsString as \"%s\"\n",subnamebuf);
				strlwcat(queryNodeHeder.subname,subnamebuf,sizeof(queryNodeHeder.subname));
				strlwcat(queryNodeHeder.subname,",",sizeof(queryNodeHeder.subname));
			}

	        }
		//fjerner ,
		queryNodeHeder.subname[strlen(queryNodeHeder.subname) -1] = '\0';
	        boithoad_respons_list_free(respons_list);
		userToSubname_close(&userToSubnameDb);
	}

	#endif



/***************************************/
	 


	if ((cfgcollections = config_lookup(&cfg, "collections")) == NULL) {
		
		printf("can't load \"collections\" from config\n");
		exit(1);
	}

	if ((cfgcollection = config_setting_get_member(cfgcollections, "defaults")) == NULL ) {
		printf("can't load \"collections defaults\" from config\n");
		exit(1);

	}


	/****************/
	if ( (cfgstring = config_setting_get_member(cfgcollection, "summary") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.summary = config_setting_get_string(cfgstring);

	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameUrl") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameUrl = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameUrl") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameUrl = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameDomain") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameDomain = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterTLDs") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterTLDs = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterResponse") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterResponse = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameCrc32") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.filterSameCrc32 = config_setting_get_bool(cfgstring);


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankAthorArray") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankAthorArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankAthorArrayLen) {
		subnamesDefaultsConfig.rankAthorArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankAthorArrayLen;i++) {
		subnamesDefaultsConfig.rankAthorArray[i] = config_setting_get_int_elem(cfgstring,i);
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankTittelArray") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankTittelArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankTittelArrayLen) {
		subnamesDefaultsConfig.rankTittelArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankTittelArrayLen;i++) {
		subnamesDefaultsConfig.rankTittelArray[i] = config_setting_get_int_elem(cfgstring,i);
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankHeadlineArray") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankHeadlineArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankHeadlineArrayLen) {
		subnamesDefaultsConfig.rankHeadlineArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankHeadlineArrayLen;i++) {
		subnamesDefaultsConfig.rankHeadlineArray[i] = config_setting_get_int_elem(cfgstring,i);
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankBodyArray") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankBodyArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankBodyArrayLen) {
		subnamesDefaultsConfig.rankBodyArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankBodyArrayLen;i++) {


		subnamesDefaultsConfig.rankBodyArray[i] = config_setting_get_int_elem(cfgstring,i);
	}

	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankUrlArray") ) == NULL) {
                printf("can't load \"summary\" from config\n");
                exit(1);
        }
	
	subnamesDefaultsConfig.rankUrlArrayLen = config_setting_length(cfgstring);
	if (BMAX_RANKARRAY < subnamesDefaultsConfig.rankUrlArrayLen) {
		subnamesDefaultsConfig.rankUrlArrayLen = BMAX_RANKARRAY;
	}
	for(i=0;i<subnamesDefaultsConfig.rankUrlArrayLen;i++) {
		subnamesDefaultsConfig.rankUrlArray[i] = config_setting_get_int_elem(cfgstring,i);
	}

	//rankTittelFirstWord
	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankTittelFirstWord") ) == NULL) {
                printf("can't load \"rankTittelFirstWord\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.rankTittelFirstWord = config_setting_get_int(cfgstring);

	//rankUrlMainWord
	if ( (cfgstring = config_setting_get_member(cfgcollection, "rankUrlMainWord") ) == NULL) {
                printf("can't load \"rankUrlMainWord\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.rankUrlMainWord = config_setting_get_int(cfgstring);

	if ( (cfgstring = config_setting_get_member(cfgcollection, "defaultthumbnail") ) == NULL) {
                printf("can't load \"defaultthumbnail\" from config\n");
        }
	else {
		subnamesDefaultsConfig.defaultthumbnail = config_setting_get_string(cfgstring);
	}


	if ( (cfgstring = config_setting_get_member(cfgcollection, "isPaidInclusion") ) == NULL) {
                printf("can't load \"isPaidInclusion\" from config\n");
                exit(1);
        }

	subnamesDefaultsConfig.isPaidInclusion = config_setting_get_bool(cfgstring);

	/****************/

	printf("subname \"%s\"\n",queryNodeHeder.subname);

	//dekoder subname

	Count = split(queryNodeHeder.subname, ",", &Data);

	subnames = malloc(sizeof(struct subnamesFormat) * Count ); 
	

  	Count = 0;
	nrOfSubnames = 0; 


	printf("nrOfSubnames %i\n",nrOfSubnames);
  	while( (Data[Count] != NULL) && (nrOfSubnames < MAX_COLLECTIONS)) {
    		printf("\t\taa: %d\t\"%s\"\n", Count, Data[Count]);

		//tar ikke med tomme subnames (som bare er en \0)
		if (Data[Count][0] == '\0') {

		}
		else if (isInSubname(subnames,nrOfSubnames,Data[Count])) {
			printf("all redy have \"%s\" as a subname\n",Data[Count]);
		} 
		else {
	    		printf("\t\taa: added : %d\t\"%s\" (len %i)\n", Count, Data[Count],strlen(Data[Count]));

			strscpy(subnames[nrOfSubnames].subname,Data[Count],sizeof(subnames[nrOfSubnames].subname));

			//setter at vi først skal bruke defult config
			subnames[nrOfSubnames].config = subnamesDefaultsConfig;

			if ((cfgcollection = config_setting_get_member(cfgcollections, subnames[nrOfSubnames].subname)) == NULL ) {
				printf("can't load \"collections\" from config for \"%s\"\n",subnames[nrOfSubnames].subname);

			}
			else {


				/****************/
				if ( (cfgstring = config_setting_get_member(cfgcollection, "summary") ) == NULL) {
                			printf("can't load \"summary\" from config\n");
        			}
				else {
				subnames[nrOfSubnames].config.summary = config_setting_get_string(cfgstring);
				}

				if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameUrl") ) == NULL) {
                			printf("can't load \"filterSameUrl\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.filterSameUrl = config_setting_get_bool(cfgstring);
				}


				if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameDomain") ) == NULL) {
                			printf("can't load \"filterSameDomain\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.filterSameDomain = config_setting_get_bool(cfgstring);
				}

				if ( (cfgstring = config_setting_get_member(cfgcollection, "filterTLDs") ) == NULL) {
                			printf("can't load \"filterTLDs\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.filterTLDs = config_setting_get_bool(cfgstring);
				}

				if ( (cfgstring = config_setting_get_member(cfgcollection, "filterResponse") ) == NULL) {
                			printf("can't load \"filterResponse\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.filterResponse = config_setting_get_bool(cfgstring);
				}

				if ( (cfgstring = config_setting_get_member(cfgcollection, "filterSameCrc32") ) == NULL) {
                			printf("can't load \"filterSameCrc32\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.filterSameCrc32 = config_setting_get_bool(cfgstring);
				}


				if ( (cfgstring = config_setting_get_member(cfgcollection, "rankTittelFirstWord") ) == NULL) {
                			printf("can't load \"rankTittelFirstWord\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.rankTittelFirstWord = config_setting_get_int(cfgstring);
				}

				//rankUrlMainWord
				if ( (cfgstring = config_setting_get_member(cfgcollection, "rankUrlMainWord") ) == NULL) {
                			printf("can't load \"rankUrlMainWord\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.rankUrlMainWord = config_setting_get_int(cfgstring);
				}

				printf("filterSameUrl: %i\n",subnames[nrOfSubnames].config.filterSameUrl);
				printf("filterSameDomain: %i\n",subnames[nrOfSubnames].config.filterSameDomain);
				printf("filterTLDs: %i\n",subnames[nrOfSubnames].config.filterTLDs);
				printf("filterResponse: %i\n",subnames[nrOfSubnames].config.filterResponse);
				printf("filterSameCrc32: %i\n",subnames[nrOfSubnames].config.filterSameCrc32);

				if ( (cfgstring = config_setting_get_member(cfgcollection, "rankAthorArray") ) == NULL) {
                			printf("can't load \"filterSameCrc32\" from config\n");
        			}
				else {

					subnames[nrOfSubnames].config.rankAthorArrayLen = config_setting_length(cfgstring);
					if (BMAX_RANKARRAY < subnames[nrOfSubnames].config.rankAthorArrayLen) {
						subnames[nrOfSubnames].config.rankAthorArrayLen = BMAX_RANKARRAY;
					}

					for(i=0;i<subnames[nrOfSubnames].config.rankAthorArrayLen;i++) {
						subnames[nrOfSubnames].config.rankAthorArray[i] = config_setting_get_int_elem(cfgstring,i);
					}

				}


				if ( (cfgstring = config_setting_get_member(cfgcollection, "rankTittelArray") ) == NULL) {
                			printf("can't load \"filterSameCrc32\" from config\n");
        			}
				else {

					subnames[nrOfSubnames].config.rankTittelArrayLen = config_setting_length(cfgstring);
					if (BMAX_RANKARRAY < subnames[nrOfSubnames].config.rankTittelArrayLen) {
						subnames[nrOfSubnames].config.rankTittelArrayLen = BMAX_RANKARRAY;
					}

					for(i=0;i<subnames[nrOfSubnames].config.rankTittelArrayLen;i++) {
						subnames[nrOfSubnames].config.rankTittelArray[i] = config_setting_get_int_elem(cfgstring,i);
					}

				}


				if ( (cfgstring = config_setting_get_member(cfgcollection, "rankHeadlineArray") ) == NULL) {
                			printf("can't load \"filterSameCrc32\" from config\n");
        			}
				else {

					subnames[nrOfSubnames].config.rankHeadlineArrayLen = config_setting_length(cfgstring);
					if (BMAX_RANKARRAY < subnames[nrOfSubnames].config.rankHeadlineArrayLen) {
						subnames[nrOfSubnames].config.rankHeadlineArrayLen = BMAX_RANKARRAY;
					}

					for(i=0;i<subnames[nrOfSubnames].config.rankHeadlineArrayLen;i++) {
						subnames[nrOfSubnames].config.rankHeadlineArray[i] = config_setting_get_int_elem(cfgstring,i);
					}

				}


				if ( (cfgstring = config_setting_get_member(cfgcollection, "rankBodyArray") ) == NULL) {
                			printf("can't load \"filterSameCrc32\" from config\n");
        			}
				else {

					subnames[nrOfSubnames].config.rankBodyArrayLen = config_setting_length(cfgstring);
					if (BMAX_RANKARRAY < subnames[nrOfSubnames].config.rankBodyArrayLen) {
						subnames[nrOfSubnames].config.rankBodyArrayLen = BMAX_RANKARRAY;
					}

					for(i=0;i<subnames[nrOfSubnames].config.rankBodyArrayLen;i++) {
						subnames[nrOfSubnames].config.rankBodyArray[i] = config_setting_get_int_elem(cfgstring,i);
					}

				}

				if ( (cfgstring = config_setting_get_member(cfgcollection, "rankUrlArray") ) == NULL) {
                			printf("can't load \"filterSameCrc32\" from config\n");
        			}
				else {

					subnames[nrOfSubnames].config.rankUrlArrayLen = config_setting_length(cfgstring);
					if (BMAX_RANKARRAY < subnames[nrOfSubnames].config.rankUrlArrayLen) {
						subnames[nrOfSubnames].config.rankUrlArrayLen = BMAX_RANKARRAY;
					}

					for(i=0;i<subnames[nrOfSubnames].config.rankUrlArrayLen;i++) {
						subnames[nrOfSubnames].config.rankUrlArray[i] = config_setting_get_int_elem(cfgstring,i);
					}

				}


				if ( (cfgstring = config_setting_get_member(cfgcollection, "defaultthumbnail") ) == NULL) {
                			printf("can't load \"defaultthumbnail\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.defaultthumbnail = config_setting_get_string(cfgstring);
				}

				if ( (cfgstring = config_setting_get_member(cfgcollection, "isPaidInclusion") ) == NULL) {
                			printf("can't load \"isPaidInclusion\" from config\n");
        			}
				else {
					subnames[nrOfSubnames].config.isPaidInclusion = config_setting_get_bool(cfgstring);
				}


				/****************/
			
			}

			
			printf("nrOfSubnames a %i\n",nrOfSubnames);	
			++nrOfSubnames;
			printf("nrOfSubnames b %i\n",nrOfSubnames);

		}
		++Count;

	}


	printf("\n");

	FreeSplitList(Data);

/*
	nrOfSubnames = 1;

	subnames = malloc(sizeof(struct subnamesFormat) * nrOfSubnames);

	strscpy(subnames[0].subname,"www",sizeof(subnames[0].subname));
*/

	for (i=0;i<nrOfSubnames;i++) {
		printf("subname nr %i: \"%s\"\n",i,subnames[i].subname);
	}

	SiderHeder.filtypesnrof = MAXFILTYPES;

	SiderHeder.errorstrlen=sizeof(SiderHeder.errorstr);
	//v3 dosearch(queryNodeHeder.query, strlen(queryNodeHeder.query),Sider,&SiderHeder,SiderHeder.hiliteQuery,servername,subnames,SiderHeder.nrOfSubnames,queryNodeHeder.MaxsHits,queryNodeHeder.start, queryNodeHeder.filterOn, queryNodeHeder.languageFilter);

	printf("queryNodeHeder.getRank %i\n",queryNodeHeder.getRank);

	if (!queryNodeHeder.getRank) {

		if (!dosearch(queryNodeHeder.query, strlen(queryNodeHeder.query),Sider,&SiderHeder,SiderHeder.hiliteQuery,
			servername,subnames,nrOfSubnames,queryNodeHeder.MaxsHits,
			queryNodeHeder.start, queryNodeHeder.filterOn, 
			"",queryNodeHeder.orderby,SiderHeder.dates,queryNodeHeder.search_user,
			&SiderHeder.filters,
			searchd_config,
			SiderHeder.errorstr, &SiderHeder.errorstrlen,
			&global_DomainIDs, queryNodeHeder.HTTP_USER_AGENT
			)) 
		{
			printf("dosearch did not return 1\n");
			SiderHeder.responstype 	= searchd_responstype_error;
			//setter at vi ikke hadde noen svar
			SiderHeder.TotaltTreff 	= 0;
			SiderHeder.showabal	= 0;

			printf("Error: cand do dosearch: \"%s\"\n",SiderHeder.errorstr);
		}
	}
	else if (queryNodeHeder.getRank)  {
		printf("########################################### Ranking document: %d\n", queryNodeHeder.getRank);
		if (dorank(queryNodeHeder.query, strlen(queryNodeHeder.query),Sider,&SiderHeder,SiderHeder.hiliteQuery,
			servername,subnames,nrOfSubnames,queryNodeHeder.MaxsHits,
			queryNodeHeder.start, queryNodeHeder.filterOn, 
			"",queryNodeHeder.orderby,SiderHeder.dates,queryNodeHeder.search_user,
			&SiderHeder.filters,
			searchd_config,
			SiderHeder.errorstr, &SiderHeder.errorstrlen,
			&global_DomainIDs, RANK_TYPE_FIND, queryNodeHeder.getRank, &ranking
		)) {
			int status;
			int n;
			
			if (ranking == -1) {
				status = net_nomatch;
				if ((n=send(mysocfd, &status, sizeof(status),0)) != sizeof(status)) {
					printf("send only %i of %i\n",n,sizeof(status));
					perror("sendall status");
					return;
				}
			} else {
				status = net_match;
				if ((n=send(mysocfd, &status, sizeof(status),0)) != sizeof(status)) {
					printf("send only %i of %i\n",n,sizeof(status));
					perror("sendall status2");
					return;
				}
				if ((n=send(mysocfd, &ranking, sizeof(ranking),0)) != sizeof(ranking)) {
					printf("send only %i of %i\n",n,sizeof(ranking));
					perror("sendall ranking");
					return;
				}
			}

			if (recv(mysocfd, &ranking, sizeof(ranking), 0) == -1) {
				perror("recv ranking");
				return;
			}

			if (!dorank(queryNodeHeder.query, strlen(queryNodeHeder.query),Sider,&SiderHeder,SiderHeder.hiliteQuery,
				servername,subnames,nrOfSubnames,queryNodeHeder.MaxsHits,
				queryNodeHeder.start, queryNodeHeder.filterOn, 
				"",queryNodeHeder.orderby,SiderHeder.dates,queryNodeHeder.search_user,
				&SiderHeder.filters,
				searchd_config,
				SiderHeder.errorstr, &SiderHeder.errorstrlen,
				&global_DomainIDs, RANK_TYPE_SUM, 0/*queryNodeHeder.getRank*/, &ranking)) {

				perror("Got some kind of an error?");
				return;
			} else {
				printf("Let us see how this ranking went: %d\n", ranking);
				if ((n=send(mysocfd, &ranking, sizeof(ranking),0)) != sizeof(ranking)) {
					printf("send only %i of %i\n",n,sizeof(ranking));
					perror("sendall ranking2");
					return;
				}
			}

			SiderHeder.responstype = searchd_responstype_ranking;
			return;
		} else {
			SiderHeder.responstype = searchd_responstype_error;
			SiderHeder.TotaltTreff 	= 0;
			SiderHeder.showabal	= 0;
		}


		printf("doRank()\n");
		//setter at vi ikke hadde noen svar
	}
	else {
		SiderHeder.responstype = searchd_responstype_normalsearch;
	}

	//kopierer inn subnames. Kan bare sende over MAX_COLLECTIONS, men søker i alle


	for (i=0;((i<MAX_COLLECTIONS) && (i<nrOfSubnames));i++) {
		//memcpy(SiderHeder.subnames[i],subnames[i],sizeof(struct subnamesFormat));
		SiderHeder.subnames[i] = subnames[i];
	}
	SiderHeder.nrOfSubnames = i--;

	printf("subnames:\n");
	for (i=0;i<SiderHeder.nrOfSubnames;i++) {
		printf("\t%s: %i\n",SiderHeder.subnames[i].subname,SiderHeder.subnames[i].hits);
	}
	printf("\n");

	//finer først tiden vi brukte
        gettimeofday(&globalend_time, NULL);
        SiderHeder.total_usecs = getTimeDifference(&globalstart_time,&globalend_time);


	printf("TotaltTreff %i,showabal %i,filtered %i,total_usecs %f\n",SiderHeder.TotaltTreff,SiderHeder.showabal,SiderHeder.filtered,SiderHeder.total_usecs);

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	/*
	//finner faktisk showabal, vi kan ha slettede sider. Vil ikke sende de
	for(i=0;i<SiderHeder.showabal;i++) {
		if (!Sider[i].deletet) {

		}
	}
	*/


	//if ((n=sendall(mysocfd,&SiderHeder, sizeof(SiderHeder))) != sizeof(SiderHeder)) {
	if ((n=send(mysocfd,&SiderHeder, sizeof(SiderHeder),MSG_NOSIGNAL)) != sizeof(SiderHeder)) {
		printf("send only %i of %i\n",n,sizeof(SiderHeder));
		perror("sendall SiderHeder");
	}
	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: sending SiderHeder %f\n",getTimeDifference(&start_time,&end_time));
	#endif
	

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	/*
	for(i=0;i<SiderHeder.showabal;i++) {
	//for (i=0;i<queryNodeHeder.MaxsHits;i++) {
		//if (!Sider[i].deletet) {		
			printf("sending %s, deletet %i\n",Sider[i].DocumentIndex.Url,Sider[i].deletet);	
			//printf("bb: -%s-\n",Sider[i].title);
			if ((n=sendall(mysocfd,&Sider[i], sizeof(struct SiderFormat))) != sizeof(struct SiderFormat)) {
				printf("send only %i of %i\n",n,sizeof(struct SiderFormat));
				perror("sendall");
			}		

		//}
		//else {
		//	printf("page is deleted\n");
		//}
	}
	*/	

	//if ((n=sendall(mysocfd,Sider, sizeof(struct SiderFormat) * queryNodeHeder.MaxsHits)) != (sizeof(struct SiderFormat) * queryNodeHeder.MaxsHits)) {

	if ((n=send(mysocfd,Sider, sizeof(struct SiderFormat) * queryNodeHeder.MaxsHits, MSG_NOSIGNAL)) != (sizeof(struct SiderFormat) * queryNodeHeder.MaxsHits)) {
		printf("send only %i of %i\n",n,sizeof(struct SiderFormat)*queryNodeHeder.MaxsHits);
		perror("sendall");
	}		
	
	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: sendig sider %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	/* close the socket and exit this thread */
	close(mysocfd);
	//thr_exit((void *)0);

	free(Sider);
	free(subnames);

	printf("exiting\n");

	//pthread_exit((void *)0); /* exit with status */


/***************************************/
		#ifdef WITH_PROFILING
			++profiling_runcount;

			if (profiling_runcount >= profiling_maxruncount) {
				printf("exiting to do profiling. Hav done %i runs\n",profiling_runcount);
				sleep(1);
				exit(1);
			}
			printf("hav runed %i times\n",profiling_runcount);
		#endif


 }
