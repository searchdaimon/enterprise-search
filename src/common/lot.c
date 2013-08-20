#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <err.h>
#include <sys/file.h>
#include <limits.h>
#include <signal.h>

#include "lot.h"
#include "define.h"
#include "bstr.h"
#include "bfileutil.h"
#include "boithohome.h"
#include "strlcat.h"
#include "atomicallyio.h"
#include "getpath.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <inttypes.h>

#include <sys/stat.h>
#include <unistd.h>


//hvis det er mac os :
//#include <sys/param.h> // for statfs 
//#include <sys/mount.h> // for statfs 
#include <sys/vfs.h> // for statfs

//formater på cashe over opne filhontarere
struct OpenFilesFormat {
        int LotNr;
        FILE *FILEHANDLER;
	char subname[maxSubnameLength];
	char type[5];
	char filename[PATH_MAX];
	char resource[PATH_MAX];	
};


//array med mapper som kan brukes til å lagre i
struct StorageDirectorysFormat dataDirectorys[NrOfDataDirectorys];

//holder om vi har inalisert. Slik at MakeMapListMap() bare bli kalt 1 gang, nemlig første gangen vi buker denne rutinen.
static int MapListInitialised = 0;

static int LotFilesInalisert = 0;

//array med åpne filhånterere
static struct OpenFilesFormat OpenFiles[MaxOpenFiles];

#ifndef DEFLOT
//LotForDOCid returnerer hvilken lot DOCid skal i
int rLotForDOCid (unsigned int DocID) {
	int lot;
	
	lot	= (int)((DocID / NrofDocIDsInLot) +1);

	/*
	//gjode litt om her så vi ikke trenger å bruke ceil. Da det er trekt
	//testen nedenfor sjekker at vi fortsatt får samme resultat
	if (lot != ceil((DocID / NrofDocIDsInLot) +1)) {
		printf("lot proble\n");
		exit(1);
	}
	*/
	return lot;
}

/*
Antall DocIDer som er før denne lotten. Dette er viktig å vite da da indekser 
bruker DocID for adresering, og lotene er på en måte som en stabel med kort, man 
må vite hvor nåverende kort bgynner
*/
int LotDocIDOfset (int LotNr) {

	return ((NrofDocIDsInLot * LotNr) - NrofDocIDsInLot);
}

#endif

void makeLotPath(int lotNr,char folder[],char subname[]) {

	char path[PATH_MAX];

	GetFilPathForLot(path,lotNr,subname);

	strcat(path,"/");
	strcat(path,folder);
	

	makePath(path);	

}

int HasSufficientSpace(char FilePath[], int needSpace) {


	struct statfs buf;
	long fssize;

	if (statfs(FilePath,&buf) != 0) {
		perror(FilePath);
		return 0;
	}	
	fssize = (long)((long)buf.f_bavail * ((long)buf.f_bsize / 1024));

	//printf("f_bavail %li,f_bsize %li, free %li\n",(long)buf.f_bavail,(long)buf.f_bsize,fssize);


	if ((needSpace * 1024) > fssize) {
		return 0;
	}
	else {
		return 1;
	}

}

int lotHasSufficientSpace(int lot, int needSpace,char subname[]) {

	char FilePath[512];
	int i;

	GetFilPathForLot(FilePath,lot,subname);
	//må lage, slik at vi er 100% sikker på at vi har noe å teste mot

	#ifdef DEBUG
		printf("will hav to make lot path sow we can test for space\n");
	#endif
	makePath(FilePath);
	
	i=HasSufficientSpace(FilePath,needSpace);

	return i;
}

#ifdef DO_DIRECT

int
lotOpenFileNoCache_direct(unsigned int DocID, char *resource, char *type, char lock, char *subname)
{
	

	unsigned int LotNr = rLotForDOCid(DocID);
	int i;
	char FilePath[PATH_MAX];
	char File [PATH_MAX];
	int fd;

	printf("lotOpenFileNoCache_direct(subname: \"%s\", resource %s)\n",subname,resource);
	GetFilPathForLot(FilePath,LotNr,subname);
	strcpy(File,FilePath);
	strncat(File,resource,PATH_MAX); //var 128

#ifdef DEBUG
	printf("lotOpenFileNoCasheByLotNr: opening file \"%s\" for %s\n",File,type);
#endif

	//hvis dette er lesing så hjelper det ikke og prøve å opprette path. Filen vil fortsatt ikke finnes
	if ((strcmp(type,"rb") == 0) || (strcmp(type,"r") == 0)) {
		if ((fd = open64(File, O_RDONLY|O_DIRECT|O_LARGEFILE)) == -1) {
			warn("open64: %d", fd);
#ifdef DEBUG
			perror(File);
#endif
			return -1;
		}
	} else {
		errx(1, "We can only open this for reading right now");
	}

#ifdef DEBUG
	printf("lotOpenFile: tryint to obtain lock \"%c\"\n",lock);
#endif
	//honterer låsning
	if (lock == 'e') {
		//skal vi ha flock64() her ?
		flock(fd, LOCK_EX);
	}
	else if (lock == 's') {
		flock(fd, LOCK_SH);
	}
#ifdef DEBUG
	printf("lotOpenFile: lock obtained\n");
#endif

#ifdef DEBUG
	printf("lotOpenFileNoCasheByLotNr: finished\n");
#endif
	return fd;

}
#endif



FILE *lotOpenFileNoCashe(unsigned int DocID,char resource[],char type[], char lock,char subname[]) {

	return lotOpenFileNoCasheByLotNr(rLotForDOCid(DocID),resource,type,lock,subname);
}
FILE *lotOpenFileNoCasheByLotNr(int LotNr,char resource[],char type[], char lock,char subname[]) {


	FILE *FILEHANDLER;
	int i;
	char FilePath[PATH_MAX]; 	//var 128
	char File [PATH_MAX];	//var 128

	#ifdef DEBUG
		printf("lotOpenFileNoCasheByLotNr(LotNr=%i, resource= \"%s\", type=\"%s\", lock=\"%c\", subname=\"%s\")\n",LotNr,resource,type,lock,subname);
	#endif

		GetFilPathForLotFile(File,resource,LotNr,subname);
		//finer hvilken mappe vi skal i, tar også hensyn til sub mapper i lotten.
		strscpy(FilePath,getpath(File),sizeof(FilePath));

		#ifdef DEBUG
                	printf("lotOpenFileNoCasheByLotNr: opening file \"%s\" for %s\n",File,type);
		#endif

		if (strcmp(type,">>") == 0) {
			//emulating perl's >>. If the file eksist is is opene for reading and writing.
			//if not it is createt and open for reading and writing
			if ( (FILEHANDLER = (FILE *)fopen64(File,"r+")) == NULL ) {
                        	makePath(FilePath);

				if ( (FILEHANDLER = (FILE *)fopen64(File,"r+")) == NULL ) {

                        		if ( (FILEHANDLER = (FILE *)fopen64(File,"w+")) == NULL ) {
                        		        perror(File);
                        		        //exit(0);
                        		        return NULL;
                        		}
				}
                	}
			#ifdef SD_CLOEXEC
				if (!fcloseAtExexo(FILEHANDLER)) {
					fprintf(stderr,"fcloseAtExexo.\n");
					perror(File);
				}
			#endif

		}
		//hvis dette er lesing så hjelper det ikke og prøve å opprette path. Filen vil fortsatt ikke finnes
		else if ((strcmp(type,"rb") == 0) || (strcmp(type,"r") == 0) || (strcmp(type,"r+") == 0)) {
			if ( (FILEHANDLER = (FILE *)fopen64(File,type)) == NULL ) {
				#ifdef DEBUG
				perror(File);
				#endif
				return NULL;
			}
			#ifdef SD_CLOEXEC
				if (!fcloseAtExexo(FILEHANDLER)) {
					fprintf(stderr,"fcloseAtExexo.\n");
					perror(File);
				}
			#endif

		}
		else if ((strcmp(type,"wb") == 0) || (strcmp(type,"w") == 0) ) {
			printf("making path \"%s\"\n",FilePath);
                        makePath(FilePath);
			if ((FILEHANDLER = batomicallyopen(File,type)) == NULL) {
				perror(File);
				return NULL;
			}
		}
		else {
                	//temp: Bytte ut FilePath med filnavnet
                	if ( (FILEHANDLER = (FILE *)fopen64(File,type)) == NULL ) {
                        	makePath(FilePath);

				//hvorfår har vi type "File" her ???, det verste er at det ser ut til å fungere også
                        	//if ( (FILEHANDLER = (FILE *)fopen64(File,"File")) == NULL ) {
                        	if ( (FILEHANDLER = (FILE *)fopen64(File,type)) == NULL ) {
                        	        perror(File);
                        	        //exit(0);
					return NULL;
        	                }
	                }
			#ifdef SD_CLOEXEC
				if (!fcloseAtExexo(FILEHANDLER)) {
					fprintf(stderr,"fcloseAtExexo.\n");
					perror(File);
				}
			#endif


		}


            	#ifdef DEBUG
                        printf("lotOpenFile: tryint to obtain lock \"%c\"\n",lock);
                #endif
                //honterer låsning
                if (lock == 'e') {
			//skal vi ha flock64() her ?
                        flock(fileno(FILEHANDLER),LOCK_EX);
                }
                else if (lock == 's') {
                        flock(fileno(FILEHANDLER),LOCK_SH);
                }
		#ifdef DEBUG
                        printf("lotOpenFile: lock obtained\n");
                #endif
 
		#ifdef DEBUG
			printf("lotOpenFileNoCasheByLotNr: finished\n");
		#endif
                return FILEHANDLER;

}

int lotOpenFileNoCashel(unsigned int DocID,char resource[],char type[], char lock,char subname[]) {

	return lotOpenFileNoCasheByLotNrl(rLotForDOCid(DocID),resource,type,lock,subname);
}

int lotOpenFileNoCasheByLotNrl(int LotNr,char resource[],char type[], char lock,char subname[]) {


	int fd;
	int i;
	char FilePath[PATH_MAX]; 	//var 128
	char File [PATH_MAX];	//var 128

	#ifdef DEBUG
		printf("lotOpenFileNoCasheByLotNrl(LotNr=%i, subname=\"%s\", resource=\"%s\")\n",LotNr,subname,resource);
	#endif

                 GetFilPathForLot(FilePath,LotNr,subname);
                 strcpy(File,FilePath);
                 strncat(File,resource,PATH_MAX); //var 128

		#ifdef DEBUG
                	printf("lotOpenFileNoCasheByLotNr: opening file \"%s\" for %s\n",File,type);
		#endif

		if (strcmp(type,">>") == 0) {
			//emulating perl's >>. If the file eksist is is opene for reading and writing.
			//if not it is createt and openf for writing and reading
			if ( (fd = open64(File,O_CREAT|O_RDWR,0664)) == -1 ) {
                        	makePath(FilePath);

				if ( (fd = open64(File,O_CREAT|O_RDWR,0664)) == -1 ) {
                       		        perror(File);
                       		        return -1;                        		
				}
                	}
			
		}
		//hvis dette er lesing så hjelper det ikke og prøve å opprette path. Filen vil fortsatt ikke finnes
		else if ((strcmp(type,"rb") == 0) || (strcmp(type,"r") == 0)) {
			if ( (fd = open64(File,O_RDONLY)) == -1 ) {
				#ifdef DEBUG
				perror(File);
				#endif
				return -1;
			}
		}
		//hvis dette er lesing så hjelper det ikke og prøve å opprette path. Filen vil fortsatt ikke finnes
		else if ((strcmp(type,"r+b") == 0) || (strcmp(type,"r+") == 0)) {
			if ( (fd = open64(File,O_RDWR)) == -1 ) {
				#ifdef DEBUG
				perror(File);
				#endif
				return -1;
			}
		}
		else {
			fprintf(stderr,"lotOpenFileNoCasheByLotNrl: ikke implementert\n");
			exit(1);
		/*
                //temp: Bytte ut FilePath med filnavnet
                if ( (fd = open64(File,type)) == NULL ) {
                        makePath(FilePath);

			//hvorfår har vi type "File" her ???, det verste er at det ser ut til å fungere også
                        //if ( (FILEHANDLER = (FILE *)fopen64(File,"File")) == NULL ) {
                        if ( (fd = (open64(File,type)) == NULL ) {
                                perror(File);
                                //exit(0);
				return NULL;
                        }
                }
		*/
		}

            	#ifdef DEBUG
                        printf("lotOpenFile: tryint to obtain lock \"%c\"\n",lock);
                #endif
                //honterer låsning
                if (lock == 'e') {
			//skal vi ha flock64() her ?
                        flock(fd,LOCK_EX);
                }
                else if (lock == 's') {
                        flock(fd,LOCK_SH);
                }
		#ifdef DEBUG
                        printf("lotOpenFile: lock obtained\n");
                #endif
 
		#ifdef DEBUG
			printf("lotOpenFileNoCasheByLotNr: finished\n");
		#endif
                return fd;

}

//stenger ned filer (og frigjør låser)
void lotCloseFiles() {
	int i;

	printf("lotCloseFiles\n");
	if (LotFilesInalisert) {
		for(i=0; i < MaxOpenFiles; i++) {

                	if (OpenFiles[i].LotNr != -1) {
				printf("closing lot fil nr %i for lot %i. fh %p. file \"%s\"\n",
					i,
					OpenFiles[i].LotNr,
					OpenFiles[i].FILEHANDLER,
					OpenFiles[i].filename
				);

   				if (fclose(OpenFiles[i].FILEHANDLER) != 0) {
					perror(OpenFiles[i].filename);
				}
			}
		}
	}

	LotFilesInalisert = 0;
}


//gir andre tilgan til lot filer. Casher opne filhandlere
FILE *lotOpenFile(unsigned int DocID,char resource[],char type[], char lock,char subname[]) {

        int LotNr;
        int i;
        char FilePath[128];
        char File [128];

	if (!LotFilesInalisert) {
		for(i=0; i < MaxOpenFiles; i++) {
			OpenFiles[i].LotNr = -1;
		}

		LotFilesInalisert = 1;
	}

        File[0] = '\0';

        //finner i hvilken lot vi skal lese fra
        LotNr = rLotForDOCid(DocID);

	//printf("LotNr: %i, DocID: %i\n",LotNr,DocID);

        //begynner med å søke cashen. Lopper til vi enten er ferdig, eller til vi har funne ønskede i cashen
	i = 0;
        while ((i < MaxOpenFiles) && (OpenFiles[i].LotNr != LotNr)) {
                i++;
        }
        //temp: skrur av søking her med i=0
        //type of og subname er også lagt til uten at det tar hensyn til det i søket
        i = 0;



        //hvis vi fant i casehn returnerer vi den
        if (OpenFiles[i].LotNr == LotNr  
		&& (strcmp(OpenFiles[i].subname,subname) == 0)
        	&& (strcmp(OpenFiles[i].type,type)==0)
        	&& (strcmp(OpenFiles[i].resource,resource)==0)
	) {
		#ifdef DEBUG
		printf("lotOpenFile: fant en tildigere åpnet fil, returnerer den.\n");
		printf("lotOpenFile: returnerer: i %i, subname \"%s\", type \"%s\", LotNr %i\n",i,OpenFiles[i].subname,OpenFiles[i].type,OpenFiles[i].LotNr);
		printf("lotOpenFile: file is \"%s\"\n",OpenFiles[i].filename);
		printf("lotOpenFile: returning file handler %p\n",OpenFiles[i].FILEHANDLER);
		#endif

		if (OpenFiles[i].FILEHANDLER == NULL) {
			printf("Error: FILEHANDLER is NULL\n");
			#ifdef DEBUG
				exit(-1);
			#endif
		}
                return OpenFiles[i].FILEHANDLER;
        }
        //hvis ikke åpner vi og returnerer
        else {

		//hvis dette er en åpen filhånterer, må vi lukke den
		if (OpenFiles[i].LotNr != -1) {
			printf("lotOpenFile: closeing: i %i\n",i);
			fclose(OpenFiles[i].FILEHANDLER);
			OpenFiles[i].LotNr = -1;
			
		}
	
		if ((OpenFiles[i].FILEHANDLER = lotOpenFileNoCasheByLotNr( LotNr, resource,type, lock,subname)) == NULL) {
			printf("lotOpenFileNoCashe: can't open file\n");
			return NULL;
		}

                GetFilPathForLot(FilePath,LotNr,subname);
                strscpy(File,FilePath,sizeof(File));
                strlcat(File,resource,sizeof(File));

		strscpy(OpenFiles[i].filename,File,sizeof(OpenFiles[i].filename));
		strscpy(OpenFiles[i].resource,resource,sizeof(OpenFiles[i].resource));
		strscpy(OpenFiles[i].subname,subname,sizeof(OpenFiles[i].subname));
		strscpy(OpenFiles[i].type,type,sizeof(OpenFiles[i].type));

		//#ifdef DEBUG
                	printf("lotOpenFile: opening file \"%s\" for %s\n",File,type);
		//#endif



		OpenFiles[i].LotNr = LotNr;

                return OpenFiles[i].FILEHANDLER;

        }
	
}


//fjerner \n på slutten av strenger

FILE *openMaplist() {

	FILE *MAPLIST;
	char *cptr;

	//sjekker først om vi har en env variabel kalt "BOITHOMAPLIST". Hvis vi har det så bruker vi den filen
	//gjør det slik slik at vi kan ha lokal maplist, på hver bbs, man fortsat ha resten likt på alle, og på read onlu nfs.
	if ((cptr = getenv("BOITHOMAPLIST")) != NULL) {
		if ( (MAPLIST = fopen(cptr,"r")) == NULL) {
			perror(cptr);
			exit(1);
		}
	}
        //leser liten over mapper vi kan bruke.
        else if ( (MAPLIST = bfopen("config/maplist.conf","r")) == NULL) {
                perror(bfile("config/maplist.conf"));
                exit(1);
        }


	return MAPLIST;
}

int GetDevIdForLot(int LotNr) {

	#ifdef DEBUG
		printf("GetDevIdForLot: path %s, id %i\n",dataDirectorys[LotNr % NrOfDataDirectorys].Name,dataDirectorys[LotNr % NrOfDataDirectorys].devid);
	#endif

	return dataDirectorys[LotNr % NrOfDataDirectorys].devid;

}

//lager/henter ut en unik id for hver device
int MakeMapListMap_getfsid (const char *path) {

	struct stat buf;
	int i;

	if (stat(path, &buf) != 0) {
		//Er ikke sikkert at mappen er oppretttet enda, det er helt normalt.
		#ifdef DEBUG
			perror(path);
		#endif
		return 0;
	}

	i = (buf.st_dev % NrOfDataDirectorys);

	return i;
}

/*
Laster mappene som ligger i maplist.conf og lager en oversikt over de.
*/
void MakeMapListMap () {

	FILE *MAPLIST;
	char *line;
	char buff[1024];
	short int i;

	MAPLIST = openMaplist();

	i = 0;
	while((fgets(buff,sizeof(buff),MAPLIST) != NULL) && (NrOfDataDirectorys > i)) {
		chomp(buff);
		//printf("line -%s-\n",buff);
		sprintf(dataDirectorys[i].Name,"%s",buff);

		dataDirectorys[i].devid = MakeMapListMap_getfsid(buff);

		++i;
	}

	fclose(MAPLIST);
}



// gir hav som er første DocID i en lot
int GetStartDocIFForLot (int LotNr) {
	return ((LotNr * NrofDocIDsInLot) - NrofDocIDsInLot);
}

int sadd(int in, int max) {

	int ret = in;

	++ret;

	ret = ret % max;

	return ret;

}

int indexNrOffset(int IndexNr,char Type[]) {


	if (strcmp(Type,"Url") == 0) {
		return sadd(IndexNr +1,63);
	}
	else if (strcmp(Type,"Anchor") == 0) {
		return sadd(IndexNr +2,63);
	}
	else {
		return IndexNr;		
	}
}


void GetFilePathForIDictionary (char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]) {

	//hvis vi ikke har inlisert mapplisten enda gjør vi det.
	if (!MapListInitialised) {
		//printf("aaa");
		MakeMapListMap();
		MapListInitialised = 1;
	}

	IndexNr = indexNrOffset(IndexNr,Type);

	//printf("dataDirectorys: %s\n",dataDirectorys[IndexNr].Name);
	if (strcmp(subname,"www") == 0) {
		sprintf(FilePath,"%s/iindex/%s/dictionary/%s/",dataDirectorys[IndexNr].Name,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);
	}
	else {
		sprintf(FilePath,"%s/iindex/%s/%s/dictionary/%s/",dataDirectorys[IndexNr].Name,subname,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);

	}
}


void GetFilePathForIindex (char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]) {

	//hvis vi ikke har inlisert mapplisten enda gjør vi det.
	if (!MapListInitialised) {
		//printf("aaa");
		MakeMapListMap();
		MapListInitialised = 1;
	}

	//printf("dataDirectorys: %s\n",dataDirectorys[IndexNr].Name);

	IndexNr = indexNrOffset(IndexNr,Type);

	if (strcmp(subname,"www") == 0) {
		sprintf(FilePath,"%s/iindex/%s/index/%s/",dataDirectorys[IndexNr].Name,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);
	}
	else {
		sprintf(FilePath,"%s/iindex/%s/%s/index/%s/",dataDirectorys[IndexNr].Name,subname,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);
	}
}

void GetFilPathForLotFile(char *FilePath,char lotfile[],int LotNr,char subname[]) {

	GetFilPathForLot(FilePath,LotNr,subname);

	strcat(FilePath,lotfile);
}


char *returnFilPathForLot(int LotNr,char subname[]) {
	static char FilePath[PATH_MAX];
	GetFilPathForLot(FilePath,LotNr,subname);

	return FilePath;
}

/*
Gir oss pats for en lot. 
*/
void GetFilPathForLot(char *FilePath,int LotNr,char subname[]) {
	//char FilePath[64];
	int subdir;

	
	//hvis vi ikke har inlisert mapplisten enda gjør vi det.
	if (!MapListInitialised) {
		MakeMapListMap();
		MapListInitialised = 1;
	}

	//subdir = fmod(LotNr,64);
	//subdir = LotNr % 64;

	//sprintf(FilePath,"%s/%i/%i/",dataDirectorys[subdir].Name,subdir,LotNr);

	if (strcmp(subname,"www") == 0) {
		sprintf(FilePath,"%s/%i/",dataDirectorys[LotNr % 64].Name,LotNr);
	}
	else {
		sprintf(FilePath,"%s/%i/%s/",dataDirectorys[LotNr % 64].Name,LotNr,subname);
	}

	//printf("%s\n",FilePath);
	//printf("dataDirectorys: %s\n",dataDirectorys[subdir].Name);

	

}

/*
Finner path for en lot fra docid
*/
void GetFilPathForLotByDocID(char *FilePath,int DocID,char subname[]) {

	int lot;
	lot = rLotForDOCid(DocID);
	
	GetFilPathForLot(FilePath,lot,subname);
	
}


//gir ful path for et bilde fra DocID
void GetFilPathForThumbnaleByDocID(char *FileName,int DocID,char subname[]) {

	int LotNr;
        int ImageBucket;

        ImageBucket = fmod(DocID,512);

        //finner path
        LotNr = rLotForDOCid(DocID);
        GetFilPathForLot(FileName,LotNr,subname);


        sprintf(FileName,"%simages/%i/%i.jpg",FileName,ImageBucket,DocID);

	
}


void makePath (char path[]) {
	DIR *dp;

	if ((dp = opendir(path)) == NULL) {
		bmkdir_p(path,0755);
	}
	else {
		#ifdef DEBUG
			printf("dir exsist. Wont make\n");
		#endif

		closedir(dp);
	}

/*
        int i;
        char temp[128];
        int tempnr;
        char partdir[128];
        char command[128];

        tempnr = 0;
        partdir[0] = '\0';

        //printf("gf: -%s-\n",path);
	
	sprintf(command,"mkdir -p %s",path);

	system(command);

	//printf("%s\n",command);
*/
//temp: fjerner dette gamle skrullet.
//det nye ovenfor er utestet
/*
        for (i=0; path[i] != '\0'; i++) {

                if (path[i] == '/') {
                        temp[tempnr] = '\0';

                        strncat(partdir,temp,tempnr);
                        strncat(partdir,"/",1);

                        printf("%s\n",command);

                        sprintf(command,"mkdir %s",partdir);

                        system(command);
                        tempnr = 0;
                }
                else {
                        temp[tempnr] = path[i];
                        tempnr++;
                }

        }
*/

}


DIR *listAllColl_start() {

        DIR *dp;
 	char FilePath[PATH_MAX];


	//hvis vi ikke har inlisert mapplisten enda gjør vi det.
	if (!MapListInitialised) {
		MakeMapListMap();
		MapListInitialised = 1;
	}

	sprintf(FilePath,"%s/%i/",dataDirectorys[1 % 64].Name,1);
	

        return opendir(FilePath);
}

char *listAllColl_next(DIR * ll) {

	static char subname[PATH_MAX];
	struct dirent *dp;

        while ((dp = readdir(ll)) != NULL) {
		if (dp->d_name[0] == '.') {

                }
               	else if (dp->d_type == DT_DIR) {
			strscpy(subname,dp->d_name,sizeof(subname));
			return subname;
		}
	}

	return NULL;

}

void listAllColl_close(DIR * ll) {
	closedir(ll);
}


int lotDeleteFile(char File[], int LotNr,char subname[]) {

	char FilePath[PATH_MAX];
	
	GetFilPathForLotFile(FilePath, File, LotNr, subname);

	printf("unlinking %s\n",FilePath);
	return unlink(FilePath);
}

void
lot_get_closed_collections_file(char *buf)
{
	sbfile(buf, "var/closed_collections");
}

void lot_recache_collection(char subname[]) {




			{
				char collpath[LINE_MAX];
				FILE *fp;

				lot_get_closed_collections_file(collpath);
				fp = fopen(collpath, "a");
				if (fp == NULL) {
					warn("fopen(%s, append)", collpath);
				} else {
					flock(fileno(fp), LOCK_EX);
					fseek(fp, 0, SEEK_END);
					fprintf(fp, "%s\n", subname);
					fclose(fp);
				}
			}

			{
				int pid;
				char pidpath[LINE_MAX];
				FILE *fp;

				sbfile(pidpath, "var/searchd.pid");
				if ((fp = fopen(pidpath, "r")) == NULL) {
					warn("Unable to open pidfile for searchdbb: fopen(%s)", pidpath);
				} else {
					int scanc = fscanf(fp, "%d", &pid);
					if (scanc != 1) {
						fprintf(stderr,"Unable to get a valid pid from %s\n",pidpath);
					}
					else {
						printf("pid %i, scanc %i\n", pid, scanc);
						kill(pid, SIGUSR2);
					}
						fclose(fp);

				}

			}


}
