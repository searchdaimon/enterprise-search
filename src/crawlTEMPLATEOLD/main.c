#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../bbdocument/bbdocument.h"

void recursiveDir (char collection[], char dirname[]);

main (int argc, char *argv[]) {

	if (argc != 3) {
		printf("usage ./crawlSMB colectione dir\n");
	}

	bbdocument_init();

	recursiveDir(argv[1],argv[2]);
}


void recursiveDir (char collection[], char dirname[]) {

	DIR *DIRH;
	struct dirent *dp;
	unsigned int lastmodified;

	char path[PATH_MAX];

	if ((DIRH = opendir(dirname)) == NULL) {
                perror(dirname);
        }

        while ((dp = readdir(DIRH)) != NULL) {

		sprintf(path,"%s/%s",dirname,dp->d_name);
		printf("%s\n",path);
		if (dp->d_name[0] == '.') {
			//do noting
                }
		else if (dp->d_type == DT_DIR) {
			recursiveDir(collection,path);
		}
		else if (dp->d_type == DT_REG) {
			//for alle filer
			//kal stat eller noe for å få den.
			lastmodified = 0;
			//int bbdocument_exist(char subname[],char documenturi[],unsigned int lastmodified);
			if (!bbdocument_exist(collection,path,lastmodified)) {

				//opne og les filen. Sende den så inn med bbdocument_add();
				//int bbdocument_add(char subname[],char documenturi[],char documenttype[],char document[],int dokument_size,unsigned int lastmodified,char *acl[])			

				
			}

		}

	}	

}
