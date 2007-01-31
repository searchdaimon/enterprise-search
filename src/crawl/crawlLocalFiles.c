#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>


#include "crawl.h"


int LocalFilesRecursiveDir (
	struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	char prefix[],
	char dirname[],
	int accessmode );




int crawlLocalFiles (
	struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	char prefix[],
	char path[]
	) {

	char path_real[512];

	printf("path %s\n",path);

	//fjerner eventuel / på slutten
	strncpy(path_real,path,sizeof(path_real));
	if (path_real[strlen(path_real)] == '/') {
		printf("removed / at end\n");
		path_real[strlen(path_real)] = '\0';
	}

	printf("prefix: %s, path %s\n",prefix,path);

	LocalFilesRecursiveDir(collection,documentExist,documentAdd,prefix,path,00777);

	printf("finished\n");

	return 1;

}
int LocalFilesRecursiveDir (
	struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	char prefix[],
	char dirname[],
	int accessmode ) {


	//char *dokument_buff = malloc(_dokument_buff_size +1);
	char *dokument_buff;

	printf("opening dir \"%s\", prefix %s\n",dirname,prefix);

	DIR *DIRH;
	struct dirent *dp;
	char nextdirname[512];
	char filname[512];
	char lotinternfilname[512];
	FILE *FH;
	struct stat inode;      // lager en struktur for fstat å returnere.
	int dokument_size;
	int diraccessmode;
	int filecessmode;
	struct passwd *pw;
	struct group  *gp;
	//char acl[4][64];
	char acl[3 * 64];
	int count;
	char uri[512];

	struct crawldocumentExistFormat crawldocumentExist;
        struct crawldocumentAddFormat crawldocumentAdd;

	if (stat(dirname,&inode) != 0) {
		perror("stat");
		return 0;
	}
	//har ownaccessmode som den laveste i hele pathen
	int ownaccessmode = inode.st_mode & accessmode;
	
	if ((DIRH = opendir(dirname)) == NULL) {
		perror(dirname);	
		return;
	}	
	
	while ((dp = readdir(DIRH)) != NULL) {

		sprintf(nextdirname,"%s/%s",dirname,dp->d_name);

		if (stat(nextdirname,&inode) != 0) {
			perror("fstat");
			continue;
		}


		if (dp->d_name[0] == '.') {
			printf(". domain (\"%s\")\n",dp->d_name);
		}
		//else if (dp->d_type == DT_DIR) {
		else if (S_ISDIR(inode.st_mode)) {

			sprintf(nextdirname,"%s/%s",dirname,dp->d_name);
			printf("dir (nextdirname %s)\n",nextdirname);

			//kaller seg selv rekurift
			LocalFilesRecursiveDir(collection,documentExist,documentAdd,prefix,nextdirname,ownaccessmode);
		}
		//else if (dp->d_type == DT_REG) {
		else if (S_ISREG(inode.st_mode)) {
			sprintf(filname,"%s/%s",dirname,dp->d_name);
			snprintf(uri,sizeof(uri),"%s/%s",prefix,filname);
			//sprintf(lotinternfilname,"%s%s",lotinternpath,dp->d_name);
			printf("file %s\n",filname);	

			//rSendFile(filname,lotinternfilname,lotNr, "w",subname);
			if ((FH = fopen(filname,"rb")) == NULL) {
				perror(filname);
				//exit(1);
				continue;
			}
			


			crawldocumentExist.documenturi = uri;
                        crawldocumentExist.lastmodified = inode.st_mtime;
                        crawldocumentExist.dokument_size = inode.st_size;


			//spør Boitho om filområdet finnes
			//if (!(documentExist)(collection, &crawldocumentExist ) ) {
				dokument_size = inode.st_size;
				dokument_buff = malloc(dokument_size +1);

				printf("uid %i, gid %i\n",inode.st_uid,inode.st_gid);
				//lager acl
				acl[0] = '\0';
				filecessmode = ownaccessmode & inode.st_mode;
				printf("mode %i\n",(int)inode.st_mode);
				if (filecessmode & S_IRUSR) {
					printf("ovner have read permission. S_IRUSR %i\n",inode.st_mode & S_IRUSR);
					//find username and appendit
					if ((pw = getpwuid(inode.st_uid)) == NULL) {
						printf("unknown user id %i\n",inode.st_uid);
					}
					else {
						printf("user name is %s\n",pw->pw_name);
						//strcmp(acl[nrofacls++],pw->pw_name);
						strcat(acl,pw->pw_name);
						strcat(acl,",");
					}
				}
				if (filecessmode & S_IRGRP) {
					printf("group have read permission. S_IRGRP %i\n",inode.st_mode & S_IRGRP);
					if ((gp = getgrgid(inode.st_gid)) == NULL) {
						printf("unknown group id %i\n",inode.st_gid);
					}
					else {
						printf("group is %s\n",gp->gr_name);					
						//strcmp(acl[nrofacls++],gp->gr_name);
						strcat(acl,gp->gr_name);
						strcat(acl,",");

					}
				}
				if (filecessmode & S_IROTH) {
					printf("others have read permission. S_IROTH %i\n",inode.st_mode & S_IROTH);
					//strcmp(acl[nrofacls++],"EVERYONE");
					strcat(acl,"Everyone"); //toDo bruker msad sin gruppe fro alle her. Bør kansje ha en engen??
					strcat(acl,",");

				}
				//uefektift, bruker strlen() to ganger
				if (acl[strlen(acl)] == ',') {
					acl[strlen(acl)] = '\0';
				}				

				//leser hele dokumentet
				fread(dokument_buff,sizeof(char),dokument_size,FH);


				crawldocumentAdd.documenturi    = uri;
                                crawldocumentAdd.documenttype   = "";
                                crawldocumentAdd.document       = dokument_buff;
                                crawldocumentAdd.dokument_size  = dokument_size;
                                crawldocumentAdd.lastmodified   = inode.st_mtime;
                                crawldocumentAdd.acl            = acl;
                                crawldocumentAdd.title          = dp->d_name;
                                crawldocumentAdd.doctype        = "";

				printf("\tdokument_size \"%i\"\n",dokument_size);

                                (*documentAdd)(collection ,&crawldocumentAdd);


				//bbdn_docadd(bbdh,collection,filname,"",dokument_buff,dokument_size,(unsigned int)inode.st_mtime,acl,dp->d_name,"");
				free(dokument_buff);
			//}

			fclose(FH);
		}
		else {
			printf("unknown type %i. Name \"%s\"\n",inode.st_mode,dp->d_name);
		}
		//printf("%s %i\n",dp->d_name,dp->d_type);

	}

	closedir(DIRH);

}
