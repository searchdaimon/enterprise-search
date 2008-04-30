
#include "daemon.h"
#include <unistd.h>
#include <fcntl.h>

#define CANT_IGNORE_SIGCHLD 1;

const int TIMEOUT = 15; /* or whatever... */ 

    /*
    ** server.c -- a stream socket server liabery
    ** Mya av koden er hentet fra http://www.ecst.csuchico.edu/~beej/guide/net/html/
    */


void sigchld_handler(int sig) {
        //while(wait(NULL) > 0){
	//	printf("waiting\n");
	//};
	int s;
    	while (waitpid(-1,&s,WNOHANG) > 0)
     continue;

}

int socketsendsaa(int socketha,char **respons_list[],int nrofresponses) {

	int i,len;	

	fprintf(stderr, "daemon: socketsendsaa(socket=%i)\n", socketha);
	printf("socketsendsaa: nr %i\n",nrofresponses);

	sendall(socketha,&nrofresponses, sizeof(int));

	for (i=0;i<nrofresponses;i++) {
		len = (strlen((*respons_list)[i]) +1); //+1 da vi sender \0 ogsaa

		if (!sendall(socketha,&len,sizeof(len))) {
			perror("sendall");
			return 0;
		}

		if (!sendall(socketha,(*respons_list)[i],len)) {
			perror("sendall");
			return 0;
		}
	}

	return 1;
}

//motar en enkel respons liste. Den begynner med en int som sier hov lang den er
int socketgetsaa(int socketha,char **respons_list[],int *nrofresponses) {

	fprintf(stderr, "daemon: socketgetsaa(socket=%i)\n", socketha);
        //char ldaprecord[MAX_LDAP_ATTR_LEN];
        int intresponse,i,len;

        if (!recvall(socketha,&intresponse,sizeof(intresponse))) {
                return 0;
        }

        #ifdef DEBUG
        printf("nr of elements %i\n",intresponse);
        #endif

        (*respons_list) = malloc((sizeof(char *) * intresponse) +1);
        (*nrofresponses) = 0;

        for (i=0;i<intresponse;i++) {
		//laster ned hvor lang den er
		if (!recvall(socketha,&len,sizeof(len))) {
                	return 0;
        	}
		(*respons_list)[(*nrofresponses)] = malloc(len +1);

                if (!recvall(socketha,(*respons_list)[(*nrofresponses)],len)) {
                        return 0;
                }

                #ifdef DEBUG
                printf("record \"%s\"\n",(*respons_list)[(*nrofresponses)]);
                #endif


                ++(*nrofresponses);
        }

        (*respons_list)[(*nrofresponses)] = '\0';

	return 1;
}

//rutine som binder seg til PORT og kaller sh_pointer hver gang det kommer en ny tilkobling
int sconnect (void (*sh_pointer) (int), int PORT) {

        int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
        struct sockaddr_in my_addr;    // my address information
        struct sockaddr_in their_addr; // connector's address information
        socklen_t sin_size;
        struct sigaction sa;
        int yes=1;

	fprintf(stderr, "daemon: sconnect(port=%i)\n", PORT);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

	#ifdef DEBUG
	printf("will listen on port %i\n",PORT);
        #endif

        my_addr.sin_family = AF_INET;         // host byte order
        my_addr.sin_port = htons(PORT);     // short, network byte order
        my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
        memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

        if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
	    fprintf(stderr,"Can't bind to port %i\n",PORT);
            perror("bind");
            exit(1);
        }

        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }

	
	// http://groups.google.com/groups?hl=no&lr=&c2coff=1&rls=GGLD,GGLD:2004-19,GGLD:en&selm=87iuv0b7td.fsf%40erlenstar.demon.co.uk&rnum=2
	// bare ignorerer signalet og lar barne dø hvis dette er ok. På rare platformer kan vi få problemer
	// må i såfalt gjøre om flagget
	#ifdef CANT_IGNORE_SIGCHLD
		//sliter met at denne ikke fungerer. Må fikses om vi skal kjøre på andre plattformer
		printf("CANT_IGNORE_SIGCHLD: on\n");
		sa.sa_handler = sigchld_handler; // reap all dead processes
		sa.sa_flags = SA_RESTART;		
	#else
		printf("CANT_IGNORE_SIGCHLD: off\n");
	   	sa.sa_handler = SIG_IGN;
	    	sa.sa_flags = 0;
	#endif
	sigemptyset(&sa.sa_mask);
	
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                        perror("sigaction");
                        exit(1);
                }

	printf("bind to port %i ok. Antering accept loop\n",PORT);
        while(1) {  // main accept() loop
            sin_size = sizeof(struct sockaddr_in);
            if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
                perror("accept");
                continue;
            }
            printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
	    #ifdef DEBUG
		printf("runing in debug mode\n");
		#ifdef DEBUG_WITH_FORK
			printf("roning in debug but form mode (DEBUG_WITH_FORK)\n");

	            if (!fork()) { // this is the child process
			printf("Forket to new prosses\n");
	
	                close(sockfd); // child doesn't need the listener
	                sh_pointer(new_fd);

			//if (send(new_fd, "Hello, world!\n", 14, 0) == -1)
	                //    perror("send");
                
			close(new_fd);

			printf("server: closeing connection from %s\n", inet_ntoa(their_addr.sin_addr));

	                exit(0);
	            }
		    close(new_fd);  // parent doesn't need this

		#else
			printf("runing in debug mode. Wont fork. (problematik to us gdb then)\n");
                	sh_pointer(new_fd);
			close(new_fd);
			printf("sconnect: socket closed\n");

		#endif
	    #else
		#ifdef NO_FORK
			printf("Not in debug mode, but Wont fork.\n");
                        sh_pointer(new_fd);
                        close(new_fd);
			printf("sconnect: socket closed\n");

		#else 
		    	printf("runing in normal fork mode\n");

        	    	if (!fork()) { // this is the child process
				printf("Forket to new prosses\n");

        		        close(sockfd); // child doesn't need the listener
        		        sh_pointer(new_fd);

				//if (send(new_fd, "Hello, world!\n", 14, 0) == -1)
        		        //    perror("send");
                
				close(new_fd);

				printf("server: closeing connection from %s\n", inet_ntoa(their_addr.sin_addr));

            		    exit(0);
            		}
	    		close(new_fd);  // parent doesn't need this
		#endif
	    #endif
        }

	fprintf(stderr, "daemon: ~sconnect()\n");
        return 0;
    } 



int cconnect (char *hostname, int PORT) {

        int sockfd;
        //char buf[MAXDATASIZE];
	int sockedmode;
	int Conectet;
        struct hostent *he;
        struct sockaddr_in their_addr; // connector's address information 

	#ifdef DEBUG
	fprintf(stderr, "daemon: cconnect(hostname=\"%s\", port=%i)\n", hostname, PORT);
	#endif

	#ifdef DEBUG
	//extern int errno;
	//int errnosave;
	#endif

        //if (argc != 2) {
        //    fprintf(stderr,"usage: client hostname\n");
        //    exit(1);
        //}

	#ifdef DEBUG
	printf("conecting to %s:%i",hostname,PORT);
	#endif

        if ((he=gethostbyname(hostname)) == NULL) {  // get the host info 
            perror("gethostbyname");
	    fprintf(stderr, "daemon: ~cconnect()\n");
            return(0);
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
	    fprintf(stderr, "daemon: ~cconnect()\n");
            return(0);
        }

        their_addr.sin_family = AF_INET;    // host byte order 
        their_addr.sin_port = htons(PORT);  // short, network byte order 
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	//printf("trying to connect\n");


        //if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        //    perror("connect");
        //    exit(1);
        //}

	/* Set sockfd to non-blocking mode. */ 
	
	sockedmode = fcntl (sockfd, F_GETFL, 0); 
	fcntl (sockfd, F_SETFL, sockedmode | O_NONBLOCK); 


	
	Conectet = 0;
	//while (!Conectet) {
	// Establish non-blocking connection server.  
	if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) { 
  	  if (errno == EINPROGRESS) { 
    		struct timeval tv = {TIMEOUT, 0}; 
    		fd_set rd_fds, wr_fds; 
    		FD_ZERO (&rd_fds); 
    		FD_ZERO (&wr_fds); 
    		FD_SET (sockfd, &wr_fds); 
    		FD_SET (sockfd, &rd_fds); 


    		// Wait for up to TIMEOUT seconds to connect. 
    		if (select (sockfd + 1, &rd_fds, &wr_fds, 0, &tv) <= 0) {
			#ifdef DEBUG
			//errnosave = errno;
      			//perror ("connection timedout");
			//errno = errnosave;
			#endif
			//close(sockfd); 
			//sleep(5);
	        	fprintf(stderr, "daemon: ~cconnect()\n");
			return 0;
		}
    		// Can use getpeername() here instead of connect(). 
    		else if (connect (sockfd, (struct sockaddr *) &their_addr, sizeof their_addr) == -1 && errno != EISCONN) { 
			#ifdef DEBUG
			//errnosave = errno;
      			//perror (hostname);
			//errno = errnosave;
			#endif
			//close(sockfd); 
			//sleep(5);
			fprintf(stderr, "daemon: ~cconnect()\n");
			return 0;
		}
		else {
			Conectet = 1;
		}
	  } 
	}
	//}
	//setter den tilabke til blokin mode
	fcntl (sockfd, F_SETFL, sockedmode);
	
	//printf("ferdig med å konekte\n");
	//sh_pointer(sockfd);

        //if ((numbytes=recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        //    perror("recv");
        //    exit(1);
        //}

        //buf[numbytes] = '\0';

        //printf("Received: %s",buf);

        //close(sockfd);
	#ifdef DEBUG
        fprintf(stderr, "daemon: ~cconnect(socket=%i)\n", sockfd);
	#endif

	return sockfd;
        //return 0;
} 

int sendall(int s, void *buf, int len) {

        int total = 0;        // how many bytes we've sent
        int bytesleft = len; // how many we have left to send
        int n;
	int tosend;
	
	#ifdef DEBUG
	//to noisy
	//printf("sendall: will send %i b\n",len);
	#endif

        while(total < len) {

	    if (bytesleft > 16392) {
		tosend = 16392;
	    }
	    else {
		tosend = bytesleft;
	    }

            if ((n = send(s, buf+total, tosend, MSG_NOSIGNAL)) == -1) {
            //if ((n = send(s, buf+total, tosend, 0)) == -1) {
            //if ((n = send(s, buf+total, bytesleft, 0)) == -1) {
			//perror("send");
			return 0;
		}

            if (n == -1) { 
		printf("dident manage to send all the data as %s:%f.\n",__FILE__,__LINE__);
		//break; 
		return 0;
	    }
	    #ifdef DEBUG
		//to noisy
		//printf("sendall: sent %i b. %i b left.\n",n,bytesleft);		
	    #endif

            total += n;
            bytesleft -= n;
        }

        //*len = total; // return number actually sent here

        //return n==-1?-1:0; // return -1 on failure, 0 on success
	#ifdef DEBUG
		//to noisy
		//printf("sendall: ending. Will return that we sent %i b\n",total);
	#endif

	return total;
}

int recvall(int sockfd, void *buf, int len) {
	
	//debug("!!!!resiving data");	

/*
	if (recv(sockfd, buf,len,MSG_WAITALL) == -1) {
		return 0;
	}
	else {
		return 1;
	}
*/


	int total = 0;
        int bytesleft = len; // how many we have left to send
	int n;

	#ifdef DEBUG
	printf("will read %i",len);
	#endif

	while(total < len) {

		//runarb: 17.07.2007
		//hum, når vi bruker bloacking i/o så skal det vel bare bli 0 hvis det uikke er mere data og lese? 
		//read skal altid blokke til det er noe data og lese, uanset hvor lang tid det tar
		//if ((n = read(sockfd, buf+total, bytesleft)) == -1) {
		if ((n = read(sockfd, buf+total, bytesleft)) <= 0) {
			return 0;
		}

		#ifdef DEBUG
		printf("recved %i bytes. total red %i, left %i, total to get %i\n",n,total,bytesleft,len);
		#endif

		total += n;
            	bytesleft -= n;
	}

	return total;

}

int sendpacked(int socket,short command, short version, int dataSize, void *data,char subname[]) {

        struct packedHedderFormat packedHedder;
	void *buf;
	size_t len;
	int forret = 0;
        //int i;

	//siden vi skal sende pakken over nettet er det like gått å nullstille all data. Da slipper vi at valgring klager også.
	memset(&packedHedder,0,sizeof(packedHedder));

        //setter sammen hedder
        packedHedder.size       = sizeof(struct packedHedderFormat) + dataSize;
        packedHedder.version    = version;
        packedHedder.command    = command;
	strscpy(packedHedder.subname,subname,sizeof(packedHedder.subname));

	if (data != NULL) {
		len = sizeof(packedHedder) + dataSize;
		if ((buf = malloc(len)) == NULL) {
			printf("sendpacked: malloc");
			return 0;
		}
		memcpy(buf, &packedHedder, sizeof(packedHedder));
		memcpy(buf+sizeof(packedHedder), data, dataSize);
	} else {
		buf = &packedHedder;
		len = sizeof(packedHedder);
	}

	if (!sendall(socket, buf, len)) {
		goto end_error;
	}

	//setter at vi skal returnere 1, som er OK
	forret =  1;

	end_error:
		if (data != NULL) {
			free(buf);
		}
		return forret;
}
