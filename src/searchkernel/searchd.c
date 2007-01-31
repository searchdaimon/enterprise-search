#include "searchkernel.h"

#include "../common/poprank.h"
#include "../common/adultWeight.h"
#include "../common/daemon.h"
#include "../acls/acls.h"
#include "../boithoadClientLib/liboithoaut.h"
#include "../common/timediff.h"
#include <sys/time.h>


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

#ifdef WITH_PROFILING
	static int profiling_runcount = 0;
#endif

int main(int argc, char *argv[])
{
	int 	sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;
	FILE *LOGFILE;
	FILE *LOCK;


	/***********************************************************************************/
	//prøver å få fil lock. Bare en deamon kan kjøre avgangen

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
	/***********************************************************************************/
	

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

	strncpy(servername,argv[1],sizeof(servername) -1);

	printf("servername %s\n",servername);

	//ToDo: må ha låsing her
        if ((LOGFILE = fopen("/home/boitho/config/query.log","a")) == NULL) {
                perror("logfile");
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

	IIndexInaliser();

	#ifdef WITH_MEMINDEX
		IIndexLoad();
	#endif

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fprintf(stderr,"server: can't open stream socket\n"), exit(0);




	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(BSDPORT);

	//seter at sokket kan rebrukes
        int yes=1;
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr,"server: can't bind local address. Port %i\n",BSDPORT);
		exit(0);
	}	

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
				pthread_create(&chld_thr, NULL, do_chld, (void *) newsockfd);
				/* the server is now free to accept another socket request */
			#else
				do_chld((void *) newsockfd);	
			#endif
		}

	}
	
	return(0);
}


/* 
	This is the routine that is executed from a new thread 
*/
void *do_chld(void *arg)
{
	FILE *LOGFILE;
	int 	mysocfd = (int) arg;
	char 	data[100];
	char **Data;
	int Count;

	#ifdef DEBUG
	struct timeval start_time, end_time;
	#endif


	int nrOfSubnames;
	//struct subnamesFormat subnames[10];
	struct subnamesFormat *subnames;

	int 	i,n;
	struct queryNodeHederFormat queryNodeHeder;
	struct SiderFormat *Sider;
	int net_status;

	//struct SiderFormat Sider[MaxsHits * 2];

	struct SiderHederFormat SiderHeder;
	
	#ifdef WITH_THREAD
		printf("Child thread [%d]: Socket number = %d\n", pthread_self(), mysocfd);
	#else
		printf("Socket number = %d\n",mysocfd);
	#endif


	/* read from the given socket */

	if ((i=recv(mysocfd, &queryNodeHeder, sizeof(queryNodeHeder),MSG_WAITALL)) == -1) {
		perror("recv");
	}

	//sender svar med en gang at vi kan gjøre dette
	net_status = net_CanDo;
	if ((n=sendall(mysocfd,&net_status, sizeof(net_status))) != sizeof(net_status)) {
		printf("send only %i of %i\n",n,sizeof(net_status));
		perror("sendall net_status");
	}


	printf("MaxsHits %i\n",queryNodeHeder.MaxsHits);
	Sider  = (struct SiderFormat *)malloc(sizeof(struct SiderFormat) * (queryNodeHeder.MaxsHits));


	//ToDo: må ha låsing her
        if ((LOGFILE = fopen("/home/boitho/config/query.log","a")) == NULL) {
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

	printf("subname \"%s\"\n",queryNodeHeder.subname);

	//dekoder subname

	Count = split(queryNodeHeder.subname, ",", &Data);

	subnames = malloc(sizeof(struct subnamesFormat) * Count);

  	Count = 0;
	nrOfSubnames = 0;
	printf("nrOfSubnames %i\n",nrOfSubnames);
  	while( (Data[Count] != NULL) && (nrOfSubnames < MAX_COLLECTIONS)) {
    		printf("\t\taa: %d\t\"%s\"\n", Count, Data[Count]);

		//tar ikke med tomme subnames (som bare er en \0)
		if (Data[Count][0] != '\0') {
	    		printf("\t\taa: added : %d\t\"%s\" (len %i)\n", Count, Data[Count],strlen(Data[Count]));

			strscpy(subnames[nrOfSubnames].subname,Data[Count],sizeof(subnames[nrOfSubnames].subname));

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

	//v3 dosearch(queryNodeHeder.query, strlen(queryNodeHeder.query),Sider,&SiderHeder,SiderHeder.hiliteQuery,servername,subnames,SiderHeder.nrOfSubnames,queryNodeHeder.MaxsHits,queryNodeHeder.start, queryNodeHeder.filterOn, queryNodeHeder.languageFilter);
	dosearch(queryNodeHeder.query, strlen(queryNodeHeder.query),Sider,&SiderHeder,SiderHeder.hiliteQuery,
			servername,subnames,nrOfSubnames,queryNodeHeder.MaxsHits,
			queryNodeHeder.start, queryNodeHeder.filterOn, 
			"",queryNodeHeder.orderby,SiderHeder.dates,queryNodeHeder.search_user);

	//kopierer inn subnames. Kan bare sende over MAX_COLLECTIONS, men søker i alle

	for (i=0;((i<MAX_COLLECTIONS) && (i<nrOfSubnames));i++) {
		//memcpy(SiderHeder.subnames[i],subnames[i],sizeof(struct subnamesFormat));
		SiderHeder.subnames[i] = subnames[i];
	}
	SiderHeder.nrOfSubnames = i--;

	printf("subnames:\n");
	for (i=0;i<SiderHeder.nrOfSubnames;i++) {
		printf("%s: %i\n",SiderHeder.subnames[i].subname,SiderHeder.subnames[i].hits);
	}
	printf("\n");

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

	if ((n=sendall(mysocfd,&SiderHeder, sizeof(SiderHeder))) != sizeof(SiderHeder)) {
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

	
	for(i=0;i<SiderHeder.showabal;i++) {
	//for (i=0;i<queryNodeHeder.MaxsHits;i++) {
		//if (!Sider[i].deletet) {		
			//printf("sending %s, deletet %i\n",Sider[i].DocumentIndex.Url,Sider[i].deletet);	
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

		#ifdef WITH_PROFILING
			if (profiling_runcount >= 0) {
				printf("exiting to do profiling. Hav done %i runs\n",profiling_runcount);
				sleep(1);
				exit(1);
			}
			printf("hav runed %i times\n",profiling_runcount);
			++profiling_runcount;
		#endif

}

