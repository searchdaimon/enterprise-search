#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>

#include "daemon.h"

#define CANT_IGNORE_SIGCHLD 1;

const int TIMEOUT = 15; /* or whatever... */



int cconnect (char *hostname, int PORT) {

        int sockfd;
        //char buf[MAXDATASIZE];
	int sockedmode;
	int Conectet;
        struct hostent *he;
        struct sockaddr_in their_addr; // connector's address information 

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
            return(0);
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
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
	return sockfd;
        //return 0;
} 

int sendall(int s, void *buf, int len) {

        int total = 0;        // how many bytes we've sent
        int bytesleft = len; // how many we have left to send
        int n;
	int tosend;
	
	#ifdef DEBUG
	printf("will send %i b\n",len);
	#endif

        while(total < len) {

	    if (bytesleft > 16392) {
		tosend = 16392;
	    }
	    else {
		tosend = bytesleft;
	    }

            if ((n = send(s, buf+total, tosend, 0)) == -1) {
            //if ((n = send(s, buf+total, bytesleft, 0)) == -1) {
			//perror("send");
			return 0;
		}
            if (n == -1) { 
		printf("dident manage to send all the data as %s:%d.\n",__FILE__,__LINE__);
		//break; 
		return 0;
	    }
	    #ifdef DEBUG
		printf("sendall: sent %i b. %i b left.\n",n,bytesleft);		
	    #endif

            total += n;
            bytesleft -= n;
        }

        //*len = total; // return number actually sent here

        //return n==-1?-1:0; // return -1 on failure, 0 on success
	#ifdef DEBUG
	printf("sendall: ending. Will return that we sent %i b\n",total);
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
		if ((n = read(sockfd, buf+total, bytesleft)) == -1) {
			return 0;
		}

		if (n == 0)
			return 0;

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
        int i;
        //setter sammen hedder
        packedHedder.size       = sizeof(struct packedHedderFormat) + dataSize;
        packedHedder.version    = version;
        packedHedder.command    = command;
	strcpy(packedHedder.subname,subname);

	//printf("sendpacked: start\n");

        i = sendall(socket, (char*)&packedHedder, sizeof(struct packedHedderFormat));

        //printf("sent %i of %i packedsize: %i\n",i, sizeof(struct packedHedderFormat), packedHedder.size,packedHedder.size);
	//printf("command: %i\n",packedHedder.command);

	//hvi data er null betur det at denne vil bli sent siden, av annen kode
        if (data != NULL) {
		#ifdef DEBUG
		printf("sendpacked: data NOT NULL. Will sendit");
		#endif
                i = send(socket,data,dataSize,0);
        }

	//printf("sendpacked: end\n");

	return 1;
}

