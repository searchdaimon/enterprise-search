#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <stdarg.h>

#include "define.h"

#define BACKLOG 15     // how many pending connections queue will hold

#define MAXDATASIZE 100 // max number of bytes we can get at once

struct packedHedderFormat {
	int size;
	short version;
	short command;
	char subname[maxSubnameLength];
};

//tar inn en peker til en rutine som tar en sokket inn og behandler den
//

int sconnect (void (*sh_pointer) (int), int PORT, int noFork, int breakAfter);
int sconnect_thread(void (*sh_pointer) (int), int port);
int sconnect_simple(void (*sh_pointer) (int), int port);

int cconnect (char *hostname, int PORT);

int sendall(int s, const void *buf, int len);

int recvall(int sockfd, void *buf, int len);


int sendpacked(int socket,short command, short version, int dataSize, void *data, char subname[]);

int socketsendsaa(int socketha,char **respons_list[],int nrofresponses);
int socketgetsaa(int socketha,char **respons_list[],int *nrofresponses);

void wait_loglock(char *name);

int sendallPack(int s, int numargs, ...);
