#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/lot.h"
#include "../common/reposetoryNET.h"


void recursiveDir (char lotpath[],char lotinternpath[],unsigned int lotNr, char subname[], char server[]);

main (int argc, char *argv[]) {

	char lotpath[512];
	int lotNr;
	char file[512];

	char *optHost = NULL;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"h:"))!=-1) {
                switch (c) {
                        case 'h':
                                optHost = optarg;
                                fprintf(stderr, "Will send to host %s\n",optHost);

                                break;
                        default:
                                exit(1);
                }

        }
        --optind;



	if (argc -optind < 3) {
               	printf("Dette programet kopierer en lot til en annen server\n\n\t./lotcp lotnr subname [file]\n\n");
		printf("Options:\n\t-h host\n\n");
               	exit(0);
        }


	lotNr = atol(argv[1 +optind]);
	char *subname = argv[2 +optind];
	
	//int rmkdir(char dest[], int LotNr,char subname[]);
	rmkdir("",lotNr,subname);


	//void GetFilPathForLot(char *FilePath,int LotNr,char subname[]);		
	GetFilPathForLot(lotpath,lotNr,argv[2 +optind]);
	
	#ifdef DEBUG
	printf("lotpath %s\n",lotpath);
	#endif

	if ((argc -optind) == 3) {
		recursiveDir(lotpath,"",lotNr,subname,optHost);
	}
	else if ((argc -optind) == 4) {
		strcpy(file,argv[3 +optind]);
		//hvis vi er en mappe må vi slutte på /. Hvis vi ikke her det legger vi til en /
		if((strchr(file,'/') != NULL) && (file[strlen(file) -1] != '/')) {
			printf("adding a / to \"%s\"\n",file);
			strcat(file,"/");
		}
		recursiveDir(lotpath,file,lotNr,subname,optHost);
	}
	else {
		fprintf(stderr,"wrong argc count.\n");
	}
}



void recursiveDir (char lotpath[],char lotinternpath[],unsigned int lotNr,char subname[], char server[]) {

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
		if (server != NULL) {
			if (!rSendFileToHostname(dirname,lotinternpath,lotNr, "w", subname, server)) {
                                printf("can't send file %s\n",filname);
				exit(-1);
        		}			
		}
		else {
			rSendFile(dirname,lotinternpath,lotNr, "w",subname);
		}
	}
	else if (S_ISDIR(inode.st_mode)) {

	if ((DIRH = opendir(dirname)) == NULL) {
		perror(dirname);
		return;
	}	

	while ((dp = readdir(DIRH)) != NULL) {

		if ((dp->d_type == DT_DIR) && ((strcmp(dp->d_name,".") == 0) || (strcmp(dp->d_name,"..") == 0))) {
			#ifdef DEBUG
				printf(". domain\n");
			#endif
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
			recursiveDir(lotpath,nextdirname,lotNr,subname,server);
		}
		else if (dp->d_type == DT_REG) {
			printf("file - ");
			sprintf(filname,"%s%s",dirname,dp->d_name);
			sprintf(lotinternfilname,"%s%s",lotinternpath,dp->d_name);
			printf("file %s, %u %s\n",filname,lotNr,lotinternfilname);	

			if (server != NULL) {
				if (!rSendFileToHostname(filname,lotinternfilname,lotNr, "w",subname, server)) {
                	                printf("can't send file %s\n",filname);
        	                }			
			}
			else {
				rSendFile(filname,lotinternfilname,lotNr, "w",subname);
			}
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
