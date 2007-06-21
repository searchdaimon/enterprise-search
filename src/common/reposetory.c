#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "define.h"

#include "stdlib.h"
#include "lot.h"
#include "reposetory.h"
#include "sha1.h"
#include "bstr.h"
//#include "define.h"
//#include <errno.h>
//extern int errno;

#include <arpa/inet.h> //for inet_aton()
#include <sys/types.h> //for time()
#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>


#include <inttypes.h>

//temp77
#include <locale.h>

#define CurrentReposetoryVersion 4

off_t getImagepFromRadres(unsigned int radress64bit,unsigned int htmlbufferSize) {

	#ifdef BLACK_BOKS
		return (radress64bit + sizeof(unsigned int) + sizeof(struct ReposetoryHeaderFormat) + htmlbufferSize);
	#else
		return (radress64bit + sizeof(struct ReposetoryHeaderFormat) + htmlbufferSize);
	#endif
}

//popper en post av en filhonterer
void fpop(char *buff,int *length,FILE *file,char separator,int nrOfseparators) {

	char lastchar;

	int foundSeparators = 0;
	int BuffLength;

	BuffLength = *length;
	
	*length = 0;
	
	while ((! feof(file)) && (foundSeparators != nrOfseparators)) {
	//while(*length == 0) {
		lastchar = getc(file);

		//printf("%c",(char)lastchar);

		if (lastchar == separator) {
			foundSeparators++;
		}
		else {
			foundSeparators = 0;
		}
		
		if (*length < BuffLength) {	
			buff[*length] = lastchar;
			//printf("%i: %c - %c\n",*length,lastchar,buff[*length]);
		}
		
		(*length)++;
	}

	*length = (*length - nrOfseparators);

	buff[*length] = '\0';
	
	//printf("buff: -%s-\n%i\n%i\n%i\n",buff,*length,foundSeparators,nrOfseparators);

}
//inaliserer. Må kalles først
//void ropen () {
//	short int i;
//	
//	for ( i = 0; i < MaxOpenReposetoryFiles; i++) {
//		OpenReposetoryFiles[i].LotNr = -1;
//	}
//
//}
//stenger ned. Må kallles sist.
void rclose () {
//	short int i;
//
//	//stenger ned alle open filhandlerer
//	for ( i = 0; i < MaxOpenReposetoryFiles; i++) {
//
//		//stenger ned filhandler
//		if (OpenReposetoryFiles[i].LotNr != -1) {
//			fclose(OpenReposetoryFiles[i].FILEHANDLER);
//		}
//	}

	lotCloseFiles();
}

void setLastIndexTimeForLot(int LotNr,int httpResponsCodes[],char subname[]){
	FILE *RFILE;
	unsigned int now;
	int i;

        RFILE = lotOpenFileNoCasheByLotNr(LotNr,"IndexTime","wb",'e',subname);

	now = (unsigned int)time(NULL);
	
 	fwrite(&now,sizeof(unsigned int),1,RFILE);

	fclose(RFILE);	


	RFILE = lotOpenFileNoCasheByLotNr(LotNr,"HttpResponsCodes.txt","wb",'e',subname);

        //skriver ut en oversikt over hvilkene http responser vi kom over
        for(i=0;i<nrOfHttpResponsCodes;i++) {
              if (httpResponsCodes[i] != 0) {
                      fprintf(RFILE,"%i: %i\n",i,httpResponsCodes[i]);
              }
        }
	fclose(RFILE);
	
}

unsigned int GetLastIndexTimeForLot(int LotNr,char subname[]){
        
	FILE *RFILE;
        unsigned int now;

        if ((RFILE = lotOpenFileNoCasheByLotNr(LotNr,"IndexTime","rb",'s',subname)) != NULL) {

		fread(&now,sizeof(unsigned int),1,RFILE);

        	fclose(RFILE);
		
		return now;
	}
	else {
		return 0;
	}
}



//opner en filen der docid er
/*
FILE *ropenlot(int DocID,char type[]) {

	int LotNr;
	int i = 0;
	char FilePath[128];
	char File [128];

	File[0] = '\0';

	//finner i hvilken lot vi skal lese fra
	LotNr = rLotForDOCid(DocID);
	
	//begynner med å søke cashen. Lopper til vi enten er ferdig, eller til vi har funne ønskede i cashen
	while ((i < MaxOpenReposetoryFiles) && (OpenReposetoryFiles[i].LotNr != LotNr)) {
		i++;
	}
	//temp: skrur av søking her med i=0
	//type er også lagt til uten at det tar hensyn til det i cashe arrayen
	i = 0;

	//hvis vi fant i casehn returnerer vi den
	if (OpenReposetoryFiles[i].LotNr == LotNr) {
		return OpenReposetoryFiles[i].FILEHANDLER;
	}
	//hvis ikke opner vi og returnerer
	else {
	
		OpenReposetoryFiles[i].LotNr = LotNr;


		 GetFilPathForLot(FilePath,LotNr);

		 strcpy(File,FilePath);
		 strncat(File,"reposetory",128);

		// printf("%s\n",File);
		
		//temp: Bytte ut FilePath med filnavnet
		if ( (OpenReposetoryFiles[i].FILEHANDLER = fopen(File,type)) == NULL ) {
			makePath(FilePath);

			if ( (OpenReposetoryFiles[i].FILEHANDLER = fopen(File,"a+b")) == NULL ) {
				perror(File);
				exit(0);
			}
		}
		

		return OpenReposetoryFiles[i].FILEHANDLER;
	
	}
}
*/

unsigned int rLastDocID(char subname[]) {

	FILE *DocIDFILE;
	struct stat inode;      // lager en struktur for fstat å returnere.
	char buff[64];
	unsigned int DocID;
	int n;

	DocIDFILE = lotOpenFileNoCasheByLotNr(1,"DocID","r",'e',subname);


	if (DocIDFILE == 0) {
		printf("file sise is 0\n");
		DocID = 0;
	}
	else {
		fstat(fileno(DocIDFILE),&inode);

		if ((n =fread(&buff,sizeof(char),inode.st_size,DocIDFILE)) != inode.st_size) {
			printf("dident read %i char, but %i\n",inode.st_size,n);
			perror("fread");
		}
		//ToDO: støt unsigned int, ikke bare nt
		DocID = atoi(buff);
		//printf("new docid %u = %s\n",DocID,buff);

		fclose(DocIDFILE);
	}		

	return DocID;
}

//hvis vi ikke har et system som slev tar vare på docider
unsigned int rGeneraeADocID (char subname[]) {

	FILE *DocIDFILE;
	struct stat inode;      // lager en struktur for fstat å returnere.
	char buff[64];
	unsigned int DocID;
	int n;

	DocIDFILE = lotOpenFileNoCasheByLotNr(1,"DocID","r",'e',subname);


	if (DocIDFILE == 0) {
		printf("file sise is 0\n");
		DocID = 0;
	}
	else {
		fstat(fileno(DocIDFILE),&inode);

		if ((n =fread(&buff,sizeof(char),inode.st_size,DocIDFILE)) != inode.st_size) {
			printf("dident read %i char, but %i\n",inode.st_size,n);
			perror("fread");
		}
		buff[inode.st_size] = '\0';

		DocID = atou(buff);
		//printf("new docid %u = %s\n",DocID,buff);

		fclose(DocIDFILE);
	}		


	++DocID; //filen holder siste DOcID. Så vi må øke med 1 får dette er neste


	DocIDFILE = lotOpenFileNoCasheByLotNr(1,"DocID","w",'e',subname);

	printf("rGeneraeADocID: writing DocID %u\n",DocID);

	fprintf(DocIDFILE,"%u",DocID);

	fclose(DocIDFILE);

	return DocID;
}

int rApendPostcompress (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl_allow[], char acl_denied[]) {
	#ifdef DEBUG
		printf("rApendPostcompress: starting\n");
	#endif

	int error;
	int WorkBuffSize = (*ReposetoryHeader).htmlSize;
	char *WorkBuff;

	WorkBuff = malloc(WorkBuffSize);
	int HtmlBufferSize = (*ReposetoryHeader).htmlSize;	

	
	if ( (error = compress((Bytef *)WorkBuff,(uLongf *)&WorkBuffSize,(Bytef *)htmlbuffer,(uLongf)HtmlBufferSize)) != 0) {
                printf("compress error. Code: %i\n",error);
		return 0;
	}

	(*ReposetoryHeader).htmlSize = WorkBuffSize;


	rApendPost(ReposetoryHeader,WorkBuff,imagebuffer,subname,acl_allow,acl_denied);

	free(WorkBuff);

	#ifdef DEBUG
		printf("rApendPostcompress: finished\n");
	#endif
}
int rApendPost (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl_allow[], char acl_denied[]) {

	//finner ut når dette ble gjort
	#ifdef BLACK_BOKS
	(*ReposetoryHeader).storageTime = time(NULL);
	#endif

	FILE *RFILE;

	if ((*ReposetoryHeader).DocID == 0) {
		printf("støte for 0 DocIDer er avsluttet. buk bbdocoment_* interfase i steden\n");
		exit(1);
	}

        RFILE = lotOpenFile((*ReposetoryHeader).DocID,"reposetory","r+b",'e',subname);


        //søker til slutten
        fseek(RFILE,0,SEEK_END);

	//skriver verson
	#ifdef BLACK_BOKS
		unsigned int CurrentReposetoryVersionAsUInt = CurrentReposetoryVersion;
		fwrite(&CurrentReposetoryVersionAsUInt,sizeof(unsigned int),1,RFILE);
	#endif

	//skriver hedder
	fwrite(ReposetoryHeader,sizeof(struct ReposetoryHeaderFormat),1,RFILE);

	//skriver html
	fwrite(htmlbuffer,(*ReposetoryHeader).htmlSize,1,RFILE);

	//skriver bilde
	fwrite(imagebuffer,(*ReposetoryHeader).imageSize,1,RFILE);
	debug("did write image of %i bytes",(*ReposetoryHeader).imageSize);

	//skriver acl
	#ifdef BLACK_BOKS
		fwrite(acl_allow,(*ReposetoryHeader).acl_allowSize,1,RFILE);
		#ifdef IIACL
		fwrite(acl_denied,(*ReposetoryHeader).acl_deniedSize,1,RFILE);
		#endif
	#endif

        //skriver record seperator
        fwrite("***",sizeof(char),3,RFILE);

	#ifdef BLACK_BOKS
		//markerer at den er skitten
		FILE *dirtfh;
		dirtfh = lotOpenFileNoCashe((*ReposetoryHeader).DocID,"dirty","ab",'e',subname);
		fwrite("1",1,1,dirtfh); 
		fclose(dirtfh);
	#endif
	
}

int rReadSummary(const unsigned int DocID,char **metadesc, char **title, char **body ,unsigned int radress64bit,unsigned short rsize,char subname[]) {

	FILE *SFILE;
	char WorkBuff[300000];
	char HtmlBuffer[300000];
	char *cptr, *HtmlBufferPtr;
	int size;
	int i;
	unsigned int DocID_infile;

	unsigned int HtmlBufferSize;
	int n, nerror;

	if ((SFILE = lotOpenFileNoCashe(DocID,"summary","rb",'s',subname)) == NULL) {
		return 0;
	}

	if (fseeko64(SFILE,(off_t)radress64bit,SEEK_SET) == -1) {
        	printf("seek problem\n");
                perror("fseeko64");
		return 0;
	}
	
	if ((n=fread(&DocID_infile,sizeof(unsigned int),1,SFILE)) != 1) {
                perror("read");
	}
	printf("DocID %u, DocID_infile %u\nrsize %u\n",DocID,DocID_infile,rsize);

	if (DocID_infile != DocID) {
		printf("DocID_infile != DocID. Summery point to wron summery\n");
		return 0;
	}

	if ((n=fread(WorkBuff,rsize,1,SFILE)) != 1) {
        	printf("cant read. n = %i, rsize = %i\n",n,rsize);
                perror("read");
	}

	HtmlBufferSize = sizeof(HtmlBuffer);
	if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)&HtmlBufferSize,(Bytef *)WorkBuff,rsize)) != 0) {
        	printf("uncompress error. Code: %i for DocID %u-%i\n",nerror,DocID,rLotForDOCid(DocID));

		return 0;
	}

	//er det ikke \0 med i buferren ??
       	HtmlBuffer[HtmlBufferSize] = '\0';

	//finner titel, meat og body
	//printf("HtmlBuffer:\n%s\n\n",HtmlBuffer);


	HtmlBufferPtr = HtmlBuffer;


	if ((cptr = strchr(HtmlBufferPtr,'\n')) != NULL) {
	
		//title
		size = (unsigned int)((unsigned int)cptr - (unsigned int)HtmlBufferPtr);
		printf("size %i\n",size);
	
		//printf("title size %i\n",size);
		*title = malloc(size +1);
		//strncpy(&(*title)[0],HtmlBufferPtr,size);
		for (i=0;i<size;i++) {
			(*title)[i] = HtmlBufferPtr[i];
		}
		(*title)[size] = '\0';

		printf("title %s, size %i, len %i, len2 %i\n",*title,size,strlen(*title),strlen(&(*title)[0]));

		++cptr;
		HtmlBufferPtr = cptr;

	}
	else {
		printf("cant find title\n");
		*title = malloc(1);
		*title[0] = '\0';
	}


	//metadesc
	if ((cptr = strchr(HtmlBufferPtr,'\n')) != NULL ) {
		cptr = strchr(HtmlBufferPtr,'\n');
		size = (cptr - HtmlBufferPtr);
		
		//printf("metadesc size %i\n",size);
		*metadesc = malloc(size +1);
		//strncpy(*metadesc,HtmlBufferPtr,size);
		for (i=0;i<size;i++) {
			(*metadesc)[i] = HtmlBufferPtr[i];
		}
		(*metadesc)[size] = '\0';

		++cptr;
		HtmlBufferPtr = cptr;

	}
	else {
		*metadesc = malloc(1);
		*metadesc[0] = '\0';
	}




	*body = malloc(strlen(HtmlBufferPtr) +1);
	strcpy(*body,HtmlBufferPtr);


	//printf("title: %s\nmetadesc: %s\n",*title,*metadesc);

	fclose(SFILE);

	return 1;
}

//leser en post
int rReadHtml (char HtmlBuffer[],unsigned int *HtmlBufferSize,unsigned int radress64bit,unsigned int 
		rsize,unsigned int DocID,char subname[],struct ReposetoryHeaderFormat *ReposetoryHeader,
		char **acl_allowbuffer,char **acl_deniedbuffer) {

	FILE *RFILE;
	int error;
	int forreturn = 0;
	char WorkBuff[300000];
	char recordseparator[5];

	//temp44: Bytef *WorkBuff;
	int n;
	//64bitadress;
	//off_t radress64bit;


	//kontrolerer at vi ikke blir bett om å lese 0 bytes
	if (rsize == 0) {
		printf("rsize is 0\n");
		return 0;
	}
	else {

		//ToDo: domt å ralikere minne for hvergang vi trenger buffer plass
		//temp44: WorkBuff = malloc(rsize * 5);

		//opner riktig fil
        	//RFILE = ropenlot(DocID,"rb");
		//temp: noe bugg her som gjør at vi segfeiler. Prøver å endre fra "rb" til "r+b"
		//RFILE = lotOpenFile(DocID,"reposetory","rb",'s',subname);
		RFILE = lotOpenFileNoCashe(DocID,"reposetory","rb",'s',subname);

		//3 nov 2006: radress64bit = radress64bit + sizeof(struct ReposetoryHeaderFormat);

		//printf("fseeko64\n");
		if (fseeko64(RFILE,(off_t)radress64bit,SEEK_SET) == -1) {
			printf("seek problem\n");
			perror("fseeko64");
		}		
		//printf("end fseeko64 errno: %i\n",errno);

		//rReadPost(FILE *LotFileOpen,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
                //        char imagebuffer[],char **aclbuffer,char recordseparator[])

		
		rReadPost(RFILE,ReposetoryHeader,WorkBuff,sizeof(WorkBuff),NULL,acl_allowbuffer,acl_deniedbuffer,recordseparator);

		#ifdef DDEBUG
		printf("acl \"%s\"\n",(*aclbuffer));
		#endif

	
		if ( (error = uncompress((Bytef*)HtmlBuffer,(uLong *)HtmlBufferSize,(Bytef *)WorkBuff,rsize)) != 0) {
                	printf("uncompress error. Code: %i for DocID %u-%i\n",error,DocID,rLotForDOCid(DocID));
        		
			forreturn = 0;
		}
		else {

			//er det ikke \0 med i buferren ??
			HtmlBuffer[(*HtmlBufferSize)] = '\0';

			forreturn = 1;
		}

			//temp44: free(WorkBuff);

		//printf("returning %i\n",forreturn);

		fclose(RFILE);

		return forreturn;
	}
	

}

int rReadPost(FILE *LotFileOpen,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
			char imagebuffer[],char **acl_allowbuffer,char **acl_deniedbuffer,char recordseparator[]) {
		
		int n;

		#ifdef BLACK_BOKS
	                unsigned int CurrentReposetoryVersionAsUInt;
	                fread(&CurrentReposetoryVersionAsUInt,sizeof(unsigned int),1,LotFileOpen);
        	#endif

		//leser hedder
		if (fread(ReposetoryHeader,sizeof(struct ReposetoryHeaderFormat),1,LotFileOpen) != 1) {
			perror("cant read ReposetoryHeader");
		}

		#ifdef DEBUG
			printf("ReposetoryHeader:\n");
			printf("\tDocID: %u\n",(*ReposetoryHeader).DocID);
			printf("\turl: \"%s\"\n",(*ReposetoryHeader).url);
			printf("\thtmlSize: %ho\n",(*ReposetoryHeader).htmlSize);
			printf("\timageSize: %ho\n",(*ReposetoryHeader).imageSize);
			printf("\n");
		#endif


		if (htmlbufferSize < (*ReposetoryHeader).htmlSize) {
			printf("htmlSize lager then buffer. %i\n",htmlbufferSize);
		}


		if ((*ReposetoryHeader).htmlSize == 0) {
			#ifdef DEBUG
				printf("htmlSize is 0. Skipping to read it\n");
			#endif
		}
		else if (htmlbuffer == NULL) {
			//hvis vi ikke har en buffer å putte htmlen inn i søker vi bare over
			fseek(LotFileOpen,(*ReposetoryHeader).htmlSize,SEEK_CUR);
		}
		else {
			if (fread(htmlbuffer,(*ReposetoryHeader).htmlSize,1,LotFileOpen) != 1) {
				printf("can't read html. HtmlSize %hu \n",(*ReposetoryHeader).htmlSize);
				perror("fread");
			}
		}

		//må ha #ifdef, slik at vi ikke kaller ftell unødvendig, når vi ikke er i debug modus
		#ifdef DEBUG
		debug("image is at %u\n",(unsigned int)ftell(LotFileOpen));
		#endif

		if ((*ReposetoryHeader).imageSize == 0) {
			//printf("imageSize is 0. Skipping to read it\n");
		}
		else if (imagebuffer == NULL) {
			//hvis vi ikke har en buffer å putte bilde inn i søker vi bare over
			fseek(LotFileOpen,(*ReposetoryHeader).imageSize,SEEK_CUR);

		}
		else {
			if ((n=fread(imagebuffer,1,(*ReposetoryHeader).imageSize,LotFileOpen)) != (*ReposetoryHeader).imageSize) {
				printf("can't read image. Did read %i bytes of ImageSize %hu \n",n,(*ReposetoryHeader).imageSize);
			}
		}

		//leser acl
		#ifdef BLACK_BOKS

			//begrenser størelsen på en acl. Slik at en klikk ikke gjør at alt ikke fungerer. Må tenke på om 200 er nokk størelse her
			if ((*ReposetoryHeader).acl_allowSize > 200) {
				printf("bad acl_allowSize. size %i\n",(*ReposetoryHeader).acl_allowSize);
				return 0;
			}
			#ifdef IIACL
			if ((*ReposetoryHeader).acl_deniedSize > 200) {
				printf("bad acl_deniedSize. size %i\n",(*ReposetoryHeader).acl_deniedSize);
				return 0;
			}
			#endif

			
			//#ifdef DEBUG
			printf("acl_allow size %i\n",(*ReposetoryHeader).acl_allowSize);
			//#endif
			(*acl_allowbuffer) = malloc((*ReposetoryHeader).acl_allowSize +1);
			if ((*ReposetoryHeader).acl_allowSize != 0) {
				if (fread((*acl_allowbuffer),(*ReposetoryHeader).acl_allowSize,1,LotFileOpen) != 1) {
					printf("cant't read acl_allow. acl_allow size %i\n",(*ReposetoryHeader).acl_allowSize);
					perror("");
				}
			}
			(*acl_allowbuffer)[(*ReposetoryHeader).acl_allowSize] = '\0';

			#ifdef IIACL
			#ifdef DEBUG
			printf("did read acl_allow %i b, that vas \"%s\"\n",(*ReposetoryHeader).acl_allowSize,(*acl_allowbuffer));
			#endif


			//#ifdef DEBUG
			printf("acl_denied size %i\n",(*ReposetoryHeader).acl_deniedSize);
			//#endif
			(*acl_deniedbuffer) = malloc((*ReposetoryHeader).acl_deniedSize +1);
			if ((*ReposetoryHeader).acl_deniedSize != 0) {
				if (fread((*acl_deniedbuffer),(*ReposetoryHeader).acl_deniedSize,1,LotFileOpen) != 1) {
					printf("cant't read acl_denied. acl_denied size %i\n",(*ReposetoryHeader).acl_deniedSize);
					perror("");
				}
			}
			(*acl_deniedbuffer)[(*ReposetoryHeader).acl_deniedSize] = '\0';

			#ifdef DEBUG
			printf("did read acl_denied %i b, that vas \"%s\"\n",(*ReposetoryHeader).acl_deniedSize,(*acl_deniedbuffer));
			#endif

			#endif
		#else
			//(*aclbuffer) = NULL;
		#endif
		
	

		if(fread(recordseparator,sizeof(char),3,LotFileOpen) != 3) {
			perror("cant read recordseperator");
		}

}

/*
tar et lott nr inn og henter neste post

Den vil retunere 1 så lenge det er data og lese. slik at man kan ha en lopp slik

while (rGetNext(LotNr,ReposetoryData)) {

	..gjør noe med ReposetoryData..

}
*/
int rGetNext (unsigned int LotNr, struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], 
int htmlbufferSize, char imagebuffer[], unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset,
char subname[], char **acl_allowbuffer,char **acl_deniedbuffer) {

	//global variabel for rGetNext
	static FILE *LotFileOpen;
	static int LotOpen = -1;
	unsigned int startOffset,stoppOffset;
	int rscount;
	int found = 0;	
	char c;
	int i;
	struct stat inode;      // lager en struktur for fstat å returnere.

	
	char recordseparator[5];


	char FileName[128];
	int error;

	//hvis vi tidligere har indikert at dette er siste run, stopper vi her

	//tester om reposetoriet allerede er open, eller ikke
	if (LotOpen != LotNr) {
		//hvis den har vært open, lokker vi den. Hvi den er -1 er den ikke brukt enda, så ingen vits å å lokke den da :-)
		if (LotOpen != -1) {
			fclose(LotFileOpen);
		}
		GetFilPathForLot(FileName,LotNr,subname);
		strncat(FileName,"reposetory",128);

		//printf("Opending lot %s\n",FileName);

		if ( (LotFileOpen = fopen(FileName,"rb")) == NULL) {
			perror(FileName);
			exit(1);
		}
		LotOpen = LotNr;

		//hvis vi fikk en offset skal vi søke ditt
		if (FileOffset != 0) {
			fseek(LotFileOpen,FileOffset,SEEK_SET);
		}
	}

	/************************************/
	//ToDo: hvorfor får vi ikke eof lenger nede når vi når eof? Men må sjkke her??
	//laterlig
	//struct stat inode;      // lager en struktur for fstat å returnere.
	//fstat(fileno(LotFileOpen),&inode);
	//if (stoppOffset == inode.st_size) {
	//	printf("stoppOffset == inode.st_size\n");
	//	return 0;
	//}
	/************************************/

	#ifdef DEBUG
	printf("\n\nstart rGetNext()\n");
	#endif

	while (!feof(LotFileOpen) && (!found)) { 
	

		//finner hvor i filen vi starter
		startOffset = ftell(LotFileOpen);
		//rReadPost(struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
                //        char imagebuffer[],char **aclbuffer,char recordseparator[])


		rReadPost(LotFileOpen,ReposetoryHeader,htmlbuffer,htmlbufferSize,imagebuffer,acl_allowbuffer,acl_deniedbuffer,recordseparator);

		//runpack(ReposetoryData,buff,bufflength);
		
		//printf("DocID %u url: %s\n",(*ReposetoryHeader).DocID,(*ReposetoryHeader).url);

		//recovery av bad records
		//sjekker om vi kan kalkulere en gyldig lot id fra docIDen, hvis ikke her vi nokk en bad record.
		//eller hvis recordseparator ikke er ***
		//skal søke til record seperatoren og prøve på ny
		if ((rLotForDOCid((*ReposetoryHeader).DocID) != LotNr) || (strncmp(recordseparator,"***",3) != 0)) {
			#ifdef DEBUG
				printf("bad reposetory record\n");
				printf("rLotForDOCid() : %i, LotNr %i\n",rLotForDOCid((*ReposetoryHeader).DocID),LotNr);
				printf("recordseparator %c%c%c\n",recordseparator[0],recordseparator[1],recordseparator[2]);
				fstat(fileno(LotFileOpen),&inode);
				printf("ftel64() %u, inode.st_size %u\n",(unsigned int)ftello64(LotFileOpen),(unsigned int)inode.st_size);
			#endif
			//printf("Recordseparator vas %c%c%c\n",recordseparator[0],recordseparator[1],recordseparator[2]);

			//printf("lotfordociD %lu != lot %lu\n",rLotForDOCid((*ReposetoryHeader).DocID),LotNr);
			//søker oss tilbake til der vi var
			fseek(LotFileOpen,startOffset,SEEK_SET);
			rscount = 0;
			i =0;
			while ((!feof(LotFileOpen)) && (rscount != 3)) {
			
				c = fgetc(LotFileOpen);

				//printf("c %c\n",c);

				if (c == '*') {
					++rscount;
				}
				else {
					rscount = 0;
				}
				++i;
			}
			#ifdef DEBUG
				printf("bad reposetory record recovered. Did search %i bytes forword\n",i);
			#endif

			if (feof(LotFileOpen)) {
				printf("did hit eof trying to recover from bad reposetory record\n");
			}
			//printf("søk for rs ok: %i",rscount);
			//sleep(2);
			//exit(1);
		}
		else {

			//finner adressen på denne recorden
			stoppOffset = ftello64(LotFileOpen);
			#ifdef BLACK_BOKS
				//*radress = ((stoppOffset - sizeof(struct ReposetoryHeaderFormat) - (*ReposetoryHeader).htmlSize - (*ReposetoryHeader).imageSize - (*ReposetoryHeader).aclSize) -3);
			#else
				//*radress = ((stoppOffset - sizeof(struct ReposetoryHeaderFormat) - (*ReposetoryHeader).htmlSize - (*ReposetoryHeader).imageSize) -3);
			#endif
				*radress = startOffset;
			//*rsize = bufflength;

			#ifdef BLACK_BOKS
			//hvis dette er en ny nokk rekord retunerer vi denne
			if ((*ReposetoryHeader).storageTime >= FilterTime){
			#else
			if ((*ReposetoryHeader).time >= FilterTime){
			#endif
				found = 1;
			}
			else {
				#ifdef DEBUG
				printf("To old record. Time \"%u\"\n",(unsigned int)(*ReposetoryHeader).time);
				#endif
			}
		}

		/************************************/
		//ToDo: hvorfor får vi ikke eof lenger nede når vi når eof? Men må sjkke her??
		//laterlig
		fstat(fileno(LotFileOpen),&inode);
		if (ftello64(LotFileOpen) == inode.st_size) {
			#ifdef DEBUG
			printf("stoppOffset == inode.st_size\n");
			printf("reading to eof\n");
			#endif

			//fremprovoserer eof
			while (!feof(LotFileOpen)) {
        	        	c = fgetc(LotFileOpen);
			}
			//return 1; //returnerer at vi har data da dette er siste, og den skal være med
		}
		/************************************/


	}


	/*
	//hvis vi ikke stoppet på grunn EOF retunerer vi 1, slik at dataene kan leses
	if (!feof(LotFileOpen)) {
		return 1;
	}
	else {
	//hvis vi er tom for data stenger vi filen, og retunerer en 0 som sier at vi er ferdig.
		printf("ferdig\n");
		fclose(LotFileOpen);
		return 0;
	}
	*/
	return found;
	
}
void runpack(struct ReposetoryFormat *ReposetoryData,char *inndata,int length) {

	
	int error;
	//char compressBuff[sizeof(ReposetoryData)];

	uLong comprLen = sizeof((*ReposetoryData));

	//printf("comprLen: %i\n",comprLen);
	//uLong comprLen = sizeof(ReposetoryData);

	//int uncompress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
	//compress((Bytef*)compr, &comprLen, (const Bytef*)&compressBuff, len)
	/*
	int i;
	for(i=0;i < length;i++) {
		printf("%i %c - %c\n",i,inndata[i],temp[i]);
	}
	*/
	
	//if ( uncompress((Bytef*)compressBuff,&comprLen,inndata,length) != 0) {
	if ( (error = uncompress((Bytef*)ReposetoryData,&comprLen,(Bytef *)inndata,length)) != 0) {
		//temp: haker ut denne. Bør returnere noe om at vi hadde en feil
		printf("uncompress error. Code: %i\n",error);
	}

	//printf("url: %s\n",(*ReposetoryData).url);

	//ReposetoryData.url[0] = 'e';
	//printf("%s\ncomprLen: %i\n",compressBuff,comprLen);
	
	//sscanf(compressBuff,ReposetoryStorageFormat,&ReposetoryData.DocID,&ReposetoryData.url,&ReposetoryData.content_type,&ReposetoryData.content);
	
	
}



/*
Bilde rutiner
*/

//finer path for et bilde
void rigetpath (int DocID,char *FileName, char subname[]) {

	int LotNr;
	int ImageBucket;
	
	ImageBucket = fmod(DocID,512);

	//finner path
	LotNr = rLotForDOCid(DocID);
	GetFilPathForLot(FileName,LotNr,subname);


	sprintf(FileName,"%simages/%i/",FileName,ImageBucket);

}

//lagre bilde
void risave (int DocID, char *image, int size,char subname[]) {

	char FilePath[128];

	FILE *IMAGEFILEHA;

	char FileName[128];

	rigetpath(DocID,FilePath,subname);

	//lager filnavn fra docid og path
	sprintf(FileName,"%s%i.jpg",FilePath,DocID);

	printf("image: %s\n",FileName);

	//prøver å åpne filen
	if ( (IMAGEFILEHA = fopen(FileName,"r+b")) == NULL ) {
		makePath(FilePath);

		if ( (IMAGEFILEHA = fopen(FileName,"a+b")) == NULL ) {
			perror(FileName);
			exit(0);
		}
	}

	fwrite(image,sizeof(Byte),size,IMAGEFILEHA);

	fclose(IMAGEFILEHA);

}

//legger til en "anchor" (tekst på link)
void anchoradd(unsigned int DocID,char *text,int textsize,char subname[]) {

        FILE *ANCHORFILE;
        int i;

        //printf("textsize %i\n",textsize);

        //fjerner eventuelle SPACE på slutten
        for(i=textsize; ((i>0) && ((text[i] == ' ') || (text[i] == '\0'))); i--) {
                //printf("i: %i, c: %c cn: %i\n",i,text[i],(int)text[i]);
        }
        text[i +1] = '\0';


        //printf("%i : -%s-\n",DocID,text);
        //skaf filhandler
        //FILE *lotOpenFile(int DocID,char resource[],char type[]);
        ANCHORFILE = lotOpenFile(DocID,"anchors","ab",'s',subname);

        //skriver DocID
        fwrite(&DocID,sizeof(unsigned int),1,ANCHORFILE);
        //skriver teksten
        fputs(text,ANCHORFILE);
        //skriver line seperator
        fwrite("***",sizeof(char),3,ANCHORFILE);

}

int anchorGetNext (int LotNr,unsigned int *DocID,char *text,int textlength, unsigned int *radress,unsigned int *rsize,char subname[]) {

        //global variabel for rGetNext
        static FILE *LotFileOpen;
        static int LotOpen = -1;

        int bufflength;




        char FileName[128];
        //char buff[sizeof(*ReposetoryData)];

        //tester om reposetoriet allerede er open, eller ikke
        if (LotOpen != LotNr) {
                //hvis den har vært open, lokker vi den. Hvi den er -1 er den ikke brukt enda, så ingen vits å å lokke den da :-)
                if (LotOpen != -1) {
                        fclose(LotFileOpen);
                }
                GetFilPathForLot(FileName,LotNr,subname);
                strncat(FileName,"anchors",128);

                //printf("Opending lot %s\n",FileName);

                if ( (LotFileOpen = fopen(FileName,"rb")) == NULL) {
                        perror(FileName);
                        exit(1);
                }
                LotOpen = LotNr;
 }


        //hvis det det er data igjen i filen leser vi den
        if (!feof(LotFileOpen)) {

                //inaliserer med lengden på buff, slik at vi ikke får en buffer overflow
                
		//leser DocID
		fread(DocID,sizeof(unsigned int),1,LotFileOpen);

		
		//poper av teksten
                fpop(text,&textlength,LotFileOpen,'*',3);

                //printf("buff: %s\nbufflength: %i\n",buff,bufflength);


                //ReposetoryData.url[0] = 'd';

                //runpack(ReposetoryData,buff,bufflength);

                //printf("url: %s\n",ReposetoryData.url);


                //finner adressen på denne recorden
                *radress = ((ftell(LotFileOpen) - bufflength) -3);
                *rsize = bufflength;

                return 1;
        }
        else {
        //hvis vi er tom for data stenger vi filen, og retunerer en 0 som sier at vi er ferdig.
                printf("ferdig\n");
                fclose(LotFileOpen);
                return 0;
        }

}



void addNewUrlOpen(struct addNewUrlhaFormat *addNewUrlha,int lotNr, char openmode[],char subname[], int nr) {
	(*addNewUrlha).OpenLot = -1;

	char filepath[512];

	sprintf(filepath,"nyeurler.%i",nr);

	if (((*addNewUrlha).NYEURLER = lotOpenFileNoCasheByLotNr(lotNr,filepath,openmode,'e',subname)) == NULL) {
		perror("nyeurler");
		exit(1);
	}
}

void addNewUrl (struct addNewUrlhaFormat *addNewUrlha, struct updateFormat *updatePost) {


       
        
	int y;
        SHA1Context sha;
        char FilePath[512];
        int LotNr;
     	char LotPath[512];
	int err;



                /***********************************************************************
                Kalkulerer sha1 verdi
                ************************************************************************/

                err = SHA1Reset(&sha);
                if (err) {
                        printf("SHA1Reset Error %d.\n", err );
                }


                err = SHA1Input(&sha, (const unsigned char *)(*updatePost).url, strlen((char *)(*updatePost).url));
                if (err) {
                        printf("SHA1Input Error %d.\n", err );
                }

                err = SHA1Result(&sha, (*updatePost).sha1);
                if (err) {
                        printf("SHA1Result Error %d, could not compute message digest.\n", err );
                }

                //printf("%s",Message_Digest);
                /************************************************************************/

                /*
                //debug. vis sha1'n
                for (y=0;y<20;y++) {
                        printf("%i",(int)(*updatePost).sha1[y]);
                }
                printf("\n\n");
                */
                fwrite(updatePost,sizeof(struct updateFormat),1,(*addNewUrlha).NYEURLER);

}


