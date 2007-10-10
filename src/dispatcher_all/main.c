    #include "../common/define.h"
    #include "../common/lot.h"
    #include "../common/vid.h"
    #include "../common/stdlib.h"
    #include "../UrlToDocID/search_index.h"

    #include <stdarg.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
#include <sys/stat.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h> 
    #include <time.h>
    #include <errno.h>
#include <zlib.h>
    #include "../common/boithohome.h"
    #include "../maincfg/maincfg.h"

#define DefultMaxsHits 20
    
#ifndef BLACK_BOKS
    #include <libconfig.h>

    #define cashedir "cashedir"
    #define prequerydir "prequerydir"
#endif

    #define cfg_dispatcher "config/dispatcher.conf"

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

#ifdef DEBUG
#define dprintf(str, args...) printf(str, ##args)
#else
#define dprintf(str, args...) 
#endif


/* Set if you want to write prequery data */
int prequerywriteFlag = 0;



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
		char writeprequery;
		const char *UrlToDocID;

		const char *webdb_host;
		const char *webdb_user;
		const char *webdb_password;
		const char *webdb_db;
	};



void die(int errorcode,const char *fmt, ...) {
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
	printf("<RESULT_INFO TOTAL=\"0\" QUERY=\"\" HILITE=\"\" TIME=\"0\" FILTERED=\"0\" SHOWABAL=\"0\" BOITHOHOME=\"%s\"/>\n",bfile(""));
	printf("</search>\n");


        //ToDo: må ha låsing her
        if ((LOGFILE = bfopen("logs/query.log","a")) == NULL) {
                perror(bfile("logs/query.log"));
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
			die(10,"to many errors occurd");
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

		dprintf("reciving %i of %i. SocketWait %i\n",dataReceived,datasize,socketWait);

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

	return returnstatus;

}

int
bsQuery(int *sock, struct queryNodeHederFormat *queryNodeHeder)
{
	if (sock == 0)
		return 0;

	if (sendall(*sock, queryNodeHeder, sizeof(*queryNodeHeder)) != sizeof(*queryNodeHeder)) {
		perror("sendall");
		*sock = 0;
		return 0;
	}

	return 1;
}


int bsConectAndQuery(int *sockfd,int nrOfServers, char *servers[],struct queryNodeHederFormat *queryNodeHeder,int alreadynr, int port) {

	int i;
#ifdef DEBUG
	struct timeval start_time, end_time;
#endif

#ifdef DEBUG
	gettimeofday(&start_time, NULL);
#endif

	//kobler til vanlige servere
	for (i=0;i<nrOfServers;i++) {
		if (bsconnect (&sockfd[i +alreadynr], servers[i], port)) {
			dprintf("can connect\n");
		}
		else {
			dprintf("can NOT connect\n");
			sockfd[i] = 0;
		}
	}	
	//sender ut forespørsel
	for (i=0;i<nrOfServers;i++) {
		//sender forespørsel

		if (sockfd[i+alreadynr] != 0) {
			if (!bsQuery(&sockfd[i+alreadynr], queryNodeHeder))
				fprintf(stderr,"Error: Can't connect. Server %s:%i\n",servers[i], port);
		}
		dprintf("sending of queryNodeHeder end\n");
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

int brGetPages(int *sockfd,int nrOfServers,struct SiderHederFormat *SiderHeder,struct SiderFormat *Sider, 
	int *pageNr,int alreadynr) {

	int i,y;
	int net_status;

	#ifdef DEBUG
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	#endif

	#ifdef DEBUG
	printf("brGetPages: alreadynr %i, *pageNr %i\n",alreadynr,*pageNr);
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

				/*
				for(y=0;y<SiderHeder[i].showabal;y++) {

					if (bsread (&sockfd[i],sizeof(struct SiderFormat),(char *)&Sider[(*pageNr)],maxSocketWait_SiderHeder)) {
						(*pageNr)++;
					}
				}
				*/
				
				if (bsread (&sockfd[i],sizeof(struct SiderFormat) * SiderHeder[i].showabal,(char *)&Sider[(*pageNr)],maxSocketWait_SiderHeder)) {
					(*pageNr) += SiderHeder[i].showabal;
				}



			}
	}


	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: brGetPages.reading pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif

}


/* Cache helper functions */

// sprintf(cashefile,"%s/%s.%i.%s","/home/boitho/var/cashedir",QueryData.queryhtml,QueryData.start,QueryData.GeoIPcontry);

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

enum cache_type {
	CACHE_PREQUERY,
	CACHE_SEARCH
};

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
		cache = "var/cache/prequery";
		break;
	case CACHE_SEARCH:
		cache = "var/cache/search";
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
           size_t sider_header_len, struct SiderFormat *sider, int cachetimeout)
{
	gzFile *cache;
	int fd, i, ret;
	struct stat st;

	if ((fd = open(path, O_RDONLY)) == -1) {
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
	ret = 1;
 out:
	gzclose(cache);
	flock(fd, LOCK_UN);
	close(fd);
	return ret;
}

int
cache_write(char *path, int *page_nr, struct SiderHederFormat *final_sider, struct SiderHederFormat *sider_header,
            size_t sider_header_len, struct SiderFormat *sider, size_t sider_len)
{
	gzFile *cache;
	int fd, i, ret;

	if ((fd = open(path, O_CREAT|O_WRONLY|O_EXCL, 0644)) == -1) {
		return 0;
	}
	flock(fd, LOCK_EX);
	if ((cache = gzdopen(fd, "w")) == NULL) {
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


void
init_cgi(struct QueryDataForamt *QueryData, struct config_t *cfg)
{
	int res;
	config_setting_t *cfgarray;
	struct timeval start_time, end_time;

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
		strscpy(QueryData->query,cgi_getentrystr("query"),sizeof(QueryData->query) -1);
	}


	if (cgi_getentrystr("search_bruker") == NULL) {
		fprintf(stderr,"Did'n receive any username.");
		QueryData->search_user[0] = '\0';
	}
	else {
		strscpy((char *)QueryData->search_user,cgi_getentrystr("search_bruker"),sizeof(QueryData->query) -1);
	}


	if (cgi_getentrystr("userip") == NULL) {
		fprintf(stderr,"Did'n receive any user ip.");
		QueryData->userip[0] = '\0';
	}
	else {
		strscpy(QueryData->userip,cgi_getentrystr("userip"),sizeof(QueryData->userip) -1);
	}
	//temp:setter ip manuelt for å teste
	//strcpy(QueryData->userip,"64.236.24.28");

	if (cgi_getentrystr("subname") == NULL) {
		//die(2,"Did'n receive any subname.");
		strscpy(QueryData->subname,"www",sizeof(QueryData->subname) -1);
	}
	else {
		strscpy(QueryData->subname,cgi_getentrystr("subname"),sizeof(QueryData->subname) -1);
	}


#ifdef BLACK_BOKS
	if (cgi_getentrystr("tkey") == NULL) {
		die(2,"Did'n receive any tkey.");
	}
	else if (strlen(cgi_getentrystr("tkey")) != 32) {
		die(2,"tkey isent 32 bytes long\n");
	}
	else {
		strscpy(QueryData->tkey,cgi_getentrystr("tkey"),sizeof(QueryData->tkey));

		//sjek tkey
		if (!tkeyisok(QueryData->tkey)) {
			die(2,"Wrong tkey.");
		}
	}
#endif

	if (cgi_getentrystr("orderby") == NULL) {
		QueryData->orderby[0] = '\0';
	}
	else {
		strscpy(QueryData->orderby,cgi_getentrystr("orderby"),sizeof(QueryData->orderby) -1);
	}


	if (cgi_getentryint("start") == 0) {
		QueryData->start = 1;
	}
	else {
		QueryData->start = cgi_getentryint("start");
	}

	if ((cgi_getentrystr("filter") == NULL) || (strcmp(cgi_getentrystr("filter"),"") == 0) ) {
		QueryData->filterOn = 1;
	}
	else {
		QueryData->filterOn = cgi_getentryint("filter");
	}


	if (cgi_getentrystr("HTTP_ACCEPT_LANGUAGE") != NULL) {
		strscpy(QueryData->HTTP_ACCEPT_LANGUAGE,cgi_getentrystr("HTTP_ACCEPT_LANGUAGE"),sizeof(QueryData->HTTP_ACCEPT_LANGUAGE));
	}
	else {
		QueryData->HTTP_ACCEPT_LANGUAGE[0] = '\0';
		fprintf(stderr,"Dident get a HTTP_ACCEPT_LANGUAGE\n");
	}

	if (cgi_getentrystr("HTTP_USER_AGENT") != NULL) {
		strscpy(QueryData->HTTP_USER_AGENT,cgi_getentrystr("HTTP_USER_AGENT"),sizeof(QueryData->HTTP_USER_AGENT));
	}
	else {
		QueryData->HTTP_USER_AGENT[0] = '\0';
		fprintf(stderr,"Dident get a HTTP_USER_AGENT\n");
	}

	if (cgi_getentrystr("HTTP_REFERER") != NULL) {
		strscpy(QueryData->HTTP_REFERER,cgi_getentrystr("HTTP_REFERER"),sizeof(QueryData->HTTP_REFERER));
	}
	else {
		QueryData->HTTP_REFERER[0] = '\0';
		fprintf(stderr,"Dident get a HTTP_REFERER\n");
	}



	if (cgi_getentrystr("languageFilter") != NULL) {
		//v13 strscpy(QueryData->languageFilter,cgi_getentrystr("languageFilter"),sizeof(QueryData->languageFilter) -1);
	}
	else {
		//v13 QueryData->languageFilter[0] = '\0';
		//v13 fprintf(stderr,"Dident get a languageFilter\n");
	}


	if (cgi_getentrystr("AmazonAssociateTag") != NULL) {
		strscpy(QueryData->AmazonAssociateTag,cgi_getentrystr("AmazonAssociateTag"),sizeof(QueryData->AmazonAssociateTag) -1);
	}
	else {
		QueryData->AmazonAssociateTag[0] = '\0';
		fprintf(stderr,"Dident get a AmazonAssociateTag\n");
	}

	if (cgi_getentrystr("AmazonSubscriptionId") != NULL) {
		strscpy(QueryData->AmazonSubscriptionId,cgi_getentrystr("AmazonSubscriptionId"),sizeof(QueryData->AmazonSubscriptionId) -1);
	}
	else {
		QueryData->AmazonSubscriptionId[0] = '\0';
		fprintf(stderr,"Dident get a AmazonSubscriptionId\n");
	}

	if (cgi_getentrystr("getrank") == NULL) {
		QueryData->rankUrl[0] = '\0';
	} else {
		strscpy(QueryData->rankUrl, cgi_getentrystr("getrank"), sizeof(QueryData->rankUrl));
	}

	if (cgi_getentryint("maxshits") == 0) {
		QueryData->MaxsHits = DefultMaxsHits;
	}
	else {
		QueryData->MaxsHits = cgi_getentryint("maxshits");			
	}


	//Runarb: Dette er vel bare aktuelt for black boks. For web trnger eksterne klienter å kalle dispatcher_all direkte?
#ifdef BLACK_BOKS

#ifdef DEBUG
	gettimeofday(&start_time, NULL);
#endif
	char *remoteaddr = getenv("REMOTE_ADDR");

	int accesshosts, hasaccess = 0;
	if (remoteaddr && strcmp(remoteaddr, "127.0.0.1") == 0) {
		hasaccess = 1;
	}

	if (remoteaddr != NULL && hasaccess == 0 &&
	    (cfgarray = config_lookup(cfg, "access")) != NULL && (accesshosts = config_setting_length(cfgarray)) > 0) {
		int i;

		for(i=0; i < accesshosts; i++) {
			const char *p;
			char *p2;

			p = config_setting_get_string_elem(cfgarray, i);
			p2 = strchr(p, ':');
			if (p2 == NULL) {
				fprintf(stderr, "Invalid string in config file: '%s'\n", p);
				continue;
			}
			*p2 = '\0';
			p2++;
			if (strcmp(p, remoteaddr) == 0 || strcmp(p, "all") == 0) {
				char *key;

				if ((key = cgi_getentrystr("secret")) != NULL) {
					if (strcmp(key, p2) == 0) {
						hasaccess = 1;
					}
				}
				break;
			}
		}
	}
	if (hasaccess == 0)
		die(1, "Not allowed to handle request from ip address \"%s\".",remoteaddr);
#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: access %f\n",getTimeDifference(&start_time,&end_time));
#endif

#endif

}


int
handle_results(int *sockfd, struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder,
               struct QueryDataForamt *QueryData,
               struct SiderHederFormat *FinalSiderHeder, int fromCache, struct errorhaFormat *errorha,
               int pageNr, int nrOfServers, int nrOfPiServers, struct filtersTrapedFormat *dispatcherfiltersTraped,
	       int *nrRespondedServers) 
{
	int AdultPages, NonAdultPages;
	int posisjon, i;
	int funnet;
	struct timeval start_time, end_time;


	*nrRespondedServers = 0;
	if (!fromCache) {

		FinalSiderHeder->TotaltTreff = 0;
		FinalSiderHeder->filtered = 0;

		for (i=0;i<(nrOfServers + nrOfPiServers);i++) {
			//aaaaa
			if (sockfd[i] != 0) {
				FinalSiderHeder->TotaltTreff += SiderHeder[i].TotaltTreff;
				FinalSiderHeder->filtered += SiderHeder[i].filtered;
				/*
				runarb: 31.08.07seg feiler her.
				dprintf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" tid=\"%f\" filtered=\"%i\" showabal=\"%i\" servername=\"%s\"/>\n",
					SiderHeder[i].TotaltTreff,
					QueryData->query,
					SiderHeder[i].hiliteQuery,
					SiderHeder[i].total_usecs,
					SiderHeder[i].filtered,
					SiderHeder[i].showabal,
					SiderHeder[i].servername);
				*/
				(*nrRespondedServers)++;
			}
		}

#if 0
#ifdef DEBUG
		dprintf("addservers (have %i):\n",nrOfAddServers);
		for (i=0;i<nrOfAddServers;i++) {
			//aaaaa
			if (addsockfd[i] != 0) {
				dprintf("\t<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" tid=\"%f\" filtered=\"%i\" showabal=\"%i\" servername=\"%s\"/>\n",AddSiderHeder[i].TotaltTreff,QueryData->query,AddSiderHeder[i].hiliteQuery,AddSiderHeder[i].total_usecs,AddSiderHeder[i].filtered,AddSiderHeder[i].showabal,AddSiderHeder[i].servername);

			}
			else {
				dprintf("addserver nr %i's sockfd is 0\n",i);
			}
		}
#endif
#endif

		//finner en hillitet query
		if (*nrRespondedServers != 0) {
			funnet = 0;
			for(i=0;i<(nrOfServers + nrOfPiServers) && !funnet;i++) {
				if ((sockfd[i] != 0) && (SiderHeder[i].hiliteQuery != '\0')) {
					strcpy(FinalSiderHeder->hiliteQuery,SiderHeder[i].hiliteQuery);
					funnet =1;
				}
			}
		}
		else {
			FinalSiderHeder->hiliteQuery[0] = '\0';
		}

		//hvis vi ikke fikk svar fra noen
		if(*nrRespondedServers == 0) {
			die(16,"Couldnt contact the Boitho search system. Please try again later.");
		}
		//genererer feil om at ikke alle server svarte på queryet
		if (*nrRespondedServers != (nrOfServers + nrOfPiServers)) {
			addError(errorha,11,"Not all the search nodes responded to your query. Result quality may have been negatively effected.");
		}
		//hånterer error. Viser den hvis vi hadde noen
		if (*nrRespondedServers != 0) {
			for(i=0;i<(nrOfServers + nrOfPiServers);i++) {
				if (SiderHeder[i].responstype == searchd_responstype_error) {
					addError(errorha,11,SiderHeder[i].errorstr);
				}
			}
		}

		//fjerner eventuelle adult sider
		AdultPages = 0;
		NonAdultPages = 0;
		for(i=0;i<QueryData->MaxsHits * nrOfServers + nrOfPiServers;i++) {	
			if (!Sider[i].deletet) {

				if (Sider[i].DocumentIndex.AdultWeight > 50) {
					++AdultPages;
				}
				else {
					++NonAdultPages;
				}

			}		
		}

		dprintf("AdultPages %i, NonAdultPages: %i\n",AdultPages,NonAdultPages);
		//hvis vi har adult pages sjekker vi om vi har nokk ikke adult pages å vise, hvis ikke viser vi bare adult

#ifdef DEBUG
		gettimeofday(&start_time, NULL);
#endif
		//sorterer resultatene
		dprintf("mgsort: pageNr %i\n",pageNr);
		mgsort(Sider, pageNr , sizeof(struct SiderFormat), compare_elements);

#ifdef DEBUG
		gettimeofday(&end_time, NULL);
		dprintf("Time debug: mgsort_1 %f\n",getTimeDifference(&start_time,&end_time));
#endif

#ifdef DEBUG
		gettimeofday(&start_time, NULL);
#endif		

		filtersTrapedReset(dispatcherfiltersTraped);

		//dette er kansje ikke optimalet, da vi går gjenom alle siden. Ikke bare de som skal være med
		for(i=0;i<QueryData->MaxsHits * nrOfServers + nrOfPiServers;i++) {

			dprintf("looking on url %s, deleted %i, adult %i, allrank %u, i %i, type %i\n",Sider[i].DocumentIndex.Url,Sider[i].deletet,Sider[i].DocumentIndex.AdultWeight,Sider[i].iindex.allrank,i,Sider[i].type);
			if (!Sider[i].deletet) {
				//setter som slettet
				Sider[i].deletet = 1;


				if ((QueryData->filterOn) && (Sider[i].subname.config.filterSameUrl) 
						&& (filterSameUrl(i,Sider[i].url,Sider)) ) {
					dprintf("Hav seen url befor. Url '%s', DocID %u\n",Sider[i].url,Sider[i].iindex.DocID);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterSameUrl;					
					continue;
				}

#ifndef BLACK_BOKS

#if 0
				// 19. juni
				//ToDo: fjerner adult vekt filtrering her. Er det trykt. Hvis vi for eks har misket resultater, men ikke noen noder hadde fø sider, og tilot adoult
				// hva er egentlig adoult filter statur på searchd nå?
				if ((QueryData->filterOn) && Sider[i].DocumentIndex.AdultWeight > 50) {
					dprintf("slettet adult side %s ault %i\n",Sider[i].url,Sider[i].DocumentIndex.AdultWeight);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterAdultWeight_value;
					continue;
				}
#endif
				if ((QueryData->filterOn) && (Sider[i].subname.config.filterSameCrc32) 
						&& filterSameCrc32(i,&Sider[i],Sider)) {
					dprintf("hav same crc32. crc32 from DocumentIndex. Will delete \"%s\"\n",Sider[i].DocumentIndex.Url);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterSameCrc32_1;

					continue;
				}

				if ((QueryData->filterOn) && (Sider[i].subname.config.filterSameDomain) 
						&& (filterSameDomain(i,&Sider[i],Sider))) {
					dprintf("hav same domain \"%s\"\n",Sider[i].domain);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					++dispatcherfiltersTraped->filterSameDomain;

					continue;
				}

#if 0
				if ((QueryData->filterOn) && filterDescription(i,&Sider[i],Sider)) {
					dprintf("hav same Description. DocID %i\n",Sider[i].iindex.DocID);
					//(*SiderHeder).filtered++;
					FinalSiderHeder->filtered++;
					--FinalSiderHeder->TotaltTreff;
					continue;
				}
#endif
#endif

				//printf("url %s\n",Sider[i].DocumentIndex.Url);

				//hvis siden overlevde helt hit er den ok
				Sider[i].deletet = 0;
			}
		}

	} // !hascashe && !hasprequery
	else {
		*nrRespondedServers = 1;

	}

	//why was sort here???
	posisjon=0;
	for(i=0;i<QueryData->MaxsHits * nrOfServers + nrOfPiServers;i++) {
		if (!Sider[i].deletet) {
			Sider[i].posisjon = posisjon++;
		}

		//dprintf("%s\n",Sider[i].url);
	}	


#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: filter pages %f\n",getTimeDifference(&start_time,&end_time));
#endif


#ifdef DEBUG
	gettimeofday(&start_time, NULL);
#endif

	//resorterer query
	//mgsort(Sider, nrOfServers * QueryData->MaxsHits , sizeof(struct SiderFormat), compare_elements_posisjon);
	mgsort(Sider, pageNr , sizeof(struct SiderFormat), compare_elements_posisjon);

#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: mgsort_2 %f\n",getTimeDifference(&start_time,&end_time));
#endif

	/* tempaa */
#if 0
	FinalSiderHeder->showabal = 0;
	for(i=0;i<QueryData->MaxsHits * nrOfServers;i++) {
		if (!Sider[i].deletet) {
			++FinalSiderHeder->showabal;
		}
	}
#endif

	FinalSiderHeder->showabal = pageNr;
	if (FinalSiderHeder->showabal > QueryData->MaxsHits) {
		FinalSiderHeder->showabal = QueryData->MaxsHits;
	}
}


int main(int argc, char *argv[])
{

	//#ifdef WITH_PROFILING
	//int pcount;
	//for (pcount=0;pcount<=50;pcount++) {	
	//#endif

	char queryEscaped[MaxQueryLen*2+1];

        int sockfd[maxServers];
        int addsockfd[maxServers];
	int i,y,n,x;
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

        #define salt "sdjbjolQdfgkkf"
        char vidbuf[64];
        time_t etime;
        time(&etime);
	
        //struct SiderHederFormat SiderHeder[maxServers];
        //struct SiderHederFormat AddSiderHeder[maxServers];

        struct SiderHederFormat *SiderHeder = malloc(sizeof(struct SiderHederFormat) * maxServers);
        struct SiderHederFormat *AddSiderHeder = malloc(sizeof(struct SiderHederFormat) * maxServers);


	struct SiderHederFormat FinalSiderHeder;
	//char buff[4096]; //generell buffer
	struct in_addr ipaddr;
        struct QueryDataForamt QueryData;
	//int connected[maxServers];
	//int NonAdultPages,AdultPages;
	struct timeval main_start_time, main_end_time;
	struct timeval start_time, end_time;
	int nrRespondedServers;
	char errormessage[maxerrorlen];
	struct errorhaFormat errorha;
	errorha.nr = 0;
	//int posisjon;
	struct timeval timeout;
	struct timeval time;
	int socketWait;	
	int hascashe;
        int hasprequery;
	char prequeryfile[512];
	char cashefile[512];
	struct filtersTrapedFormat dispatcherfiltersTraped;

	char *cpnt;
	char *lastdomain = NULL;

	unsigned int getRank; /* Set if we are looking for the rank of a specific query on a url */

	unsigned int wantedDocId;
	struct queryNodeHederFormat queryNodeHeder;

	struct dispconfigFormat dispconfig;
	int cachetimeout;
	config_setting_t *cfgstring;

	#ifdef DEBUG
	gettimeofday(&start_time, NULL);
	#endif
	dprintf("struct SiderFormat size %i\n",sizeof(struct SiderFormat));

	//starter å ta tiden
	gettimeofday(&main_start_time, NULL);


	struct config_t maincfg;

	maincfg = maincfgopen();

	int searchport = maincfg_get_int(&maincfg,"BSDPORT");

	//config
	config_setting_t *cfgarray;
	struct config_t cfg;


	/* Initialize the configuration */
	config_init(&cfg);

	/* Load the file */
	dprintf("loading [%s]..\n", bfile(cfg_dispatcher) );

	if (!config_read_file(&cfg, bfile(cfg_dispatcher) )) {
		printf("[%s]failed: %s at line %i\n",bfile(cfg_dispatcher),config_error_text(&cfg),config_error_line(&cfg));
		exit(1);
	}

	if ((cfgstring = config_lookup(&cfg, "cachetimeout")) == NULL) {
		cachetimeout = 0;
	} else {
		cachetimeout = config_setting_get_int(cfgstring);
	}

  	#ifndef BLACK_BOKS

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

	    	if ( (cfgarray = config_lookup(&cfg, "writeprequery") ) == NULL) {
			printf("can't load \"writeprequery\" from config\n");
			exit(1);
	  	}

		dispconfig.writeprequery = config_setting_get_bool(cfgarray);

	    	if ( (cfgarray = config_lookup(&cfg, "UrlToDocID") ) == NULL) {
			printf("can't load \"UrlToDocID\" from config\n");
			exit(1);
	  	}

		dispconfig.UrlToDocID = config_setting_get_string(cfgarray);


		// mysql web db config
	        if ((cfgarray = config_lookup(&cfg, "mysql_webdb")) == NULL) {
	
        	        printf("can't load \"mysql_webdb\" from config\n");
                	exit(1);
        	}


        	if ( (cfgstring = config_setting_get_member(cfgarray, "host") ) == NULL) {
                	printf("can't load \"host\" from config\n");
                	exit(1);
        	}

        	dispconfig.webdb_host = config_setting_get_string(cfgstring);


        	if ( (cfgstring = config_setting_get_member(cfgarray, "user") ) == NULL) {
                	printf("can't load \"user\" from config\n");
                	exit(1);
        	}

        	dispconfig.webdb_user = config_setting_get_string(cfgstring);


        	if ( (cfgstring = config_setting_get_member(cfgarray, "password") ) == NULL) {
                	printf("can't load \"password\" from config\n");
                	exit(1);
        	}

        	dispconfig.webdb_password = config_setting_get_string(cfgstring);


        	if ( (cfgstring = config_setting_get_member(cfgarray, "db") ) == NULL) {
                	printf("can't load \"db\" from config\n");
                	exit(1);
        	}

        	dispconfig.webdb_db = config_setting_get_string(cfgstring);

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

		//ToDO: vi har nrOfPiServers med her, men uten at jeg kan se at vi fakrisk 
		//legger inn de. tar bort
		//servers = malloc(sizeof(char *) * (nrOfServers + nrOfPiServers));
		servers = malloc(sizeof(char *) * (nrOfServers));

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

	        char *optRank = NULL;

        	extern char *optarg;
        	extern int optind, opterr, optopt;
        	char c;
        	while ((c=getopt(argc,argv,"pr:"))!=-1) {
        	        switch (c) {
				case 'p':
					prequerywriteFlag = 1;
					dispconfig.writeprequery = 1;
					printf("Forcing prequery write\n");
					break;
        	                case 'r':
        	                        optRank = optarg;
                	                printf("will look up rank for \"%s\"\n",optRank);
                	                break;
                        	default:
					printf("ukjent option\n");
                                	exit(1);
                	}

        	}
        	--optind;

        	printf("argc %i, optind %i\n",argc,optind);




                if (argc < 3 ) {
                        printf("Error ingen query spesifisert eller subname .\n\nEksempel på bruk for å søke på boitho:\n");
			#ifdef BLACK_BOKS
				printf("\tdispatcher_all boitho www bruker\n\n\n");
			#else
				printf("\tdispatcher_all boitho www\n\n\n");
			#endif
                }
                else {
			strcpy(QueryData.userip,"213.179.58.99");

                        strcpy(QueryData.query,argv[1 +optind]);
			strcpy(QueryData.subname,argv[2 +optind]);
			#ifdef BLACK_BOKS
				strcpy(QueryData.search_user,argv[3 +optind]);
			#else
				QueryData.search_user[0] = '\0';
			#endif

			if (optRank == NULL) {
				getRank = 0;
				QueryData.rankUrl[0] = '\0';
			}
			else {
				getRank = 1;
				strscpy(QueryData.rankUrl,optRank,sizeof(QueryData.rankUrl));
				printf("will rank \"%s\"\n",QueryData.rankUrl);
			}

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
		init_cgi(&QueryData, &cfg);
		if (QueryData.rankUrl[0] == '\0')
			getRank = 0;
		else
			getRank = 1;
        }

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: init %f\n",getTimeDifference(&start_time,&end_time));
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


	if (getRank) {
		//normaliserer url. Setter for eks / på slutten
		url_normalization(QueryData.rankUrl,sizeof(QueryData.rankUrl));

        	char            db_index[strlen(dispconfig.UrlToDocID)+7];
        	sprintf(db_index, "%s.index", dispconfig.UrlToDocID);
	        urldocid_data   *data;

		if ((data = urldocid_search_init(db_index, dispconfig.UrlToDocID)) == NULL) {
			die(100, "Unable to open index file \"%s\".",dispconfig.UrlToDocID);
		}


                if (!getDocIDFromUrl(data, QueryData.rankUrl, &wantedDocId)) {
		//if (!getDocIDFromUrl(dispconfig.UrlToDocID, QueryData.rankUrl, &wantedDocId)) {
			die(100, "Unable to find docId");
		} else {
			getRank = wantedDocId;
			queryNodeHeder.getRank = wantedDocId;
		
			#ifdef DEBUG
				printf("found DocID %u ( for url \"%s\" )\n",wantedDocId,QueryData.rankUrl);
			#endif	
		}
	}
	else {
		queryNodeHeder.getRank = 0;
	}

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
		dprintf("Time debug: query normalizeing %f\n",getTimeDifference(&start_time,&end_time));
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
			strcpy(QueryData.GeoIPcontry, gir->country_code);

        		dprintf("GeoIP: %s\t%s\t%s\t%s\t%s\t%f\t%f\t%d\t%d\n", queryNodeHeder.userip,
                                         gir->country_code,
                                         gir->region,
                                         gir->city,
                                         gir->postal_code,
                                         gir->latitude,
                                         gir->longitude,
                                         gir->dma_code,
                                         gir->area_code);
			dprintf("GeoIPcontry: %s\n",QueryData.GeoIPcontry);
        		GeoIPRecord_delete(gir);
		}

		GeoIP_delete(gi);
	}
	#else
		sprintf(QueryData.GeoIPcontry,"na");
	#endif

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: geoip %f\n",getTimeDifference(&start_time,&end_time));
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
	if (nrOfServers >= 3) {
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
	dprintf("Time debug: query copying %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	#ifdef WITH_CASHE

	char cachepath[1024];

	cache_path(cachepath, sizeof(cachepath), CACHE_SEARCH, QueryData.queryhtml, QueryData.start, QueryData.GeoIPcontry);
	cache_path(prequeryfile, sizeof(cachepath), CACHE_PREQUERY, QueryData.queryhtml, QueryData.start, QueryData.GeoIPcontry);

	if (!prequerywriteFlag && getRank == 0 && (dispconfig.useprequery) && (QueryData.filterOn) &&
	    cache_read(prequeryfile, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, 0)) {
		hasprequery = 1;

		debug("can open prequeryfile file \"%s\"",prequeryfile);

	}
	else if (!prequerywriteFlag && getRank == 0 && (dispconfig.usecashe) && (QueryData.filterOn) &&
	         cache_read(cachepath, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, cachetimeout)) {
		hascashe = 1;

		debug("can open cashe file \"%s\"",cashefile);
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

	if (getRank) {
		int endranking = 0;
		int ranking = -1;
		int net_status;
		int n;

		/* XXX: Need to handle the paid inclusion servers as well? */
		for (i = 0; i < nrOfServers; i++) {
			if (sockfd[i] != 0) {
				int status;
				//motter hedderen for svaret
				if (bsread (&sockfd[i],sizeof(net_status), (char *)&net_status, maxSocketWait_CanDo)) {
					if (net_status != net_CanDo) {
						fprintf(stderr, "net_status wasn't net_CanDo but %i\n",net_status);
						sockfd[i] = 0;
						continue;
					}
				} else {
					perror("initial protocol read failed");
					sockfd[i] = 0;
					continue;
				}
 
				if (!bsread(&sockfd[i],sizeof(status), (char *)&status, 1000)) //maxSocketWait_CanDo))
					die(2, "Unable to get rank status");
				else if (status == net_match) {
					if (!bsread(&sockfd[i],sizeof(ranking), (char *)&ranking, 1000))//maxSocketWait_CanDo))
						perror("recv read");
				} else if (status == net_nomatch) {
					//return 1;
				} else {
					//die(1, "searchd does not support ranking?");
				}
			}
		}
		if (ranking != -1) {
			for (i = 0; i < nrOfServers; i++) {
				if (sockfd[i] != 0) {
					if (send(sockfd[i], &ranking, sizeof(ranking), 0) != sizeof(ranking))
						perror("send...");
				}
			}

			for (i = 0; i < nrOfServers; i++) {
				if (sockfd[i] != 0) {
					if (!bsread(&sockfd[i], sizeof(ranking), (char *)&ranking, 10000))
						perror("endranking");
					endranking += ranking;
				}
			}
		}
		else {
			die(1, "No rank found");
		}

		if (endranking < QueryData.MaxsHits) {
			queryNodeHeder.getRank = 0;

			for (i=0;i<nrOfServers + nrOfPiServers;i++) {
				if (sockfd[i] != 0) {
					bsQuery(&sockfd[i], &queryNodeHeder);
				}
			}

			for (i=0;i<nrOfServers + nrOfPiServers;i++) {
				if (sockfd[i] != 0) {
					close(sockfd[i]);
				}
			}
//
//			brGetPages(sockfd,nrOfPiServers,SiderHeder,Sider,&pageNr,nrOfServers);
//			if ((!hascashe) && (!hasprequery)) {
//				brGetPages(sockfd,nrOfServers,SiderHeder,Sider,&pageNr,0);
//			}
//


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

			//addservere
			//addserver bruker som regel mest tid, så tar den sist slik at vi ikke trenger å vente unødvendig
			brGetPages(addsockfd,nrOfAddServers,AddSiderHeder,Sider,&pageNr,0);

			handle_results(sockfd, Sider, SiderHeder, &QueryData, &FinalSiderHeder, (hascashe || hasprequery), &errorha, pageNr,
				       nrOfServers, nrOfPiServers, &dispatcherfiltersTraped, &nrRespondedServers);

			//for((x<FinalSiderHeder.showabal) && (i < (QueryData.MaxsHits * (nrOfServers + nrOfPiServers)))) {


			x = 0;
			int printed = 0;
			for(i=0;x < FinalSiderHeder.showabal && i < (QueryData.MaxsHits * (nrOfServers + nrOfPiServers)); i++) {
				if (Sider[i].deletet)
					continue;
				if (Sider[i].iindex.DocID == wantedDocId) {
					endranking = printed+1;
					break;
				}
				if (Sider[i].type == siderType_normal) {
					x++;
				}
				printed++;
			}
		}

		printf("<ranking>\n");

		if (endranking == 0) {
			printf("\t<noresult />\n");
		} else {
			printf("\t<rank>%d</rank>\n", endranking);
			printf("\t<url>%s</url>\n", QueryData.rankUrl);
			printf("\t<docid>%d</docid>\n", wantedDocId);
		}

		printf("</ranking>\n");

		/* Free the configuration */
		config_destroy(&cfg);
		maincfgclose(&maincfg);

		free(SiderHeder);
		free(AddSiderHeder);
		free(Sider);

		for (i=0;i<nrOfServers + nrOfPiServers;i++) {
			if (sockfd[i] != 0) {
				close(sockfd[i]);
			}
		}

#ifndef BLACK_BOKS
		for(i=0;i<nrOfServers;i++) {
			free(servers[i]);
		}
		for(i=0;i<nrOfPiServers;i++) {
			free(piservers[i]);
		}
		for(i=0;i<nrOfAddServers;i++) {
			free(addservers[i]);
		}

		free(servers);
		free(piservers);
		free(addservers);
#endif

		return 0;
	}

	//Paid inclusion
	brGetPages(sockfd,nrOfPiServers,SiderHeder,Sider,&pageNr,0);

	if ((!hascashe) && (!hasprequery)) {
		brGetPages(sockfd,nrOfServers,SiderHeder,Sider,&pageNr,nrOfPiServers);
	}

	//addservere
	//addserver bruker som regel mest tid, så tar den sist slik at vi ikke trenger å vente unødvendig
	brGetPages(addsockfd,nrOfAddServers,AddSiderHeder,Sider,&pageNr,0);

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: geting pages %f\n",getTimeDifference(&start_time,&end_time));
	#endif

	handle_results(sockfd, Sider, SiderHeder, &QueryData, &FinalSiderHeder, (hascashe || hasprequery), &errorha, pageNr,
	               nrOfServers, nrOfPiServers, &dispatcherfiltersTraped, &nrRespondedServers);


	//stopper #ta tidn og kalkulerer hvor lang tid vi brukte
	gettimeofday(&main_end_time, NULL);
	FinalSiderHeder.total_usecs = getTimeDifference(&main_start_time,&main_end_time);

	//Sier ikke noe om filtrerte treff hvis vi hadde mange nokk
	if (FinalSiderHeder.TotaltTreff>100) {
		FinalSiderHeder.filtered = 0;;
	}



	totlaAds = 0;

        //printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?> \n");
        printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?> \n");
        printf("<!DOCTYPE family SYSTEM \"http://www.boitho.com/xml/search.dtd\"> \n");

        printf("<SEARCH>\n");   
	//får rare svar fra hilite. Dropper å bruke den får nå
	FinalSiderHeder.hiliteQuery[0] = '\0';
        printf("<RESULT_INFO TOTAL=\"%i\" QUERY=\"%s\" HILITE=\"%s\" TIME=\"%f\" FILTERED=\"%i\" SHOWABAL=\"%i\" CASHE=\"%i\" \
		PREQUERY=\"%i\" GEOIPCONTRY=\"%s\" SUBNAME=\"%s\" BOITHOHOME=\"%s\" NROFSEARCHNODES=\"%i\"/>\n",
		FinalSiderHeder.TotaltTreff,
		QueryData.queryhtml,
		FinalSiderHeder.hiliteQuery,
		FinalSiderHeder.total_usecs,
		FinalSiderHeder.filtered,
		FinalSiderHeder.showabal,
		hascashe,
		hasprequery,
		QueryData.GeoIPcontry,
		QueryData.subname,
		bfile(""),
		nrRespondedServers
	);

	
	//viser info om dispatcher_all
	printf("<DISPATCHER_INFO>\n");
	printf("\t<FILTERTRAPP>\n");
	{
		printf("\t\t<filterAdultWeight_bool>%i</filterAdultWeight_bool>\n",dispatcherfiltersTraped.filterAdultWeight_bool);
		printf("\t\t<filterAdultWeight_value>%i</filterAdultWeight_value>\n",dispatcherfiltersTraped.filterAdultWeight_value);
		printf("\t\t<filterSameCrc32_1>%i</filterSameCrc32_1>\n",dispatcherfiltersTraped.filterSameCrc32_1);
		printf("\t\t<filterSameUrl>%i</filterSameUrl>\n",dispatcherfiltersTraped.filterSameUrl);
		printf("\t\t<filterNoUrl>%i</filterNoUrl>\n",dispatcherfiltersTraped.filterNoUrl);
		printf("\t\t<find_domain_no_subname>%i</find_domain_no_subname>\n",dispatcherfiltersTraped.find_domain_no_subname);
		printf("\t\t<filterSameDomain>%i</filterSameDomain>\n",dispatcherfiltersTraped.filterSameDomain);
		printf("\t\t<filterTLDs>%i</filterTLDs>\n",dispatcherfiltersTraped.filterTLDs);
		printf("\t\t<filterResponse>%i</filterResponse>\n",dispatcherfiltersTraped.filterResponse);
		printf("\t\t<cantpopResult>%i</cantpopResult>\n",dispatcherfiltersTraped.cantpopResult);
		printf("\t\t<cmc_pathaccess>%i</cmc_pathaccess>\n",dispatcherfiltersTraped.cmc_pathaccess);
		printf("\t\t<filterSameCrc32_2>%i</filterSameCrc32_2>\n",dispatcherfiltersTraped.filterSameCrc32_2);
	}
	printf("\t</FILTERTRAPP>\n");
	printf("</DISPATCHER_INFO>\n");


	if ((!hascashe) && (!hasprequery)) {

		//viser info om serverne som svarte
		//printf("<SEARCHNODES_INFO NROFSEARCHNODES=\"%i\" />\n",nrRespondedServers);

		for (i=0;i<nrOfServers + nrOfPiServers;i++) {
			if (sockfd[i] != 0) {
				printf("<SEARCHNODES>\n");
				printf("\t<NODENAME>%s</NODENAME>\n",SiderHeder[i].servername);
				printf("\t<TOTALTIME>%f</TOTALTIME>\n",SiderHeder[i].total_usecs);
				printf("\t<FILTERED>%i</FILTERED>\n",SiderHeder[i].filtered);
				printf("\t<HITS>%i</HITS>\n",SiderHeder[i].TotaltTreff);

#ifndef DEBUG
				printf("\t<TIMES>\n");
				{
					printf("\t\t<AthorSearch>%f</AthorSearch>\n",SiderHeder[i].queryTime.AthorSearch);
					//printf("\t\t<AthorRank>%f</AthorRank>\n",SiderHeder[i].queryTime.AthorRank);
					printf("\t\t<UrlSearch>%f</UrlSearch>\n",SiderHeder[i].queryTime.UrlSearch);
					printf("\t\t<MainSearch>%f</MainSearch>\n",SiderHeder[i].queryTime.MainSearch);
					//printf("\t\t<MainRank>%f</MainRank>\n",SiderHeder[i].queryTime.MainRank);
					printf("\t\t<MainAthorMerge>%f</MainAthorMerge>\n",SiderHeder[i].queryTime.MainAthorMerge);
					printf("\t\t<popRank>%f</popRank>\n",SiderHeder[i].queryTime.popRank);
					printf("\t\t<responseShortning>%f</responseShortning>\n",SiderHeder[i].queryTime.responseShortning);

					printf("\t\t<allrankCalc>%f</allrankCalc>\n",SiderHeder[i].queryTime.allrankCalc);
					printf("\t\t<indexSort>%f</indexSort>\n",SiderHeder[i].queryTime.indexSort);
					printf("\t\t<searchSimple>%f</searchSimple>\n",SiderHeder[i].queryTime.searchSimple);

					printf("\t\t<popResult>%f</popResult>\n",SiderHeder[i].queryTime.popResult);
					printf("\t\t<adultcalk>%f</adultcalk>\n",SiderHeder[i].queryTime.adultcalk);

#ifdef BLACK_BOKS
					printf("\t\t<filetypes>%f</filetypes>\n",SiderHeder[i].queryTime.filetypes);
					printf("\t\t<iintegerGetValueDate>%f</iintegerGetValueDate>\n",SiderHeder[i].queryTime.iintegerGetValueDate);
					printf("\t\t<dateview>%f</dateview>\n",SiderHeder[i].queryTime.dateview);
					printf("\t\t<crawlManager>%f</crawlManager>\n",SiderHeder[i].queryTime.crawlManager);
					printf("\t\t<getUserObjekt>%f</getUserObjekt>\n",SiderHeder[i].queryTime.getUserObjekt);
					printf("\t\t<cmc_conect>%f</cmc_conect>\n",SiderHeder[i].queryTime.cmc_conect);
#endif
				}
				printf("\t</TIMES>\n");

				printf("\t<FILTERTRAPP>\n");
				{
					printf("\t\t<filterAdultWeight_bool>%i</filterAdultWeight_bool>\n",SiderHeder[i].filtersTraped.filterAdultWeight_bool);
					printf("\t\t<filterAdultWeight_value>%i</filterAdultWeight_value>\n",SiderHeder[i].filtersTraped.filterAdultWeight_value);
					printf("\t\t<filterSameCrc32_1>%i</filterSameCrc32_1>\n",SiderHeder[i].filtersTraped.filterSameCrc32_1);
					printf("\t\t<filterSameUrl>%i</filterSameUrl>\n",SiderHeder[i].filtersTraped.filterSameUrl);
					printf("\t\t<filterNoUrl>%i</filterNoUrl>\n",SiderHeder[i].filtersTraped.filterNoUrl);
					printf("\t\t<find_domain_no_subname>%i</find_domain_no_subname>\n",SiderHeder[i].filtersTraped.find_domain_no_subname);
					printf("\t\t<filterSameDomain>%i</filterSameDomain>\n",SiderHeder[i].filtersTraped.filterSameDomain);
					printf("\t\t<filterTLDs>%i</filterTLDs>\n",SiderHeder[i].filtersTraped.filterTLDs);
					printf("\t\t<filterResponse>%i</filterResponse>\n",SiderHeder[i].filtersTraped.filterResponse);
					printf("\t\t<cantpopResult>%i</cantpopResult>\n",SiderHeder[i].filtersTraped.cantpopResult);
					printf("\t\t<cmc_pathaccess>%i</cmc_pathaccess>\n",SiderHeder[i].filtersTraped.cmc_pathaccess);
					printf("\t\t<filterSameCrc32_2>%i</filterSameCrc32_2>\n",SiderHeder[i].filtersTraped.filterSameCrc32_2);
				}
				printf("\t</FILTERTRAPP>\n");
#endif

				printf("</SEARCHNODES>\n");


			}	
		}
	}
	else {
		printf("<SEARCHNODES>\n");
		printf("\t<NODENAME>cashe.boitho.com</NODENAME>\n");
		printf("\t<TOTALTIME>%f</TOTALTIME>\n",FinalSiderHeder.total_usecs);
		printf("\t<FILTERED>0</FILTERED>\n");
		printf("\t<HITS>%i</HITS>\n",FinalSiderHeder.TotaltTreff);
		printf("</SEARCHNODES>\n");
	}

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

		printf("<FILENAME>%s</FILENAME>\n<FILELONGNAME>%s</FILELONGNAME>\n<FILENR>%i</FILENR>",
				SiderHeder[0].filters.filtypes.elements[i].name,
				SiderHeder[0].filters.filtypes.elements[i].longname,
				SiderHeder[0].filters.filtypes.elements[i].nrof);
		
		printf("</FILETYPE>\n");

	}		

	char *dateview_type_names[] = { "TODAY",
					"YESTERDAY",
					"LAST_WEEK",
					"LAST_MONTH",
					"THIS_YEAR",
					"LAST_YEAR",
					"TWO_YEARS_PLUS"};

			/*
        			TODAY = 1,
        			YESTERDAY,
        			LAST_WEEK,
        			LAST_MONTH,
        			THIS_YEAR,
        			LAST_YEAR,
        			TWO_YEARS_PLUS,
			*/

	printf("<DATES>\n");
		printf("\t<ALL>0</ALL>\n");
		for (y=0;y<7;y++) {
			if (SiderHeder[0].dates > 0) {
				printf("\t<%s>%i</%s>\n",dateview_type_names[y],SiderHeder[0].dates[y],dateview_type_names[y]);
			}
		}
	printf("</DATES>\n");

	#else

		#ifdef DEBUG
        	printf("|%-10s|%-10s|%-10s||%-10s|%-10s|%-10s|%-18s|%-10s|%-10s|\n",
                	"AllRank",
                	"TermRank",
                	"PopRank",
                	"Body",
                	"Headline",
                	"Tittel",
                	"Athor (nr)",
                	"UrlM",
                	"Url"
                );
        	printf("|----------|----------|----------||----------|----------|----------|------------------|----------|----------|\n");

                for(i=0;i<FinalSiderHeder.showabal;i++) {
                        printf("|%10i|%10i|%10i||%10i|%10i|%10i|%10i (%5i)|%10i|%10i| %s\n",

				Sider[i].iindex.allrank,
                                Sider[i].iindex.TermRank,
                                Sider[i].iindex.PopRank,

                                Sider[i].iindex.rank_explaind.rankBody,
                                Sider[i].iindex.rank_explaind.rankHeadline,
                                Sider[i].iindex.rank_explaind.rankTittel,
                                Sider[i].iindex.rank_explaind.rankAthor,
                                Sider[i].iindex.rank_explaind.nrAthor,
                                Sider[i].iindex.rank_explaind.rankUrl_mainbody,
                                Sider[i].iindex.rank_explaind.rankUrl,
                                Sider[i].DocumentIndex.Url
                                );
                }

		#endif
	#endif

		//skal printe ut FinalSiderHeder.showabal sider, men noen av sidene kan være slettet
	i=0;
	x=0;


	while ((x<FinalSiderHeder.showabal) && (i < (QueryData.MaxsHits * (nrOfServers + nrOfPiServers)))) {
		
		if (!Sider[i].deletet) {

			dprintf("r %i, a: %i, bid : %f, u: %s. DocID: %u\n",Sider[i].iindex.allrank,Sider[i].DocumentIndex.AdultWeight,Sider[i].bid,Sider[i].url,Sider[i].iindex.DocID);

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

			//finner vid
        		vid_u(vidbuf,sizeof(vidbuf),salt,Sider[i].iindex.DocID,etime,QueryData.userip);
			printf("\t<VID>%s</VID>\n",vidbuf);


                	printf("\t<DOCUMENTLANGUAGE>%s</DOCUMENTLANGUAGE>\n", documentlangcode);
                	//temp: blir rare tegn her              
			printf("\t<DOCUMENTTYPE>%s</DOCUMENTTYPE>\n", Sider[i].DocumentIndex.Dokumenttype);

                	printf("\t<POSISJON>%i</POSISJON>\n",x);
                	printf("\t<REPOSITORYSIZE>%u</REPOSITORYSIZE>\n",Sider[i].DocumentIndex.htmlSize);


			if (!getRank) {
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
			}



			printf("\t<CRC32>%u</CRC32>\n",Sider[i].DocumentIndex.crc32);

			//ser ikke ut til at vi teller den
			//printf("\t<PAGEGENERATETIME>%f</PAGEGENERATETIME>\n",Sider[i].pageGenerateTime);

               		printf("\t<TERMRANK>%i</TERMRANK>\n",Sider[i].iindex.TermRank);

               		printf("\t<POPRANK>%i</POPRANK>\n",Sider[i].iindex.PopRank);
       	        	printf("\t<ALLRANK>%i</ALLRANK>\n",Sider[i].iindex.allrank);

                	printf("\t<NROFHITS>%i</NROFHITS>\n",Sider[i].iindex.TermAntall);
                	//printer ut hits (hvor i dokumenetet orde befinner seg ).
			/*
                	printf("\t<HITS>");
                	for (y=0; (y < Sider[i].iindex.TermAntall) && (y < MaxTermHit); y++) {
                	        printf("%hu ",Sider[i].iindex.hits[y]);
                	}
                	printf("</HITS>\n");
			*/

			printf("\t<RESULT_COLLECTION>%s</RESULT_COLLECTION>\n",Sider[i].subname.subname);


			#ifdef BLACK_BOKS
				char timebuf[26];
				printf("\t<TIME_UNIX>%u</TIME_UNIX>\n",Sider[i].DocumentIndex.CrawleDato);
				ctime_r((time_t *)&Sider[i].DocumentIndex.CrawleDato,timebuf);
				timebuf[24] = '\0';
				printf("\t<TIME_ISO>%s</TIME_ISO>\n",timebuf);


				//sender en tom cashe link. Må ha cashe link hvis ikke bryter vi designet
	                	printf("\t<CACHE></CACHE>\n");

			#else
				
	                	printf("\t<DOMAIN>%s</DOMAIN>\n",Sider[i].domain);

				//finer om forige treff hadde samme domene
				if (i>0 && (lastdomain != NULL) && (strcmp(Sider[i].domain,lastdomain) == 0)) {			
		                	printf("\t<DOMAIN_GROUPED>TRUE</DOMAIN_GROUPED>\n");
				}
				else {
		                	printf("\t<DOMAIN_GROUPED>FALSE</DOMAIN_GROUPED>\n");

				}
				// ikke 100% riktig dette, da vi vil få problemer med at ppc reklame får samme side kan 
				// være siste, og da blir treff 1 rykket inn
				lastdomain = Sider[i].domain;

				printf("\t<SERVERNAME>%s</SERVERNAME>\n",Sider[i].servername);

	                	printf("\t<ADULTWEIGHT>%hu</ADULTWEIGHT>\n",Sider[i].DocumentIndex.AdultWeight);
	                	printf("\t<METADESCRIPTION><![CDATA[]]></METADESCRIPTION>\n");
	                	printf("\t<CATEGORY></CATEGORY>\n");
	                	printf("\t<OFFENSIVE_CODE>FALSE</OFFENSIVE_CODE>\n");


				ipaddr.s_addr = Sider[i].DocumentIndex.IPAddress;

                		printf("\t<IPADDRESS>%s</IPADDRESS>\n",inet_ntoa(ipaddr));

                		printf("\t<RESPONSE>%hu</RESPONSE>\n",Sider[i].DocumentIndex.response);

				printf("\t<CRAWLERVERSION>%f</CRAWLERVERSION>\n",Sider[i].DocumentIndex.clientVersion);
				printf("\t<HTMLPREPARSED>%i</HTMLPREPARSED>\n",Sider[i].HtmlPreparsed);

	                	printf("\t<CACHE>%s</CACHE>\n",Sider[i].cacheLink);

	                	//printf("\t<PATHLEN>%u</PATHLEN>\n",(unsigned int)Sider[i].pathlen);


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
			dprintf("nr %i er deletet. Rank %i\n",i,Sider[i].iindex.allrank);
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

	//30 mai 2007
	//ser ut til å skape problemer når vi har cashed verdier her
	if ((!hascashe) && (!hasprequery)) {
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
	}

	//if ((LOGFILE = bfopen("logs/query.log","a")) == NULL) {
	if ((LOGFILE = fopen("/home/boitho/var/logs/query.log","a")) == NULL) {
		perror(bfile("logs/query.log"));
	}
	else {
		flock(fileno(LOGFILE),LOCK_EX);
        	fprintf(LOGFILE,"%s %i %f\n",queryNodeHeder.query,FinalSiderHeder.TotaltTreff,FinalSiderHeder.total_usecs);
        	fclose(LOGFILE);
	}

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	dprintf("Time debug: end clean up %f\n",getTimeDifference(&start_time,&end_time));
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
	else if (!QueryData.filterOn) {

	}
	//skriver bare cashe hvis vi fikk svar fra all servere, og vi var ikke ute etter ranking
	else if (getRank == 0 && pageNr > 0 && nrRespondedServers == nrOfServers) {
		if (dispconfig.writeprequery) {
			if (!cache_write(prequeryfile, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, pageNr)) {
				//fprintf(stderr, "Prequery file: %s\n",prequeryfile);
				perror("cache_write");
			}
		}
		else if (!cache_write(cachepath, &pageNr, &FinalSiderHeder, SiderHeder, maxServers, Sider, pageNr)) {
			//fprintf(stderr, "Cache file: %s\n", cachepath);
			perror("cache_write");
		}
	}
	#endif


	/********************************************************************************************/
	//mysql logging
	/********************************************************************************************/
#if MYSQLFOUR
	#ifndef NO_LOGING
		MYSQL_STMT *logstmt, *pilogstmt;
		dprintf("Connecting to mysql db\n");

		mysql_init(&demo_db);

		#ifndef BLACK_BOKS
			if(!mysql_real_connect(&demo_db, "localhost", "boitholog", "G7J7v5L5Y7", "boitholog", 3306, NULL, 0)){
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

			MYSQL_BIND bind[12];
			unsigned long len[12];
			memset(bind, 0, sizeof(bind));
			logstmt = mysql_stmt_init(&demo_db);
			pilogstmt = mysql_stmt_init(&demo_db);

			sprintf(query,"INSERT DELAYED INTO search_logg (tid,query,search_bruker,treff,search_tid,ip_adresse,betaler_keywords_treff,HTTP_ACCEPT_LANGUAGE,HTTP_USER_AGENT,HTTP_REFERER,GeoIPLang) VALUES(NOW(),?,?,?,?,?,?,?,?,?,?)");
			mysql_stmt_prepare(logstmt, query, strlen(query));
			bind[0].buffer_type = MYSQL_TYPE_STRING; // query
			bind[0].buffer = QueryData.query;
			len[0] = strlen(QueryData.query);
			bind[0].length = &len[0];

			bind[1].buffer_type = MYSQL_TYPE_STRING; // user
			bind[1].buffer = QueryData.search_user;
			len[1] = strlen(QueryData.search_user);
			bind[1].length = &len[1];

			bind[2].buffer_type = MYSQL_TYPE_LONG; // treff
			bind[2].buffer = &FinalSiderHeder.TotaltTreff;

			bind[3].buffer_type = MYSQL_TYPE_FLOAT; // sÃketid
			bind[3].buffer = &FinalSiderHeder.total_usecs;

			bind[4].buffer_type = MYSQL_TYPE_STRING; // ip
			bind[4].buffer = QueryData.userip;
			len[4] = strlen(QueryData.userip);
			bind[4].length = &len[4];

			bind[5].buffer_type = MYSQL_TYPE_LONG; // betaler
			bind[5].buffer = &totlaAds;
			
			bind[6].buffer_type = MYSQL_TYPE_STRING; // http lang
			bind[6].buffer = QueryData.HTTP_ACCEPT_LANGUAGE;
			len[6] = strlen(QueryData.HTTP_ACCEPT_LANGUAGE);
			bind[6].length = &len[6];

			bind[7].buffer_type = MYSQL_TYPE_STRING; // http user agent
			bind[7].buffer = QueryData.HTTP_USER_AGENT;
			len[7] = strlen(QueryData.HTTP_USER_AGENT);
			bind[7].length = &len[7];

			bind[8].buffer_type = MYSQL_TYPE_STRING; // http referer
			bind[8].buffer = QueryData.HTTP_REFERER;
			len[8] = strlen(QueryData.HTTP_REFERER);
			bind[8].length = &len[8];

			bind[9].buffer_type = MYSQL_TYPE_STRING; // geoip
			bind[9].buffer = QueryData.GeoIPcontry;
			len[9] = strlen(QueryData.GeoIPcontry);
			bind[9].length = &len[9];


			mysql_stmt_bind_param(logstmt, bind);

			mysql_stmt_execute(logstmt);
			mysql_stmt_close(logstmt);

			//mysql_free_result(mysqlres);

			//lopper gjenom og logger Paid Inclusion
			x = 0;
			i = 0;			
			sprintf(query,"INSERT DELAYED INTO search_logg (tid,query,search_bruker,treff,search_tid,ip_adresse,betaler_keywords_treff,HTTP_ACCEPT_LANGUAGE,HTTP_USER_AGENT,HTTP_REFERER,GeoIPLang,spot,piDocID) VALUES(NOW(),?,?,?,?,?,?,?,?,?,?,?,?)");
			mysql_stmt_prepare(pilogstmt, query, strlen(query));
			mysql_stmt_bind_param(pilogstmt, bind);
			while ((x<FinalSiderHeder.showabal) && (i < (QueryData.MaxsHits * (nrOfServers + nrOfPiServers)))) {
		
				if (!Sider[i].deletet) {
					//dprintf("pi analyse. Subname \"%s\", pi \"%i\"\n",Sider[i].subname.subname, (int)Sider[i].subname.config.isPaidInclusion);

					if (Sider[i].subname.config.isPaidInclusion) {
						unsigned int spot;

						spot = x + (QueryData.start * QueryData.MaxsHits);
								
						bind[10].buffer_type = MYSQL_TYPE_LONG ; // spot 
						bind[10].buffer = &spot;
						bind[10].is_unsigned = 1; 

						bind[11].buffer_type = MYSQL_TYPE_LONG; // piDocID
						bind[11].buffer = &Sider[i].iindex.DocID;
						bind[11].is_unsigned = 1; 


						mysql_stmt_execute(pilogstmt);

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

			mysql_close(&demo_db);
		}
	#endif

#else /* MYSQLFOUR */

	#ifndef NO_LOGING
		dprintf("Connecting to mysql db\n");

		mysql_init(&demo_db);

		#ifndef BLACK_BOKS
			//if(!mysql_real_connect(&demo_db, "localhost", "boitho", "G7J7v5L5Y7", "boithoweb", 3306, NULL, 0)){
			if(!mysql_real_connect(&demo_db, dispconfig.webdb_host, dispconfig.webdb_user, dispconfig.webdb_password, dispconfig.webdb_db, 3306, NULL, 0)){
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
			mysql_real_escape_string(&demo_db,queryEscaped,QueryData.query,strlen(QueryData.query));


			//logger til mysql
			sprintf(query,"insert DELAYED into search_logg values(NULL,NOW(),\"%s\",\"%s\",\"%i\",\"%f\",\"%s\",\"1\",\"%i\",\"%s\",\"%s\",\"%s\",\"%s\")",
				queryEscaped,QueryData.search_user,
				FinalSiderHeder.TotaltTreff,
				FinalSiderHeder.total_usecs,
				QueryData.userip,
				totlaAds,
				QueryData.HTTP_ACCEPT_LANGUAGE,
				QueryData.HTTP_USER_AGENT,
				QueryData.HTTP_REFERER,
				QueryData.GeoIPcontry
			);

			mysql_real_query(&demo_db, query, strlen(query));

			//mysql_free_result(mysqlres);

			//lopper gjenom og logger Paid Inclusion
			#ifdef DEBUG
			printf("looking for Paid Inclusion\n");
			#endif
			x = 0;
			i = 0;			
			
			while ((x<FinalSiderHeder.showabal) && (i < (QueryData.MaxsHits * (nrOfServers + nrOfPiServers)))) {
		
				if (!Sider[i].deletet) {
					#ifdef DEBUG
					printf("pi analyse. Subname \"%s\", pi \"%i\"\n",Sider[i].subname.subname, (int)Sider[i].subname.config.isPaidInclusion);
					#endif

					if (Sider[i].subname.config.isPaidInclusion) {
					

						strscpy(query,Sider[i].subname.config.sqlImpressionsLogQuery,sizeof(query));
						strsandr(query,"$DocID",utoa(Sider[i].iindex.DocID));

						strsandr(query,"$query",queryEscaped);
						strsandr(query,"$hits",bitoa(FinalSiderHeder.TotaltTreff) );
						strsandr(query,"$time",ftoa(FinalSiderHeder.total_usecs));
						strsandr(query,"$ipadress",QueryData.userip);
						strsandr(query,"$spot",bitoa(x + (QueryData.start * QueryData.MaxsHits)));

						#ifdef DEBUG
						printf("query \"%s\"\n",query);
						#endif

						mysql_real_query(&demo_db, query, strlen(query));

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

			mysql_close(&demo_db);
		}
	#endif



#endif /* MYSQLFOUR */

	/********************************************************************************************/

	free(Sider);

  	/* Free the configuration */
	config_destroy(&cfg);
	maincfgclose(&maincfg);

//	fprintf(stderr,"dispatcher_all: done\n");

	free(SiderHeder);
	free(AddSiderHeder);

	//hvis vi har web system kan vi ha flere servere, og de er da som en char **
	#ifndef BLACK_BOKS
		for(i=0;i<nrOfServers;i++) {
			free(servers[i]);
		}
		for(i=0;i<nrOfPiServers;i++) {
			free(piservers[i]);
		}
		for(i=0;i<nrOfAddServers;i++) {
			free(addservers[i]);
		}


		free(servers);
		free(piservers);
		free(addservers);
	#endif

	//må vi tvinge en buffer tømming ???
	printf("\n\n");	

	//#ifdef WITH_PROFILING
	//	}
	//#endif
        return EXIT_SUCCESS;
} 


int compare_elements (const void *_p1, const void *_p2) {
	const struct SiderFormat *p1, *p2;

	p1 = _p1;
	p2 = _p2;

	if (p1->type != p2->type) {
                //sortering på type, slik at ppc kommer sammen, og først
                //printf("type %i != %i\n",p1->type,p2->type);
                if (p1->type > p2->type)
                        return -1;
                else
                        return p1->type < p2->type;
        }
        else if (p1->iindex.allrank > p2->iindex.allrank)
                return -1;
        else
                return p1->iindex.allrank < p2->iindex.allrank;

}
int compare_elements_posisjon (const void *_p1, const void *_p2) {
	const struct SiderFormat *p1, *p2;

	p1 = _p1;
	p2 = _p2;

	if (p1->type != p2->type) {
		//sortering på type, slik at ppc kommer sammen, og først. Må så sorteres etter pris og relevans
		//printf("type %i != %i\n",p1->type,p2->type);
		if (p1->type > p2->type)
			return -1;
		else
			return p1->type < p2->type;
	}
	/*
	else if (p1->subname.config.isPaidInclusion || p2->subname.config.isPaidInclusion) {
			if ( p2->subname.config.isPaidInclusion) {
				return 1;
			}
			else {
				return -1;
			}
	}
	*/
	//hvis vi har en normal side, og har forskjelig path lengde
	else if (p1->type == siderType_normal) {
		if (p1->posisjon == p2->posisjon ){

			/*
			//printf("a: %s (%i) - %s (%i)\n",p1->DocumentIndex.Url,p1->pathlen,p2->DocumentIndex.Url,p2->pathlen);
			if (p1->pathlen == p2->pathlen) {
				if (p1->iindex.allrank > p2->iindex.allrank) {
                        		return -1;
                		}
                		else {
                	        	return p1->iindex.allrank < p2->iindex.allrank;
        	        	}			
	
			}
			else if (p1->pathlen < 
				p2->pathlen ) {
				return -1;
			}
			else {
				return 1;
			}	
			*/	
			if (p1->iindex.allrank > p2->iindex.allrank) {
                        	return -1;
                	}
                	else {
                        	return p1->iindex.allrank < p2->iindex.allrank;
                	}			

		}
        	else if (p1->posisjon < p2->posisjon)
        	        return -1;
        	else {
        	        return p1->posisjon > p2->posisjon;
		}
	}
	else {
	//for ppc og ppc_side sider. Sorterer først på bud, så pr relevans
		if (p1->bid == p2->bid) {
			
			if (p1->iindex.allrank > p2->iindex.allrank) {
                        	return -1;
                	}
                	else {
                        	return p1->iindex.allrank < p2->iindex.allrank;
                	}			
		}
		else if (p1->bid > p2->bid) {
                        return -1;
		}
                else {
                        return p1->bid < p2->bid;
                }
	}
}


