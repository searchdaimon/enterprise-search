#include <stdio.h>
#include <dirent.h>
#include <err.h>
#include <unistd.h>
#include "../common/lot.h"

int main(int argc, char *argv[]) {


/*
	Usage:
		-c Vis antall dokumenter
		-f Vi menesklig lesbart format
		-m vi bare collections som inneholder argumentet.
*/

	int optCount = 0;
	int optFormat = 0;
	char *optmatch = NULL;

	extern char *optarg;
        char c;
        while ((c=getopt(argc,argv,"cfm:"))!=-1) {
                        switch (c) {
                                case 'c':
                                        optCount = 1;
                                        break;
                                case 'f':
                                        optFormat = 1;
                                        break;
                                case 'm':
                                        optmatch = optarg;
                                        break;
                                default:
					exit(-1);
                        }
        }

	if (optFormat == 1) {
		printf("|--------------------------------|------------|\n");	
		printf("| Collection                     | Nr of docs |\n");
		printf("|--------------------------------|------------|\n");	
	
	}

	DIR * dirh = listAllColl_start();
	if (dirh == NULL) 
		err(1, "listAllColl_start()");
	
	char * subname;
	int docCount = 0;
	while ((subname = listAllColl_next(dirh)) != NULL) {

		if ( (optmatch != NULL) && (strstr(subname,optmatch) == NULL) ) {
			continue;
		}

		if (optCount == 1) {
			docCount = rLastDocID(subname);
		}

		if ((optCount == 1) && (optFormat == 1)) {
			printf("| %30s | %10u |\n", subname, docCount);
		}
		else if (optCount == 1) {
			printf("subname: %s, docCount: %u\n", subname, docCount);
		}
		else {
			printf("subname: %s\n", subname);
		}
	}

	listAllColl_close(dirh);

	if (optFormat == 1) {
		printf("|--------------------------------|------------|\n");	
	}

	return 0;
}

