/*
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <sys/file.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <netinet/in.h>
*/
    #include "../common/boithohome.h"
    #include "../common/timediff.h"   
    #include "../common/bstr.h"   
    #include "../common/daemon.h"   
    #include "../common/stdlib.h"

    #include <mysql_version.h>
    #include <stdarg.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <errno.h>
    #include <time.h>
    #include <errno.h>
    #include <locale.h>
    #include <fcntl.h>
    #include <zlib.h>
    #include <sys/file.h>
#include <err.h>
#include <assert.h>

    #include "library.h"
#include "../common/debug.h"

    #define MAXDATASIZE 100 // max number of bytes we can get at once 


    #define maxerrors 5
    #define maxerrorlen 201

#ifdef DEBUG
#define dprintf(str, args...) printf(str, ##args)
#else
#define dprintf(str, args...) 
#endif


void die_va(int errorcode,char query[], const char *fmt, va_list ap) {
	FILE *LOGFILE;

	
	char queryesc[MaxQueryLen*2+1];
	escapeHTML(queryesc, sizeof queryesc, query);

	printf("<search>\n");
	printf("<error>\n");
	printf("  <errorcode>%i</errorcode>\n",errorcode); 

	printf("  <errormessage>");
		vprintf(fmt,ap);
	printf("</errormessage>\n");
 
	printf("</error>\n");
	printf("<RESULT_INFO TOTAL=\"0\" QUERY=\"%s\" HILITE=\"\" TIME=\"0\" FILTERED=\"0\" SHOWABAL=\"0\" BOITHOHOME=\"%s\"/>\n",queryesc,bfile(""));
	printf("</search>\n");


	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");


        va_end(ap);

	exit(1);

}

void die(int errorcode,char query[] ,const char *fmt, ...) {

	va_list     ap;
	va_start(ap, fmt);

	die_va(errorcode,query,fmt, ap);
}

void dieLog(MYSQL *demo_db, struct QueryDataForamt *QueryData, int errorcode, char query[] ,const char *fmt, ...) {
	mysql_search_logg(demo_db,QueryData,NULL,0,NULL,0,NULL,0);

	va_list     ap;
        va_start(ap, fmt);

        die_va(errorcode,query,fmt, ap);
	
}

void dumpQueryDataForamt(struct QueryDataForamt *d) {
	warnx("=dump start ptr: %p size: %d", d, sizeof *d);
	warnx("query: %s", d->query);
	warnx("queryhtml: %s", d->queryhtml);
	warnx("userip: %s", d->userip);
	warnx("subname: %s", d->subname);
	warnx("MaxHits: %d", d->MaxsHits);
	warnx("start: %d", d->start);
	warnx("filterOn: %d", d->filterOn);
	warnx("outformat: %d", d->outformat);
	warnx("version: %f", d->version);
	warnx("GeoIPcontry: %s", d->GeoIPcontry);
	warnx("search_user: %s", d->search_user);
	warnx("HTTP_ACCEPT_LANGUAGE: %s", d->HTTP_ACCEPT_LANGUAGE);
	warnx("HTTP_USER_AGENT: %s", d->HTTP_USER_AGENT);
	warnx("HTTP_REFERER: %s", d->HTTP_REFERER);
	warnx("orederby: %s", d->orderby);
#ifdef BLACK_BOKS
	warnx("anonymous: %d", d->anonymous);
#endif
	warnx("tkey: %s", d->tkey);
	warnx("rankUrl: %s", d->rankUrl);
	warnx("navmenu: %s", d->navmenucfg);
	warnx("=dump end");
}


//legger tilen error til køen
void addError(struct errorhaFormat *errorha, int errorcode,char errormessage[]) {

		if ((*errorha).nr < maxerrors) {
			(*errorha).errorcode[(*errorha).nr] = errorcode;
			
			strscpy((*errorha).errormessage[(*errorha).nr],errormessage,maxerrorlen);

			++(*errorha).nr;
		}
		else {
			die(10,"","Too many errors occured.");
		}
}


int bsconnect (int *sockfd, char server[], int port, int timeoutInSec) {


	int returnstatus = 0;
        struct sockaddr_in their_addr; // connectors address information 
	struct timeval timeout;
	int nerror;
	struct hostent *he;
	
	dprintf("server: %s\n",server);

	if ((he = gethostbyname(server)) == NULL) {  // get the host info 
		perror("gethostbyname");
		die(6,"","Gethostbyname error.");
	}

	if (((*sockfd) = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		die(7,"","Socket error.");
	}
	fcntl((*sockfd), F_SETFL, O_NONBLOCK);

	their_addr.sin_family = AF_INET;    // host byte order 
	their_addr.sin_port = htons(port);  // short, network byte order 
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	/* Initialize the timeout data structure. */
	timeout.tv_sec = timeoutInSec;
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
		dprintf("somthing is wrong. Closeing socket\n");
		(*sockfd) = 0;
	}

	return returnstatus;
}

int bsread (int *sockfd,int datasize, char buff[], int maxSocketWait) {


	int dataReceived = 0;
	int y = 0;
        struct timespec time;
        int socketWait;
	int n;
	//int i;
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
				dprintf("EAGAIN. wait %i < %i\n",socketWait,maxSocketWait);
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
			dprintf("n==0. Peer has performed an orderly shutdown when trying to read\n");
#ifdef DEBUG
			perror("recv SiderHeder");
#endif
			//connected[i] = 0;
			returnstatus = 0;
			break;

		}
		else {
			dataReceived += n;
		}

		dprintf("bsread: reciving %i of %i. SocketWait %i\n",dataReceived,datasize,socketWait);

	}
	//vi timet ut socketetn. Setter den som ikke tilkoblet
	if (socketWait >= maxSocketWait) {
		dprintf("bsread: timed out\n");
		returnstatus = 0;
	}

	if (returnstatus == 0) {
		(*sockfd) = 0;
		dprintf("somthing is wrong. Nulling socket\n");
	}
	
	dprintf("Done geting data for this server. Did get %i of %i\n",dataReceived,datasize);

	return returnstatus;

}

int
bsQuery(int *sock, void *data, size_t data_size) {
	//printf("Sending %d\n", data_size);
	if (sock == 0)
		return 0;

	if (sendall(*sock, data, data_size) != data_size) {
		perror("sendall");
		*sock = 0;
		return 0;
	}

	return 1;
}

int bsMultiConnect(int *sockfd, int server_cnt, char **servers, int port, int offset) {
	int i;
	for (i = 0; i < server_cnt; i++) {
		dprintf("connecting to \"%s\" as sockfd nr %i\n",servers[i],i);

		if (!bsconnect(&sockfd[i + offset], servers[i], port, maxSocketWait_Connect)) {
			fprintf(stderr,"can't connect to server \"%s\"\n", servers[i]);
			sockfd[i] = 0;
			// Runarb 23 nov 2009: Skal vi virkelig returnere her. Kan vi ikke ha andre servere å koble til ?
			return 0;
		}
	}
	return 1;
}

void bsConnectAndQuery(int *sockfd,int server_cnt, char **servers, 
	struct queryNodeHederFormat *header, 
	struct subnamesFormat *colls, int colls_cnt,
	int offset, int port) {
	
	
	bsMultiConnect(sockfd, server_cnt, servers, port, offset);
	
	// send data.
	int i;
	for (i = 0; i < server_cnt; i++) {
		if (!sockfd[i + offset]) 
			continue;
		if (!bsQuery(&sockfd[i + offset], header, sizeof *header)
		 || !bsQuery(&sockfd[i + offset], &colls_cnt, sizeof colls_cnt)
		 || (colls_cnt > 0 && !bsQuery(&sockfd[i + offset], colls,
		 	sizeof(struct subnamesFormat) * colls_cnt))) {
			warnx("Can't send. Server %s:%i", servers[i], port);
		}
	}
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
void bdread(int sockfd[],int nrof,int datasize, void *result, int maxSocketWait) {

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
					dprintf("EAGAIN. wait %i < %i\n",readsdone,maxSocketWait);
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
				dprintf("n==0. Peer has performed an orderly shutdown when trying to read\n");
                                //connected[i] = 0;
				close(sockfd[i]);
				sockfd[i] = 0;


			}
			else {
				sendt[i] +=n;
				bytesleft[i] -=n;
			}

			dprintf("reciving %i of %i. readsdone %i\n",n,datasize,readsdone);
	}

}

void brGetPages(int *sockfd,int nrOfServers,struct SiderHederFormat *SiderHeder,struct SiderFormat *Sider, 
	int *pageNr,int alreadynr) {

	int i,n;
	int net_status;

	#ifdef DEBUG_TIME
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	#endif

	#ifdef DEBUG
	printf("brGetPages: alreadynr %i, *pageNr %i, nrOfServers %i\n",alreadynr,*pageNr,nrOfServers);
	#endif

	//sejjer om vi har fåt et midlertidig svar på at jobben har begynt
	/****************************************************************/
	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {

		if (sockfd[i] != 0) {

			//motter hedderen for svaret
			if (bsread (&sockfd[i],sizeof(net_status),(char *)&net_status,maxSocketWait_CanDo)) {

				if (net_status != net_CanDo) {
					dprintf("net_status wasen net_CanDo but %i\n",net_status);
					sockfd[i] = 0;
				}
			}
		}			
		
	}
	/****************************************************************/
	#ifdef DEBUG_TIME
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: brGetPages.jobstart pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef DEBUG_TIME
	gettimeofday(&start_time, NULL);
	#endif

	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {

		//inaliserer til 0 slik at vi ikke tror at vi har noe data fra servere som ikke svarte
		SiderHeder[i].showabal = 0;
	
		//motter hedderen for svaret
		if (sockfd[i] != 0) {

			if (!bsread (&sockfd[i],sizeof(struct SiderHederFormat),(char *)&SiderHeder[i],maxSocketWait_SiderHeder)) {
				dprintf("brGetPages: Failed to bsread SiderHeder");
				continue; // ingen vits i å fortsette å lese fra denne hvis vi ikke fik til å gjøre dette read kallet
			}

			#ifdef ATTRIBUTES
			if ((SiderHeder[i].navigation_xml = malloc( sizeof(char) * (SiderHeder[i].navigation_xml_len +1)) ) == NULL) {
				perror("Can't malloc data for navigation_xml");
				continue; // ingen vits å å fortsette mer
			}

			if (!bsread (&sockfd[i], SiderHeder[i].navigation_xml_len, SiderHeder[i].navigation_xml, maxSocketWait_SiderHeder)) {
				dprintf("brGetPages: Failed to bsread navigation_xml");
				continue; // ingen vits å å fortsette mer
		    	}

			SiderHeder[i].navigation_xml[SiderHeder[i].navigation_xml_len] = '\0';

			#ifdef DEBUG
				printf("\n\nnavigation_xml: %s\n\n", SiderHeder[i].navigation_xml);
			#endif

			#endif
		}
	}

	#ifdef DEBUG_TIME
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: brGetPages.reading heder pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	#ifdef DEBUG_TIME
	gettimeofday(&start_time, NULL);
	#endif


	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {


			if (sockfd[i] == 0) {
				dprintf("Server nr %i don't have a open socket.",i);
			}
			else {
				int j;

				dprintf("aa: i: %i. Server \"%s\" that has %i pages. Soctet %i\n",
					i,
					SiderHeder[i].servername,
					SiderHeder[i].showabal,
					sockfd[i]);
		


				#ifdef DEBUG
					printf("brGetPages: trying to read %i bytes from server %i\n",sizeof(struct SiderFormat) * SiderHeder[i].showabal,i);
					printf("have %i pages\n",SiderHeder[i].showabal);
				#endif

				for (j = 0; j < SiderHeder[i].showabal; j++) {
					if ((n=bsread(&sockfd[i],sizeof(struct SiderFormat), (char *)&Sider[(*pageNr)],maxSocketWait_SiderHeder))) {
						size_t len;
						int k;
						/* Get urls ... */
						Sider[*pageNr].urls = calloc(Sider[*pageNr].n_urls, sizeof(*(Sider->urls)));
						if (Sider[*pageNr].url == NULL)
							err(1, "calloc(urls)");

						for (k = 0; k < Sider[*pageNr].n_urls; k++) {

							bsread(&sockfd[i], sizeof(len), (char *)&len, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].url = malloc(len+1);
							bsread(&sockfd[i], len, Sider[*pageNr].urls[k].url, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].url[len] = '\0';

							bsread(&sockfd[i], sizeof(len), (char *)&len, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].uri = malloc(len+1);
							bsread(&sockfd[i], len, Sider[*pageNr].urls[k].uri, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].uri[len] = '\0';

							bsread(&sockfd[i], sizeof(len), (char *)&len, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].fulluri = malloc(len+1);
							bsread(&sockfd[i], len, Sider[*pageNr].urls[k].fulluri, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].fulluri[len] = '\0';

							#ifdef DEBUG
								printf("n_urls=%i\n", Sider[*pageNr].n_urls);
								printf("url=\"%s\", uri=\"%s\", fulluri=\"%s\"\n", Sider[*pageNr].urls[k].url, Sider[*pageNr].urls[k].uri, Sider[*pageNr].urls[k].fulluri);
							#endif

						}

						bsread(&sockfd[i], sizeof(len), (char *)&len, maxSocketWait_SiderHeder);
						Sider[*pageNr].attributelen = len;

						Sider[*pageNr].attributes = malloc(len+1);
						bsread(&sockfd[i], len, Sider[*pageNr].attributes, maxSocketWait_SiderHeder);
						Sider[*pageNr].attributes[len] = '\0';

						

						(*pageNr) += 1;
					}
				}

				#ifdef DEBUG
					printf("brGetPages: did read %i element\n",n);
				#endif
			}
	}


	#ifdef DEBUG_TIME
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: brGetPages.reading pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif
}


void bsConectAndQueryOneServer(char server[], int searchport, char query[], char subname[], int maxHits, int start, 
	struct SiderFormat **Sider, int *pageNr) {

	errx(1, "bsConectAndQueryOneServer() is broken. %s, line %d", __FILE__, __LINE__);


	int i;

        int nrOfServers;
        int nrOfPiServers;
        int nrOfAddServers;
	int SiderSize;
        int sockfd[maxServers];
        //int addsockfd[maxServers];

	struct QueryDataForamt QueryData;
	struct queryNodeHederFormat queryNodeHeder;

                char *servers[] = { server };
                char *addservers[] = { };
                char *piservers[] = { };


                nrOfServers = (sizeof(servers) / sizeof(char *));
                nrOfPiServers = (sizeof(piservers) / sizeof(char *));
                nrOfAddServers = (sizeof(addservers) / sizeof(char *));

        memset(&QueryData,'\0',sizeof(QueryData));
        memset(&queryNodeHeder,'\0',sizeof(queryNodeHeder));


        QueryData.MaxsHits = maxHits;
        QueryData.start = start;
        QueryData.filterOn = 1;
        QueryData.HTTP_ACCEPT_LANGUAGE[0] = '\0';
        QueryData.HTTP_USER_AGENT[0] = '\0';
        QueryData.HTTP_REFERER[0] = '\0';
        QueryData.AmazonAssociateTag[0] = '\0';
        QueryData.AmazonSubscriptionId[0] = '\0';
        QueryData.orderby[0] = '\0';

	strscpy(QueryData.query,query,sizeof(QueryData.query));
	strscpy(QueryData.subname,subname,sizeof(QueryData.subname));


	struct SiderHederFormat *SiderHeder = malloc(sizeof(struct SiderHederFormat) * maxServers);


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


        queryNodeHeder.start = 0;

        //queryNodeHeder.MaxsHits = QueryData.MaxsHits;
        queryNodeHeder.MaxsHits = QueryData.MaxsHits * QueryData.start;



        //på første side kan vi la være og hente alle treff siden dataene er fordelt, men for de neste
        //sidene trenger vi de.
        if (QueryData.start == 1) {
                if (nrOfServers >= 3) {
                        queryNodeHeder.MaxsHits = (queryNodeHeder.MaxsHits / 2); // datane er fordelt, så hver server trenger ikke å generere$
                }
        }
        queryNodeHeder.filterOn = QueryData.filterOn;



	SiderSize = queryNodeHeder.MaxsHits * maxServers * sizeof(struct SiderFormat);

        if ((*Sider = malloc(SiderSize)) == NULL) {
		perror("malloc Sider");
		exit(1);
	}

	printf("SiderSize: %i\n",SiderSize);

        //inaliserer side arrayen
        for(i=0;i<(nrOfServers + nrOfPiServers) * queryNodeHeder.MaxsHits;i++) {
                (*Sider)[i].iindex.allrank = 0;
                (*Sider)[i].deletet = 1;

        }


	printf("trying to get %i hist from server \"%s\", starting on %i\n",QueryData.MaxsHits,server,QueryData.start);

	//TODO: Unbreak querying
	//bsConectAndQuery(sockfd,nrOfServers,servers,&queryNodeHeder,nrOfPiServers,searchport);

	*pageNr = 0;
	brGetPages(sockfd,nrOfServers,SiderHeder,*Sider,pageNr,0);


	free(SiderHeder);
}


#ifdef WITH_CASHE

/* Probably very weak */
unsigned int
cache_hash(char *query, int start, char *country)
{
	unsigned int hash = 0xf23c203;
	char *p;

	hash *= start;
	for(p = query; *p; p++)
		hash += *p * 38;
	for(p = country; *p; p++)
		hash += *p * 39;
	
	return hash;
}

int
cache_collhash(struct subnamesFormat *collections, int num_colls) {

	int i;
	int ret = 0;
	char *p;

	for (i=0;i<num_colls;i++) {
		for(p = collections[i].subname; *p; p++)
			ret += *p;
	}


	return ret;
}

char *
cache_path(char *path, size_t len, enum cache_type type, char *query, int start, char *country, int anonymous, char search_user[], struct subnamesFormat *collections, int num_colls)
{
	char tmppath[PATH_MAX];
	char modquery[PATH_MAX];
	unsigned int hash;
	char *p;
	char *cache;

	switch (type) {
	case CACHE_PREQUERY:
		cache = "var/cache/prequery_v_" CACHE_STRUCT_VERSION;
		break;
	case CACHE_SEARCH:
		cache = "var/cache/search_v_" CACHE_STRUCT_VERSION;
		break;
	}

	/* XXX: Base 64 encode the query */
	strcpy(modquery, query);
	for (p = modquery; *p; p++) {
		if (*p == '/')
			*p = '-';
		else if (*p == ' ')
			*p = '#';
	}
	hash = cache_hash(modquery, start, country);
	p = (char *)&hash;
	strncpy(path, bfile(cache), len);
	mkdir(path, 0755);
	snprintf(tmppath, sizeof(tmppath), "/%x%x", *p & 0xF, (*p >> 4) & 0xF);
	strncat(path, tmppath, len);
	mkdir(path, 0755);
	p++;
	snprintf(tmppath, sizeof(tmppath), "/%x%x", *p & 0xF, (*p >> 4) & 0xF);
	strncat(path, tmppath, len);
	mkdir(path, 0755);
	snprintf(tmppath, sizeof(tmppath), "/%s.%d.%s.%s.%i", modquery, start, country, anonymous ? "anonymous":search_user, cache_collhash(collections, num_colls));
	strncat(path, tmppath, len);

	return path;
}


int
cache_read(char *path, int *page_nr, struct SiderHederFormat *final_sider, struct SiderHederFormat *sider_header,
           size_t sider_header_len, struct SiderFormat *sider, int cachetimeout, size_t max_sizer)
{
	gzFile *cache;
	int fd, i, ret, k, len;
	struct stat st;

	if ((fd = open(path, O_RDONLY)) == -1) {
		perror(path);
		return 0;
	}
	flock(fd, LOCK_SH);

	/* Invalidate cache ? */
	if (cachetimeout > 0 && fstat(fd, &st) != -1) {
		time_t now;

		now = time(NULL);

		if (now - st.st_mtime > cachetimeout) {
			fprintf(stderr, "Cache too old, invalidating\n");
			unlink(path);
			flock(fd, LOCK_UN);
			close(fd);
			return 0;
		}
	}	

	if ((cache = gzdopen(fd, "r")) == NULL) {
		fprintf(stderr, "can't gzopen().\n");
		perror("gzopen");
		flock(fd, LOCK_UN);
		close(fd);
		return 0;
	}

	ret = 1;
	if (gzread(cache, page_nr, sizeof(*page_nr)) != sizeof(*page_nr)) {
		perror("gzread(page_nr)");
		goto err;
	}

	if (gzread(cache, final_sider, sizeof(*final_sider)) != sizeof(*final_sider)) {
		perror("gzread(final_sider)");
		goto err;
	}

	if (gzread(cache, sider_header, sizeof(*sider_header)*sider_header_len) != sizeof(*sider_header)*sider_header_len) {
		perror("gzread(final_sider)");
		goto err;
	}


	#ifdef ATTRIBUTES
		for(i=0; i<1 ; i++) {
			
			if ((sider_header[i].navigation_xml = malloc(sider_header[i].navigation_xml_len +1)) == NULL) {
				perror("malloc navigation_xml");
				goto err;
			}

		        if (gzread(cache, sider_header[i].navigation_xml, sider_header[i].navigation_xml_len) != sider_header[i].navigation_xml_len) {
		                perror("gzwrite(final_sider)");
		                goto err;
		        }
			sider_header[i].navigation_xml[sider_header[i].navigation_xml_len] = '\0';
		}
	#endif


	//ser ut til at vi av og til kan ha flere sider en vi har plass til i bufferen
	if (*page_nr > max_sizer) {
		fprintf(stderr, "Bug?: cache_read: have more pages in cache then space in buffer! nr of pages was %i\n",*page_nr);
		*page_nr = max_sizer;
	}
	//fprintf(stderr, "Got %d cached pages\n", *page_nr);
	for (i = 0; i < *page_nr; i++) {
		if (gzread(cache, &sider[i], sizeof(*sider)) != sizeof(*sider)) {
			perror("Unable to read cache");
			goto err;
		}

		#ifdef ATTRIBUTES
		if ((sider[i].attributes = malloc(sider[i].attributelen +1)) == NULL) {
			perror("Mallco attributes");
			goto err;
		}

		if (gzread(cache, sider[i].attributes, sider[i].attributelen) != (sider[i].attributelen)) {
			perror("Unable to write cache attributelen");
			goto err;
		}

		sider[i].attributes[sider[i].attributelen] = '\0';


                /* write urls ... */
                sider[i].urls = calloc(sider[i].n_urls, sizeof(*(sider->urls)));
                if (sider[i].url == NULL)
                        err(1, "calloc(urls)");

                for (k = 0; k < sider[i].n_urls; k++) {



			if (gzread(cache, &len, sizeof(len)) != (sizeof(len))) {
				perror("Unable to read url len");
        			goto err;
                	}
			sider[i].urls[k].url = malloc(len+1);
			if (gzread(cache, sider[i].urls[k].url, len) != (len)) {
				perror("Unable to read url len");
        			goto err;
                	}
			sider[i].urls[k].url[len] = '\0';


			if (gzread(cache, &len, sizeof(len)) != (sizeof(len))) {
				perror("Unable to read uri len");
        			goto err;
                	}
			sider[i].urls[k].uri = malloc(len+1);
			if (gzread(cache, sider[i].urls[k].uri, len) != (len)) {
				perror("Unable to read uri len");
        			goto err;
                	}
			sider[i].urls[k].uri[len] = '\0';


			if (gzread(cache, &len, sizeof(len)) != (sizeof(len))) {
				perror("Unable to read fulluri len");
        			goto err;
                	}
			sider[i].urls[k].fulluri = malloc(len+1);
			if (gzread(cache, sider[i].urls[k].fulluri, len) != (len)) {
				perror("Unable to read fulluri len");
        			goto err;
                	}
			sider[i].urls[k].fulluri[len] = '\0';

			#ifdef DEBUG
                        printf("n_urls=%i\n", sider[i].n_urls);
                        printf("url=\"%s\", uri=\"%s\", fulluri=\"%s\"\n", sider[i].urls[k].url, sider[i].urls[k].uri, sider[i].urls[k].fulluri);
			#endif				

		}



		#endif

		//fprintf(stderr, "cache: read: %s\n", sider[i].uri);
	}

	goto out;
 err:
	//Runarb: 20 may 2008: hvorfor ble denne satt til 1 hvis den returnerte ok? Da returnerer den altid 1, selv på feil som kaller goto  err. 
	ret = 0;
 out:
	gzclose(cache);
	//Runarb: 20 may 2008: LOCK_UN før close er vel ikke nødvendig for read, da close låser opp.  
	//flock(fd, LOCK_UN);
	close(fd);
	return ret;
}

int
cache_write(char *path, int *page_nr, struct SiderHederFormat *final_sider, struct SiderHederFormat *sider_header,
            size_t sider_header_len, struct SiderFormat *sider, size_t sider_len)
{
	gzFile *cache;
	int fd, i, ret, k, len;

	//temp jayde 
	//final_sider->TotaltTreff = 20;
	//final_sider->showabal = 20;
	//sider_len = 20;
	//*page_nr = 20;

	if ((fd = open(path, O_CREAT|O_WRONLY|O_EXCL, 0644)) == -1) {
		fprintf(stderr,"cache_write: can't open cache file\n");
		perror(path);
		return 0;
	}
	flock(fd, LOCK_EX);
	if ((cache = gzdopen(fd, "w")) == NULL) {
		fprintf(stderr,"cache_write: can't gzdopen\n");
		return 0;
	}

	// Debug: Disable compresion
	//if (gzsetparams(cache,Z_NO_COMPRESSION,Z_DEFAULT_STRATEGY) !=  Z_OK) {
	//	fprintf(stderr,"Cant disable commpresion\n");
	//	exit(-1);
	//}

	ret = 1;
	if (gzwrite(cache, page_nr, sizeof(*page_nr)) != sizeof(*page_nr)) {
		perror("gzwrite(page_nr)");
		goto err;
	}

	if (gzwrite(cache, final_sider, sizeof(*final_sider)) != sizeof(*final_sider)) {
		perror("gzwrite(final_sider)");
		goto err;
	}

	if (gzwrite(cache, sider_header, sizeof(*sider_header)*sider_header_len) != sizeof(*sider_header)*sider_header_len) {
		perror("gzwrite(final_sider)");
		goto err;
	}

	#ifdef ATTRIBUTES
		// wi ownly have one server for no
		for(i=0; i<1 ; i++) {
	
		        if (gzwrite(cache, sider_header[i].navigation_xml, sider_header[i].navigation_xml_len) != sider_header[i].navigation_xml_len) {
		                perror("gzwrite(navigation_xml)");
		                goto err;
		        }

		}
	#endif


	for (i = 0; i < sider_len; i++) {
		if (gzwrite(cache, &sider[i], sizeof(*sider)) != sizeof(*sider)) {
			perror("Unable to write cache");
			goto err;
		}

		#ifdef ATTRIBUTES
		if (gzwrite(cache, sider[i].attributes, sider[i].attributelen) != (sider[i].attributelen)) {
			perror("Unable to write cache attributelen");
			goto err;
		}
            

                for (k = 0; k < sider[i].n_urls; k++) {

			len = strlen(sider[i].urls[k].url);
			if (gzwrite(cache, &len, sizeof(len)) != (sizeof(len))) {
				perror("Unable to read url len");
        			goto err;
                	}
			if (gzwrite(cache, sider[i].urls[k].url, len) != (len)) {
				perror("Unable to read url len");
        			goto err;
                	}


			len = strlen(sider[i].urls[k].uri);
			if (gzwrite(cache, &len, sizeof(len)) != (sizeof(len))) {
				perror("Unable to read uri len");
        			goto err;
                	}
			if (gzwrite(cache, sider[i].urls[k].uri, len) != (len)) {
				perror("Unable to read uri len");
        			goto err;
                	}


			len = strlen(sider[i].urls[k].fulluri);
			if (gzwrite(cache, &len, sizeof(len)) != (sizeof(len))) {
				perror("Unable to read fulluri len");
        			goto err;
                	}
			if (gzwrite(cache, sider[i].urls[k].fulluri, len) != (len)) {
				perror("Unable to read fulluri len");
        			goto err;
                	}
							

		}



		#endif


	}
	

	goto out;
 err:
	ret = 0;
	unlink(path);
 out:
	gzclose(cache);
	flock(fd, LOCK_UN);
	close(fd);
	return ret;
}

#endif


void mysql_search_logg(MYSQL *demo_db, struct QueryDataForamt *QueryData, 
	struct SiderHederFormat *FinalSiderHeder, int totlaAds, 
	struct queryNodeHederFormat *queryNodeHeder, int nrOfServers, struct SiderFormat *Sider, 
	int nrOfPiServers) {
	/********************************************************************************************/
	//mysql logging
	/********************************************************************************************/

#ifndef MYSQL_VERSION_ID

	#error "MYSQL_VERSION_ID fra mysql_version.h er ikke definert"

#elif MYSQL_VERSION_ID==50045


	#ifndef NO_LOGING
		MYSQL_STMT *logstmt, *pilogstmt;
		my_ulonglong  affected_rows;
		char query [2048];
		int x, i;



		MYSQL_BIND bind[12];
		unsigned long len[12];
		memset(bind, 0, sizeof(bind));
		logstmt = mysql_stmt_init(demo_db);
		pilogstmt = mysql_stmt_init(demo_db);

		if ((logstmt==NULL) || (pilogstmt==NULL)) {
			fprintf(stderr, "out of memory. Cant Create logstmt or pilogstmt");
		}

		sprintf(query,"INSERT DELAYED INTO search_logg (tid,query,search_bruker,treff,search_tid,ip_adresse,betaler_keywords_treff,HTTP_ACCEPT_LANGUAGE,HTTP_USER_AGENT,HTTP_REFERER,GeoIPLang,side) VALUES(NOW(),?,?,?,?,?,?,?,?,?,?,?)");

		if (mysql_stmt_prepare(logstmt, query, strlen(query)) != 0) {
			fprintf(stderr, " mysql_stmt_prepare(), INSERT INTO search_logg failed\n");
			fprintf(stderr, " Error: \"%s\"\n", mysql_stmt_error(logstmt));			
			return;
		}

		bind[0].buffer_type = MYSQL_TYPE_STRING; // query
		bind[0].buffer = QueryData->query;	// Ax: Max lengde i databasen er 30 tegn. Lage en nice-write?
		len[0] = strlen(QueryData->query);
		bind[0].length = &len[0];

		bind[1].buffer_type = MYSQL_TYPE_STRING; // user
		bind[1].buffer = QueryData->search_user;
		len[1] = strlen(QueryData->search_user);
		bind[1].length = &len[1];

		if (FinalSiderHeder==NULL) {

			bind[2].buffer_type = MYSQL_TYPE_NULL; // treff
			bind[2].buffer = NULL;

			bind[3].buffer_type = MYSQL_TYPE_NULL; // sÃketid
			bind[3].buffer = NULL;
		}
		else {
			bind[2].buffer_type = MYSQL_TYPE_LONG; // treff
			bind[2].buffer = &FinalSiderHeder->TotaltTreff;

			bind[3].buffer_type = MYSQL_TYPE_DOUBLE; // sÃketid
			bind[3].buffer = &FinalSiderHeder->total_usecs;
		}
		bind[4].buffer_type = MYSQL_TYPE_STRING; // ip
		bind[4].buffer = QueryData->userip;
		len[4] = strlen(QueryData->userip);
		bind[4].length = &len[4];

		bind[5].buffer_type = MYSQL_TYPE_LONG; // betaler
		bind[5].buffer = &totlaAds;
			
		bind[6].buffer_type = MYSQL_TYPE_STRING; // http lang
		bind[6].buffer = QueryData->HTTP_ACCEPT_LANGUAGE;
		len[6] = strlen(QueryData->HTTP_ACCEPT_LANGUAGE);
		bind[6].length = &len[6];

		bind[7].buffer_type = MYSQL_TYPE_STRING; // http user agent
		bind[7].buffer = QueryData->HTTP_USER_AGENT;
		len[7] = strlen(QueryData->HTTP_USER_AGENT);
		bind[7].length = &len[7];

		bind[8].buffer_type = MYSQL_TYPE_STRING; // http referer
		bind[8].buffer = QueryData->HTTP_REFERER;
		len[8] = strlen(QueryData->HTTP_REFERER);
		bind[8].length = &len[8];

		bind[9].buffer_type = MYSQL_TYPE_STRING; // geoip
		bind[9].buffer = QueryData->GeoIPcontry;
		len[9] = strlen(QueryData->GeoIPcontry);
		bind[9].length = &len[9];

		bind[10].buffer_type = MYSQL_TYPE_LONG; // side
		bind[10].buffer = &QueryData->start;


		mysql_stmt_bind_param(logstmt, bind);

		mysql_stmt_execute(logstmt);
		mysql_stmt_close(logstmt);


		/************************************************************************************************
		Logging av Paid Inclusion til sql db.
		************************************************************************************************/
		#ifndef BLACK_BOKS
			if (Sider != NULL) {
				sprintf(query,"insert into pi_search_logg (tid,query,treff,search_tid,ip_adresse,spot,piDocID ) \
					select NOW(),?,?,?,?,?,id from pi_sider where WWWDocID=? ");
	

				if (mysql_stmt_prepare(pilogstmt, query, strlen(query)) != 0) {
					fprintf(stderr, " mysql_stmt_prepare(), INSERT into pi_search_logg failed\n");
					fprintf(stderr, " %s\n", mysql_stmt_error(pilogstmt));
					return;
				}

				i = QueryData->MaxsHits * (QueryData->start -1);
				x = i;

				while ((x<(QueryData->MaxsHits * QueryData->start)) && (x<FinalSiderHeder->showabal) && (i < (queryNodeHeder->MaxsHits * (nrOfServers + nrOfPiServers)))) {

					if (!Sider[i].deletet) {
						dprintf("pi analyse. Subname \"%s\", pi \"%i\"\n",Sider[i].subname.subname, (int)Sider[i].subname.config.isPaidInclusion);

						if (Sider[i].subname.config.isPaidInclusion) {
							unsigned int spot;

							spot = x + 1;		

							memset(bind, 0, sizeof(bind));
							memset(len, 0, sizeof(len)); // må vi ha denne?

							bind[0].buffer_type = MYSQL_TYPE_STRING; // query
							bind[0].buffer = QueryData->query;
							len[0] = strlen(QueryData->query);
							bind[0].length = &len[0];
							
							bind[1].buffer_type = MYSQL_TYPE_LONG; // treff
							bind[1].buffer = &FinalSiderHeder->TotaltTreff;
							
							bind[2].buffer_type = MYSQL_TYPE_DOUBLE; // sÃketid
							bind[2].buffer = &FinalSiderHeder->total_usecs;
							
							bind[3].buffer_type = MYSQL_TYPE_STRING; // ip
							bind[3].buffer = QueryData->userip;
							len[3] = strlen(QueryData->userip);
							bind[3].length = &len[3];

							bind[4].buffer_type = MYSQL_TYPE_LONG ; // spot 
							bind[4].buffer = &spot;
							bind[4].is_unsigned = 1; 
			
							bind[5].buffer_type = MYSQL_TYPE_LONG; // piDocID
							bind[5].buffer = &Sider[i].iindex.DocID;
							bind[5].is_unsigned = 1; 


							if (mysql_stmt_bind_param(pilogstmt, bind) != 0) {
								fprintf(stderr, " mysql_stmt_bind_param() failed\n");
								fprintf(stderr, " %s\n", mysql_stmt_error(pilogstmt));
							}

							if (mysql_stmt_execute(pilogstmt) != 0) {
								fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
								fprintf(stderr, " %s\n", mysql_stmt_error(pilogstmt));
							}

							/* Get the total number of affected rows */
							affected_rows= mysql_stmt_affected_rows(pilogstmt);

							if (affected_rows != 1) /* validate affected rows */
							{
								fprintf(stderr, " invalid affected rows by MySQL\n");
								fprintf(stderr, " total affected rows(insert 1): %lu\n", (unsigned long) affected_rows);
								break;
							}
						}
						else {
							//dprintf("is NOT pi! :(\n");
						}
					}
					//teller bare normale sider (hva med Paid Inclusion ?)
					// Denne skal vel vaere innenfor !deletet?
					if (Sider[i].type == siderType_normal) {
						++x;
					}

					++i;
				}

				mysql_stmt_close(pilogstmt);
			}
		#endif
		/************************************************************************************************/


	#endif


#elif MYSQL_VERSION_ID==32358


	#ifndef NO_LOGING
		char queryEscaped[MaxQueryLen*2+1];
		char query [2048];
		int x, i;

		char TotaltTreff[10], total_usecs[23];
		
		if (FinalSiderHeder==NULL) {
			strcpy(TotaltTreff,"NULL");
			strcpy(total_usecs,"NULL");
		}
		else {
			snprintf(TotaltTreff,sizeof(TotaltTreff),"\"%i\"",FinalSiderHeder->TotaltTreff);
			snprintf(total_usecs,sizeof(total_usecs),"\"%f\"",FinalSiderHeder->total_usecs);
		}

		//escaper queryet rikit
		mysql_real_escape_string(demo_db,queryEscaped,QueryData->query,strlen(QueryData->query));

		// Magnus: Kolonnene 'spot' og 'piDocID' finnes ikke. Har fjernet dem.
		//logger til mysql
		sprintf(query,"insert DELAYED into search_logg (id,tid,query,search_bruker,treff,search_tid,ip_adresse,betaler_keywords_treff,\
			HTTP_ACCEPT_LANGUAGE,HTTP_USER_AGENT,HTTP_REFERER,GeoIPLang,side) \
			values(NULL,NOW(),\"%s\",\"%s\",%s,%s,\"%s\",\"%i\",\"%s\",\"%s\",\"%s\",\"%s\",\"%i\")",
			queryEscaped,
			QueryData->search_user,
			TotaltTreff,
			total_usecs,
			QueryData->userip,
			totlaAds,
			QueryData->HTTP_ACCEPT_LANGUAGE,
			QueryData->HTTP_USER_AGENT,
			QueryData->HTTP_REFERER,
			QueryData->GeoIPcontry,
			QueryData->start
		);


		mysql_real_query(demo_db, query, strlen(query));


		/************************************************************************************************
		Logging av Paid Inclusion til sql db.
		************************************************************************************************/
		#ifndef BLACK_BOKS

			if (Sider != NULL) {
			
				x = 0;
				i = 0;			
			
				while ((x<FinalSiderHeder->showabal) && (i < (queryNodeHeder->MaxsHits * (nrOfServers + nrOfPiServers)))) {
				
					if (!Sider[i].deletet) {
						#ifdef DEBUG
						printf("pi analyse. Subname \"%s\", pi \"%i\"\n",Sider[i].subname.subname, (int)Sider[i].subname.config.isPaidInclusion);
						#endif

						if (Sider[i].subname.config.isPaidInclusion) {
						

							strscpy(query,Sider[i].subname.config.sqlImpressionsLogQuery,sizeof(query));
							strsandr(query,"$DocID",utoa(Sider[i].iindex.DocID));

							strsandr(query,"$query",queryEscaped);
							strsandr(query,"$hits",bitoa(FinalSiderHeder->TotaltTreff) );
							strsandr(query,"$time",ftoa(FinalSiderHeder->total_usecs));
							strsandr(query,"$ipadress",QueryData->userip);
							strsandr(query,"$spot",bitoa(x + (QueryData->start * queryNodeHeder->MaxsHits)));

							#ifdef DEBUG
							printf("query \"%s\"\n",query);
							#endif

							mysql_real_query(demo_db, query, strlen(query));
	
						}
						else {
							#ifdef DEBUG
							printf("is NOT pi! :(\n");
							#endif
						}
					}
					//teller bare normale sider (hva med Paid Inclusion ?)
					if (Sider[i].type == siderType_normal) {
						++x;
					}
	
					++i;
				}
			}
		#endif
		/************************************************************************************************/

	#endif

#else
	#error "Ukjent mysql versjon i MYSQL_VERSION_ID fra mysql_version.h. Må være 32358 eller 50045."
#endif 

	/********************************************************************************************/


}
