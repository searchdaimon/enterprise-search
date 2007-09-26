#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "define.h"
#include "boithohome.h"


#define local_unknown -1
#define local_true 1
#define local_false 0

struct MemoryLotlistFormat {
	char server[64];
	int local;
	int hasServer;
};

struct MemoryLotlistFormat MemoryLotlist[maxLots];
char overflowServer[64];

void lotlistAdd(char server[], int lot) {
	//printf("adding %s %i\n",server,lot);

	strcpy(MemoryLotlist[lot].server,server);
	MemoryLotlist[lot].hasServer = 1;
}

void lotlistGetServer(char server[], int lot) {
	if (MemoryLotlist[lot].hasServer != 0) {
		strcpy(server,MemoryLotlist[lot].server);
	}
	else {
		strcpy(server,overflowServer);
	}
}

void lotlistMarkLocals(char server[]) {

	int i;

        //inaliserer
        for (i=0;i<maxLots;i++) {

		if ((MemoryLotlist[i].hasServer) && (strcmp(MemoryLotlist[i].server,server) == 0)) {
			//printf("%s == %s\n",MemoryLotlist[i].server,server);
                	MemoryLotlist[i].local = local_true;
          	}
		else {
			MemoryLotlist[i].local = local_false;
		}
        }

}

int lotlistIsLocal(int LorNr) {

	if (MemoryLotlist[LorNr].local == local_true) {
		return 1;
	}
	else {
		return 0;
	}

}

void lotlistLoad() {

	char buff[512];

	FILE *LOTLISTFH;
	char host[512];
	int lotFrom,lotTo,lotNr;
	int lineCount;
	int i;
	char overflowchar;

	//inaliserer
	for (i=0;i<maxLots;i++) {
		MemoryLotlist[i].local = local_unknown;
		MemoryLotlist[i].hasServer = 0;
	}

        //sjekker først om vi har en env variabel kalt "BOITHOLOTLIST". Hvis vi har det så bruker vi den filen
        //gjør det slik slik at vi kan ha lokal lotlist, på hver bbs, man fortsat ha resten likt på alle, og på read$
        if (getenv("BOITHOLOTLIST") != NULL) {
                if ( (LOTLISTFH = fopen(getenv("BOITHOLOTLIST"),"r")) == NULL) {
                        perror(getenv("BOITHOLOTLIST"));
                        exit(1);
                }
        }
	else if ((LOTLISTFH = bfopen("config/lotlist.conf","r")) == NULL) {
		perror(bfile("config/lotlist.conf"));
		exit(1);
	}

	
	lineCount = 0;
	while (fgets(buff,sizeof(buff) -1,LOTLISTFH) != NULL) {
		++lineCount;
		//printf("%i: %s\n",lineCount,buff);

		if ((buff[0] == '\n') || (buff[0] == '#')) {
			//blan linje eller komentar	
		}
		else if (strchr(buff,'*')) {
			if (sscanf(buff,"%s %c\n",host,&overflowchar) != 2) {
                                printf("* bad lotlist format on line %i\n",lineCount);
                                printf("buff %s isnt in format %%s %%c\n",buff);
                                exit(1);
                        }
			if (overflowchar != '*') {
				printf("overflowchar isnt *. At line %i\n",lineCount);
				exit(1);
			}
			//printf("overflov is %s\n",host);
			strcpy(overflowServer,host);

		}
		else if (strchr(buff,':') == NULL) {
			
			if (sscanf(buff,"%s %i\n",host,&lotNr) != 2) { 
                                printf("bad lotlist format on line %i\n",lineCount);
				printf("buff %s isnt in format \%s \%i\n",buff);
                                exit(1);
                        }
			lotlistAdd(host,lotNr);
                        //printf("aa: %s aa: %i\n",host,lotNr);
		}
		else {

			if (sscanf(buff,"%s %i:%i\n",host,&lotFrom,&lotTo) != 3) {
				printf("bad lotlist format on line %i\n",lineCount);
				exit(1);
			}
			for (i=lotFrom;i<lotTo +1;i++) {
				lotlistAdd(host,i);
			}

			//printf("aa: %s aa: %i aa: %i\n",host,lotFrom,lotTo);
		}
	}

	fclose(LOTLISTFH);

}

