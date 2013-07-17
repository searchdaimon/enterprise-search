#ifndef BLACK_BOX
	// runarb, 14 mai 2008:
	// for posix_fadvise()
	// dette kan skape problemer i andre header filer!!!!!
	#define _XOPEN_SOURCE 600
#endif

//preopen
#include "../common/lot.h"

#include "../logger/logger.h"

#include "../common/boithohome.h"
#include "../common/bstr.h"

#include "../common/poprank.h"
#include "../common/integerindex.h"
#include "../common/adultWeight.h"
#include "../common/daemon.h"
#include "../common/iindex.h"
#include "../acls/acls.h"
#include "../common/timediff.h"
#include "../parser2/html_parser.h"
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
#include <libconfig.h>

#include "../common/sigaction.h"

#include "preopen.h"
#include "verbose.h"
#include "searchkernel.h"

#ifdef WITH_SPELLING
	#include "../newspelling/spelling.h"
	spelling_t *spelling = NULL;
#endif

#define cfg_searchd "config/searchd.conf"

#ifdef WITH_THREAD
	#include <pthread.h>
#endif


/* function prototypes and global variables */
void do_chld(void *);
int	service_count;
//global variabel for å holde servernavn
char servername[MAX_SERVERNAME_LEN];
struct config_t cfg;


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




/* The signal handler exit the program. . */
void catch_alarm (int sig) {
	bblog(ERROR, "searchd: Warning! Recieved alarm signal. Exiting.");
	exit(1);
}

/* The signal handler exit the program. . */
void catch_alarm_nolog (int sig) {
	fprintf(stderr, "searchd->catch_alarm_nolog(): Warning! Recieved alarm signal. Exiting.");
	exit(1);
}


void lotPreOpenStartl(int *preOpen[], char filename[], char subname[], int use) {

	int i;

	if ((*preOpen = malloc(sizeof(int) * maxLots)) == NULL) {
		bblog_errno(ERROR, "malloc preOpen");
		exit(-1);
	}

	for (i=0;i<maxLots;i++) {

		(*preOpen)[i] = -1;

		#ifndef BLACK_BOX
			int n;
			char buf[1];

			if (use) {
				(*preOpen)[i] = lotOpenFileNoCasheByLotNrl(i,filename,"rb", 'r',subname);
				if ((*preOpen)[i] != -1) {

					if ((n=posix_fadvise((*preOpen)[i], 0,0,POSIX_FADV_RANDOM)) == 0) {
	
					} 

					read((*preOpen)[i],&buf,sizeof(buf));
					bblog(INFO, "opening %i, as %i",i,(*preOpen)[i]);
				}
			}
		#endif
		
	}

}

extern unsigned int spelling_min_freq;

int main(int argc, char *argv[])
{
	bblog_init("searchd");
	bblog(CLEAN, "Initializing...");

	int 	sockfd;
	int runCount;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;
	FILE *LOGFILE;
	struct searchd_configFORMAT searchd_config;

	struct config_t maincfg;

        searchd_config.searchport 	= 0;
	searchd_config.optLog 		= 0;
	searchd_config.optMax 		= 0;
	searchd_config.optSingle 	= 0;
	searchd_config.optrankfile 	= NULL;
	searchd_config.optPreOpen 	= 0;
	searchd_config.optFastStartup 	= 0;
	searchd_config.optCacheIndexes 	= 1;
	searchd_config.optAlarm 	= 60;
	
	// Needed for the speller to properly convert utf8 to wchar_t
	setlocale(LC_ALL, "en_US.UTF-8");

	/* Ignore collection updates for now */
	signal(SIGUSR2, SIG_IGN);
	/* And ignore spelling updates */
	signal(SIGUSR1, SIG_IGN);

        char c;
        while ((c=getopt(argc,argv,"clp:m:b:vsofA:L:S:a:"))!=-1) {
                switch (c) {
                        case 'p':
                                searchd_config.searchport = atoi(optarg);
				bblog(CLEAN, "searchd: Option -p: Using port %i.",searchd_config.searchport);
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
                        case 'v': /* XXX: Remove now that we have severity in the logger? */
				bblog(CLEAN, "searchd: Option -v: Verbose output.");
				globalOptVerbose = 1;
                                break;
                        case 's':
				bblog(INFO, "Option -s: Won't fork for new connections");
				searchd_config.optSingle = 1;
                                break;
			case 'f':
				searchd_config.optFastStartup = 1;
				break;
			case 'c':
				searchd_config.optCacheIndexes = 0;
				break;
			case 'A':
				bblog_set_appenders(atoi(optarg));
				break;
			case 'L':
				bblog_set_severity(atoi(optarg));
				break;
			case 'S':
				spelling_min_freq = strtol(optarg, NULL, 10);
				break;
                        case 'a':
				searchd_config.optAlarm = atoi(optarg);
                                break;

			default:
				bblog(ERROR, "Unknown argument: %c", c);
				errx(1, "Unknown argument: %c", c);
                }
        
	}

	#ifdef BLACK_BOX
		bblog(CLEAN, "Blackbox mode (searchdbb)");

		time_t starttime;
		time(&starttime);

		if (searchd_config.optLog) {
			/* Only add file logging if syslog is disabled */
			if ((bblog_get_appenders() & LOGGER_APPENDER_SYSLOG) == 0)
				bblog_set_appenders(LOGGER_APPENDER_FILE|bblog_get_appenders());
		}

		/* Write pidfile */
		FILE  *pidfile = fopen(bfile("var/searchd.pid"), "w");

		if (pidfile != NULL) {
			fprintf(pidfile, "%d", getpid());
			fclose(pidfile);
		} else {
			bblog(WARN, "Unable to write to pidfile");
		}

		bblog(CLEAN, "searchd: Starting. Time is %s",ctime(&starttime));
	#endif



	#ifdef DEBUG
		bblog(DEBUGINFO, "searchd: Debug: argc %i, optind %i",argc,optind);
	#endif

	if (searchd_config.optrankfile == NULL) {
		searchd_config.optrankfile = "Brank";
	}

	#ifdef WITH_SPELLING
		if (searchd_config.optFastStartup != 1) {
			if ((spelling = train(bfile("var/dictionarywords"))) == NULL) {
				bblog(ERROR, "Can't init spelling.");
			}

			cache_spelling_keepalive(&spelling);
			signal(SIGUSR1, cache_spelling_hup);
		}
	#endif

	if (argc > optind) {
		strncpy(servername,argv[optind], sizeof(servername) -1);
	} else {
		// No hostname supplied. Will assume "localhost"
		strncpy(servername,"localhost", sizeof(servername) -1);
	}
	
	lotPreOpenStartl(&searchd_config.lotPreOpen.DocumentIndex,"DocumentIndex","www",searchd_config.optPreOpen);
	lotPreOpenStartl(&searchd_config.lotPreOpen.Summary,"summary","www",searchd_config.optPreOpen);

	#ifdef BLACK_BOX
		if (searchd_config.optCacheIndexes == 1) {
			if (searchd_config.optFastStartup != 1) {
				bblog(INFO, "Reading indexes");
				cache_indexes(0);
				bblog(INFO, "Cached indexes: %dMB, cached indexes: %d", indexcachescached[0]/(1024*1024), indexcachescached[1]);
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

	

  	/* Initialize the configuration */
  	config_init(&cfg);


  	/* Load the file */
	#ifdef DEBUG
		bblog(DEBUGINFO, "searchd: Debug: Loading [%s] ...",bfile(cfg_searchd));
	#endif

  	if (!config_read_file(&cfg, bfile(cfg_searchd))) {
		bblog(ERROR, "config read failed: [%s]: %s at line %i",bfile(cfg_searchd),config_error_text(&cfg),config_error_line(&cfg));
		exit(1);
	}


	html_parser_init();


	bblog(CLEAN, "Servername: %s", servername);

	//ToDo: må ha låsing her
        if ((LOGFILE = bfopen("config/query.log","a")) == NULL) {
                bblog_errno(ERROR, "%s", bfile("config/query.log"));
        }
        else {
                fprintf(LOGFILE,"starting server %s\n",servername);
                fclose(LOGFILE);
        }


	#ifdef BLACK_BOX
		// Initialiser thesaurus med ouput-filene fra 'build_thesaurus_*':
		searchd_config.thesaurus_all = NULL;
		#ifndef WITHOUT_THESAURUS
			bblog(INFO, "init thesaurus");

			searchd_config.thesaurus_all = NULL;
			if (searchd_config.optFastStartup != 1) {
				searchd_config.thesaurus_all = load_all_thesauruses(bfile("data/thesaurus/"));

				if (searchd_config.thesaurus_all == NULL) {
					bblog(ERROR, "Unable to open thesaurus. Disabling stemming");
				} else {
					bblog(INFO, "init thesaurus done");
				}
			}
		#endif
		
		bblog(INFO, "init file-extensions");
		searchd_config.getfiletypep = fte_init(bfile("config/file_extensions.conf"));
		if (searchd_config.getfiletypep == NULL) {
			bblog(ERROR, "Unable to open file-extensions configuration file. Disabling file-extensions.");
		}

		bblog(INFO, "init attribute descriptions");
		searchd_config.attrdescrp = adf_init(bfile("config/attribute_descriptions.conf"));
		if (searchd_config.attrdescrp == NULL) {
			bblog(ERROR, "Unable to open attribute descriptions configuration file. Disabling attribute descriptions.");
		}

		bblog(INFO, "init show-attributes");

	#else

		//starter opp
		bblog(INFO, "Loading domain-ids...");
		iintegerLoadMemArray2(&global_DomainIDs,"domainid",sizeof(unsigned short), "www");

        	//laster inn alle poprankene
        	bblog(INFO, "Loading pop MemArray...");
        	popopenMemArray2("www",searchd_config.optrankfile); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får et inn enda
		bblog(INFO, "Loading adultWeight MemArray...");
		adultWeightopenMemArray2("www"); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får et inn enda
	#endif


	IIndexInaliser();

	#ifdef WITH_MEMINDEX
		IIndexLoad();
	#endif

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		bblog(ERROR, "Server error! Can't open stream socket.");
		exit(1);
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(searchd_config.searchport);
	bblog(INFO, "Will bind to port %i",searchd_config.searchport);
	//seter at sokket kan rebrukes
        int yes=1;
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		bblog_errno(ERROR, "setsockopt()");
		exit(1);
        }
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		bblog_errno(ERROR, "Can't bind local address. Port %i", searchd_config.searchport);
		exit(1);
	}	


	listen(sockfd, 5);

	runCount = 0;

	#ifndef NEW_CHILD_CATCHER
		signal(SIGCLD, SIG_IGN);  /* now I don't have to wait() for forked children! */
	#else
		{
			// runarb 28 juni 2009: 
			// Hvis vi har verbose output så skal vi wait()e for våre barn, og vise prity print når de dør.
			// desverre har det vært mye kød, der hovedprosessen blir hengene i sigchild_handler() og ikke kan
			// fork'e flere barn. For å ungå dtte venter vi ikke på våre barn til vanlig.
			if (globalOptVerbose) {
				struct sigaction sa;
				int ret;

				sa.sa_sigaction = sigchild_handler;
				sigemptyset(&sa.sa_mask);
				sa.sa_flags = SA_SIGINFO;

				ret = sigaction(SIGCHLD, &sa, 0);
				if (ret) {
					bblog_errno(ERROR, "sigaction()");
					exit(1);
				}
			}
			else {
				signal(SIGCLD, SIG_IGN);  /* now I don't have to wait() for forked children! */
			}
		}
	#endif

	bblog(CLEAN, "|------------------------------------------------------------------------------------------------|");
	bblog(CLEAN, "|%-40s | %-11s | %-11s | %-11s | %-11s|","query", "TotaltTreff", "showabal", "filtered", "total_usecs");
	bblog(CLEAN, "|------------------------------------------------------------------------------------------------|");

	for((clilen = sizeof(cli_addr));;)
	{
		searchd_config.newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);


		if(searchd_config.newsockfd < 0) {
			/* Just restart */
			if (errno == EINTR)
				continue;

			bblog(WARN, "searchd: Server warning! Accept error");
		}
		else {

			if (searchd_config.optSingle) {
				do_chld((void *) &searchd_config);
			}
			else {
			#ifdef DEBUG
				bblog(DEBUGINFO, "Debug mode; will not fork to new process.");
				do_chld((void *) &searchd_config);
			#else
				bblog(DEBUGINFO, "Forking new prosess.");
				if (fork() == 0) { // this is the child process

					close(sockfd); // child doesn't need the listener

					do_chld((void *) &searchd_config);	

					close(searchd_config.newsockfd);
					bblog(DEBUGINFO, "Terminating child.");
		
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
			bblog(WARN, "have reached Max runs. Exiting...");
			break;
		}

	}

	html_parser_exit();

	free(searchd_config.lotPreOpen.Summary);	
	free(searchd_config.lotPreOpen.DocumentIndex);	
	adf_destroy(searchd_config.attrdescrp);
	fte_destroy(searchd_config.getfiletypep);
	if (searchd_config.optFastStartup != 1) {
		thesaurus_destroy(searchd_config.thesaurusp);
	}
	//freegjør spelling. Trekt, men kjekt av valgring kan finne ut om noe ikke her blirr frigjort.
	if (searchd_config.optFastStartup != 1) {
		untrain(spelling);
	}

	return(0);
}

attr_conf *parse_navmenu_cfg(char *cfgstr, int *failed, int verbose) {
	char *warnings;
	attr_conf *cfg = show_attributes_init(cfgstr, &warnings, failed, verbose);
	
	if (*failed)
		bblog(WARN, "navmenucfg parsing failed");

	if (warnings[0] != '\0')
		bblog(WARN, "navmenucfg parsing warnings: %s", warnings);

	free(warnings);

	return cfg;
}

//copy a memory area, and return the size copyed
#ifdef DEBUG_MEMCPYRC
static size_t memcpyrc(void *s1, const void *s2, size_t n) {
#else
static inline size_t memcpyrc(void *s1, const void *s2, size_t n) {
#endif
        memcpy(s1,s2,n);

        return n;
}


/* 
	This is the routine that is executed from a new thread 
*/
void do_chld(void *arg)
{


	// starter loging
	bblog(DEBUGINFO, "searchd_child: Starting new thread.");
	bblog(DEBUGINFO, "searchd: do_chld()");

	// deklarerer variabler
	struct searchd_configFORMAT *searchd_config = arg;
	int   mysocfd = (*searchd_config).newsockfd;
	struct timeval globalstart_time, globalend_time;
	int 	i,n;
	struct queryNodeHederFormat queryNodeHeder;
	struct SiderFormat *Sider;
	int net_status;
	int nrOfSubnames;
        char groupOrQuery[1024];
        groupOrQuery[0] = '\0';


	struct SiderHederFormat *SiderHeder;


	// Setter signalhånterer for allarm. Hvis allarm skjer i straten av programet, er noe seriøst galt. For eks er vi tom for minne.
	//Da fungerer det dårlig å logge, syslog kan kalle malloc og slikt. Vil resette den til en versjon som logger før vi kjører søk.	
       	signal (SIGALRM, catch_alarm_nolog);

	/* Set an alarm to go off in a little while. This so we don't run forever if we get en ever loop */
       	alarm (searchd_config->optAlarm);

	dp_priority_locl_start();


	// første malloc. Har vi forlite minne vil vi henge her.
	if ((SiderHeder  = malloc(sizeof(struct SiderHederFormat))) == NULL) {
		bblog(ERROR, "malloc()");
		bblog(CLEAN, "~do_chld()");
		return;
	}


	gettimeofday(&globalstart_time, NULL);

	
	#ifdef WITH_THREAD
		bblog(DEBUGINFO, "Child thread [%d]: Socket number = %d", pthread_self(), mysocfd);
	#else
		bblog(DEBUGINFO, "Socket number = %d",mysocfd);
	#endif

	#ifdef DEBUG
        	struct timeval start_time, end_time;
	#endif

	// Setter signalhånterer for allarm. Hvis allarm skjer i straten av programet, er noe seriøst galt. For eks er vi tom for minne.
	//Da fungerer det dårlig å logge, syslog kan kalle malloc og slikt. Nå resetter vi den den til en versjon som logger søk.	
       	signal (SIGALRM, catch_alarm);

	/* Disable tcp delay */
	int nodelayflag = 1;
	setsockopt(mysocfd, IPPROTO_TCP, TCP_NODELAY, &nodelayflag, sizeof(int));


	/* read from the given socket */

	if ((i=recv(mysocfd, &queryNodeHeder, sizeof(queryNodeHeder),MSG_WAITALL)) == -1) {
		bblog_errno(ERROR, "recv()");
	}

	// Read collections /w cfg.
	if ((recv(mysocfd, &nrOfSubnames, sizeof nrOfSubnames, MSG_WAITALL)) == -1)
		bblog_errno(ERROR, "recv nrOfSubnames");

	bblog(DEBUGINFO, "nrOfSubnames: %d",nrOfSubnames);
	struct subnamesFormat subnames[nrOfSubnames];
	if (nrOfSubnames > 0) {
		if ((recv(mysocfd, &subnames, sizeof subnames, MSG_WAITALL)) == -1)
			bblog_errno(ERROR, "recv subnames");
	}


	//sender svar med en gang at vi kan gjøre dette
	net_status = net_CanDo;

	if ((n=send(mysocfd,&net_status, sizeof(net_status),MSG_NOSIGNAL)) != sizeof(net_status)) {
		bblog_errno(ERROR, "searchd_child: Warning! Sent only %i of %i bytes at %s:%d",n,sizeof(net_status),__FILE__,__LINE__);
	}


	bblog(DEBUGINFO, "MaxsHits %i",queryNodeHeder.MaxsHits);
	bblog(DEBUGINFO, "Ranking search?");


	#ifndef BLACK_BOX
	//ToDo: må ha låsing her
        if ((LOGFILE = bfopen("config/query.log","a")) == NULL) {
                bblog_errno(ERROR, "logfile");
        }
        else {
                fprintf(LOGFILE,"%s\n",queryNodeHeder.query);
                fclose(LOGFILE);
        }
	#endif

	bblog(DEBUGINFO, "searchd_child: Incoming query: %s",queryNodeHeder.query);

	strcpy(SiderHeder->servername,servername);

	#ifndef WITHOUT_THESAURUS
		// TODO: Denne må skalere til flere språk:
		char	*lang = NULL;
		switch (queryNodeHeder.lang)
		    {
			case LANG_NBO: lang = "nbo"; break;
			case LANG_ENG: lang = "eng"; break;
		    }

		bblog(INFO, "[searchd] lang_id = %i", queryNodeHeder.lang);
		bblog(INFO, "[searchd] lang = %s", lang);

		searchd_config->thesaurusp = NULL;
		if (lang != NULL && searchd_config->thesaurus_all != NULL) {
			iterator	it = map_find(searchd_config->thesaurus_all, lang);
			if (it.valid)
			    {
				bblog(INFO, "[searchd] Loading %s thesaurus", lang);
				searchd_config->thesaurusp = map_val(it).ptr;
			    }
			else
			    {
				bblog(INFO, "[searchd] No thesaurus for %s", lang);
			    }
		}
	#endif


	bblog(DEBUGINFO, "nrOfSubnames %i",nrOfSubnames);
  	

	#ifdef DEBUG
		bblog(DEBUGINFO, "searchd_child: ");
		bblog(DEBUGINFO, "##########################################################");
		bblog(DEBUGINFO, "searchd_child: subnames:");
		bblog(DEBUGINFO, "Total of %i", nrOfSubnames);
		for (i=0;i<nrOfSubnames;i++) {
			bblog(DEBUGINFO, "searchd_child: subname nr %i: \"%s\"",i,subnames[i].subname);
		}
		bblog(DEBUGINFO, "searchd_child: ##########################################################");
	#endif

	SiderHeder->filtypesnrof = MAXFILTYPES;
	SiderHeder->errorstrlen = sizeof(SiderHeder->errorstr);


	int parsing_failed;
	attr_conf *navmenu_cfg = parse_navmenu_cfg(queryNodeHeder.navmenucfg, &parsing_failed, globalOptVerbose);
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
		SiderHeder->errorstr, SiderHeder->errorstrlen,
		&global_DomainIDs, queryNodeHeder.HTTP_USER_AGENT,
		groupOrQuery, queryNodeHeder.anonymous, navmenu_cfg,
		spelling
		)) 
	{
		bblog(WARN, "searchd_child: dosearch did not return success");
		SiderHeder->responstype 	= searchd_responstype_error;
		//setter at vi ikke hadde noen svar
		SiderHeder->TotaltTreff 	= 0;
		SiderHeder->showabal		= 0;

		bblog(ERROR, "searchd_child: can't do dosearch: \"%s\"", SiderHeder->errorstr);
	}

	show_attributes_destroy(navmenu_cfg);

		

	dp_priority_locl_end();

	//kopierer inn subnames. Kan bare sende over MAX_COLLECTIONS, men søker i alle
	for (i=0;((i<MAX_COLLECTIONS) && (i<nrOfSubnames));i++) {
		SiderHeder->subnames[i] = subnames[i];
	}

	SiderHeder->nrOfSubnames = i--;

	if (globalOptVerbose) {
		bblog(INFO, "searchd_child: subnames:");
		for (i=0;i<SiderHeder->nrOfSubnames;i++) {
			bblog(INFO, "searchd_child: \t%s: %i",SiderHeder->subnames[i].subname,SiderHeder->subnames[i].hits);
		}
	
		bblog(INFO, "searchd_child:");
	}
	//finer først tiden vi brukte
        gettimeofday(&globalend_time, NULL);
        SiderHeder->total_usecs = getTimeDifference(&globalstart_time,&globalend_time);

	
	bblog(CLEAN, "|%-40s | %-11i | %-11i | %-11i | %-11f|",
		queryNodeHeder.query,
		SiderHeder->TotaltTreff,
		SiderHeder->showabal,
		SiderHeder->filtered,
		SiderHeder->total_usecs);

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif


	#if 1


	#ifdef DEBUG
		gettimeofday(&end_time, NULL);
		bblog(DEBUGINFO, "searchd_child: Time debug: sending SiderHeder %f",getTimeDifference(&start_time,&end_time));
	#endif
	

	#ifdef DEBUG
		gettimeofday(&start_time, NULL);
	#endif

	struct sendarrayFormat{
		int size;
		void *p;
		int copy;
	};

	void send_to_array (struct sendarrayFormat *sendarray, int *sendarraylength, void *p, int size, int copy) {

		#ifdef DEBUG
			printf("send_to_array(sendarraylength=%i,size=%i)\n",*sendarraylength,size);
		#endif

		if (size == 0) {
			return;
		}

		sendarray[*sendarraylength].copy = copy;
	
		if (copy) {
			sendarray[*sendarraylength].p = malloc(size);
			memcpy(sendarray[*sendarraylength].p,p,size); 
			sendarray[*sendarraylength].size = size;
		}
		else {
			sendarray[*sendarraylength].p = p;
			sendarray[*sendarraylength].size = size; 

		}
		*sendarraylength += 1;
	}

	int sendarraylength = 0;
	struct sendarrayFormat sendarray[SiderHeder->showabal + 2 + (SiderHeder->showabal * 32)];


	send_to_array(sendarray, &sendarraylength, SiderHeder, sizeof(struct SiderHederFormat),0);

	#ifdef ATTRIBUTES
		if (SiderHeder->navigation_xml_len > 0) {
			send_to_array(sendarray, &sendarraylength, SiderHeder->navigation_xml, SiderHeder->navigation_xml_len, 0);
		}
	#endif


	for (i = 0; i < SiderHeder->showabal; i++) {
		int j;
		struct SiderFormat *s = Sider+i;
		size_t len;

		// da vi må vite maks lengde på dataen på forhond, slik at vi ikke går over sendarray må vi sette en maks på antal urler.
		if (s->n_urls > 10) {
			s->n_urls = 10;
		}

		send_to_array(sendarray, &sendarraylength, s, sizeof(struct SiderFormat), 0);



		/* Send duplicate urls */
		for (j = 0; j < s->n_urls; j++) {
			len = strlen(s->urls[j].url);
			send_to_array(sendarray, &sendarraylength, &len, sizeof(len), 1);
			send_to_array(sendarray, &sendarraylength, s->urls[j].url, len, 0);

			len = strlen(s->urls[j].uri);
			send_to_array(sendarray, &sendarraylength, &len, sizeof(len), 1);
			send_to_array(sendarray, &sendarraylength, s->urls[j].uri, len, 0);

			len = strlen(s->urls[j].fulluri);
			send_to_array(sendarray, &sendarraylength, &len, sizeof(len), 1);
			send_to_array(sendarray, &sendarraylength, s->urls[j].fulluri, len, 0);

		}

		/* Send attributes */
		//printf("### Send attributes: %s\n", s->attributes);
		if (s->attributes == NULL) {
			len = 0;
		}
		else {
			len = strlen(s->attributes);
		}

		send_to_array(sendarray, &sendarraylength, &len, sizeof(len), 1);
		send_to_array(sendarray, &sendarraylength, s->attributes, len, 0);

	}

	int sendtotal = 0;
	for (i=0;i<sendarraylength;i++) {
		sendtotal += sendarray[i].size;
	}

	void *sendp, *sendall;

	if ((sendall = malloc(sendtotal)) == NULL) {
		perror("sendp");
		return;
	}
	sendp = sendall;

	for (i=0;i<sendarraylength;i++) {
		sendp += memcpyrc(sendp,sendarray[i].p,sendarray[i].size);
		if (sendarray[i].copy) {
			free(sendarray[i].p);
		}
	}

	if ((n=send(mysocfd, sendall, sendtotal, MSG_NOSIGNAL)) != sendtotal ) {
		bblog(ERROR, "siderformat: send only %i of %i at %s:%d",n,sendtotal,__FILE__,__LINE__);
		return;
	}
	bblog(DEBUGINFO,"~sendall(sendarraylength=%i, sendall=%p, sendtotal=%i)\n",sendarraylength,sendall,sendtotal);

	free(SiderHeder->navigation_xml);

	free(sendall);
	
	#else 

	/* Disable tcp delay */
	#if 0
		int nodelayflag = 1;
		setsockopt(mysocfd, IPPROTO_TCP, TCP_NODELAY, &nodelayflag, sizeof(int));
	#endif

	if ((n=send(mysocfd,SiderHeder, sizeof(struct SiderHederFormat),MSG_NOSIGNAL)) != sizeof(struct SiderHederFormat)) {
		bblog_errno(ERROR, "siderheder: send only %i of %i at %s:%d",n,sizeof(struct SiderHederFormat),__FILE__,__LINE__);
	}

	#ifdef ATTRIBUTES
		if (SiderHeder->navigation_xml_len > 0) {
			if ((n=send(mysocfd, SiderHeder->navigation_xml, SiderHeder->navigation_xml_len, MSG_NOSIGNAL)) != SiderHeder->navigation_xml_len)
			    {
				bblog_errno(ERROR, "navigation xml: send only %i of %i at %s:%d",n,SiderHeder->navigation_xml_len,__FILE__,__LINE__);
			    }
		}

		free(SiderHeder->navigation_xml);
	#endif

	#ifdef DEBUG
		gettimeofday(&end_time, NULL);
		bblog(DEBUGINFO, "searchd_child: Time debug: sending SiderHeder %f",getTimeDifference(&start_time,&end_time));
	#endif
	

	#ifdef DEBUG
		gettimeofday(&start_time, NULL);
	#endif


	for (i = 0; i < SiderHeder->showabal; i++) {
		int j;
		struct SiderFormat *s = Sider+i;
		size_t len;

		if ((n=send(mysocfd, s, sizeof(struct SiderFormat), MSG_NOSIGNAL)) != (sizeof(struct SiderFormat))) {
			bblog(ERROR, "siderformat: send only %i of %i at %s:%d",n,sizeof(struct SiderFormat),__FILE__,__LINE__);
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
		}

		/* Send attributes */
		if (s->attributes == NULL) {
			len = 0;
		}
		else {
			len = strlen(s->attributes);
		}
		send(mysocfd, &len, sizeof(len), MSG_NOSIGNAL);
		send(mysocfd, s->attributes, len, MSG_NOSIGNAL);
	}
	#endif

	#ifdef DEBUG
		gettimeofday(&end_time, NULL);
		bblog(DEBUGINFO, "Time debug: sendig sider %f",getTimeDifference(&start_time,&end_time));
	#endif


	/* close the socket */
	close(mysocfd);

	free(Sider);
	free(SiderHeder);

	#ifdef DEBUG
		bblog(DEBUGINFO, "exiting");
	#endif


	/* Cancel any running alarms */
       	alarm (0);


	#ifdef WITH_PROFILING
		++profiling_runcount;

		if (profiling_runcount >= WITH_PROFILING) {
			bblog(INFO, "searchd_child: exiting to do profiling. Have done %i runs",profiling_runcount);
			sleep(1);
			exit(1);
		}
		bblog(INFO, "searchd_child: Has runned %i times",profiling_runcount);
	#endif


	bblog(DEBUGINFO, "searchd: Normal exit.");
	bblog(INFO, "searchd: ~do_chld()");
}
