/*
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
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



void die(int errorcode,char query[] ,const char *fmt, ...) {
	FILE *LOGFILE;

	va_list     ap;
	va_start(ap, fmt);

	printf("<search>\n");
	printf("<error>\n");
	printf("  <errorcode>%i</errorcode>\n",errorcode); 

	printf("  <errormessage>");
		vprintf(fmt,ap);
	printf("</errormessage>\n");
 
	printf("</error>\n");
	printf("<RESULT_INFO TOTAL=\"0\" QUERY=\"%s\" HILITE=\"\" TIME=\"0\" FILTERED=\"0\" SHOWABAL=\"0\" BOITHOHOME=\"%s\"/>\n",query,bfile(""));
	printf("</search>\n");


        //ToDo: må ha låsing her
        if ((LOGFILE = fopen(QUERY_LOG_FILE,"a")) == NULL) {
                perror(QUERY_LOG_FILE);
        }
        else {
		
		flock(fileno(LOGFILE),LOCK_EX);
                //fprintf(LOGFILE,"error: %i %s\n",errorcode,errormessage);
		vfprintf(LOGFILE,fmt,ap);
		fprintf(LOGFILE,"\n");
                fclose(LOGFILE);
        }

        va_end(ap);

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
			die(10,"","to many errors occurd");
		}
}


int bsconnect (int *sockfd, char server[], int port) {


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
		dprintf("somthing is wrong. Closeing socket\n");
		(*sockfd) = 0;
	}

	return returnstatus;
}

int bsread (int *sockfd,int datasize, char buff[], int maxSocketWait) {


	int dataReceived = 0;
	int y = 0;
//      struct timeval timeout;
//      struct timeval time;
        //struct timespec timeout;
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
		dprintf("timed out\n");
		//connected[i] = 0;
		returnstatus = 0;
	}

	if (returnstatus == 0) {
		(*sockfd) = 0;
		dprintf("somthing is wrong. Closeing socket\n");
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
		dprintf("connecting to \"%s\" as sockfd nr %i\n",servers[i],(i +alreadynr));

		if (bsconnect(&sockfd[i + offset], servers[i], port)) {
			dprintf("can connect\n");
		}
		else {
			dprintf("can NOT connect\n");
			sockfd[i] = 0;
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
		 || !bsQuery(&sockfd[i + offset], colls, 
		 	sizeof(struct subnamesFormat) * colls_cnt)) {
			warnx("Can't send. Server %s:%i", servers[i], port);
		}
	}
}

/*void bsConectAndQuery(int *sockfd,int nrOfServers, char *servers[],struct queryNodeHederFormat *queryNodeHeder,int alreadynr, int port) {

	int i;
#ifdef DEBUG
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
#endif
	
	bsMultiConnect(sockfd, nrOfServers, servers, port, alreadynr);
		
	//sender ut forespørsel
	for (i=0;i<nrOfServers;i++) {
		//sender forespørsel

		if (sockfd[i+alreadynr] != 0) {
			if (!bsQuery(&sockfd[i+alreadynr], queryNodeHeder, sizeof *queryNodeHeder))
				fprintf(stderr,"Error: Can't connect. Server %s:%i\n",servers[i], port);
		}
		dprintf("sending of queryNodeHeder end\n");
	}

#ifdef DEBUG
	gettimeofday(&end_time, NULL);

	printf("Time debug: bsConectAndQuery %f\n",getTimeDifference(&start_time,&end_time));

#endif
}*/

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

	#ifdef DEBUG
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
	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: brGetPages.jobstart pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	/****************************************************************/

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif

	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {

		//inaliserer til 0 slik at vi ikke tror at vi har noe data fra servere som ikke svarte
		SiderHeder[i].showabal = 0;
	
		//motter hedderen for svaret
		if (sockfd[i] != 0) {

			if (bsread (&sockfd[i],sizeof(struct SiderHederFormat),(char *)&SiderHeder[i],maxSocketWait_SiderHeder)) {

			}

			#ifdef ATTRIBUTES
			SiderHeder[i].navigation_xml = malloc(sizeof(char) * (SiderHeder[i].navigation_xml_len +1));
			if (bsread (&sockfd[i], SiderHeder[i].navigation_xml_len, SiderHeder[i].navigation_xml, maxSocketWait_SiderHeder))
			    {
			    }
			SiderHeder[i].navigation_xml[SiderHeder[i].navigation_xml_len] = '\0';
			#endif
		}
	}

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: brGetPages.reading heder pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	//int bdread(int sockfd[],int nrof,int datasize, void *result, int maxSocketWait)

//	bdread(sockfd,nrOfServers,sizeof(struct SiderHederFormat),SiderHeder, maxSocketWait_SiderHeder);

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif


	//(*pageNr) = 0;
	for (i=alreadynr;i<nrOfServers+alreadynr;i++) {

			dprintf("aa: i: %i. Server \"%s\" that has %i pages. Soctet %i\n",
				i,
				SiderHeder[i].servername,
				SiderHeder[i].showabal,
				sockfd[i]);
		

			if (sockfd[i] != 0) {
				int j;

				/*
				for(y=0;y<SiderHeder[i].showabal;y++) {

					if (bsread (&sockfd[i],sizeof(struct SiderFormat),(char *)&Sider[(*pageNr)],maxSocketWait_SiderHeder)) {
						(*pageNr)++;
					}
				}
				*/

				#ifdef DEBUG
					printf("brGetPages: trying to read %i bytes from server %i\n",sizeof(struct SiderFormat) * SiderHeder[i].showabal,i);
					printf("have %i pages\n",SiderHeder[i].showabal);
				#endif

				for (j = 0; j < SiderHeder[i].showabal; j++) {
					if ((n=bsread(&sockfd[i],sizeof(struct SiderFormat), &Sider[(*pageNr)],maxSocketWait_SiderHeder))) {
						size_t len;
						int k;
						/* Get urls ... */
						Sider[*pageNr].urls = calloc(Sider[*pageNr].n_urls, sizeof(*(Sider->urls)));
						if (Sider[*pageNr].url == NULL)
							err(1, "calloc(urls)");

						for (k = 0; k < Sider[*pageNr].n_urls; k++) {

							bsread(&sockfd[i], sizeof(len), &len, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].url = malloc(len+1);
							bsread(&sockfd[i], len, Sider[*pageNr].urls[k].url, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].url[len] = '\0';

							bsread(&sockfd[i], sizeof(len), &len, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].uri = malloc(len+1);
							bsread(&sockfd[i], len, Sider[*pageNr].urls[k].uri, maxSocketWait_SiderHeder);
							Sider[*pageNr].urls[k].uri[len] = '\0';

							//bsread(&sockfd[i], 64, Sider[*pageNr].urls[k].subname, maxSocketWait_SiderHeder);

						}

						bsread(&sockfd[i], sizeof(len), &len, maxSocketWait_SiderHeder);
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


	#ifdef DEBUG
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


char *
cache_path(char *path, size_t len, enum cache_type type, char *query, int start, char *country)
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
	snprintf(tmppath, sizeof(tmppath), "/%s.%d.%s", modquery, start, country);
	strncat(path, tmppath, len);

	return path;
}


int
cache_read(char *path, int *page_nr, struct SiderHederFormat *final_sider, struct SiderHederFormat *sider_header,
           size_t sider_header_len, struct SiderFormat *sider, int cachetimeout, size_t max_sizer)
{
	gzFile *cache;
	int fd, i, ret;
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
	int fd, i, ret;

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

	for (i = 0; i < sider_len; i++) {
		if (gzwrite(cache, &sider[i], sizeof(*sider)) != sizeof(*sider)) {
			perror("Unable to write cache");
			goto err;
		}
		//fprintf(stderr, "cache: wrote: %s\n", sider[i].uri);
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


