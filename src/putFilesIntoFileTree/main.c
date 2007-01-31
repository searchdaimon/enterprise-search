#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../common/bstr.h"


void recursiveDir (char fileTree[], char dirname[],unsigned char havemkdirs[]);

int main (int argc, char *argv[]) {

	unsigned char havemkdirs[255];
	int i;

	if (argc != 3) {
		printf("usage: ./putFilesIntoFileTree fileFolder fileTree\n");
		exit(1);
	}

	char *fileFolder = argv[1];
	char *fileTree = argv[2];


	for(i=0;i<255;i++) {
		printf("%i\n",i);
		havemkdirs[i] = 0;
	}

	if(mkdir(fileTree,0755) != 0){
		perror(fileTree);
		exit(1);
	}	

	recursiveDir(fileTree,fileFolder,havemkdirs);	

	return 0;

}

void recursiveDir (char fileTree[], char dirname[], unsigned char havemkdirs[]) {


        DIR *DIRH;
	struct dirent *dp;
	char nextdirname[512];
        char filname[512];
	char treeplace[512];
	char command[512];
	unsigned char bucket;

        printf("opening %s\n",dirname);

        if ((DIRH = opendir(dirname)) == NULL) {
                perror(dirname);
                return;
        }

        while ((dp = readdir(DIRH)) != NULL) {


                if (dp->d_name[0] == '.') {
                        //printf(". domain (\"%s\")\n",dp->d_name);
                }
                else if (dp->d_type == DT_DIR) {


                        sprintf(nextdirname,"%s%s/",dirname,dp->d_name);
                        printf("dir (nextdirname %s)\n",nextdirname);

                        //kaller seg selv rekurift
                        recursiveDir(fileTree,nextdirname,havemkdirs);
                }
                else if (dp->d_type == DT_REG) {
			bucket = (unsigned char)btolower(dp->d_name[0]);
			sprintf(filname,"%s%s",dirname,dp->d_name);
			sprintf(treeplace,"%s/%c/",fileTree,bucket);
			printf("\t%s %s\n",filname,treeplace);
			if (!havemkdirs[bucket]) {
				if(mkdir(treeplace,0755) != 0){
					perror(treeplace);
					exit(1);
				}				

				havemkdirs[bucket] = 1;
			}
			sprintf(command,"cp \"%s\" \"%s\"",filname,treeplace);
			printf("runing %s\n",command);
			system(command);
		}
		else {
			printf("unknow file type \"%i\"\n",dp->d_type);
		}
	}

}
