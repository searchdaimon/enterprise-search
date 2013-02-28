#include <mysql.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include "../common/define.h"
#include "../ppcXmlParser/ppcXml.h"
//#include "../searchFilters/searchFilters.h"
#include "../common/timediff.h"
#include "../common/bstr.h"


/* the TCP port that is used for this example */
#define TCP_PORT   7505

#ifdef WITH_THREAD
	#include <pthread.h>
	#define _REENTRANT 
#endif


void *issueAdd(void *arg);

int main (int argc, char *argv[]) {

	int     sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;

	printf("struct SiderFormat %i\n",sizeof(struct SiderFormat));
	
	#ifdef WITH_THREAD
	pthread_t chld_thr;
	#endif

	int searchport = TCP_PORT;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"p:"))!=-1) {
                switch (c) {
                        case 'p':
                                searchport = atoi(optarg);
                                printf("will use port %i\n",searchport);
                                break;
                        default:
                                exit(1);
                }

        }
        --optind;


	#ifdef WITH_THREAD
		printf("starting whth thread\n");
		my_init();
		if (mysql_thread_safe() == 0) {
			printf("The MYSQL client is'en compiled at thread safe! This will broboble crash.\n");
		}
	#else
		printf("starting single thread version\n");
	#endif


	//setter opp socket
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                fprintf(stderr,"server: can't open stream socket\n"), exit(0);




        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(searchport);



        //seter at sokket kan rebrukes
        int yes=1;
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                fprintf(stderr,"server: can't bind local address\n"), exit(0);

        /* set the level of thread concurrency we desire */
        //thr_setconcurrency(5);

        listen(sockfd, 5);

        for(;;)
        {
                clilen = sizeof(cli_addr);
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

                if(newsockfd < 0) {
                        //fprintf(stderr,"server: accept error\n"), exit(0);
                        fprintf(stderr,"server: accept error\n");
                }
                else {

                        #ifdef WITH_THREAD
                        /* create a new thread to process the incomming request */
                                //thr_create(NULL, 0, do_chld, (void *) newsockfd, THR_DETACHED, &chld_thr);
				printf("creating thread\n");
                                pthread_create(&chld_thr, NULL, issueAdd, (void *) newsockfd);
                                /* the server is now free to accept another socket request */
                        #else
                                //do_chld((void *) newsockfd);
				issueAdd((void *) newsockfd);

                        #endif
                }

        }


	return(0);


}


void *issueAdd(void *arg) {

	int     mysocfd = (int) arg;

	struct betaler_keywords_visninger_format {
		int kid;
		int betaler_side_id;
	};

	struct betaler_keywords_visninger_format betaler_keywords_visninger[10];
	char buff[1024];
	struct timeval globalstart_time, globalend_time;
	unsigned int addid;
        char *strpointer;
	int siderType_ppctopNr,siderType_ppcsideNr;
	
	struct queryNodeHederFormat queryNodeHeder;
	char queryEscaped[MaxQueryLen*2+1];
	char ppcprovider[32];

	int i,n, y, net_status, showabal;;

	//sjekker vårt egent anonsesystem

	char mysql_query [2048];

	static MYSQL demo_db;


        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;


	struct SiderHederFormat SiderHeder;
	struct ppcPagesFormat ppcPages[10];

	struct SiderFormat *Sider;

	gettimeofday(&globalstart_time, NULL);

	
        if ((i=recv(mysocfd, &queryNodeHeder, sizeof(queryNodeHeder),MSG_WAITALL)) == -1) {
                perror("recv");
        }

	printf("Query %s\n",queryNodeHeder.query);

	Sider  = (struct SiderFormat *)malloc(sizeof(struct SiderFormat) * (queryNodeHeder.MaxsHits));

        //setter alle sidene som sletett
        for (i=0;i<queryNodeHeder.MaxsHits;i++) {
                Sider[i].deletet = 1;
        }

        //sender svar med en gang at vi kan gjøre dette
        net_status = net_CanDo;
        if ((n=sendall(mysocfd,&net_status, sizeof(net_status))) != sizeof(net_status)) {
                printf("send only %i of %i\n",n,sizeof(net_status));
                perror("sendall net_status");
        }



        /********************************************************************************************/
        #ifdef DEBUG
                printf("sending query to ppc db\n");
        #endif


        mysql_init(&demo_db);

	#ifdef WITH_THREAD
		my_thread_init(); // kalt mysql_thread_init() i mysql 5.0
	#endif




        //if(!mysql_real_connect(&demo_db, "www2.boitho.com", "boitho_remote", "G7J7v5L5Y7", "boitho", 3306, NULL, 0)){
        if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", "boithoweb", 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                //return(1);
		pthread_exit((void *)1); /* exit with status */
        }

        //escaper queryet rikit
        mysql_real_escape_string(&demo_db,queryEscaped,queryNodeHeder.query,strlen(queryNodeHeder.query));

        sprintf(mysql_query, "select tittel,url,beskrivelse,betaler_sider.bruker_navn,betaler_keywords.betaler,betaler_keywords.kid,betaler_sider.id from betaler_keywords,betaler_sider where betaler_keywords.keyword ='%s' and betaler_keywords.betaler_side_id=betaler_sider.id order by betaler desc",queryEscaped);


        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
             	printf(mysql_error(&demo_db));
             	//return(1);
		pthread_exit((void *)1); /* exit with status */
        }
        #ifdef DEBUG
                printf("sending query to ppc db end\n");
        #endif

        /********************************************************************************************/


	SiderHeder.TotaltTreff = 0;
	int nrOfppcPages = 0;
	int nrOfBoithoAds = 0;
        //printer ut eventuelt ppc ord
        mysqlres=mysql_store_result(&demo_db); /* Download result from server */
        while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */
                        //printf("\t<beskrivelse>%s</beskrivelse>\n",mysqlrow[2]);
			//Sider[showabal].type = siderType_ppctop;

			strncpy(ppcPages[nrOfppcPages].title,mysqlrow[0],sizeof(ppcPages[nrOfppcPages].title));
			strncpy(ppcPages[nrOfppcPages].url,mysqlrow[1],sizeof(ppcPages[nrOfppcPages].url));
			strncpy(ppcPages[nrOfppcPages].uri,mysqlrow[1],sizeof(ppcPages[nrOfppcPages].uri));
			strncpy(ppcPages[nrOfppcPages].description,mysqlrow[2],sizeof(ppcPages[nrOfppcPages].description));
			strncpy(ppcPages[nrOfppcPages].user,mysqlrow[3],sizeof(ppcPages[nrOfppcPages].user));

			ppcPages[nrOfppcPages].thumbnail[0] = '\0';

			ppcPages[nrOfppcPages].bid = atof(mysqlrow[4]);
			ppcPages[nrOfppcPages].keyword_id = atoi(mysqlrow[5]);
			ppcPages[nrOfppcPages].DocID = strtoul(mysqlrow[6], (char **)NULL, 10);

			ppcPages[nrOfppcPages].allrank = 10000;

			#ifdef DEBUG
			printf("aa bid %f\n",ppcPages[nrOfppcPages].bid);
			printf("\tUrl: %s\n",ppcPages[nrOfppcPages].url);
	                printf("\tTitle: %s\n",ppcPages[nrOfppcPages].title);
			printf("keyword_id -%s-\n",mysqlrow[5]);
			#endif

			betaler_keywords_visninger[nrOfBoithoAds].kid = ppcPages[nrOfppcPages].keyword_id;
			betaler_keywords_visninger[nrOfBoithoAds].betaler_side_id = ppcPages[nrOfppcPages].DocID;
		++nrOfppcPages;
		++nrOfBoithoAds;
        }
	mysql_free_result(mysqlres);


	/*********************************/

	printf("contry: %s\n",queryNodeHeder.GeoIPcontry);

	if (strcmp(queryNodeHeder.GeoIPcontry,"NO") == 0) {
		strcpy(ppcprovider,"hent");
		//strcpy(ppcprovider,"revenuepilot");

	}
	else {
		//alle språk
		//strcpy(ppcprovider,"revenuepilot");
		//strcpy(ppcprovider,"searchboss");
	}
	strcpy(ppcprovider,"amazon");

	//temp: skrur av 3p xml feeds
	//getPpcAds(ppcprovider,ppcPages,&nrOfppcPages,&queryNodeHeder);

	//temp: Viser en mindre side da vi får problemer med siste?
	//nrOfppcPages--;

	showabal = 0;        
        for (i=0;i<nrOfppcPages;i++) {


		
		/*********************************************/
		//Sider[showabal].type = siderType_ppcside;
		#ifdef DEBUG
		printf("issue add. keyword_id %i\n",ppcPages[i].keyword_id);
		#endif
        	sprintf(mysql_query, "insert into issuedadds values(%s,'%s','%f','%s',%s,'%s','%s','%s','%s','%s','%s','%s','%i','%i')",
			"NULL",
			queryEscaped,
			ppcPages[i].bid,
			ppcPages[i].uri,
			"NOW()",
			0,
			ppcPages[i].user,
			queryNodeHeder.search_user,
			queryNodeHeder.userip,
			queryNodeHeder.HTTP_ACCEPT_LANGUAGE,
			queryNodeHeder.HTTP_USER_AGENT,
			queryNodeHeder.HTTP_REFERER,
			ppcPages[i].keyword_id,
			ppcPages[i].DocID
			);
        
		
		#ifdef DEBUG
		printf("ppc user %s\naffuser %s\n",Sider[i].user,queryNodeHeder.search_user);
		#endif
        	if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
        	     	printf("Cant insert into issuedadds: %s\nSql query vas %s\n",mysql_error(&demo_db),mysql_query);
        	     	//return(1);
			pthread_exit((void *)1); /* exit with status */
	        }

		addid = mysql_insert_id(&demo_db);

		#ifdef DEBUG
		printf("addid %u\n",addid);
		#endif
		//sprintf(ppcPages[showabal].uri,"http://search.boitho.com/cgi-bin/addout.cgi?addid=%u&addurl=%s",addid,ppcPages[showabal].url);
		sprintf(ppcPages[showabal].uri,"http://bbh-001.boitho.com/cgi-bin/addout.cgi?addid=%u&addurl=%s",addid,ppcPages[showabal].url);
		
		//strcpy(Sider[i].uri,buff);
		/*********************************************/

			if (strlen(ppcPages[i].title) == (sizeof(ppcPages[i].title) -1)) {
				//strcpy(Sider[showabal].title,"Title to long.");
				
				strncpy(Sider[showabal].title,ppcPages[i].title,sizeof(Sider[showabal].title) -3);
				strcat(Sider[showabal].title,"..");
			}
			else {
				strncpy(Sider[showabal].title,ppcPages[i].title,sizeof(Sider[showabal].title));
			}


			strncpy(Sider[showabal].description,ppcPages[i].description,sizeof(Sider[showabal].description));
                        
			strncpy(Sider[showabal].url,ppcPages[i].url,sizeof(Sider[showabal].url));
			strncpy(Sider[showabal].uri,ppcPages[i].uri,sizeof(Sider[showabal].uri));
			strncpy(Sider[showabal].user,ppcPages[i].user,sizeof(Sider[showabal].user));
                        
			strscpy(Sider[showabal].domain,ppcPages[i].domain,sizeof(Sider[showabal].domain));
			
			strscpy(Sider[showabal].thumbnale,ppcPages[i].thumbnail,sizeof(Sider[showabal].thumbnale));

			Sider[showabal].thumbnailwidth = atol(ppcPages[i].thumbnailwidth);
			Sider[showabal].thumbnailheight = atol(ppcPages[i].thumbnailheight);
			
			Sider[showabal].bid = ppcPages[i].bid;

			Sider[showabal].iindex.allrank = ppcPages[i].allrank;


			#ifdef DEBUG
                        printf("%s\t%s\t%f\n",Sider[showabal].url,Sider[showabal].title,ppcPages[i].bid);
              		#endif
			

		++showabal;

        }

	/*********************************/
	siderType_ppctopNr = 0;
	siderType_ppcsideNr = 0;

        for(i=0;i<showabal;i++) {

			#ifdef DEBUG
			printf("uri %s\n",Sider[i].uri);
			#endif

			Sider[i].DocumentIndex.crc32 = crc32boitho(Sider[i].description);
                        Sider[i].deletet = 0;





		//lager fin beskrivlse som slutter på .. isteden får bare et kappet ord, hvis beskrivlese er for lang                
                if (strlen(Sider[i].description) >= 250) {
                	//søker oss til siste space , eller ; og avslutter der
                        if ((strpointer = (char *)strrchr(Sider[i].description,' ')) != NULL) {
                        	strpointer[0] = '\0';
                        }
                        else if ((strpointer = (char *)strrchr(Sider[i].description,';')) != NULL) {
                        	++strpointer; //pekeren peker på semikolonet. SKal ha det med, så må legge il en
                                strpointer[0] = '\0';
                        }
                        strncat(Sider[i].description,"..",2);
            	}
               	
		//hiliter ordet
		sprintf(buff,"<b>%s</b>",queryNodeHeder.query);
		strcasesandr(Sider[i].description,sizeof(Sider[i].description),queryNodeHeder.query,buff);

		//bestemmer ppc type
		//Sider[showabal].type = siderType_ppcside
		//Sider[i].type = siderType_ppctop;
		if ((siderType_ppctopNr < 2) && (strcasestr(Sider[i].description,queryNodeHeder.query) != 0)) {
			Sider[i].type = siderType_ppctop;
			++siderType_ppctopNr;
		}
		else {
			Sider[i].type = siderType_ppcside;
			++siderType_ppcsideNr;
		}

	}	

	//legger datane in i mysql database.
        for(i=0;i<showabal;i++) {


	}


	gettimeofday(&globalend_time, NULL);
	SiderHeder.total_usecs = getTimeDifference(&globalstart_time,&globalend_time);

	SiderHeder.TotaltTreff = showabal;
	SiderHeder.showabal = showabal;
	SiderHeder.filtered = 0;
	SiderHeder.hiliteQuery[0] = '\0';
	sprintf(SiderHeder.servername,"adserver.boitho.com");

	//SiderHeder.queryTime = 0;

        if ((n=sendall(mysocfd,&SiderHeder, sizeof(SiderHeder))) != sizeof(SiderHeder)) {
                printf("send only %i of %i\n",n,sizeof(SiderHeder));
                perror("sendall SiderHeder");
        }

        for(i=0;i<SiderHeder.showabal;i++) {
        //for (i=0;i<queryNodeHeder.MaxsHits;i++) {
		#ifdef DEBUG
                       printf("sending %s, deletet %i\n",Sider[i].url,Sider[i].deletet);
                       printf("bb: -%s-\n",Sider[i].title);
                       printf("url: -%s-\n",Sider[i].url);
			
		#endif

                //if (!Sider[i].deletet) {
                        if ((n=sendall(mysocfd,&Sider[i], sizeof(struct SiderFormat))) != sizeof(struct SiderFormat)) {
                                printf("send only %i of %i\n",n,sizeof(struct SiderFormat));
                                perror("sendall");
                        }

                //}
        }

	//logger alle visningene vi har hatt på egen ppc ord
	for (i=0;i<nrOfBoithoAds;i++) {
		sprintf(mysql_query, "insert DELAYED into betaler_keywords_visninger values(NULL,'%i','%i',NOW())",betaler_keywords_visninger[i].kid,betaler_keywords_visninger[i].betaler_side_id);


	        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
	             	printf(mysql_error(&demo_db));
	             	//return(1);
			pthread_exit((void *)0); /* exit with status */
	        }
		
	}


        mysql_close(&demo_db);

        //close(mysocfd);


	free(Sider);

	close(mysocfd);

 	#ifdef WITH_THREAD
		my_thread_end(); // kalt mysql_thread_end() i mysql 5.0
            	pthread_exit((void *)0); /* exit with status */
        #endif

	printf("end\n");
       

	//return 0;
}
