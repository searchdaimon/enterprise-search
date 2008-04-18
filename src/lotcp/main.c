#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/lot.h"
#include "../common/reposetoryNET.h"


void recursiveDir (char lotpath[],char lotinternpath[],unsigned int lotNr, char subname[]);

main (int argc, char *argv[]) {

	char lotpath[512];
	int lotNr;
	char file[512];

	if (argc < 3) {
               printf("Dette programet kopierer en lot til en annen server\n\n\t./lotcp lotnr subname [file]\n\n");
               exit(0);
        }

	printf("argc: %i\n",argc);

	lotNr = atol(argv[1]);
	char *subname = argv[2];

	//int rmkdir(char dest[], int LotNr,char subname[]);
	rmkdir("",lotNr,subname);


	//void GetFilPathForLot(char *FilePath,int LotNr,char subname[]);		
	GetFilPathForLot(lotpath,lotNr,argv[2]);

	printf("lotpath %s\n",lotpath);

	if (argc == 3) {
		recursiveDir(lotpath,"",lotNr,subname);
	}
	else if (argc == 4) {
		strcpy(file,argv[3]);
		//hvis vi er en mappe må vi slutte på /. Hvis vi ikke her det legger vi til en /
		if((strchr(file,'/') != NULL) && (file[strlen(file) -1] != '/')) {
			printf("adding a / to \"%s\"\n",file);
			strcat(file,"/");
		}
		recursiveDir(lotpath,file,lotNr,subname);
	}
	else {
		fprintf(stderr,"wrong argc count.\n");
	}
}



void recursiveDir (char lotpath[],char lotinternpath[],unsigned int lotNr,char subname[]) {

	DIR *DIRH;
	struct dirent *dp;
	char nextdirname[512];
	char filname[512];
	char dirname[512];
	char lotinternfilname[512];

	struct stat inode;

	sprintf(dirname,"%s%s",lotpath,lotinternpath);

	if (stat(dirname,&inode) != 0) {
		perror(dirname);
		return;
	}

	if (S_ISREG(inode.st_mode)) {
		printf("sending file \"%s\" as \"%s\"\n",dirname,lotinternpath);
		rSendFile(dirname,lotinternpath,lotNr, "w",subname);

	}
	else if (S_ISDIR(inode.st_mode)) {

	if ((DIRH = opendir(dirname)) == NULL) {
		perror(dirname);
		return;
	}	

	while ((dp = readdir(DIRH)) != NULL) {

		if ((dp->d_type == DT_DIR) && ((strcmp(dp->d_name,".") == 0) || (strcmp(dp->d_name,"..") == 0))) {
			printf(". domain\n");
		}
		else if (dp->d_type == DT_DIR) {
			//if (lotinternpath[0] == '\0') {
				sprintf(nextdirname,"%s%s/",lotinternpath,dp->d_name);
			//}
			//else {
			//	strcpy(nextdirname,dp->d_name);
			//}
			printf("dir (nextdirname %s)\n",nextdirname);
			rmkdir(nextdirname,lotNr,subname);

			//kaller seg selv rekurift
			recursiveDir(lotpath,nextdirname,lotNr,subname);
		}
		else if (dp->d_type == DT_REG) {
			printf("file - ");
			sprintf(filname,"%s%s",dirname,dp->d_name);
			sprintf(lotinternfilname,"%s%s",lotinternpath,dp->d_name);
			printf("file %s, %u %s\n",filname,lotNr,lotinternfilname);	

			rSendFile(filname,lotinternfilname,lotNr, "w",subname);
			
		}
		else {
			printf("unknown type %i\n",dp->d_type);
		}
		//printf("%s %i\n",dp->d_name,dp->d_type);

	}

	closedir(DIRH);

	}
	else {
		fprintf(stderr,"File \"%s\" is neader directory, nor normal file!\n",dirname);
		exit(1);
	}
}
