/*#include <openssl/ssl.h>*/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

/*#include "sslcommon.h"*/

/*
#define KEYFILE "client.pem"
#define PASSWORD "password"
*/

#define BACKLOG 5

char *bindaddr = NULL;
char *connaddr = NULL;
int bindport = 0;
int connport = 0;
int commbindport = 0;

void
errx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

void
usage(void)
{
	errx("./prog -b bindaddr -B bindport -c connaddr -C connport");
}



void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

#define MAXBUF 2048

void
gotdata(int recvsock, int writesock)
{
	char buf[MAXBUF];
	int nbytes, sbytes;

	if ((nbytes = recv(recvsock, buf, MAXBUF, 0)) <= 0) {
		if (nbytes == 0) {
			close(writesock);
			errx("Server hung up.\n");
		}
		else {
			perror("recv");
			exit(1);
		}
	}

	if (send(writesock, buf, nbytes, 0) == -1) {
		perror("send");
		exit(1);
	}
}

void
dotunneling(int clientsock)
{
	int serversock;
	struct hostent *he;
	struct sockaddr_in remote_addr;
	int maxfd = 0;
	fd_set read_fds;

	FD_ZERO(&read_fds);

	if ((he = gethostbyname(connaddr)) == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	if ((serversock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	remote_addr.sin_family = AF_INET;    // host byte order 
	remote_addr.sin_port = htons(connport);  // short, network byte order 
	remote_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(remote_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	if (connect(serversock, (struct sockaddr *)&remote_addr,
	            sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}

	FD_SET(serversock, &read_fds);
	FD_SET(clientsock, &read_fds);
	maxfd = serversock > clientsock ? serversock : clientsock;

	for (;;) {
		fd_set tmpread_fds = read_fds; /* Ugh */
		if (select(maxfd+1, &tmpread_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		
		if (FD_ISSET(serversock, &tmpread_fds)) {
			// push data to client
			gotdata(serversock, clientsock);
		}
		if (FD_ISSET(clientsock, &tmpread_fds)) {
			// push data to server
			gotdata(clientsock, serversock);
		}
	}
}

void
acceptloop(int sock, int commsock)
{
	socklen_t sin_size;
	struct sockaddr_in remote_addr;
	int new_fd, pid;
	int maxfd;
	fd_set read_fds;

	FD_ZERO(&read_fds);


	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sock, (struct sockaddr *)&remote_addr,
			             &sin_size)) == -1) {
			perror("accept");
			continue;
		}
		printf("server: got connection from %s\n", inet_ntoa(remote_addr.sin_addr));
		if (!(pid = fork())) { // this is the child process
			close(sock); // child doesn't need the listener
			dotunneling(new_fd);
			close(new_fd);
			exit(0);
		}
		if (pid == -1) {
			perror("Could not fork");
		}
		close(new_fd); // parent doesn't need this
	}
}


int
main(int argc, char **argv)
{
	int ch;
	struct sigaction sa;
	struct sockaddr_in my_addr;
	int ssock;
	int yes = 1;

	while ((ch = getopt(argc, argv, "b:B:c:C:")) != -1) {
		switch (ch) {
			case 'b':
			bindaddr = optarg;
			break;
			case 'B':
			if ((bindport = atoi(optarg)) == 0)
				errx("Invalid argument '%s'", optarg);
			break;
			case 'c':
			connaddr = optarg;
			break;
			case 'C':
			if ((connport = atoi(optarg)) == 0)
				errx("Invalid argument '%s'", optarg);
			break;
			default:
			case 'A':
			if ((commbindport = atoi(optarg)) == 0)
				errx("Invalid argument '%s'", optarg);
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (connaddr == NULL || bindport == 0 || connport == 0 ||
	    commbindport == 0)
		usage();

	/* set up child reaper */
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		errx("sigaction, %s", strerror(errno));
		exit(1);
	}


	/* bind server */
	if ((ssock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	my_addr.sin_family = AF_INET;         // host byte order
	my_addr.sin_port = htons(bindport);     // short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(ssock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(ssock, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	acceptloop(ssock);

	return 0;
}

