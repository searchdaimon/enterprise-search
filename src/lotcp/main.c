#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#include "../common/lot.h"
#include "../common/reposetoryNET.h"

#define subname "www"

void recursiveDir (char lotpath[],char lotinternpath[],unsigned int lotNr);

main (int argc, char *argv[]) {

	char lotpath[512];
	int lotNr;

	if (argc < 3) {
                printf("Dette programet kopierer en lot til en annen server\n\n\t./lotcp lotnr subname\n\n");
               exit(0);
        }

	lotNr = atol(argv[1]);

	//int rmkdir(char dest[], int LotNr,char subname[]);
	rmkdir("",lotNr,subname);


	//void GetFilPathForLot(char *FilePath,int LotNr,char subname[]);		
	GetFilPathForLot(lotpath,lotNr,argv[2]);

	printf("lotpath %s\n",lotpath);

	recursiveDir(lotpath,"",lotNr);
}



void recursiveDir (char lotpath[],char lotinternpath[],unsigned int lotNr) {

	DIR *DIRH;
	struct dirent *dp;
	char nextdirname[512];
	char filname[512];
	char dirname[512];
	char lotinternfilname[512];

	sprintf(dirname,"%s%s",lotpath,lotinternpath);

	if ((DIRH = opendir(dirname)) == NULL) {
		perror(dirname);
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
			recursiveDir(lotpath,nextdirname,lotNr);
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
