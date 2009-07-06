#ifndef BLACK_BOKS
	// runarb, 14 mai 2008:
	// for posix_fadvise()
	// dette kan skape problemer i andre header filer!!!!!
	#define _XOPEN_SOURCE 600
#endif

//preopen
#include "../common/lot.h"


#include "../common/boithohome.h"
#include "../common/bstr.h"

#include "../common/poprank.h"
#include "../common/integerindex.h"
#include "../common/adultWeight.h"
#include "../common/daemon.h"
#include "../common/iindex.h"
#include "../acls/acls.h"
#include "../boithoadClientLib/liboithoaut.h"
#include "../common/timediff.h"
#include "../parser/html_parser.h"
#include "../maincfg/maincfg.h"
#include "../dp/dp.h"
#include "../query/query_parser.h"
#include "../query/stemmer.h"
#include "../ds/dcontainer.h"
#include "../ds/dmap.h"

#define _REENTRANT

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <err.h>
#include <signal.h>
#include <locale.h>

#include "../common/sigaction.h"

#include "preopen.h"
#include "verbose.h"
#include "searchkernel.h"

#ifdef WITH_SPELLING
	#include "../newspelling/spelling.h"
#endif

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

//global variabel for å holde servernavn
char servername[MAX_SERVERNAME_LEN];

//#ifndef BLACK_BOKS
	#include <libconfig.h>
	struct config_t cfg;
//#endif

#ifdef WITH_PROFILING
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


#ifdef WITH_SPELLING
spelling_t spelling;

void
init_spelling(char *dict)
{
	fprintf(stderr, "searchd: init_spelling()\n");
	//spelling = spelling_init(lang);
	train(&spelling, bfile(dict));
}

void
catch_sigusr1(int sig)
{
	fprintf(stderr, "searchd: Warning! Caught sigusr1. Reinitializing spelling.\n");
	untrain(&spelling);
	init_spelling("var/dictionarywords");

}
#endif


/* The signal handler exit the program. . */
void
catch_alarm (int sig)
{
	fprintf(stderr, "searchd: Warning! Recieved alarm signal. Exiting.\n");
	exit(1);
}


void lotPreOpenStartl(int *preOpen[], char filename[], char subname[], int use) {

	int i, n;
	char buf[1];

	if ((*preOpen = malloc(sizeof(int) * maxLots)) == NULL) {
		perror("malloc preOpen");
		exit(-1);
	}

	for (i=0;i<maxLots;i++) {

		(*preOpen)[i] = -1;

		#ifndef BLACK_BOKS
			if (use) {
				(*preOpen)[i] = lotOpenFileNoCasheByLotNrl(i,filename,"rb", 'r',subname);
				if ((*preOpen)[i] != -1) {

					if ((n=posix_fadvise((*preOpen)[i], 0,0,POSIX_FADV_RANDOM)) == 0) {
	
					} 

					read((*preOpen)[i],&buf,sizeof(buf));
					printf("opening %i, as %i\n",i,(*preOpen)[i]);
				}
			}
		#endif
		
	}

}

extern unsigned int spelling_min_freq;

int main(int argc, char *argv[])
{
	fprintf(stderr, "searchd: Initializing...\n");

	int 	sockfd;
	int runCount;
	//int newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;
	FILE *LOGFILE;
	//FILE *LOCK;
	struct searchd_configFORMAT searchd_config;

	struct config_t maincfg;

        searchd_config.searchport = 0;
	searchd_config.optLog = 0;
	searchd_config.optMax = 0;
	searchd_config.optSingle = 0;
	searchd_config.optrankfile = NULL;
	searchd_config.optPreOpen = 0;
	searchd_config.optFastStartup = 0;
	searchd_config.optCacheIndexes = 1;
	
	// Needed for the speller to properly convert utf8 to wchar_t
	setlocale(LC_ALL, "en_US.UTF-8");

	/* Ignore collection updates for now */
	signal(SIGUSR2, SIG_IGN);
	/* And ignore spelling updates */
	signal(SIGUSR1, SIG_IGN);

        char c;
        while ((c=getopt(argc,argv,"clp:m:b:vsofS:"))!=-1) {
                switch (c) {
                        case 'p':
                                searchd_config.searchport = atoi(optarg);
				fprintf(stderr, "searchd: Option -p: Using port %i.\n",searchd_config.searchport);
                                break;
                        case 'l':
				searchd_config.optLog = 1;
                                break;
                        case 'o':
				searchd_config.optPreOpen = 1;
                                break;
                        case 'm':
				searchd_config.optMax = atoi(optarg);
                                break;
                        case 'b':
				searchd_config.optrankfile = optarg;
                                break;
                        case 'v':
				fprintf(stderr, "searchd: Option -v: Verbose output.\n");
				globalOptVerbose = 1;
                                break;
                        case 's':
				fprintf(stderr, "searchd: Option -s: Won't fork for new connections\n");
				searchd_config.optSingle = 1;
                                break;
			case 'f':
				searchd_config.optFastStartup = 1;
				break;
			case 'c':
				searchd_config.optCacheIndexes = 0;
				break;
			case 'S':
				spelling_min_freq = strtol(optarg, NULL, 10);
				break;
			default:
				errx(1, "Unknown argument: %c", c);
                }
        
	}

	#ifdef BLACK_BOKS
	fprintf(stderr, "searchd: Blackboxmode (searchdbb).\n");

	time_t starttime;

	time(&starttime);

	FILE *FH;

	#define stdoutlog "logs/searchdbb_stdout"
	#define stderrlog "logs/searchdbb_stderr"

	if (searchd_config.optLog) {
		fprintf(stderr, "searchd: Opening log \"%s\"\n",bfile(stdoutlog));
	
		if ((FH = freopen (bfile(stdoutlog), "a+", stdout)) == NULL) {
			perror(bfile(stdoutlog));

		}

		fprintf(stderr, "searchd: Opening log \"%s\"\n",bfile(stderrlog));
	
		if ((FH = freopen (bfile(stderrlog), "a+", stderr)) == NULL) {
			perror(bfile(stderrlog));

		}

		//setter filene til å være linjebuferet, akurat slik en terminal er, ikke block bufferede slik en fil er
		//hvis ikke får vi ikke med oss siste del hvis vi får en seg feil
		setvbuf(stdout, NULL, _IOLBF, 0); 
		setvbuf(stderr, NULL, _IOLBF, 0); 
	}

	/* Write pidfile */
	FILE  *pidfile = fopen(bfile("var/searchd.pid"), "w");

	if (pidfile != NULL) {
		fprintf(pidfile, "%d", getpid());
		fclose(pidfile);
	}

	fprintf(stderr, "searchd: Starting. Time is %s",ctime(&starttime));
	#endif



	#ifdef DEBUG
        fprintf(stderr, "searchd: Debug: argc %i, optind %i\n",argc,optind);
	#endif

	if (searchd_config.optrankfile == NULL) {
		searchd_config.optrankfile = "Brank";
	}

	#ifdef WITH_SPELLING
	if (searchd_config.optFastStartup != 1) {
		init_spelling("var/dictionarywords");
	}
	#endif

	if (argc > 0) {
		strncpy(servername,argv[optind], sizeof(servername) -1);
	} else {
		errx(1, "You have to supply a hostname");
	}
	
	lotPreOpenStartl(&searchd_config.lotPreOpen.DocumentIndex,"DocumentIndex","www",searchd_config.optPreOpen);
	lotPreOpenStartl(&searchd_config.lotPreOpen.Summary,"summary","www",searchd_config.optPreOpen);

#ifdef BLACK_BOKS
	if (searchd_config.optCacheIndexes == 1) {
		if (searchd_config.optFastStartup != 1) {
			printf("Reading indexes...\n");
			cache_indexes(0);
			printf("Cached indexes: %dMB, cached indexes: %d\n", indexcachescached[0]/(1024*1024), indexcachescached[1]);
			preopen();
			cache_fresh_lot_collection();

			cache_indexes_keepalive();
			signal(SIGUSR2, cache_indexes_hup);
		}
		else {
			signal(SIGUSR2, SIG_IGN);
		}
	} else {
		signal(SIGUSR2, SIG_IGN);
	}

#endif


        maincfg = maincfgopen();

	if (searchd_config.searchport == 0) {
        	searchd_config.searchport = maincfg_get_int(&maincfg,"BSDPORT");
	}

	searchd_config.cmc_port = maincfg_get_int(&maincfg,"CMDPORT");

	maincfgclose(&maincfg);

	#ifdef WITH_SPELLING
		signal(SIGUSR1, catch_sigusr1);
	#endif
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

  	/* Initialize the configuration */
  	config_init(&cfg);


  	/* Load the file */
	#ifdef DEBUG
  	fprintf(stderr, "searchd: Debug: Loading [%s] ...\n",bfile(cfg_searchd));
	#endif

  	if (!config_read_file(&cfg, bfile(cfg_searchd))) {
    		fprintf(stderr, "searchd: Error! [%s] failed: %s at line %i\n",bfile(cfg_searchd),config_error_text(&cfg),config_error_line(&cfg));
		exit(1);
	}
	//#endif	

	html_parser_init();

	/*
	#ifdef WITH_THREAD
		pthread_t chld_thr;

		printf("starting whth thread\n");
	#else
		printf("starting single thread version\n");
	#endif
	*/

	fprintf(stderr, "searchd: Servername %s\n",servername);

	//ToDo: må ha låsing her
        if ((LOGFILE = bfopen("config/query.log","a")) == NULL) {
                perror(bfile("config/query.log"));
        }
        else {
                fprintf(LOGFILE,"starting server %s\n",servername);
                fclose(LOGFILE);
        }


	#ifdef BLACK_BOKS
		// Initialiser thesaurus med ouput-filene fra 'build_thesaurus_*':
		searchd_config.thesaurus_all = NULL;
#ifndef WITHOUT_THESAURUS
		printf("init thesaurus\n");

		if (searchd_config.optFastStartup != 1) {
    			//searchd_config.thesaurusp = thesaurus_init(bfile("data/thesaurus.text"), bfile2("data/thesaurus.id"));
			searchd_config.thesaurus_all = load_all_thesauruses(bfile("data/thesaurus/"));
		}
		printf("init thesaurus done\n");

		if (searchd_config.thesaurus_all == NULL)
		    {
			fprintf(stderr, "searchd: ERROR!! Unable to open thesauruses. Disabling stemming.\n");
		    }

#endif
		fprintf(stderr, "searchd: init file-extensions\n");
		searchd_config.getfiletypep = fte_init(bfile("config/file_extensions.conf"));
		if (searchd_config.getfiletypep == NULL)
		    {
			fprintf(stderr, "searchd: ERROR!! Unable to open file-extensions configuration file. Disabling file-extensions.\n");
		    }

		fprintf(stderr, "searchd: init attribute descriptions\n");
		searchd_config.attrdescrp = adf_init(bfile("config/attribute_descriptions.conf"));
		if (searchd_config.attrdescrp == NULL)
		    {
			fprintf(stderr, "searchd: ERROR!! Unable to open attribute descriptions configuration file. Disabling attribute descriptions.\n");
		    }

		fprintf(stderr, "searchd: init show-attributes\n");
		char	*warnings;
		/*searchd_config.showattrp = show_attributes_init(bfile("config/show_attributes.conf"), &warnings);
		if (searchd_config.showattrp == NULL)
		    {
			fprintf(stderr, "searchd: ERROR!! Unable to open show-attributes configuration file. Disabling attributes.\n");
		    }
		else if (warnings[0]!='\0')
		    {
			fprintf(stderr, "searchd: ******************* Warnings reading show-attributes config: ********************\n");
			fprintf(stderr, "%s", warnings);
			fprintf(stderr, "searchd: *********************************************************************************\n");
		    }*/

	#else

		//starter opp
		fprintf(stderr, "searchd: Loading domain-ids..."); fflush(stderr);
		iintegerLoadMemArray2(&global_DomainIDs,"domainid",sizeof(unsigned short), "www");
		fprintf(stderr, "done\n");

        	//laster inn alle poprankene
        	fprintf(stderr, "searchd: Loading pop MemArray..."); fflush(stderr);
        	popopenMemArray2("www",searchd_config.optrankfile); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får et inn enda
        	fprintf(stderr, "done\n");

		fprintf(stderr, "searchd: Loading adultWeight MemArray..."); fflush(stderr);
		adultWeightopenMemArray2("www"); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får et inn enda
		fprintf(stderr, "done\n");
	#endif


	IIndexInaliser();

	#ifdef WITH_MEMINDEX
		IIndexLoad();
	#endif

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fprintf(stderr,"searchd: Server error! Can't open stream socket.\n"), exit(0);




	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(searchd_config.searchport);
	fprintf(stderr, "searchd: Will bind to port %i\n",searchd_config.searchport);
	//seter at sokket kan rebrukes
        int yes=1;
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr,"searchd: Server error! Can't bind local address. Port %i\n",searchd_config.searchport);
		exit(0);
	}	

	/* set the level of thread concurrency we desire */
	//thr_setconcurrency(5);

	listen(sockfd, 5);

	runCount = 0;

#ifndef NEW_CHILD_CATCHER
	signal(SIGCLD, SIG_IGN);  /* now I don't have to wait() for forked children! */
#else
	{
		struct sigaction sa;
		int ret;

		sa.sa_sigaction = sigchild_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_SIGINFO;

		ret = sigaction(SIGCHLD, &sa, 0);
		if (ret) {
			errx(1, "sigaction()");
		}
	}
#endif

	printf("|------------------------------------------------------------------------------------------------|\n");
	printf("|%-40s | %-11s | %-11s | %-11s | %-11s|\n","query", "TotaltTreff", "showabal", "filtered", "total_usecs");
	printf("|------------------------------------------------------------------------------------------------|\n");

	for((clilen = sizeof(cli_addr));;)
	{
		searchd_config.newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if(searchd_config.newsockfd < 0) {
			/* Just restart */
			if (errno == EINTR)
				continue;

			warn("searchd: Server warning! Accept error");
		}
		else {

			if (searchd_config.optSingle) {
				do_chld((void *) &searchd_config);
			}
			else {
			#ifdef DEBUG
				fprintf(stderr, "searchd: Debug mode; will not fork to new process.\n");
				do_chld((void *) &searchd_config);
			#else
				/*
				#ifdef WITH_THREAD
			 		//create a new thread to process the incomming request
					//thr_create(NULL, 0, do_chld, (void *) searchd_config, THR_DETACHED, &chld_thr);
					pthread_create(&chld_thr, NULL, do_chld, (void *) &searchd_config);
					//the server is now free to accept another socket request
				#else
					do_chld((void *) &searchd_config);	
				#endif
				*/
				vboprintf("searchd: Forking new prosess.\n");
				if (fork() == 0) { // this is the child process

					close(sockfd); // child doesn't need the listener

					do_chld((void *) &searchd_config);	

					close(searchd_config.newsockfd);
					vboprintf("searchd: Terminating child.\n");
		
					exit(0);
				}
				else {
					close(searchd_config.newsockfd); // perent doesn't need the new socket
				}
			#endif
			}
		}

		++runCount;

		if ((searchd_config.optMax != 0) && (runCount >= searchd_config.optMax)) {
			//venter på siste trå. Ikke helt optimalt dette, da vi kan ha flere tråer som kjører i paralell
			/*
			#ifdef WITH_THREAD
				pthread_join(chld_thr, NULL);
			#endif
			*/
			fprintf(stderr, "searchd: have reached Max runs. Exiting\n");
			break;
		}

	}

	html_parser_exit();
	
	return(0);
}

attr_conf *parse_navmenu_cfg(char *cfgstr, int *failed) {
	char *warnings;
	attr_conf *cfg = show_attributes_init(cfgstr, &warnings, failed);
	
	if (*failed)
		warnx("navmenucfg parsing failed");

	if (warnings[0] != '\0')
		warnx("navmenucfg parsing warnings: %s", warnings);

	return cfg;
}

/* 
	This is the routine that is executed from a new thread 
*/
void *do_chld(void *arg)
{
	vboprintf("searchd_child: Starting new thread.\n");
	vboprintf("searchd: do_chld()\n");
	//int 	mysocfd = (int) arg;
	struct searchd_configFORMAT *searchd_config = arg;
	int   mysocfd = (*searchd_config).newsockfd;

	struct timeval globalstart_time, globalend_time;


	FILE *LOGFILE;
	//char 	data[100];
	int 	i,n;
	struct queryNodeHederFormat queryNodeHeder;
	struct SiderFormat *Sider;
	int net_status;
	int ranking;
	struct hashtable *crc32hashmap;

	//struct SiderFormat Sid

	int nrOfSubnames;

	config_setting_t *cfgstring;
	config_setting_t *cfgcollection;
	config_setting_t *cfgcollections;

	struct subnamesConfigFormat subnamesDefaultsConfig;
	char **Data;
        int Count;

	
        char groupOrQuery[1024];
        groupOrQuery[0] = '\0';



	//struct SiderFormat Sider[MaxsHits * 2];

	struct SiderHederFormat *SiderHeder;


	dp_priority_locl_start();


	if ((SiderHeder  = malloc(sizeof(struct SiderHederFormat))) == NULL) {
		perror("malloc");
		fprintf(stderr, "searchd: ~do_chld()\n");
		return 0;
	}


	gettimeofday(&globalstart_time, NULL);

	//make a timeout
	/* Establish a handler for SIGALRM signals. */
       	signal (SIGALRM, catch_alarm);

	/* Set an alarm to go off in a little while. This so we don't run forever if we get en ever loop */
       	alarm (60);

	
	#ifdef WITH_THREAD
		vboprintf("Child thread [%d]: Socket number = %d\n", pthread_self(), mysocfd);
	#else
		vboprintf("Socket number = %d\n",mysocfd);
	#endif

	#ifdef DEBUG
        	struct timeval start_time, end_time;
	#endif

	/* read from the given socket */

	if ((i=recv(mysocfd, &queryNodeHeder, sizeof(queryNodeHeder),MSG_WAITALL)) == -1) {
		perror("recv");
	}

	// Read collections /w cfg.
	if ((recv(mysocfd, &nrOfSubnames, sizeof nrOfSubnames, MSG_WAITALL)) == -1)
		perror("recv nrOfSubnames");

	struct subnamesFormat subnames[nrOfSubnames];
	if (nrOfSubnames > 0) {
		if ((recv(mysocfd, &subnames, sizeof subnames, MSG_WAITALL)) == -1)
			perror("recv subnames");
	}


	//sender svar med en gang at vi kan gjøre dette
	net_status = net_CanDo;
	//if ((n=sendall(mysocfd,&net_status, sizeof(net_status))) != sizeof(net_status)) {
	if ((n=send(mysocfd,&net_status, sizeof(net_status),MSG_NOSIGNAL)) != sizeof(net_status)) {
		fprintf(stderr, "searchd_child: Warning! Sent only %i of %i bytes at %s:%d\n",n,sizeof(net_status),__FILE__,__LINE__);
		perror("sendall net_status");
	}


	vboprintf("MaxsHits %i\n",queryNodeHeder.MaxsHits);
	//Sider  = (struct SiderFormat *)malloc(sizeof(struct SiderFormat) * (queryNodeHeder.MaxsHits));
	vboprintf("Ranking search?\n");


	//ToDo: må ha låsing her
        if ((LOGFILE = bfopen("config/query.log","a")) == NULL) {
                perror("logfile");
        }
        else {
                fprintf(LOGFILE,"%s\n",queryNodeHeder.query);
                fclose(LOGFILE);
        }

	vboprintf("searchd_child: Incoming query: %s\n",queryNodeHeder.query);

	strcpy(SiderHeder->servername,servername);

#ifndef WITHOUT_THESAURUS
	// TODO: Denne må skalere til flere språk:
	char	*lang = NULL;
	switch (queryNodeHeder.lang)
	    {
		case LANG_NBO: lang = "nbo"; break;
		case LANG_ENG: lang = "eng"; break;
	    }

	printf("[searchd] lang_id = %i\n", queryNodeHeder.lang);
	printf("[searchd] lang = %s\n", lang);

	if (lang!=NULL)
	    {
		iterator	it = map_find(searchd_config->thesaurus_all, lang);
		if (it.valid)
		    {
			printf("[searchd] Loading %s thesaurus\n", lang);
			searchd_config->thesaurusp = map_val(it).ptr;
		    }
		else
		    {
			printf("[searchd] No thesaurus for %s\n", lang);
			searchd_config->thesaurusp = NULL;
		    }
	    }
#endif

/***************************************/
	 	/****************/

	//dekoder subname

	vboprintf("nrOfSubnames %i\n",nrOfSubnames);
  	
/*
	nrOfSubnames = 1;

	subnames = malloc(sizeof(struct subnamesFormat) * nrOfSubnames);

	strscpy(subnames[0].subname,"www",sizeof(subnames[0].subname));
*/

	#ifdef DEBUG
	fprintf(stderr, "searchd_child: \n##########################################################\n");
	fprintf(stderr, "searchd_child: subnames:\nTotal of %i\n",nrOfSubnames);
	for (i=0;i<nrOfSubnames;i++) {
		fprintf(stderr, "searchd_child: subname nr %i: \"%s\"\n",i,subnames[i].subname);
	}
	fprintf(stderr, "searchd_child: ##########################################################\n\n");
	#endif

	SiderHeder->filtypesnrof = MAXFILTYPES;

	SiderHeder->errorstrlen=sizeof(SiderHeder->errorstr);

	#ifdef DEBUG
	fprintf(stderr, "searchd_child: queryNodeHeder.getRank %u\n",queryNodeHeder.getRank);
	#endif

	if (!queryNodeHeder.getRank) {

		int parsing_failed;
		attr_conf *navmenu_cfg = parse_navmenu_cfg(queryNodeHeder.navmenucfg, &parsing_failed);
		if (parsing_failed) {
			SiderHeder->responstype = searchd_responstype_error;
			snprintf(SiderHeder->errorstr, sizeof SiderHeder->errorstr, 
				"An error occurred while parsing configuration for navigation menu.");
			SiderHeder->errorstrlen = strlen(SiderHeder->errorstr); // TODO: is errorstrlen even used?
		}


		if (!dosearch(queryNodeHeder.query, strlen(queryNodeHeder.query),&Sider,SiderHeder,SiderHeder->hiliteQuery,
			servername,subnames,nrOfSubnames,queryNodeHeder.MaxsHits,
			queryNodeHeder.start, queryNodeHeder.filterOn, 
			"",queryNodeHeder.orderby,SiderHeder->dates,queryNodeHeder.search_user,
			&SiderHeder->filters,
			searchd_config,
			SiderHeder->errorstr, &SiderHeder->errorstrlen,
			&global_DomainIDs, queryNodeHeder.HTTP_USER_AGENT,
			groupOrQuery, queryNodeHeder.anonymous, navmenu_cfg
			)) 
		{
			fprintf(stderr, "searchd_child: dosearch did not return 1\n");
			SiderHeder->responstype 	= searchd_responstype_error;
			//setter at vi ikke hadde noen svar
			SiderHeder->TotaltTreff 	= 0;
			SiderHeder->showabal		= 0;

			fprintf(stderr, "searchd_child: Error: cand do dosearch: \"%s\"\n",SiderHeder->errorstr);
		}

		show_attributes_destroy(navmenu_cfg);

		
	}
	else if (queryNodeHeder.getRank)  {
		fprintf(stderr, "searchd_child: ########################################### Ranking document: %u\n", queryNodeHeder.getRank);

		if (dorank(queryNodeHeder.query, strlen(queryNodeHeder.query),&Sider,SiderHeder,SiderHeder->hiliteQuery,
			servername,subnames,nrOfSubnames,queryNodeHeder.MaxsHits,
			queryNodeHeder.start, queryNodeHeder.filterOn, 
			"",queryNodeHeder.orderby,SiderHeder->dates,queryNodeHeder.search_user,
			&SiderHeder->filters,
			searchd_config,
			SiderHeder->errorstr, &SiderHeder->errorstrlen,
			&global_DomainIDs, RANK_TYPE_FIND, queryNodeHeder.getRank, &ranking
		)) {
			int status;
			int n;
			
			if (ranking == -1) {
				status = net_nomatch;
				fprintf(stderr, "searchd_child: 1 Sending: %d\n", sizeof(status));
				if ((n=send(mysocfd, &status, sizeof(status),0)) != sizeof(status)) {
					fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n",n,sizeof(status),__FILE__,__LINE__);
					perror("sendall status");
					fprintf(stderr, "searchd: ~do_chld()\n");
					return;
				}
			} else {
				int data[2];
				data[0] = net_match;
				data[1] = ranking;
#if 0
				if ((n=send(mysocfd, &status, sizeof(status),0)) != sizeof(status)) {
					fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n",n,sizeof(status),__FILE__,__LINE__);
					perror("sendall status2");
					fprintf(stderr, "searchd: ~do_chld()\n");
					return;
				}
				if ((n=send(mysocfd, &ranking, sizeof(ranking),0)) != sizeof(ranking)) {
					fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n",n,sizeof(ranking),__FILE__,__LINE__);
					perror("sendall ranking");
					fprintf(stderr, "searchd: ~do_chld()\n");
					return;
				}
#else
				fprintf(stderr, "searchd_child: 2 Sending: %d\n", sizeof(data));
				if ((n = send(mysocfd, data, sizeof(data),0)) != sizeof(data)) {
					fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n",n,sizeof(data),__FILE__,__LINE__);
					perror("sendall data");
					fprintf(stderr, "searchd: ~do_chld()\n");
					return;
				}
#endif
			}

			fprintf(stderr, "searchd_child: 3 Receiving: %d\n",sizeof(ranking));
			if (recv(mysocfd, &ranking, sizeof(ranking), 0) != sizeof(ranking)) {
			//if (recvall(mysocfd, &ranking, sizeof(ranking))!= sizeof(ranking)) {
				fprintf(stderr, "searchd_child: recv ranking %s:%d\n",__FILE__,__LINE__);
				perror("");
				fprintf(stderr, "searchd: ~do_chld()\n");
				return;
			}
			fprintf(stderr, "searchd_child: Received ranking: %d\n", ranking);

			if (!dorank(queryNodeHeder.query, strlen(queryNodeHeder.query),&Sider,SiderHeder,SiderHeder->hiliteQuery,
				servername,subnames,nrOfSubnames,queryNodeHeder.MaxsHits,
				queryNodeHeder.start, queryNodeHeder.filterOn, 
				"",queryNodeHeder.orderby,SiderHeder->dates,queryNodeHeder.search_user,
				&SiderHeder->filters,
				searchd_config,
				SiderHeder->errorstr, &SiderHeder->errorstrlen,
				&global_DomainIDs, RANK_TYPE_SUM, 0/*queryNodeHeder.getRank*/, &ranking)) {

				perror("Got some kind of an error?");
				fprintf(stderr, "searchd: ~do_chld()\n");
				return;
			} else {
				int ranking2;
				fprintf(stderr, "searchd_child: Let us see how this ranking went: %d\n", ranking);
				ranking2 = ranking;
				status = 0xabdedd0f;
				fprintf(stderr, "searchd_child: 4 Sending: %d %d\n", sizeof(ranking), ranking);
				fprintf(stderr, "searchd_child: Ranking: %p\n", &ranking);
#if 1
				if ((n = send(mysocfd, &ranking2, sizeof(ranking2), 0)) != sizeof(ranking2)) {
					fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n", n, sizeof(ranking2),__FILE__,__LINE__);
					perror("sendall ranking2");
					fprintf(stderr, "searchd: ~do_chld()\n");
					return;
				}
#endif
				fprintf(stderr, "searchd_child: Sent: %d %d\n", ranking, n);
				sleep(5);
			}

			SiderHeder->responstype = searchd_responstype_ranking;
			close(mysocfd);
			fprintf(stderr, "searchd: ~do_chld()\n");
			return;
		} else {
			SiderHeder->responstype = searchd_responstype_error;
			SiderHeder->TotaltTreff 	= 0;
			SiderHeder->showabal	= 0;
		}


		fprintf(stderr, "searchd_child: doRank()\n");
		//setter at vi ikke hadde noen svar
	}
	else {
		SiderHeder->responstype = searchd_responstype_normalsearch;
	}

	dp_priority_locl_end();

	//kopierer inn subnames. Kan bare sende over MAX_COLLECTIONS, men søker i alle


	for (i=0;((i<MAX_COLLECTIONS) && (i<nrOfSubnames));i++) {
		SiderHeder->subnames[i] = subnames[i];
	}

	SiderHeder->nrOfSubnames = i--;

	if (globalOptVerbose) {
		fprintf(stderr, "searchd_child: subnames:\n");
		for (i=0;i<SiderHeder->nrOfSubnames;i++) {
			fprintf(stderr, "searchd_child: \t%s: %i\n",SiderHeder->subnames[i].subname,SiderHeder->subnames[i].hits);
		}
	
		fprintf(stderr, "searchd_child: \n");
	}
	//finer først tiden vi brukte
        gettimeofday(&globalend_time, NULL);
        SiderHeder->total_usecs = getTimeDifference(&globalstart_time,&globalend_time);

	
	//printf("query \"%s\", TotaltTreff %i,showabal %i,filtered %i,total_usecs %f\n",queryNodeHeder.query,SiderHeder->TotaltTreff,SiderHeder->showabal,SiderHeder->filtered,SiderHeder->total_usecs);

	fprintf(stderr, "|%-40s | %-11i | %-11i | %-11i | %-11f|\n",
		queryNodeHeder.query,
		SiderHeder->TotaltTreff,
		SiderHeder->showabal,
		SiderHeder->filtered,
		SiderHeder->total_usecs);

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	/* Disable tcp delay */
#if 0
	int nodelayflag = 1;
	setsockopt(mysocfd, IPPROTO_TCP, TCP_NODELAY, &nodelayflag, sizeof(int));
#endif

	if ((n=send(mysocfd,SiderHeder, sizeof(struct SiderHederFormat),MSG_NOSIGNAL)) != sizeof(struct SiderHederFormat)) {
		fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n",n,sizeof(struct SiderHederFormat),__FILE__,__LINE__);
		perror("sendall SiderHeder");
	}

	#ifdef ATTRIBUTES
	if (SiderHeder->navigation_xml_len > 0)
	    {
		if ((n=send(mysocfd, SiderHeder->navigation_xml, SiderHeder->navigation_xml_len, MSG_NOSIGNAL)) != SiderHeder->navigation_xml_len)
		    {
			fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n",n,SiderHeder->navigation_xml_len,__FILE__,__LINE__);
		        perror("sendall navigation_xml");
		    }
	    }

	free(SiderHeder->navigation_xml);
	#endif

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	fprintf(stderr, "searchd_child: Time debug: sending SiderHeder %f\n",getTimeDifference(&start_time,&end_time));
	#endif
	

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif


	for (i = 0; i < SiderHeder->showabal; i++) {
		int j;
		struct SiderFormat *s = Sider+i;
		size_t len;

		//printf("N_urls: %d\n", s->n_urls);
		if ((n=send(mysocfd, s, sizeof(struct SiderFormat), MSG_NOSIGNAL)) != (sizeof(struct SiderFormat))) {
			fprintf(stderr, "searchd_child: send only %i of %i at %s:%d\n",n,sizeof(struct SiderFormat),__FILE__,__LINE__);
			break;
		}
		/* Send duplicate urls */
		for (j = 0; j < s->n_urls; j++) {
			len = strlen(s->urls[j].url);
			send(mysocfd, &len, sizeof(len), MSG_NOSIGNAL);
			send(mysocfd, s->urls[j].url, len, MSG_NOSIGNAL);
			len = strlen(s->urls[j].uri);
			send(mysocfd, &len, sizeof(len), MSG_NOSIGNAL);
			send(mysocfd, s->urls[j].uri, len, MSG_NOSIGNAL);
			len = strlen(s->urls[j].fulluri);
			send(mysocfd, &len, sizeof(len), MSG_NOSIGNAL);
			send(mysocfd, s->urls[j].fulluri, len, MSG_NOSIGNAL);


			//printf("sending url: %s\n", s->urls[j].uri);

			//send(mysocfd, s->urls[j].subname, sizeof(s->urls[j].subname), MSG_NOSIGNAL);
		}

		/* Send attributes */
		//printf("### Send attributes: %s\n", s->attributes);
		if (s->attributes == NULL) {
			len = 0;
		}
		else {
			len = strlen(s->attributes);
		}
		send(mysocfd, &len, sizeof(len), MSG_NOSIGNAL);
		send(mysocfd, s->attributes, len, MSG_NOSIGNAL);
	}
	
	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	fprintf(stderr, "searchd_child: Time debug: sendig sider %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	/* close the socket and exit this thread */
	close(mysocfd);

	free(Sider);
	free(SiderHeder);

	#ifdef DEBUG
	fprintf(stderr, "searchd_child: exiting\n");
	#endif

	//pthread_exit((void *)0); /* exit with status */

	/* cansel alarm */
       	alarm (0);


/***************************************/
		#ifdef WITH_PROFILING
			++profiling_runcount;

			if (profiling_runcount >= WITH_PROFILING) {
				fprintf(stderr, "searchd_child: exiting to do profiling. Have done %i runs\n",profiling_runcount);
				sleep(1);
				exit(1);
			}
			fprintf(stderr, "searchd_child: Has runned %i times\n",profiling_runcount);
		#endif


    vboprintf("searchd: Normal exit.\n");
    vboprintf("searchd: ~do_chld()\n");
}
