#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../common/crc32.h"
#include "../common/lot.h"
#include "../common/bstr.h"
#include "../getDocIDFromUrl/getDocIDFromUrl.h"

void show (char url[], int n, unsigned int DocID) {


	if (n != 0) {

		printf("DocID: %u\n",DocID);
	}
	else {
		printf("unknown DocID\n");
	}

}

void showOK (char url[], int n) {


	if (n != 0) {

		printf("%-50s [ Found ]\n",url);
	}
	else {
		printf("%-50s [ Not Found ]\n",url);

	}

}


int main (int argc, char *argv[]) {

    	
	char *filename = NULL;
	char *bdbfiles;
	unsigned int DocID;
	int Found, NotFound;

	FILE *FH;
	int i;
	char line[512];
	int n;

	int optShowOK = 0;
 	int optFiles = 0;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
	
        while ((c=getopt(argc,argv,"fk"))!=-1) {
                switch (c) {
                        case 'k':
				optShowOK = 1;
                                break;

                        case 'f':
				optFiles = 1;
                                break;

                        default:
				printf("wron parameter\n");
                        	exit(1);
                }
        }
        --optind;


	if (optFiles != 0) {



		if (argc - optind < 2) {
               		printf("Dette programet viser DocID for urler. Eventuelt urler i text  filer\n\n\tBruke\n\tUrlToDocIDQuery -f bdbfiledir file [ file ..n ]\n\n");
               		exit(0);
       		}

		bdbfiles = argv[1 + optind];

		printf("bdbfile \"%s\"\n",bdbfiles);

		Found = NotFound = 0;
		for (i=optind +2;i<argc;i++) {

			
			filename = argv[i];

			printf("######################################################\n");
			printf(" Filename \"%s\"\n",filename);
			printf("######################################################\n");

			if ((FH = fopen(filename,"r")) == NULL) {
				perror(filename);
				continue;
			}

			while (fgets(line,sizeof(line),FH) != NULL) {

				chomp(line);
		
				url_normalization(line,sizeof(line));

				//printf("url: \"%s\"\n",line);

				n = getDocIDFromUrl(bdbfiles,line,&DocID);


				if (n) {
					++Found;
				}
				else {
					++NotFound;
				}

				if (optShowOK) {
					showOK(line,n);

				}
				else {
					show(line,n,DocID);
				}

			}
	
			close(FH);

			printf("Found %i\nNot Found %i\n\n\n",Found,NotFound);

		}

        }
	else {
		if (argc != 3) {
                	printf("Dette programet indekserer en ud file\n\n\tBruke\n\tUrlToDocIDQuery bdbfiledir url\n\n");
                	exit(0);
        	}

		n = getDocIDFromUrl(argv[1],argv[2],&DocID);

		show(argv[2],n,DocID);

	}
	
}


