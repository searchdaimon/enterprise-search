#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>

#include "../common/define.h"
#include "../common/reposetoryNET.h"
#include "../common/lotlist.h"


int main (int argc, char *argv[]) {


	off_t size;
	int i;
	char servername[64];

        if (argc != 4) {
                printf("Usage: ./listLostLots subname start stop\n");
                exit(1);
        }


        char *subname = argv[1];
	int start = atoi(argv[2]);
	int stop = atoi(argv[3]);

	for (i=start;i<=stop;i++) {
		size =  rGetFileSize("reposetory", i,subname);

		if (size < 1073741824) {
			lotlistGetServer(servername, i);

			printf("%i size %" PRId64 ", server %s \n",i,size,servername);
		}
	}

	

}
