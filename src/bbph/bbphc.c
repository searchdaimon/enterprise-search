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

#include "comm.h"

/*#include "sslcommon.h"*/

/*
#define KEYFILE "client.pem"
#define PASSWORD "password"
*/

#define MAX(a,b) (a>b?a:b)

#define BACKLOG 5

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
	errx("./prog -c connaddr -C connport");
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

void
comm_handshake(int sock)
{
	return;
}

void
dotunneling(int sock, int outport)
{
	int localsock;
	int maxfd;
	fd_set read_fds;


	if ((localsock = connect_addr("localhost", outport)) == -1) {
		fprintf(stderr, "Unable to connect to thingy... %s\n", strerror(errno));
		return;
	}


	FD_ZERO(&read_fds);
	FD_SET(sock, &read_fds);
	FD_SET(localsock, &read_fds);
	maxfd = sock > localsock ? sock : localsock;

	for (;;) {
		fd_set tmpread_fds = read_fds; /* Ugh */
		if (select(maxfd+1, &tmpread_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}
		
		if (FD_ISSET(sock, &tmpread_fds)) {
			// push data to client
			printf("Sending data to client\n");
			pushdata(sock, localsock);
		}
		if (FD_ISSET(localsock, &tmpread_fds)) {
			// push data to server
			printf("Sending data to server\n");
			pushdata(localsock, sock);
		}
	}



	
}

void
docomm(int sock)
{
	int cmd;
	int numbytes;

	comm_handshake(sock);

	cmd = COMM_HELP;
	if ((numbytes = send(sock, &cmd, sizeof cmd, 0)) == -1) {
		return;
	}

	if ((numbytes = recv(sock, &cmd, sizeof cmd, 0)) == -1) {
		return;
	}
	switch (cmd) {
	case COMM_OK:
		fprintf(stderr, "Help request acked\n");
		break;
	default:
		fprintf(stderr, "What?!? %x\n", cmd);
		close(sock);
		return;
		break;
	}

	/* Wait for data, set up tunneling when asked for a connection */
	for (;;) {
		if ((numbytes = recv(sock, &cmd, sizeof cmd, MSG_WAITALL)) == -1) {
			fprintf(stderr, "Error? %s\n", strerror(errno));
			continue;
		}
		switch(cmd) {
		case COMM_GETCONN: {
			int outport;

			if ((numbytes = recv(sock, &outport, sizeof outport, MSG_WAITALL)) == -1) {
				fprintf(stderr, "Error? %s\n", strerror(errno));
				continue;
			}
			if (numbytes == 0) {
				fprintf(stderr, "Connection closed...\n");
				return;
			}
			printf("Got a request to tunnel local(%d) to remote\n", outport);
			dotunneling(sock, outport);
		}
		
		}

	}

	close(sock);

}

int
main(int argc, char **argv)
{
	int ch;
	int sock;
	int connport = 0;
	char *connaddr = NULL;

	while ((ch = getopt(argc, argv, "c:C:")) != -1) {
		switch (ch) {
			case 'c':
			connaddr = optarg;
			break;
			case 'C':
			if ((connport = atoi(optarg)) == 0)
				errx("Invalid argument '%s'", optarg);
			break;
			default:
				fprintf(stderr, "foo\n");
				return 0;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	//if (connaddr == NULL || bindport == 0 || connport == 0 ||
	if (connport == 0 || connaddr == NULL)
		usage();



	if ((sock = connect_addr(connaddr, connport)) == -1) {
		perror("connect_addr");
		exit(1);
	}

	docomm(sock);

	return 0;
}

