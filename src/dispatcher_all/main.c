    #include "../common/define.h"
    #include "../common/lot.h"
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h> 
    #include <time.h>
    #include <errno.h>
    #include "../common/boithohome.h"
    #include "../maincfg/maincfg.h"
    
	#ifndef BLACK_BOKS
    #include <libconfig.h>

    #define cashedir "cashedir"
    #define prequerydir "prequerydir"

    #define cfg_dispatcher "config/dispatcher.conf"
	#endif

#ifndef BLACK_BOKS
    #include "GeoIP.h"
    #include "GeoIPCity.h"
#endif

    //temp
    //#define NO_LOGING

    #include <mysql.h>

    #include "../searchFilters/searchFilters.h"
    #include "../cgi-util/cgi-util.h"
    #include "../common/timediff.h"
    #include "../common/mgsort.h"
    #include "../common/bstr.h"
    #include "../tkey/tkey.h"

    //#define PORT 6500 // the port client will be connecting to 

    #define MAXDATASIZE 100 // max number of bytes we can get at once 


	//ikke sikert 1 vil være for altid, da vi blant annent snart vil støtte clustring, men trenger desperat 
	//og få ned stack støreslen, så valgrin fungerer igjen 
	#ifdef BLACK_BOKS
	    #define maxServers 30
	#else
	    #define maxServers 100
	#endif

    //extern int errno;

    //i hundredels sekunder (100 = 1sec)
    #define maxSocketWait_SiderHeder 1000
    #define maxSocketWait_CanDo 100

    #define maxerrors 5
    #define maxerrorlen 201



    struct errorhaFormat {
                int nr;
                int errorcode[maxerrors];
                char errormessage[maxerrorlen][maxerrors];
    };

    int compare_elements (const void *p1, const void *p2);
    int compare_elements_posisjon (const void *p1, const void *p2);


	struct dispconfigFormat {
		char usecashe;
		char useprequery;
	};

void die(int errorcode,char errormessage[]) {
	FILE *LOGFILE;

	printf("<search>\n");
	printf("<error>\n");
	printf("  <errorcode>%i</errorcode>\n",errorcode); 
	printf("  <errormessage>%s</errormessage>\n",errormessage); 
	printf("</error>\n");
	printf("<RESULT_INFO TOTAL=\"0\" QUERY=\"\" HILITE=\"\" TIME=\"0\" FILTERED=\"0\" SHOWABAL=\"0\" />\n");
	printf("</search>\n");

        //ToDo: må ha låsing her
        if ((LOGFILE = bfopen("logs/query.log","a")) == NULL) {
                perror(bfile("logs/query.log"));
        }
        else {
		
		flock(fileno(LOGFILE),LOCK_EX);
                fprintf(LOGFILE,"error: %i %s\n",errorcode,errormessage);
                fclose(LOGFILE);
        }


	exit(1);

}

//legger tilen error til køen
void addError(struct errorhaFormat *errorha, int errorcode,char errormessage[]) {

		if ((*errorha).nr < maxerrors) {
			(*errorha).errorcode[(*errorha).nr] = errorcode;
			
			strscpy((*errorha).errormessage[(*errorha).nr],errormessage,maxerrorlen);

			++(*errorha).nr;
		}
		else {
			die(10,"to many errors occurd");
		}
}


int bsconnect (int *sockfd, char server[], int port) {


	int returnstatus = 0;
        struct sockaddr_in their_addr; // connectors address information 
	struct timeval timeout;
	int nerror;
	struct hostent *he;
	
		#ifdef DEBUG
			printf("server: %s\n",server);
		#endif

        	if ((he = gethostbyname(server)) == NULL) {  // get the host info 
        	    perror("gethostbyname");
        	    die(6,"Gethostbyname error.");
	        }
		
	        if (((*sockfd) = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        	    perror("socket");
        	    die(7,"Socket error.");
        	}
		fcntl((*sockfd), F_SETFL, O_NONBLOCK);
		
        	their_addr.sin_family = AF_INET;    // host byte order 
        	their_addr.sin_port = htons(port);  // short, network byte order 
        	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

		/* Initialize the timeout data structure. */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		fd_set set;
		/* Initialize the file descriptor set. */
           	FD_ZERO(&set);
           	FD_SET((*sockfd), &set);

        	if ((nerror = connect((*sockfd), (struct sockaddr *)&their_addr,sizeof(struct sockaddr))) == -1) {
		
			//vi har en no bloacking socket og connect har begynt. Må så bruke select for å vente på den
			if (errno == EINPROGRESS) {

				nerror = select((*sockfd) +1, NULL ,&set, NULL,&timeout);

				//hvis vi ikke fikk noen error er vi nå kobblet til
				if (nerror == 1) {
					//connected[i] = 1;
					returnstatus = 1;
				}
				else {
					//connected[i] = 0;
					perror(server);
					returnstatus = 0;
				}
			}
			else {
        	    		perror(server);
        	    		//connected[i] = 0;
				returnstatus = 0;
			}
	        }
		else {
			//connected[i] = 1;
			returnstatus = 1;
		}

	if (returnstatus == 0) {
		#ifdef DEBUG 
			printf("somthing is wrong. Closeing socket\n");
		#endif
                (*sockfd) = 0;
        }
	
	return returnstatus;
}

int bsread (int *sockfd,int datasize, char buff[], int maxSocketWait) {


	int dataReceived = 0;
	int y = 0;
//      struct timeval timeout;
//      struct timeval time;
        struct timespec timeout;
        struct timespec time;
        int socketWait;
	int n,i;
	int returnstatus = 1;

	//hvsi vi fikk en sokket som ikke har noen verdi. Sikkert da den har blit kanselert før
	if ((*sockfd) == 0) {
		return 0;
	}
			
			socketWait = 0;
			/* Initialize the time data structure. */
	                time.tv_sec = 0;
        	        //time.tv_usec = 10 * 1000000; // milliseconds * nanosec. 10 * 1000000 = 0.01 sek
        	        time.tv_nsec = 10000000; // i nanoseconds = 0.01 sek

			//mens vi venter på data, eller til vi ikke gidder å vente mere
			while ((dataReceived < datasize) && (socketWait < maxSocketWait)) {
			
				if ((n=recv((*sockfd), (&buff[y] + dataReceived), datasize - dataReceived, MSG_DONTWAIT )) == -1 ) {
					
					if (errno == EAGAIN) {
						#ifdef DEBUG
							printf("EAGAIN. wait %i < %i\n",socketWait,maxSocketWait);
						#endif
    						nanosleep(&time,NULL);
						++socketWait;		

					}
					else {
						//noe feil har skjed, eksitter
						perror("recv SiderHeder");
		
						//ToDo: utestet
						//connected[i] = 0;
						returnstatus = 0;
						break;
					}
				}
				else if (n==0) {
					//nanosleep(&time,NULL);
					//++socketWait;
					#ifdef DEBUG
                                                printf("n==0. Peer has performed an orderly shutdown when trying to read\n");
						perror("recv SiderHeder");

                                        #endif
                                        //connected[i] = 0;
					returnstatus = 0;
                                        break;

				}
				else {
					dataReceived += n;
				}

				#ifdef DEBUG					
					printf("reciving %i of %i. SocketWait %i\n",dataReceived,datasize,socketWait);
				#endif


				
			}
			//vi timet ut socketetn. Setter den som ikke tilkoblet
			if (socketWait >= maxSocketWait) {
				#ifdef DEBUG	
					printf("timed out\n");
				#endif
				//connected[i] = 0;
				returnstatus = 0;
			}

	if (returnstatus == 0) {
		(*sockfd) = 0;
		#ifdef DEBUG 
			printf("somthing is wrong. Closeing socket\n");
		#endif

	}

	return returnstatus;

}

int bsConectAndQuery(int *sockfd,int nrOfServers, char *servers[],struct queryNodeHederFormat *queryNodeHeder,int alreadynr, int port) {

	int i;

	#ifdef DEBUG
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	#endif

 	//kobler til vanlige servere
        for (i=0;i<nrOfServers;i++) {
                if (bsconnect (&sockfd[i +alreadynr], servers[i], port)) {
			#ifdef DEBUG
                        printf("can connect\n");
			#endif
                }
		else {
			#ifdef DEBUG
			printf("can NOT connect\n");
			#endif
			sockfd[i] = 0;

		}
        }	
	//sender ut forespørsel
	for (i=0;i<nrOfServers;i++) {
		//sender forespørsel
		
		if (sockfd[i +alreadynr] != 0) {
			if (sendall(sockfd[i +alreadynr],(*queryNodeHeder).query,sizeof(struct queryNodeHederFormat)) != sizeof(struct queryNodeHederFormat)) {
				fprintf(stderr,"Eroor: Cant connect. Server %s:%i\n",servers[i], port);
				perror("sendall");
				sockfd[i] = 0;
			}
		}
		#ifdef DEBUG
			printf("sending of queryNodeHeder end\n");
		#endif
	}

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: bsConectAndQuery %f\n",getTimeDifference(&start_time,&end_time));
	#endif
}

int bdread_continue(int sockfd[], int nrof,int bytesleft[], int lastgoodread[], int readsdone, int maxSocketWait) {

	int i;
	int nrleft = 0;

	//går gjenom alle og finner hvor mange vi venter på
	for (i=0;i<nrof;i++) {
		printf("%i: sockfd %i bytesleft %i\n",i,sockfd[i],bytesleft[i]);
		if ((sockfd[i] != 0) && (bytesleft[i] != 0)) {
			++nrleft;
		}
	}

	printf("bdread_continue nrleft %i\n",nrleft);

	if (maxSocketWait < (readsdone / 0.01)) {
		debug("max time hit. Hav done %i reads.",readsdone);
		return 0;
	}
	else if (nrleft == 0) {
		debug("nrleft is 0");
		return 0;
	}


	return 1;

}

//leser datasize fra nrof og lagrer sekvensielt i result
int bdread(int sockfd[],int nrof,int datasize, void *result, int maxSocketWait) {

	int sendt[nrof];
	int bytesleft[nrof];
	int lastgoodread[nrof];
	int readsdone = 0;
	int i,n;
	void *thisresult;
        struct timespec time;
	/* Initialize the time data structure. */
        time.tv_sec = 0;
        //time.tv_usec = 10 * 1000000; // milliseconds * nanosec. 10 * 1000000 = 0.01 sek
        time.tv_nsec = 10000000; // i nanoseconds = 0.01 sek

	printf("datasize %i\n",datasize);

	//inaliserer sendt og bytesleft
	for (i=0;i<nrof;i++) {
		sendt[i] = 0;
		bytesleft[i] = datasize;
	}

	i=0;
	while ( bdread_continue(sockfd,nrof,bytesleft,lastgoodread,readsdone,maxSocketWait) ) {

		if (sockfd[i] == 0) {
			continue;
		}

		nanosleep(&time,NULL);
		++readsdone;		

		thisresult = ( result + ((datasize * i )- datasize) );

		if ((n=recv(sockfd[i], thisresult + sendt[i], bytesleft[i], MSG_DONTWAIT )) == -1 ) {

					
				if (errno == EAGAIN) {
					#ifdef DEBUG
					printf("EAGAIN. wait %i < %i\n",readsdone,maxSocketWait);
					#endif

				}
				else {
					//noe feil har skjed, eksitter
					perror("bdread: recv SiderHeder");
		
					//ToDo: utestet
					//connected[i] = 0;
					close(sockfd[i]);
					sockfd[i] = 0;
				}
			}
			else if (n==0) {
				//nanosleep(&time,NULL);
				//++socketWait;
				#ifdef DEBUG
                                               printf("n==0. Peer has performed an orderly shutdown when trying to read\n");
                                #endif
                                //connected[i] = 0;
				close(sockfd[i]);
				sockfd[i] = 0;


			}
			else {
				sendt[i] +=n;
				bytesleft[i] -=n;
			}

			#ifdef DEBUG					
				printf("reciving %i of %i. readsdone %i\n",n,datasize,readsdone);
			#endif
				
	}

}

int brGetPages(int *sockfd,int nrOfServers,struct SiderHederFormat *SiderHeder,struct SiderFormat *Sider, int *pageNr,int alreadynr) {

	#ifdef DEBUG
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	#endif

	int i,y;
	int net_status;
	//sejjer om vi har fåt et midlertidig svar på at jobben har begynt
	/****************************************************************/
	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {

		if (sockfd[i] != 0) {

			//motter hedderen for svaret
			if (bsread (&sockfd[i],sizeof(net_status),(char *)&net_status,maxSocketWait_CanDo)) {


				if (net_status != net_CanDo) {
					#ifdef DEBUG
						printf("net_status wasen net_CanDo but %i\n",net_status);
					#endif
					sockfd[i] = 0;
				}
			}
		}			
		
	}

	/****************************************************************/


	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {
	
		//motter hedderen for svaret
		if (sockfd[i] != 0) {

			if (bsread (&sockfd[i],sizeof(struct SiderHederFormat),(char *)&SiderHeder[i],maxSocketWait_SiderHeder)) {

			}
		}
	}

	//int bdread(int sockfd[],int nrof,int datasize, void *result, int maxSocketWait)

//	bdread(sockfd,nrOfServers,sizeof(struct SiderHederFormat),SiderHeder, maxSocketWait_SiderHeder);

	//(*pageNr) = 0;
	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {
		
		#ifdef DEBUG
			printf("i: %i. That has %i pages. Soctet %i\n",i,SiderHeder[i].showabal,sockfd[i]);
		#endif
			if (sockfd[i] != 0) {

				for(y=0;y<SiderHeder[i].showabal;y++) {

					if (bsread (&sockfd[i],sizeof(struct SiderFormat),(char *)&Sider[(*pageNr)],maxSocketWait_SiderHeder)) {
						(*pageNr)++;
					}

				}

			}
	}


	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: brGetPages %f\n",getTimeDifference(&start_time,&end_time));
	#endif

}
int main(int argc, char *argv[])
{
        int sockfd[maxServers];
        int addsockfd[maxServers];
	int i,y,n,funnet,x;
	int pageNr;
	char documentlangcode[4];
	int totlaAds;
	char *strpointer;  
	int net_status;
	int res;
	int nerror;
	int dataReceived[maxServers];
	//int siderDataReceived[maxServers];
        //char buf[MAXDATASIZE];
        //struct hostent *he[maxServers];
	FILE *LOGFILE;
	//struct SiderFormat Sider[20 * maxServers];
	struct SiderFormat *Sider;
	char colchecked[20];
	
        struct SiderHederFormat SiderHeder[maxServers];
        struct SiderHederFormat AddSiderHeder[maxServers];


	struct SiderHederFormat FinalSiderHeder;
	//char buff[4096]; //generell buffer
	struct in_addr ipaddr;
        struct QueryDataForamt QueryData;
	//int connected[maxServers];
	int NonAdultPages,AdultPages;
	struct timeval main_start_time, main_end_time;
	struct timeval start_time, end_time;
	int nrRespondedServers;
	char errormessage[maxerrorlen];
	struct errorhaFormat errorha;
	errorha.nr = 0;
	int posisjon;
	struct timeval timeout;
	struct timeval time;
	int socketWait;	
	int hascashe;
        int hasprequery;
	char prequeryfile[512];
	char cashefile[512];

	char *cpnt;

	struct queryNodeHederFormat queryNodeHeder;

	struct dispconfigFormat dispconfig;

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	//starter å ta tiden
	gettimeofday(&main_start_time, NULL);


	struct config_t maincfg;

	maincfg = maincfgopen();

	int searchport = maincfg_get_int(&maincfg,"BSDPORT");

	//config
  	#ifndef BLACK_BOKS
		config_setting_t *cfgarray;
		struct config_t cfg;


	  	/* Initialize the configuration */
	  	config_init(&cfg);

	  	/* Load the file */
		#ifdef DEBUG
		  	printf("loading [%s]..\n", bfile(cfg_dispatcher) );
		#endif

	  	if (!config_read_file(&cfg, bfile(cfg_dispatcher) )) {
	    		printf("[%s]failed: %s at line %i\n",bfile(cfg_dispatcher),config_error_text(&cfg),config_error_line(&cfg));
			exit(1);
		}

	    	if ( (cfgarray = config_lookup(&cfg, "usecashe") ) == NULL) {
			printf("can't load \"usecashe\" from config\n");
			exit(1);
	  	}

		dispconfig.usecashe = config_setting_get_bool(cfgarray);

	    	if ( (cfgarray = config_lookup(&cfg, "useprequery") ) == NULL) {
			printf("can't load \"useprequery\" from config\n");
			exit(1);
	  	}

		dispconfig.useprequery = config_setting_get_bool(cfgarray);
	#endif
	
	char query [2048];
	static MYSQL demo_db;

	//MYSQL_RES *mysqlres; /* To be used to fetch information into */
	MYSQL_ROW mysqlrow;

	//////////////////
	//for nå angir vi bare servere slik. Må skilles u i egen fil siden

	int nrOfServers;
	int nrOfPiServers;
	int nrOfAddServers;

	#ifdef BLACK_BOKS

		char *servers[] = { "localhost" };
		char *addservers[] = { };
		char *piservers[] = { };


		nrOfServers = (sizeof(servers) / sizeof(char *));
		nrOfPiServers = (sizeof(piservers) / sizeof(char *));
	        nrOfAddServers = (sizeof(addservers) / sizeof(char *));

	#else 

		char **servers;
		char **piservers;
		char **addservers;

		//load server array from config    
	    	if ( (cfgarray = config_lookup(&cfg, "servers") ) == NULL) {
			printf("can't load \"servers\" from config\n");
			exit(1);
	  	}

		nrOfServers = config_setting_length(cfgarray);

		servers = malloc(sizeof(char *) * (nrOfServers + nrOfPiServers));

		for(i=0;i<nrOfServers;i++) {
			servers[i] = strdup(config_setting_get_string_elem(cfgarray,i));
		}


		//load server array from config    
	    	if ( (cfgarray = config_lookup(&cfg, "piservers") ) == NULL) {
			printf("can't load \"piservers\" from config\n");
			exit(1);
	  	}

		nrOfPiServers = config_setting_length(cfgarray);

		piservers = malloc(sizeof(char *) * nrOfPiServers);

		for(i=0;i<nrOfPiServers;i++) {
			piservers[i] = strdup(config_setting_get_string_elem(cfgarray,i));
		}



		//load addserver array from config    
	    	if ( (cfgarray = config_lookup(&cfg, "addservers") ) == NULL) {
			printf("can't load \"nrOfAddServers\" from config\n");
			exit(1);
	  	}

		nrOfAddServers = config_setting_length(cfgarray);

		addservers = malloc(sizeof(char *) * nrOfAddServers);

		for(i=0;i<nrOfAddServers;i++) {
			addservers[i] = strdup(config_setting_get_string_elem(cfgarray,i));
		}


		//char *servers[] = { "bbs-002.boitho.com" , "bbs-003.boitho.com", "bbs-004.boitho.com", "bbs-005.boitho.com", "bbs-006.boitho.com" , "bbs-007.boitho.com" };
		//char *servers[] = { "bbs-002.boitho.com" , "bbs-003.boitho.com", "bbs-004.boitho.com", "bbs-005.boitho.com", "bbs-006.boitho.com" };
		//char *servers[] = { "bbs-002.boitho.com" };

	
		//char *addservers[] = { "bbh-001.boitho.com" };
		//char *addservers[] = { "www2.boitho.com" };

		//int nrOfServers = (sizeof(servers) / sizeof(char *));
		//int nrOfAddServers = (sizeof(addservers) / sizeof(char *));

	#endif



	//he = (struct hostent *) malloc(nrOfServers * sizeof(struct hostent));
	//////////////////

        //send out an HTTP header:
	#ifdef DEBUG
	        printf("Content-type: text/plain\n\n");
	#else
	        printf("Content-type: text/xml\n\n");
	        //printf("Content-type: text/xml%c%c\n",13,10);
	#endif

	memset(&QueryData,'\0',sizeof(QueryData));

        //hvis vi har argumeneter er det første et query
        if (getenv("QUERY_STRING") == NULL) {
                if (argc < 3 ) {
                        printf("Error ingen query spesifisert eller subname .\n\nEksempel på bruk for å søke på boitho:\n\tsearchkernel boitho www\n\n\n");
                }
                else {

			strcpy(QueryData.userip,"213.179.58.99");

			/*
                        QueryData.query[0] = '\0';
			
                        for(i=1;i<argc ;i++) {
                                sprintf(QueryData.query,"%s %s",QueryData.query,argv[i]);
                        }
			*/
                        strcpy(QueryData.query,argv[1]);
			strcpy(QueryData.subname,argv[2]);
			#ifdef BLACK_BOKS
				strcpy(QueryData.search_user,argv[3]);
			#else
				QueryData.search_user[0] = '\0';
			#endif

			QueryData.MaxsHits = DefultMaxsHits;
			QueryData.start = 1;
			QueryData.filterOn = 1;
			QueryData.HTTP_ACCEPT_LANGUAGE[0] = '\0';
        		QueryData.HTTP_USER_AGENT[0] = '\0';
        		QueryData.HTTP_REFERER[0] = '\0';
			QueryData.AmazonAssociateTag[0] = '\0';
			QueryData.AmazonSubscriptionId[0] = '\0';
			//v3 QueryData.languageFilter[0] = '\0';
			QueryData.orderby[0] = '\0';

                        //printf("argc :%i %s %s\n",argc,argv[1],argv[2]);
                        printf("query %s, subname %s\n",QueryData.query,QueryData.subname);

			//printer ut oversikt over serverne vi skal koble til
			printf("server(s):\n");
			for(i=0;i<nrOfServers;i++) {
				printf("%i %s\n",i,servers[i]);
			}

			printf("piserver(s):\n");
			for(i=0;i<nrOfPiServers;i++) {
				printf("%i %s\n",i,piservers[i]);
			}

			printf("adserver(s):\n");
			for(i=0;i<nrOfAddServers;i++) {
				printf("%i %s\n",i,addservers[i]);
			}


			printf("\n");
                }
        }
        else {
		// Initialize the CGI lib
        	res = cgi_init();

		// Was there an error initializing the CGI???
	        if (res != CGIERR_NONE) {
        	        printf("Error # %d: %s<p>\n", res, cgi_strerror(res));
			die(1,"Cgi-lib error.");
        	}
		
		if (cgi_getentrystr("query") == NULL) {
                	die(2,"Did'n receive any query.");
        	}
		else {
        	        strscpy(QueryData.query,cgi_getentrystr("query"),sizeof(QueryData.query) -1);
	        }


		if (cgi_getentrystr("search_bruker") == NULL) {
                	fprintf(stderr,"Did'n receive any username.");
			QueryData.search_user[0] = '\0';
        	}
		else {
        	        strscpy((char *)QueryData.search_user,cgi_getentrystr("search_bruker"),sizeof(QueryData.query) -1);
	        }


		if (cgi_getentrystr("userip") == NULL) {
                        fprintf(stderr,"Did'n receive any user ip.");
			QueryData.userip[0] = '\0';
                }
		else {
			strscpy(QueryData.userip,cgi_getentrystr("userip"),sizeof(QueryData.userip) -1);
		}
		//temp:setter ip manuelt for å teste
		//strcpy(QueryData.userip,"64.236.24.28");

		if (cgi_getentrystr("subname") == NULL) {
                        //die(2,"Did'n receive any subname.");
			strscpy(QueryData.subname,"www",sizeof(QueryData.subname) -1);
                }
		else {
			strscpy(QueryData.subname,cgi_getentrystr("subname"),sizeof(QueryData.subname) -1);
		}

		#ifdef BLACK_BOKS
		if (cgi_getentrystr("tkey") == NULL) {
                        die(2,"Did'n receive any tkey.");
                }
		else if (strlen(cgi_getentrystr("tkey")) != 32) {
			die(2,"tkey isent 32 bytes long\n");
		}
		else {
			strscpy(QueryData.tkey,cgi_getentrystr("tkey"),sizeof(QueryData.tkey));

			//sjek tkey
			if (!tkeyisok(QueryData.tkey)) {
				die(2,"Wrong tkey.");
			}
		}
		#endif

		if (cgi_getentrystr("orderby") == NULL) {
			QueryData.orderby[0] = '\0';
                }
		else {
			strscpy(QueryData.orderby,cgi_getentrystr("orderby"),sizeof(QueryData.orderby) -1);
		}

		if (cgi_getentryint("maxshits") == 0) {
			QueryData.MaxsHits = DefultMaxsHits;
		}
		else {
			QueryData.MaxsHits = cgi_getentryint("maxshits");			
		}

		
		if (cgi_getentryint("start") == 0) {
			QueryData.start = 1;
		}
		else {
			QueryData.start = cgi_getentryint("start");
		}

		if ((cgi_getentrystr("filter") == NULL) || (strcmp(cgi_getentrystr("filter"),"") == 0) ) {
			QueryData.filterOn = 1;
		}
		else {
			QueryData.filterOn = cgi_getentryint("filter");
		}


		if (cgi_getentrystr("HTTP_ACCEPT_LANGUAGE") != NULL) {
			strscpy(QueryData.HTTP_ACCEPT_LANGUAGE,cgi_getentrystr("HTTP_ACCEPT_LANGUAGE"),sizeof(QueryData.HTTP_ACCEPT_LANGUAGE));
		}
		else {
			QueryData.HTTP_ACCEPT_LANGUAGE[0] = '\0';
			fprintf(stderr,"Dident get a HTTP_ACCEPT_LANGUAGE\n");
		}

		if (cgi_getentrystr("HTTP_USER_AGENT") != NULL) {
			strscpy(QueryData.HTTP_USER_AGENT,cgi_getentrystr("HTTP_USER_AGENT"),sizeof(QueryData.HTTP_USER_AGENT));
		}
		else {
			QueryData.HTTP_USER_AGENT[0] = '\0';
			fprintf(stderr,"Dident get a HTTP_USER_AGENT\n");
		}

		if (cgi_getentrystr("HTTP_REFERER") != NULL) {
			strscpy(QueryData.HTTP_REFERER,cgi_getentrystr("HTTP_REFERER"),sizeof(QueryData.HTTP_REFERER));
		}
		else {
			QueryData.HTTP_REFERER[0] = '\0';
			fprintf(stderr,"Dident get a HTTP_REFERER\n");
		}



		if (cgi_getentrystr("languageFilter") != NULL) {
			//v13 strscpy(QueryData.languageFilter,cgi_getentrystr("languageFilter"),sizeof(QueryData.languageFilter) -1);
		}
		else {
			//v13 QueryData.languageFilter[0] = '\0';
			//v13 fprintf(stderr,"Dident get a languageFilter\n");
		}


		if (cgi_getentrystr("AmazonAssociateTag") != NULL) {
			strscpy(QueryData.AmazonAssociateTag,cgi_getentrystr("AmazonAssociateTag"),sizeof(QueryData.AmazonAssociateTag) -1);
		}
		else {
			QueryData.AmazonAssociateTag[0] = '\0';
			fprintf(stderr,"Dident get a AmazonAssociateTag\n");
		}

		if (cgi_getentrystr("AmazonSubscriptionId") != NULL) {
			strscpy(QueryData.AmazonSubscriptionId,cgi_getentrystr("AmazonSubscriptionId"),sizeof(QueryData.AmazonSubscriptionId) -1);
		}
		else {
			QueryData.AmazonSubscriptionId[0] = '\0';
			fprintf(stderr,"Dident get a AmazonSubscriptionId\n");
		}

	

        }
	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: init %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

        if (strlen(QueryData.query) > MaxQueryLen -1) {
                die(3,"Query to long.");
        }

	#ifdef BLACK_BOKS

	#else
		//hvis vi har : i seg må vi avslutte da vi ikke støtter dette
		if(strchr(QueryData.query,':') != NULL) {
			die(4,"Command not yet implemented.");
		}
	#endif
	//hvis vi har et for kort query
	if(strlen(QueryData.query) < 2) {
		die(5,"Query to short.");
	}

        //gjør om til liten case
	//21 feb 2007: collection er case sensetiv. Bare søkeord skal gjøres om. Må gjøre dette en annen plass
        //for(i=0;i<strlen(QueryData.query);i++) {
        //        QueryData.query[i] = btolower(QueryData.query[i]);
        //}

	//nårmalisere query. 
	//strcasesandr(QueryData.query,sizeof(QueryData.query),"."," ");

	for(i=0;i<strlen(QueryData.query);i++) {

		// 92: \, 32: space, 34: ", 43: +

		if(
		(isalnum(QueryData.query[i])) 
                || (43 == (unsigned int)QueryData.query[i])
                || (34 == (unsigned int)QueryData.query[i])
                || (32 == (unsigned int)QueryData.query[i])
		|| (45 == (unsigned int)QueryData.query[i])
		|| (58 == (unsigned int)QueryData.query[i])
		|| (64 == (unsigned int)QueryData.query[i]) // @
		|| (46 == (unsigned int)QueryData.query[i]) // .
		|| (128 < (unsigned int)QueryData.query[i])

		) {
			//gjø ingenting for nå
		}
		else {
			sprintf(errormessage,"Illegal character in query. \"%c\" ascii value: %u",QueryData.query[i],(unsigned int)QueryData.query[i]);
			die(15,errormessage);
		}

	}

        //fjerner tegn. " blir til &quot;
	strcpy(QueryData.queryhtml,QueryData.query);
	strsandr(QueryData.queryhtml,"\"","&quot;");
	//printf("query behandlet %s\n",QueryData.queryhtml);
	#ifdef DEBUG
		gettimeofday(&end_time, NULL);
		printf("Time debug: query normalizeing %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	#ifndef BLACK_BOKS
	//prøver å finne ut hvilket land ut fra ip
	GeoIP *gi;
	GeoIPRecord * gir;

        //gi = GeoIP_open(bfile("data/GeoLiteCity.dat"), GEOIP_MEMORY_CACHE);
        gi = GeoIP_open(bfile("data/GeoLiteCity.dat"), GEOIP_STANDARD);
        if (gi == NULL) {
                fprintf(stderr, "Error opening ip database\n");
                //exit(1);
        }
	else {
        	//iparesse
        	if ((gir = GeoIP_record_by_name (gi, (const char *)&QueryData.userip)) == NULL) {
			strscpy(QueryData.GeoIPcontry,"na",sizeof(QueryData.GeoIPcontry));
        	        fprintf(stderr, "Error looking up ip for \"%s\". Wil use \"%s\" as country code\n",QueryData.userip,QueryData.GeoIPcontry);

        	}
		else {
			sprintf(QueryData.GeoIPcontry,"%s",gir->country_code);
		
	

			#ifdef DEBUG
        		printf("GeoIP: %s\t%s\t%s\t%s\t%s\t%f\t%f\t%d\t%d\n", queryNodeHeder.userip,
                                         gir->country_code,
                                         gir->region,
                                         gir->city,
                                         gir->postal_code,
                                         gir->latitude,
                                         gir->longitude,
                                         gir->dma_code,
                                         gir->area_code);
			printf("GeoIPcontry: %s\n",QueryData.GeoIPcontry);
			#endif
        		GeoIPRecord_delete(gir);
		}

		GeoIP_delete(gi);
	}
	#else
		sprintf(QueryData.GeoIPcontry,"na");
	#endif

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: geoip %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif
		
	//kopierer inn query	
	strscpy(queryNodeHeder.query,QueryData.query,sizeof(queryNodeHeder.query) -1);
	strscpy(queryNodeHeder.subname,QueryData.subname,sizeof(queryNodeHeder.subname) -1);
	strscpy(queryNodeHeder.userip,QueryData.userip,sizeof(queryNodeHeder.userip) -1);

	//hvorfår får vi broblemer når vi bruker -02 og -1 her ? Ser ut til at strcpy ikke terminerer hvis 
	//den når max lengde.
	//strscpy(queryNodeHeder.GeoIPcontry,QueryData.GeoIPcontry,sizeof(queryNodeHeder.GeoIPcontry) -1);
	strcpy(queryNodeHeder.GeoIPcontry,QueryData.GeoIPcontry);

	strscpy(queryNodeHeder.search_user,QueryData.search_user,sizeof(queryNodeHeder.search_user) -1);

	strscpy(queryNodeHeder.HTTP_ACCEPT_LANGUAGE,QueryData.HTTP_ACCEPT_LANGUAGE,sizeof(queryNodeHeder.HTTP_ACCEPT_LANGUAGE));
	strscpy(queryNodeHeder.HTTP_USER_AGENT,QueryData.HTTP_USER_AGENT,sizeof(queryNodeHeder.HTTP_USER_AGENT));
	strscpy(queryNodeHeder.HTTP_REFERER,QueryData.HTTP_REFERER,sizeof(queryNodeHeder.HTTP_REFERER));
	//v13 strscpy(queryNodeHeder.languageFilter,QueryData.languageFilter,sizeof(queryNodeHeder.languageFilter) -1);
	strscpy(queryNodeHeder.orderby,QueryData.orderby,sizeof(queryNodeHeder.orderby) -1);


	strscpy(queryNodeHeder.AmazonAssociateTag,QueryData.AmazonAssociateTag,sizeof(queryNodeHeder.AmazonAssociateTag) -1);
	strscpy(queryNodeHeder.AmazonSubscriptionId,QueryData.AmazonSubscriptionId,sizeof(queryNodeHeder.AmazonSubscriptionId) -1);

	queryNodeHeder.MaxsHits = QueryData.MaxsHits;
	if (nrOfServers >= 4) {
		queryNodeHeder.MaxsHits = (queryNodeHeder.MaxsHits / 2); // datane er fordelt, så hver server trenger ikke å generere mer en xx deler av den
	}
	queryNodeHeder.filterOn = QueryData.filterOn;
	--QueryData.start; //maskinen begynner på 1, menske på 0
	queryNodeHeder.start = QueryData.start;



	Sider = malloc(QueryData.MaxsHits * maxServers * sizeof(struct SiderFormat));

	//inaliserer side arrayen
	//aaaaaaaaa
	for(i=0;i<(nrOfServers + nrOfPiServers) * QueryData.MaxsHits;i++) {
        	Sider[i].iindex.allrank = 0;
        	Sider[i].deletet = 1;
	}

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: query copying %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef WITH_CASHE
	//tester for cashe
	sprintf(cashefile,"%s/%s.%i.%s",bfile(cashedir),QueryData.queryhtml,QueryData.start,QueryData.GeoIPcontry);
	
	sprintf(prequeryfile,"%s/%s.%i.%s",bfile(prequerydir),QueryData.queryhtml,QueryData.start,QueryData.GeoIPcontry);
	FILE *CACHE;

	if ((dispconfig.useprequery) && ((CACHE = fopen(prequeryfile,"rb")) != NULL)) {
		hasprequery = 1;

		debug("can open prequeryfile file \"%s\"",prequeryfile);

		flock(fileno(CACHE),LOCK_SH);
		fread(&pageNr,sizeof(pageNr),1,CACHE);
		fread(&FinalSiderHeder,sizeof(FinalSiderHeder),1,CACHE);
                fread(&SiderHeder,sizeof(SiderHeder),1,CACHE);

		debug("have %i cashed pages",pageNr);
		
                for (i=0;i<nrOfServers * QueryData.MaxsHits;i++) {
                        fread(&Sider[i],sizeof(struct SiderFormat),1,CACHE);
                        //printf("%s\n",Sider[i].uri);
                }
		
		//fread(&Sider[i],(sizeof(struct SiderFormat) * nrOfServers * QueryData.MaxsHits),1,CACHE);
		fclose(CACHE);
	}
	else if ((dispconfig.usecashe) && ((CACHE = fopen(cashefile,"rb")) != NULL)) {
		hascashe = 1;

		debug("can open cashe file \"%s\"",cashefile);

		flock(fileno(CACHE),LOCK_SH);
		fread(&pageNr,sizeof(pageNr),1,CACHE);
		fread(&FinalSiderHeder,sizeof(FinalSiderHeder),1,CACHE);
                fread(&SiderHeder,sizeof(SiderHeder),1,CACHE);

		debug("have %i cashed pages",pageNr);
		
                for (i=0;i<nrOfServers * QueryData.MaxsHits;i++) {
                        if ((n=fread(&Sider[i],sizeof(struct SiderFormat),1,CACHE)) != 1) {
				debug("cant 1 read cashe page. Did read %i.",n);
				//--pageNr;
			}
                        debug("cashe: reading %s",Sider[i].uri);
                }
		
		//fread(&Sider[i],(sizeof(struct SiderFormat) * nrOfServers * QueryData.MaxsHits),1,CACHE);
		fclose(CACHE);
	}
	else {
		//fprintf(stderr,"cant aces cashe file \"%s\": %s\n",cashefile,strerror(errno));
		hascashe = 0;
		hasprequery = 0;
		pageNr = 0;

	}
	#else
		hasprequery = 0;
		hascashe = 0;
		pageNr = 0;
	#endif


	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	

	//kobler til vanlige servere
	if ((!hascashe) && (!hasprequery)) {
		bsConectAndQuery(sockfd,nrOfServers,servers,&queryNodeHeder,0,searchport);
	}

	//addservere
	bsConectAndQuery(addsockfd,nrOfAddServers,addservers,&queryNodeHeder,0,searchport);
	//Paid inclusion
	bsConectAndQuery(sockfd,nrOfPiServers,piservers,&queryNodeHeder,nrOfServers,searchport);


	//Paid inclusion
	brGetPages(sockfd,nrOfPiServers,SiderHeder,Sider,&pageNr,0);

	if ((!hascashe) && (!hasprequery)) {
		brGetPages(sockfd,nrOfServers,SiderHeder,Sider,&pageNr,nrOfPiServers);

	}


	//addserver bruker som regel mest tid, så tar den sist slik at vi ikke trenger å vente unødvendig
	brGetPages(addsockfd,nrOfAddServers,AddSiderHeder,Sider,&pageNr,0);

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: geting pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	FinalSiderHeder.TotaltTreff = 0;
	FinalSiderHeder.filtered = 0;
	nrRespondedServers = 0;

	for (i=0;i<(nrOfServers + nrOfPiServers);i++) {
		//aaaaa
		if (sockfd[i] != 0) {
			FinalSiderHeder.TotaltTreff += SiderHeder[i].TotaltTreff;
			FinalSiderHeder.filtered += SiderHeder[i].filtered;
			#ifdef DEBUG
	        	printf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" tid=\"%f\" filtered=\"%i\" showabal=\"%i\" servername=\"%s\"/>\n",SiderHeder[i].TotaltTreff,QueryData.query,SiderHeder[i].hiliteQuery,SiderHeder[i].total_usecs,SiderHeder[i].filtered,SiderHeder[i].showabal,SiderHeder[i].servername);
			#endif

			++nrRespondedServers;
		}
	}



	
	//finner en hillitet query
	if (nrRespondedServers != 0) {
		funnet = 0;
		for(i=0;i<(nrOfServers + nrOfPiServers) && !funnet;i++) {
			if ((sockfd[i] != 0) && (SiderHeder[i].hiliteQuery != '\0')) {
				strcpy(FinalSiderHeder.hiliteQuery,SiderHeder[i].hiliteQuery);
				funnet =1;
			}
		}
	}
	else {
		FinalSiderHeder.hiliteQuery[0] = '\0';
	}

	//hvis vi ikke fikk svar fra noen
	if(nrRespondedServers == 0) {
		die(16,"Couldnt contact the Boitho search system. Please try again later.");
	}
	//genererer feil om at ikke alle server svarte på queryet
	if (nrRespondedServers != (nrOfServers + nrOfPiServers)) {
		addError(&errorha,11,"Not all the search nodes responded to your query. Result quality may have been negatively effected.");
	}

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif
	//sorterer resultatene
	//mgsort(Sider, nrOfServers * QueryData.MaxsHits , sizeof(struct SiderFormat), compare_elements);
	mgsort(Sider, pageNr , sizeof(struct SiderFormat), compare_elements);

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: mgsort_1 %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	posisjon=0;
	for(i=0;i<QueryData.MaxsHits * nrOfServers + nrOfPiServers;i++) {
		if (!Sider[i].deletet) {
			Sider[i].posisjon = posisjon++;
		}

		#ifdef DEBUG
			//printf("%s\n",Sider[i].url);
		#endif
	}	



	
	//fjerner eventuelle adult sider
	AdultPages = 0;
	NonAdultPages = 0;
	for(i=0;i<QueryData.MaxsHits * nrOfServers + nrOfPiServers;i++) {	
		if (!Sider[i].deletet) {

			if (Sider[i].DocumentIndex.AdultWeight > 50) {
				++AdultPages;
			}
			else {
				++NonAdultPages;
			}

		}		
	}
	
	#ifdef DEBUG
		printf("AdultPages %i, NonAdultPages: %i\n",AdultPages,NonAdultPages);
	#endif
	//hvis vi har adult pages sjekker vi om vi har nokk ikke adult pages å vise, hvis ikke viser vi bare adult

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif		

		//dette er kansje ikke optimalet, da vi går gjenom alle siden. Ikke bare de som skal være med
		for(i=0;i<QueryData.MaxsHits * nrOfServers + nrOfPiServers;i++) {

			//printf("url %s, %i, %i\n",Sider[i].DocumentIndex.Url,Sider[i].deletet,Sider[i].DocumentIndex.AdultWeight);
			if (!Sider[i].deletet) {
				//setter som slettet
				Sider[i].deletet = 1;


				if ((QueryData.filterOn) && (Sider[i].subname.config.filterSameUrl) 
					&& (filterSameUrl(i,Sider[i].url,Sider)) ) {
					#ifdef DEBUG
                        			printf("Hav seen url befor. Url '%s', DocID %u\n",Sider[i].url,Sider[i].iindex.DocID);
					#endif
                        		(*SiderHeder).filtered++;
                        		continue;
                		}

				#ifndef BLACK_BOKS

				if ((QueryData.filterOn) && Sider[i].DocumentIndex.AdultWeight > 50) {
					#ifdef DEBUG
						printf("slettet adult side %s ault %i\n",Sider[i].url,Sider[i].DocumentIndex.AdultWeight);
					#endif
					continue;
				}

				if ((QueryData.filterOn) && (Sider[i].subname.config.filterSameCrc32) 
					&& filterSameCrc32(i,&Sider[i],Sider)) {
					#ifdef DEBUG
                        	        	printf("hav same crc32. crc32 from DocumentIndex\n");
					#endif
                        	        (*SiderHeder).filtered++;
                        	        continue;
                        	}

				if ((QueryData.filterOn) && (Sider[i].subname.config.filterSameDomain) 
					&& (filterSameDomain(i,&Sider[i],Sider))) {
					#ifdef DEBUG
                        			printf("hav same domain \"%s\"\n",Sider[i].domain);
					#endif
                        		(*SiderHeder).filtered++;
                        		continue;
                		}
				/*
				if ((QueryData.filterOn) && filterDescription(i,&Sider[i],Sider)) {
					#ifdef DEBUG
                        			printf("hav same Description. DocID %i\n",Sider[i].iindex.DocID);
					#endif
					(*SiderHeder).filtered++;
                        		continue;
                		}
				*/
                		#endif

				//printf("url %s\n",Sider[i].DocumentIndex.Url);

				//hvis siden overlevde helt hit er den ok
				Sider[i].deletet = 0;
			}
		}

//		}
//	}
	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: filter pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif
	
	//resorterer query
	//mgsort(Sider, nrOfServers * QueryData.MaxsHits , sizeof(struct SiderFormat), compare_elements_posisjon);
	mgsort(Sider, pageNr , sizeof(struct SiderFormat), compare_elements_posisjon);

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: mgsort_2 %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	/*tempaa
	FinalSiderHeder.showabal = 0;
	for(i=0;i<QueryData.MaxsHits * nrOfServers;i++) {
                if (!Sider[i].deletet) {
			++FinalSiderHeder.showabal;
		}
	}
	*/
	FinalSiderHeder.showabal = pageNr;
	if (FinalSiderHeder.showabal > QueryData.MaxsHits) {
   		FinalSiderHeder.showabal = QueryData.MaxsHits;
        }

	
	//} //casche
	totlaAds = 0;

	//stopper å ta tidn og kalkulerer hvor lang tid vi brukte
	gettimeofday(&main_end_time, NULL);
	FinalSiderHeder.total_usecs = getTimeDifference(&main_start_time,&main_end_time);
	
        //printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?> \n");
        printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?> \n");
        printf("<!DOCTYPE family SYSTEM \"http://www.boitho.com/xml/search.dtd\"> \n");

        printf("<SEARCH>\n");   
	//får rare svar fra hilite. Dropper å bruke den får nå
	FinalSiderHeder.hiliteQuery[0] = '\0';
        printf("<RESULT_INFO TOTAL=\"%i\" QUERY=\"%s\" HILITE=\"%s\" TIME=\"%f\" FILTERED=\"%i\" SHOWABAL=\"%i\" CASHE=\"%i\" PREQUERY=\"%i\" GEOIPCONTRY=\"%s\" SUBNAME=\"%s\" BOITHOHOME=\"%s\"/>\n",FinalSiderHeder.TotaltTreff,QueryData.queryhtml,FinalSiderHeder.hiliteQuery,FinalSiderHeder.total_usecs,FinalSiderHeder.filtered,FinalSiderHeder.showabal,hascashe,hasprequery,QueryData.GeoIPcontry,QueryData.subname,bfile(""));


	//viser info om serverne som svarte
	printf("<SEARCHNODES_INFO NROFSEARCHNODES=\"%i\" />",nrRespondedServers);
	
	/*
	for (i=0;i<nrOfServers;i++) {
                //tempaa:if (sockfd[i] != 0) {
			printf("<SEARCHNODES>\n");
				printf("\t<NODENAME>%s</NODENAME>\n",SiderHeder[i].servername);
                 		printf("\t<TOTALTIME>%f</TOTALTIME>\n",SiderHeder[i].total_usecs);
				printf("\t<FILTERED>%i</FILTERED>\n",SiderHeder[i].filtered);
				printf("\t<HITS>%i</HITS>\n",SiderHeder[i].TotaltTreff);
			printf("</SEARCHNODES>\n");
		//}	
	}
	*/
	//cashe eller ingen cashe. Adserverene skal vises
	for (i=0;i<nrOfAddServers;i++) {
       	        if (addsockfd[i] != 0) {
			printf("<SEARCHNODES>\n");
				printf("\t<NODENAME>%s</NODENAME>\n",AddSiderHeder[i].servername);
       	         		printf("\t<TOTALTIME>%f</TOTALTIME>\n",AddSiderHeder[i].total_usecs);
				printf("\t<FILTERED>%i</FILTERED>\n",AddSiderHeder[i].filtered);
				printf("\t<HITS>%i</HITS>\n",AddSiderHeder[i].TotaltTreff);
			printf("</SEARCHNODES>\n");
		}	
	}


	if ((!hascashe) && (!hasprequery)) {

		for (i=0;i<nrOfServers + nrOfPiServers;i++) {
                	if (sockfd[i] != 0) {
				printf("<SEARCHNODES>\n");
					printf("\t<NODENAME>%s</NODENAME>\n",SiderHeder[i].servername);
                	 		printf("\t<TOTALTIME>%f</TOTALTIME>\n",SiderHeder[i].total_usecs);
					printf("\t<FILTERED>%i</FILTERED>\n",SiderHeder[i].filtered);
					printf("\t<HITS>%i</HITS>\n",SiderHeder[i].TotaltTreff);

					#ifndef DEBUG
					printf("\t<TIMES>");
						
						printf("\t<AthorSearch>%f</AthorSearch>\n",SiderHeder[i].queryTime.AthorSearch);
						printf("\t<AthorRank>%f</AthorRank>\n",SiderHeder[i].queryTime.AthorRank);
						printf("\t<UrlSearch>%f</UrlSearch>\n",SiderHeder[i].queryTime.UrlSearch);
						printf("\t<MainSearch>%f</MainSearch>\n",SiderHeder[i].queryTime.MainSearch);
						printf("\t<MainRank>%f</MainRank>\n",SiderHeder[i].queryTime.MainRank);
						printf("\t<MainAthorMerge>%f</MainAthorMerge>\n",SiderHeder[i].queryTime.MainAthorMerge);
						printf("\t<popRank>%f</popRank>\n",SiderHeder[i].queryTime.popRank);
						printf("\t<responseShortning>%f</responseShortning>\n",SiderHeder[i].queryTime.responseShortning);

						printf("\t<allrankCalc>%f</allrankCalc>\n",SiderHeder[i].queryTime.allrankCalc);
						printf("\t<indexSort>%f</indexSort>\n",SiderHeder[i].queryTime.indexSort);
						printf("\t<searchSimple>%f</searchSimple>\n",SiderHeder[i].queryTime.searchSimple);

						printf("\t<popResult>%f</popResult>\n",SiderHeder[i].queryTime.popResult);
						printf("\t<adultcalk>%f</adultcalk>\n",SiderHeder[i].queryTime.adultcalk);

						#ifdef BLACK_BOKS
							printf("\t<filetypes>%f</filetypes>\n",SiderHeder[i].queryTime.filetypes);
							printf("\t<iintegerGetValueDate>%f</iintegerGetValueDate>\n",SiderHeder[i].queryTime.iintegerGetValueDate);
							printf("\t<dateview>%f</dateview>\n",SiderHeder[i].queryTime.dateview);
							printf("\t<crawlManager>%f</crawlManager>\n",SiderHeder[i].queryTime.crawlManager);
							printf("\t<getUserObjekt>%f</getUserObjekt>\n",SiderHeder[i].queryTime.getUserObjekt);
							printf("\t<cmc_conect>%f</cmc_conect>\n",SiderHeder[i].queryTime.cmc_conect);
						#endif
					printf("\t</TIMES>");

					printf("\t<FILTERTRAPP>");
						printf("\t<filterAdultWeight_1>%i</filterAdultWeight_1>\n",SiderHeder[i].filtersTraped.filterAdultWeight_1);
						printf("\t<filterSameCrc32_1>%i</filterSameCrc32_1>\n",SiderHeder[i].filtersTraped.filterSameCrc32_1);
						printf("\t<filterSameUrl>%i</filterSameUrl>\n",SiderHeder[i].filtersTraped.filterSameUrl);
						printf("\t<find_domain_no_subname>%i</find_domain_no_subname>\n",SiderHeder[i].filtersTraped.find_domain_no_subname);
						printf("\t<filterSameDomain>%i</filterSameDomain>\n",SiderHeder[i].filtersTraped.filterSameDomain);
						printf("\t<filterTLDs>%i</filterTLDs>\n",SiderHeder[i].filtersTraped.filterTLDs);
						printf("\t<filterResponse>%i</filterResponse>\n",SiderHeder[i].filtersTraped.filterResponse);
						printf("\t<cantpopResult>%i</cantpopResult>\n",SiderHeder[i].filtersTraped.cantpopResult);
						printf("\t<cmc_pathaccess>%i</cmc_pathaccess>\n",SiderHeder[i].filtersTraped.cmc_pathaccess);
						printf("\t<filterSameCrc32_2>%i</filterSameCrc32_2>\n",SiderHeder[i].filtersTraped.filterSameCrc32_2);

					printf("\t</FILTERTRAPP>");
					#endif

				printf("</SEARCHNODES>\n");


			}	
		}

	}
	else {
                
		printf("<SEARCHNODES>\n");
			printf("\t<NODENAME>cashe.boitho.com</NODENAME>\n");
               		printf("\t<TOTALTIME>0</TOTALTIME>\n");
			printf("\t<FILTERED>0</FILTERED>\n");
			printf("\t<HITS>0</HITS>\n");
		printf("</SEARCHNODES>\n");

	}
	


	//hvis vi har noen errorrs viser vi de
	for (i=0;i<errorha.nr;i++) {
		printf("<ERROR>\n");
		printf("  <ERRORCODE>%i</ERRORCODE>\n",errorha.errorcode[i]);
        	printf("  <ERRORMESSAGE>%s</ERRORMESSAGE>\n",errorha.errormessage[i]);
        	printf("</ERROR>\n");
	}

	#ifdef BLACK_BOKS

	for(i=0;i<SiderHeder[0].filters.collections.nrof;i++) {

		if (SiderHeder[0].filters.collections.elements[i].checked) {
			strscpy(colchecked," SELECTED=\"TRUE\"",sizeof(colchecked));
		}
		else {
			strscpy(colchecked,"",sizeof(colchecked));
		}

		printf("<COLLECTION%s>\n",colchecked);
		printf("<NAME>%s</NAME>\n",SiderHeder[0].filters.collections.elements[i].name);
		printf("<TOTALRESULTSCOUNT>%i</TOTALRESULTSCOUNT>\n",SiderHeder[0].filters.collections.elements[i].nrof);

		printf("</COLLECTION>\n");

		
	}
	/*	
	for (i=0;i<SiderHeder[0].nrOfSubnames;i++) {

		if (SiderHeder[0].subnames[i].checked) {
			strscpy(colchecked," SELECTED=\"TRUE\"",sizeof(colchecked));
		}
		else {
			strscpy(colchecked,"",sizeof(colchecked));
		}

		printf("<COLLECTION%s>\n",colchecked);

		//viser bar en del av subnamet. Må tenke på hva vi skal gjør her. Hadde vært fint om brukeren kunne 
		//bruk subname som "mail", mine filer osv, og vi mappet til til de han har tilgang til
		if ((cpnt = strchr(SiderHeder[0].subnames[i].subname,'_')) != NULL) {
			(*cpnt) = '\0';
		}
	
		printf("<NAME>%s</NAME>\n",SiderHeder[0].subnames[i].subname);
		printf("<TOTALRESULTSCOUNT>%i</TOTALRESULTSCOUNT>\n",SiderHeder[0].subnames[i].hits);

		printf("</COLLECTION>\n");


	}
	*/


	for (i=0;i<SiderHeder[0].filters.filtypes.nrof;i++) {
		printf("<FILETYPE>\n");

		printf("<FILENAME>%s</FILENAME>\n<FILENR>%i</FILENR>",SiderHeder[0].filters.filtypes.elements[i].name,SiderHeder[0].filters.filtypes.elements[i].nrof);
		
		printf("</FILETYPE>\n");

	}		

	char *dateview_type_names[] = {"","TODAY","YESTERDAY","LAST_WEEK","LAST_MONTH","THIS_YEAR","LAST_YEAR","WEEK","MONTH","YEARS_AGO"};

	printf("<DATES>\n");
		for (y=1;y<10;y++) {
			if (SiderHeder[0].dates > 0) {
				printf("\t<%s>%i</%s>\n",dateview_type_names[y],SiderHeder[0].dates[y],dateview_type_names[y]);
			}
		}
	printf("</DATES>\n");

	#endif

		//skal printe ut FinalSiderHeder.showabal sider, men noen av sidene kan være slettet
	i=0;
	x=0;
	while ((x<FinalSiderHeder.showabal) && (i < (QueryData.MaxsHits * (nrOfServers + nrOfPiServers)))) {
	

		
		if (!Sider[i].deletet) {

			#ifdef DEBUG		
				printf("r %i, a: %i, bid : %f, u: %s. DocID: %u\n",Sider[i].iindex.allrank,Sider[i].DocumentIndex.AdultWeight,Sider[i].bid,Sider[i].url,Sider[i].iindex.DocID);
			#endif

			//har url i <![CDATA[ ]]> paranteser nå 
		/*
			strsandr (Sider[i].url, "&", "&amp;");
                	//filtrerer ut tegn som ikke er lov i xml
                	while ((strpointer = strchr(Sider[i].url,'&')) != NULL) {
                	        (*strpointer) = 'a';
                	}

			strsandr (Sider[i].uri, "&", "&amp;");
			//kan ikke gjøre om en ppc url
			if ((Sider[i].type != siderType_ppctop) && (Sider[i].type != siderType_ppcside)) {
                		while ((strpointer = strchr(Sider[i].uri,'&')) != NULL) {
                		        (*strpointer) = 'a';
                		}
			}
		*/

                	//while ((strpointer = strchr(Sider[i].title,'&')) != NULL) {
                	//        (*strpointer) = 'a';
                	//}
                	//while ((strpointer = strchr(Sider[i].description,'&')) != NULL) {
                	//        (*strpointer) = 'a';
                	//}

			#ifdef DEBUG

			#else


		
			if (Sider[i].type == siderType_ppctop) {
				printf("<RESULT_PPC>\n");
				printf("\t<BID>%f</BID>\n",Sider[i].bid);
				++totlaAds; //ikke helt bra denne. Vi teller antall anonser vist, ikke totalt
			}
			else if (Sider[i].type == siderType_ppcside) {
				printf("<RESULT_PPCSIDE>\n");
				printf("\t<BID>%f</BID>\n",Sider[i].bid);
				++totlaAds; //ikke helt bra denne. Vi teller antall anonser vist, ikke totalt 
			}
			else {
                		printf("<RESULT>\n");
				//printf("\t<BID></BID>\n");
			}

                	printf("\t<DOCID>%i-%i</DOCID>\n",Sider[i].iindex.DocID,rLotForDOCid(Sider[i].iindex.DocID));


                	printf("\t<TITLE><![CDATA[%s]]></TITLE>\n",Sider[i].title);


                	//DocumentIndex
                	printf("\t<URL><![CDATA[%s]]></URL>\n",Sider[i].url);
                	printf("\t<URI><![CDATA[%s]]></URI>\n",Sider[i].uri);

			//gjør om språk fra tall til code
			//void getLangCode(char langcode[],int langnr)
			//printf("lang nr is %s\n",Sider[i].DocumentIndex.Sprok);
			getLangCode(documentlangcode,atoi(Sider[i].DocumentIndex.Sprok));
			//printf("lang is %s\n",Sider[i].DocumentIndex.Sprok);




                	printf("\t<DOCUMENTLANGUAGE>%s</DOCUMENTLANGUAGE>\n",documentlangcode);
                	//temp: blir rare tegn her              
			printf("\t<DOCUMENTTYPE>%s</DOCUMENTTYPE>\n",Sider[i].DocumentIndex.Dokumenttype);

                	printf("\t<POSISJON>%i</POSISJON>\n",x);
                	printf("\t<REPOSITORYSIZE>%u</REPOSITORYSIZE>\n",Sider[i].DocumentIndex.htmlSize);


			if (Sider[i].thumbnale[0] != '\0') {
                		printf("\t<THUMBNAIL>%s</THUMBNAIL>\n",Sider[i].thumbnale);

        	        	printf("\t<THUMBNAILWIDTH>%i</THUMBNAILWIDTH>\n",Sider[i].thumbnailwidth);
	                	printf("\t<THUMBNAILHEIGHT>%i</THUMBNAILHEIGHT>\n",Sider[i].thumbnailheight);
			}
			else {
				printf("\t<THUMBNAIL></THUMBNAIL>\n");
				printf("\t<THUMBNAILWIDTH></THUMBNAILWIDTH>\n");
				printf("\t<THUMBNAILHEIGHT></THUMBNAILHEIGHT>\n");
			}



                	printf("\t<DESCRIPTION><![CDATA[%s]]></DESCRIPTION>\n",Sider[i].description);



			printf("\t<CRC32>%u</CRC32>\n",Sider[i].DocumentIndex.crc32);

			//ser ikke ut til at vi teller den
			//printf("\t<PAGEGENERATETIME>%f</PAGEGENERATETIME>\n",Sider[i].pageGenerateTime);

                	printf("\t<NROFHITS>%i</NROFHITS>\n",Sider[i].iindex.TermAntall);
                	//printer ut hits (hvor i dokumenetet orde befinner seg ).
                	printf("\t<HITS>");
                	for (y=0; (y < Sider[i].iindex.TermAntall) && (y < MaxTermHit); y++) {
                	        printf("%hu ",Sider[i].iindex.hits[y]);
                	}
                	printf("</HITS>\n");

			#ifdef BLACK_BOKS
				char timebuf[26];
				printf("\t<TIME_UNIX>%u</TIME_UNIX>\n",Sider[i].DocumentIndex.CrawleDato);
				ctime_r((time_t *)&Sider[i].DocumentIndex.CrawleDato,timebuf);
				timebuf[24] = '\0';
				printf("\t<TIME_ISO>%s</TIME_ISO>\n",timebuf);

				printf("\t<RESULT_COLLECTION>%s</RESULT_COLLECTION>\n",Sider[i].subname.subname);

				//sender en tom cashe link. Må ha cashe link hvis ikke bryter vi designet
	                	printf("\t<CACHE></CACHE>\n");


			#else
				printf("\t<SERVERNAME>%s</SERVERNAME>\n",Sider[i].servername);

	                	printf("\t<ADULTWEIGHT>%hu</ADULTWEIGHT>\n",Sider[i].DocumentIndex.AdultWeight);
	                	printf("\t<METADESCRIPTION><![CDATA[]]></METADESCRIPTION>\n");
	                	printf("\t<CATEGORY></CATEGORY>\n");
	                	printf("\t<OFFENSIVE_CODE>FALSE</OFFENSIVE_CODE>\n");

                		printf("\t<TERMRANK>%i</TERMRANK>\n",Sider[i].iindex.TermRank);
                		printf("\t<POPRANK>%i</POPRANK>\n",Sider[i].iindex.PopRank);
        	        	printf("\t<ALLRANK>%i</ALLRANK>\n",Sider[i].iindex.allrank);

				ipaddr.s_addr = Sider[i].DocumentIndex.IPAddress;

                		printf("\t<IPADDRESS>%s</IPADDRESS>\n",inet_ntoa(ipaddr));

                		printf("\t<RESPONSE>%hu</RESPONSE>\n",Sider[i].DocumentIndex.response);

				printf("\t<CRAWLERVERSION>%f</CRAWLERVERSION>\n",Sider[i].DocumentIndex.clientVersion);
				printf("\t<HTMLPREPARSED>%i</HTMLPREPARSED>\n",Sider[i].HtmlPreparsed);

	                	printf("\t<CACHE>%s</CACHE>\n",Sider[i].cacheLink);


			#endif
		
			if (Sider[i].type == siderType_ppctop ) {
				printf("</RESULT_PPC>\n");
			}
			else if (Sider[i].type == siderType_ppcside ) {
				printf("</RESULT_PPCSIDE>\n");
			}
			else {
                		printf("</RESULT>\n");
			}
		
                
			#endif

			//teller bare normale sider
			if (Sider[i].type == siderType_normal) {
				++x;
			}
		}
		else{ 
			#ifdef DEBUG
			printf("nr %i er deletet. Rank %i\n",i,Sider[i].iindex.allrank);
			#endif
		}
		
		++i;
	}

	printf("</SEARCH>\n");

	
	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	//stenger ned stdout da det ikke skal sendes ut mer data, og dertfor er det ikke noen vits at klienten venter mer
	//frøykter apache dreper den den da. Kan ikke gjøre
	//fclose(stdout);

	//kalkulerer dette på ny, men uten pi servere
	nrRespondedServers = 0;
        for (i=0;i<nrOfServers;i++) {
                //aaaaa
                if (sockfd[i] != 0) {
                        ++nrRespondedServers;
                }
        }

	//stenger ned tilkoblingen til nodene
	for (i=0;i<nrOfServers + nrOfPiServers;i++) {
		if (sockfd[i] != 0) {
        		close(sockfd[i]);
		}
	}


	
	if ((LOGFILE = bfopen("logs/query.log","a")) == NULL) {
		perror(bfile("logs/query.log"));
	}
	else {
		flock(fileno(LOGFILE),LOCK_EX);
        	fprintf(LOGFILE,"%s %i\n",queryNodeHeder.query,FinalSiderHeder.TotaltTreff);
        	fclose(LOGFILE);
	}

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: end clean up %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	#ifdef WITH_CASHE
	//FILE *CACHE;
	if (hascashe) {
		//touch
		//CACHE = open(cashefile,"r+b");
		//fputc('1',CACHE); //fremprovoserer en oppdatering av akksestiden		
		//fclose(CACHE);
	}
	else if(hasprequery) {
		//har prequery
	}
	//skriver bare cashe hvis vi fikk svar fra all servere
	else if (nrRespondedServers == nrOfServers) {
		if ((CACHE = fopen(cashefile,"wb")) == NULL) {
			printf("Can't open cashefile for writing.\n");
			perror(cashefile);
		}
		else {

			flock(fileno(CACHE),LOCK_EX);
			fwrite(&pageNr,sizeof(pageNr),1,CACHE);
			fwrite(&FinalSiderHeder,sizeof(FinalSiderHeder),1,CACHE);
			fwrite(&SiderHeder,sizeof(SiderHeder),1,CACHE);
		
			for (i=0;i<pageNr;i++) {
				//vi casher bare normale sider
				//temp. jade online load test
				debug("cashing %s\n",Sider[i].url);
				//if ((Sider[i].type == siderType_normal) && (!Sider[i].subname.config.isPaidInclusion)) {
					fwrite(&Sider[i],sizeof(struct SiderFormat),1,CACHE);
				//}
			}

			//printf("aa %i %i %i\n",sizeof(FinalSiderHeder),sizeof(SiderHeder),sizeof(*Sider[2]));

			fclose(CACHE);
		}
	}
	#endif

	free(Sider);

	/********************************************************************************************/
	//mysql logging
	/********************************************************************************************/
	#ifndef NO_LOGING
		#ifdef DEBUG
			printf("Connecting to mysql db\n");
		#endif

		mysql_init(&demo_db);

		#ifndef BLACK_BOKS
			if(!mysql_real_connect(&demo_db, "www2.boitho.com", "boitho_remote", "G7J7v5L5Y7", "boitho", 3306, NULL, 0)){
    				fprintf(stderr,mysql_error(&demo_db));
    				//exit(1);
			}
		#else
			if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", BOITHO_MYSQL_DB, 3306, NULL, 0)){
    				fprintf(stderr,mysql_error(&demo_db));
    				//exit(1);
			}
		#endif
		else {

			//escaper queryet rikit
			mysql_real_escape_string(&demo_db,QueryData.queryEscaped,QueryData.query,strlen(QueryData.query));


			//logger til mysql
			sprintf(query,"insert DELAYED into search_logg values(NULL,NOW(),\"%s\",\"%s\",\"%i\",\"%f\",\"%s\",\"1\",\"%i\",\"%s\",\"%s\",\"%s\",\"%s\")",QueryData.queryEscaped,QueryData.search_user,FinalSiderHeder.TotaltTreff,FinalSiderHeder.total_usecs,QueryData.userip,totlaAds,QueryData.HTTP_ACCEPT_LANGUAGE,QueryData.HTTP_USER_AGENT,QueryData.HTTP_REFERER,QueryData.GeoIPcontry);

			mysql_real_query(&demo_db, query, strlen(query));

			//mysql_free_result(mysqlres);
			mysql_close(&demo_db);
		}
	#endif

	/********************************************************************************************/

  	/* Free the configuration */
	#ifndef BLACK_BOKS
		config_destroy(&cfg);
	#endif

	maincfgclose(&maincfg);

//	fprintf(stderr,"dispatcher_all: done\n");

	//må vi tvinge en buffer tømming ???
	printf("\n\n");	

        return EXIT_SUCCESS;
} 


int compare_elements (const void *p1, const void *p2) {

	if (((struct SiderFormat*)p1)->type != ((struct SiderFormat*)p2)->type) {
                //sortering på type, slik at ppc kommer sammen, og først
                //printf("type %i != %i\n",((struct SiderFormat*)p1)->type,((struct SiderFormat*)p2)->type);
                if (((struct SiderFormat*)p1)->type > ((struct SiderFormat*)p2)->type)
                        return -1;
                else
                        return ((struct SiderFormat*)p1)->type < ((struct SiderFormat*)p2)->type;
        }
        else if (((struct SiderFormat*)p1)->iindex.allrank > ((struct SiderFormat*)p2)->iindex.allrank)
                return -1;
        else
                return ((struct SiderFormat*)p1)->iindex.allrank < ((struct SiderFormat*)p2)->iindex.allrank;

}
int compare_elements_posisjon (const void *p1, const void *p2) {


	if (((struct SiderFormat*)p1)->type != ((struct SiderFormat*)p2)->type) {
		//sortering på type, slik at ppc kommer sammen, og først. Må så sorteres etter pris og relevans
		//printf("type %i != %i\n",((struct SiderFormat*)p1)->type,((struct SiderFormat*)p2)->type);
		if (((struct SiderFormat*)p1)->type > ((struct SiderFormat*)p2)->type)
			return -1;
		else
			return ((struct SiderFormat*)p1)->type < ((struct SiderFormat*)p2)->type;
	}
	/*
	else if (((struct SiderFormat*)p1)->subname.config.isPaidInclusion || ((struct SiderFormat*)p2)->subname.config.isPaidInclusion) {
			if ( ((struct SiderFormat*)p2)->subname.config.isPaidInclusion) {
				return 1;
			}
			else {
				return -1;
			}
	}
	*/
	else if (((struct SiderFormat*)p1)->type == siderType_normal) {
		if (((struct SiderFormat*)p1)->posisjon == ((struct SiderFormat*)p2)->posisjon){

			//printf("a: %s - %s\n",((struct SiderFormat*)p1)->DocumentIndex.Url,((struct SiderFormat*)p2)->DocumentIndex.Url);
	
			//printf("pos: %i, d: %i d: %i\n",((struct SiderFormat*)p1)->posisjon,((struct SiderFormat*)p1)->deletet,((struct SiderFormat*)p2)->deletet);
			if (strlen(((struct SiderFormat*)p1)->url) < 
				strlen(((struct SiderFormat*)p2)->url) ) {
				return -1;
			}
			else {
				return 1;
			}		
		}
        	else if (((struct SiderFormat*)p1)->posisjon < ((struct SiderFormat*)p2)->posisjon)
        	        return -1;
        	else {
        	        return ((struct SiderFormat*)p1)->posisjon > ((struct SiderFormat*)p2)->posisjon;
		}
	}
	else {
	//for ppc og ppc_side sider. Sorterer først på bud, så pr relevans
		if (((struct SiderFormat*)p1)->bid == ((struct SiderFormat*)p2)->bid) {
			
			if (((struct SiderFormat*)p1)->iindex.allrank > ((struct SiderFormat*)p2)->iindex.allrank) {
                        	return -1;
                	}
                	else {
                        	return ((struct SiderFormat*)p1)->iindex.allrank < ((struct SiderFormat*)p2)->iindex.allrank;
                	}			
		}
		else if (((struct SiderFormat*)p1)->bid > ((struct SiderFormat*)p2)->bid) {
                        return -1;
		}
                else {
                        return ((struct SiderFormat*)p1)->bid < ((struct SiderFormat*)p2)->bid;
                }
	}
}


