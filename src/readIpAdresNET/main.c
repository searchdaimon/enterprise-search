#include "../common/daemon.h"
#include "../common/define.h"
#include "../common/reposetoryNET.h"



int main (int argc, char *argv[]) {

	unsigned int DocID;
	unsigned long int IPAddress = 0;
	int start,stopp;
        if (argc < 4) {
                printf("Ingen hostname gitt eller lot gitt\n\n\tBruk:\nboitholdTest HostName lotnr");
                exit(0);
        }

        printf("conek ferdig \n");

	start = atoi(argv[2]);
	stopp = atoi(argv[3]);

	for (DocID = start; DocID<stopp;DocID++) {
		IPAddress = DIGetIp(argv[1],DocID);
	
		if (IPAddress != 0) {
			printf("DocID %u IPAddress %u\n",DocID,IPAddress);
		}
	}

        //close(socket);
        exit(0);
}

