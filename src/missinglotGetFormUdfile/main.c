#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>

#include "../common/define.h"



main (int argc, char *argv[]) {

        char lotpath[512];
        int lotNr;
        int i;

        FILE *REPOSETORY;

        if (argc < 3) {
               printf("Dette programet går gjenom en udfile og finner lots som mangler\n\n");
               exit(0);
        }


        char *hostname = argv[1];
        char *subname = argv[2];

	struct udfileFormat udpost;

	while (fread(&udpost,sizeof(udpost),1,stdin) > 0) {
		printf("DocID %u, Url \"%s\"\n",updost.DocID,udpost.url);
	}
}
