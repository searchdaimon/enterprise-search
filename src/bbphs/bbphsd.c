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

#include "../bbph/comm.h"

/*#include "sslcommon.h"*/

/*
#define KEYFILE "client.pem"
#define PASSWORD "password"
*/

#define MAX(a,b) (a>b?a:b)

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
	errx("./prog -A commbindport -b bindaddr -B bindport");
}


int
bind_addr(char *addr, int port, int backlog)
{
	int sock;
	int yes = 1; /* for setsockopt */
	struct sockaddr_in my_addr;

	/* bind comm server */
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return -1;
	}

	my_addr.sin_family = AF_INET;         // host byte order
	my_addr.sin_port = htons(port);     // short, network byte order
	if (addr == NULL) {
		my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	}
	else {
		errx("Does not support this new bind to ip thingy.\n");
	}

	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
	if (bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return -1;
	}

	if (listen(sock, backlog) == -1) {
		perror("listen");
		return -1;
	}
	return sock;
}

int
connect_addr(char *addr, int port)
{
	int sock;
	struct hostent *he;
	struct sockaddr_in remote_addr;
	size_t sin_size;

	if ((he = gethostbyname(addr)) == NULL) {
		perror("gethostbyname");
		return -1;
	}

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

	remote_addr.sin_family = AF_INET;    // host byte order 
	remote_addr.sin_port = htons(port);  // short, network byte order 
	remote_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(remote_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

	if (connect(sock, (struct sockaddr *)&remote_addr,
	            sizeof(struct sockaddr)) == -1) {
		perror("connect");
		return -1;
	}

	return sock;
}



void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

#define MAXBUF 2048

void
pushdata(int recvsock, int writesock)
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

static int
get_port_foo(void)
{
	static int counter = 10000;

	return ++counter;
}

void
dotunneling(int clientsock)
{
	int serversock;
	struct hostent *he;
	struct sockaddr_in remote_addr;
	int maxfd = 0;
	size_t sin_size;
	int new_fd;
	fd_set read_fds;

	if ((serversock = bind_addr(NULL, get_port_foo(), BACKLOG)) == -1) {
		perror("bind_addr");
		return;
	}

	sin_size = sizeof(struct sockaddr_in);
	if ((new_fd = accept(serversock, (struct sockaddr *)&remote_addr,
			     &sin_size)) == -1) {
		perror("accept");
		return;
	}
	printf("commserver: got connection from %s\n",
	       inet_ntoa(remote_addr.sin_addr));

	FD_ZERO(&read_fds);
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
comm_handshake(int sock)
{
	return;
}


void
docomm(int sock)
{
	int cmd;
	int numbytes;

	comm_handshake(sock);

	if ((numbytes = recv(sock, &cmd, sizeof cmd, 0)) == -1) {
		return;
	}

	switch (cmd) {
	case COMM_HELP:
		
		break;
	default:
		fprintf(stderr, "What?!? %x\n", cmd);
		break;
	}

}

void
acceptloop_comm(int commsock)
{
	socklen_t sin_size;
	struct sockaddr_in remote_addr;
	int new_fd, pid;
	int maxfd;

#if 0
	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(daemonsock, &read_fds);
	FD_SET(commsock, &read_fds);
#endif
	for (;;) {
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(daemonsock,
				     (struct sockaddr *)&remote_addr,
				     &sin_size)) == -1) {
			perror("accept");
			continue;
		}
		printf("server: got communication connection from %s\n",
		       inet_ntoa(remote_addr.sin_addr));
		if (!(pid = fork())) { // this is the child process
			close(daemonsock); // child doesn't need the listener
			docomm(new_fd);
			close(new_fd);
			exit(0);
		}
		if (pid == -1) {
			perror("Could not fork");
		}
		close(new_fd); // parent doesn't need this
	}

#if 0
	for (;;) {
		fd_set tmpread_fds = read_fds; /* Ugh */
		if (select((daemonsock>commsock?daemonsock:commsock)+1,
			   &tmpread_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		if (FD_ISSET(daemonsock, &tmpread_fds)) {
			sin_size = sizeof(struct sockaddr_in);
			if ((new_fd = accept(daemonsock,
				             (struct sockaddr *)&remote_addr,
					     &sin_size)) == -1) {
				perror("accept");
				continue;
			}
			printf("server: got connection from %s\n",
			       inet_ntoa(remote_addr.sin_addr));
			if (!(pid = fork())) { // this is the child process
				close(daemonsock); // child doesn't need the listener
				dotunneling(new_fd);
				close(new_fd);
				exit(0);
			}
			if (pid == -1) {
				perror("Could not fork");
			}
			close(new_fd); // parent doesn't need this
		}
		if (FD_ISSET(commsock, &tmpread_fds)) {
			sin_size = sizeof(struct sockaddr_in);
			if ((new_fd = accept(commsock, (struct sockaddr *)&remote_addr,
					     &sin_size)) == -1) {
				perror("accept");
				continue;
			}
			printf("commserver: got connection from %s\n",
			       inet_ntoa(remote_addr.sin_addr));
			if (!(pid = fork())) { // this is the child process
				close(commsock); // child doesn't need the listener
				docomm(new_fd);
				close(new_fd);
				exit(0);
			}
			if (pid == -1) {
				perror("Could not fork");
			}
			close(new_fd); // parent doesn't need this		
		}
	}
#endif
}

int
main(int argc, char **argv)
{
	int ch;
	struct sigaction sa;
	int ssock, commsock;

	while ((ch = getopt(argc, argv, "A:b:B:")) != -1) {
		switch (ch) {
			case 'b':
			bindaddr = optarg;
			break;
			case 'B':
			if ((bindport = atoi(optarg)) == 0)
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


#if 0
	/* bind server */
	if ((ssock = bind_addr(NULL, bindport, BACKLOG)) == -1) {
		perror("bind_addr");
		exit(1);
	}
#endif

	/* bind comm server */
	if ((commsock = bind_addr(NULL, commbindport, BACKLOG)) == -1) {
		perror("bind_addr");
		exit(1);
	}

	acceptloop_comm(commsock);

	return 0;
}

