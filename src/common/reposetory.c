#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <err.h>

#include "define.h"

#include "stdlib.h"
#include "debug.h"
#include "lot.h"
#include "reposetory.h"
#include "sha1.h"
#include "bstr.h"
#include "io.h"
#include "dp.h"
//#include "define.h"
//#include <errno.h>
//extern int errno;

#include <arpa/inet.h> //for inet_aton()
#include <sys/types.h> //for time()
#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

//#define MMAP_REPO
//#define TIME_DEBUG_L



#ifdef MMAP_REPO
       	#include <sys/types.h>
       	#include <sys/stat.h>
       	#include <fcntl.h>
       	#include <sys/mman.h>

#endif


#ifdef TIME_DEBUG_L
        #include "timediff.h"
#endif

#include <inttypes.h>

//temp77
#include <locale.h>

#define CurrentReposetoryVersion 4

int findLotToIndex(char subname[], int dirty) {

	FILE *FH;
	struct stat inode;      // lager en struktur for fstat å returnere.
	int lotNr;
	time_t tloc;

	time(&tloc);

	for (lotNr=1;lotNr<maxLots;lotNr++) {

		if (dirty == -1) {
			//hvis dirty er -1 skal vi indeksere selv om vi ikke har noen dirty fil, så vi sjekker om det er et reposetory isteden. 
			// men bare hvis vi ikke har noen dirty fil
			if ((FH = lotOpenFileNoCasheByLotNr(lotNr,"dirty","r", 's',subname)) != NULL) {

			}
			else if ((FH = lotOpenFileNoCasheByLotNr(lotNr,"reposetory","r", 's',subname)) != NULL) {

			}
			else {
	                	#ifdef DEBUG
				perror("local dirty");
				#endif
	
				continue;
        		}

		}
		else {

			if ((FH = lotOpenFileNoCasheByLotNr(lotNr,"dirty","r", 's',subname)) == NULL) {
	                	#ifdef DEBUG
				perror("local dirty");
				#endif
	
				continue;
        		}
		}

		fstat(fileno(FH),&inode);

                printf("dirty file for lotNr %i is of size %"PRId64"\n",lotNr,inode.st_size);

		if (inode.st_size == 0) {
			printf("dirty file is emty. Skipping\n");

			goto next;
		}

		if (inode.st_size < dirty) {
			printf("Lot is't dirty enough\n");

			goto next;
		}

		//må ikke ha blitt modifisert sise 3 timer
		if (inode.st_mtime > (tloc - (3600 * 3))) {
			printf("to new dirty file. Lot isent stable\n");
			
			goto next;
		}



		//tester at det er nokk plass
	        if (!lotHasSufficientSpace(lotNr,4096,subname)) {
	                printf("insufficient disk space\n");
			goto next;
                }


		//vi har en lot som er klar for indeksering
		fclose(FH);
		return lotNr;


		next:
	        fclose(FH);
		

	}

	return 0;
}

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

	if (httpResponsCodes != NULL) {
		RFILE = lotOpenFileNoCasheByLotNr(LotNr,"HttpResponsCodes.txt","wb",'e',subname);

        	//skriver ut en oversikt over hvilkene http responser vi kom over
        	for(i=0;i<nrOfHttpResponsCodes;i++) {
        	      if (httpResponsCodes[i] != 0) {
        	              fprintf(RFILE,"%i: %i\n",i,httpResponsCodes[i]);
        	      }
        	}
		fclose(RFILE);
	}
}

unsigned int GetLastIndexTimeForLot(int LotNr,char subname[]){
        
	FILE *RFILE;
        unsigned int now;
	struct stat inode;      // lager en struktur for fstat å returnere.

        if ((RFILE = lotOpenFileNoCasheByLotNr(LotNr,"IndexTime","rb",'s',subname)) != NULL) {

		fstat(fileno(RFILE),&inode);

		if (inode.st_size == 0) {
			now = 0;
		}
		else {
			fread(&now,sizeof(unsigned int),1,RFILE);
		}

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

	if ((DocIDFILE = lotOpenFileNoCasheByLotNr(1,"DocID","r",'e',subname)) == NULL) {
		return 0;
	}


	if (DocIDFILE == 0) {
		printf("file sise is 0\n");
		DocID = 0;
	}
	else {
		fstat(fileno(DocIDFILE),&inode);

		if ((n =fread(&buff,sizeof(char),inode.st_size,DocIDFILE)) != inode.st_size) {
			printf("dident read %"PRId64" char, but %i\n",inode.st_size,n);
			perror("fread");
		}
		buff[inode.st_size] = '\0';
	
		#ifdef DEBUG
			printf("DocID is \"%s\"\n",buff);
		#endif

		DocID = atou(buff);
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
			printf("dident read %"PRId64" char, but %i\n",inode.st_size,n);
			perror("fread");
		}
		buff[inode.st_size] = '\0';

		DocID = atou(buff);
		//printf("new docid %u = %s\n",DocID,buff);

		fclose(DocIDFILE);
	}		


	++DocID; //filen holder siste DOcID. Så vi må øke med 1 får dette er neste


	DocIDFILE = lotOpenFileNoCasheByLotNr(1,"DocID","w",'e',subname);

	printf("rGeneraeADocID: writing DocID %u, subname \"%s\"\n",DocID,subname);

	fprintf(DocIDFILE,"%u",DocID);

	fclose(DocIDFILE);

	return DocID;
}

int rApendPostcompress (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl_allow[], char acl_denied[], char *reponame) {
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
		printf("WorkBuffSize %i, HtmlBufferSize %i\n",WorkBuffSize,HtmlBufferSize );
		return 0;
	}

	(*ReposetoryHeader).htmlSize = WorkBuffSize;


	rApendPost(ReposetoryHeader,WorkBuff,imagebuffer,subname,acl_allow,acl_denied, reponame);

	free(WorkBuff);

	#ifdef DEBUG
		printf("rApendPostcompress: finished\n");
	#endif
}
unsigned long int rApendPost (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl_allow[], char acl_denied[], char *reponame) {

	unsigned long int offset;

	//finner ut når dette ble gjort
	#ifdef BLACK_BOKS
	if ((*ReposetoryHeader).storageTime == 0) {
		(*ReposetoryHeader).storageTime = time(NULL);
	}
	#endif

	FILE *RFILE;

	if ((*ReposetoryHeader).DocID == 0) {
		printf("støte for 0 DocIDer er avsluttet. buk bbdocoment_* interfase i steden\n");
		exit(1);
	}

        if ((RFILE = lotOpenFile((*ReposetoryHeader).DocID, reponame == NULL ? "reposetory" : reponame,"ab",'e',subname)) == NULL) {
		fprintf(stderr,"Can't open reposetory for DocID %u\n",(*ReposetoryHeader).DocID);
		perror("");
		exit(1);
	}


        //søker til slutten
        if (fseek(RFILE,0,SEEK_END) != 0){
		perror("fseek");
		return 0;
	}

	offset = ftello64(RFILE);

	//skriver verson
	#ifdef BLACK_BOKS
		unsigned int CurrentReposetoryVersionAsUInt = CurrentReposetoryVersion;
		if(fwrite(&CurrentReposetoryVersionAsUInt,sizeof(unsigned int),1,RFILE) < 0) {
                	perror("rApendPost: can't write CurrentReposetoryVersionAsUInt");
        	}
	#endif

	//skriver hedder
	if(fwrite(ReposetoryHeader,sizeof(struct ReposetoryHeaderFormat),1,RFILE) < 0) {
		perror("rApendPost: can't write ReposetoryHeader");
	}

	//skriver html
	if(fwrite(htmlbuffer,(*ReposetoryHeader).htmlSize,1,RFILE) < 0) {
                perror("rApendPost: can't write html");
        }

	//skriver bilde
	if(fwrite(imagebuffer,(*ReposetoryHeader).imageSize,1,RFILE) < 0) {
       	        perror("rApendPost: can't write image");
       	}
	debug("did write image of %i bytes",(*ReposetoryHeader).imageSize);

	//skriver acl
	#ifdef BLACK_BOKS
		if(fwrite(acl_allow,(*ReposetoryHeader).acl_allowSize,1,RFILE) < 0) {
        	        perror("rApendPost: can't write acl_allow");
        	}
		#ifdef IIACL
		if(fwrite(acl_denied,(*ReposetoryHeader).acl_deniedSize,1,RFILE) < 0) {
                	perror("rApendPost: can't write acl_denied");
        	}
		
		#endif
	#endif

        //skriver record seperator
        if(fwrite("***",sizeof(char),3,RFILE) < 0) {
                perror("rApendPost: can't write record seperator");
        }

	//#ifdef BLACK_BOKS
	if ((reponame == NULL) || (strcmp(reponame,"reposetory") == 0)) {
		//markerer at den er skitten
		FILE *dirtfh;
		dirtfh = lotOpenFileNoCashe((*ReposetoryHeader).DocID,"dirty","ab",'e',subname);
		fwrite("1",1,1,dirtfh); 
		fclose(dirtfh);
	}
	//#endif
	
	#ifdef DEBUG
	printf("rApendPost: did append %u, url: \"%s\", into subname \"%s\"\n",(*ReposetoryHeader).DocID,(*ReposetoryHeader).url,subname);
	#endif

	#ifdef BLACK_BOKS
		printf("rApendPost: acl_allow: \"%s\"\n",acl_allow);	
		printf("rApendPost: acl_denied:  \"%s\"\n",acl_denied);	
	#endif

	return offset;
}

int rReadSummary_post(const unsigned int DocID,char **metadesc, char **title, char **body ,unsigned int radress64bit,unsigned short rsize,char subname[], 	int fd) {

        #ifdef TIME_DEBUG_L
                struct timeval start_time, end_time;
		// for totalt tid i funksjonen
                struct timeval tot_start_time, tot_end_time;
        #endif

        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_start_time, NULL);
        #endif

	if (fd == -1) {
		printf("rReadSummary_l: fd is -1\n");
		return 0;
	}

	//FILE *SFILE;
	char *WorkBuff_p;
	char HtmlBuffer[300000];
	char *cptr, *HtmlBufferPtr;
	int size;
	int i;
	unsigned int DocID_infile;

	unsigned int HtmlBufferSize;
	int n, nerror;

        #ifdef DISK_PROTECTOR
                dp_lock(rLotForDOCid(DocID));
        #endif


	#ifdef MMAP_REPO
        	#ifdef TIME_DEBUG_L
        	        gettimeofday(&start_time, NULL);
        	#endif

				char *WorkBuff;

                                off_t mmap_offset, mmap_size;

                                mmap_offset = radress64bit % getpagesize();

				#ifdef DEBUG
                                printf("mmap Adress %i, page size %i, mmap_offset %i\n",radress64bit,getpagesize(),mmap_offset);
				#endif

                                mmap_size = rsize + sizeof(DocID_infile) + mmap_offset;
                                if ((WorkBuff = mmap(0,mmap_size,PROT_READ,MAP_SHARED,fd,(radress64bit - mmap_offset)) ) == NULL) {
                                        perror("mmap");
                                }

                                WorkBuff_p = WorkBuff;

                                WorkBuff_p += mmap_offset;

        	#ifdef TIME_DEBUG_L
        	        gettimeofday(&end_time, NULL);
        	        printf("Time debug: rReadSummary mmap %f for DocID %u\n",getTimeDifference(&start_time,&end_time),DocID);
        	#endif

	#else

		char WorkBuff[300000];

		WorkBuff_p = WorkBuff;
	
        	#ifdef TIME_DEBUG_L
        	        gettimeofday(&start_time, NULL);
        	#endif

		if (lseek64(fd,(off_t)radress64bit,SEEK_SET) == -1) {
        		printf("seek problem\n");
                	perror("fseeko64");
			return 0;
		}

        	#ifdef TIME_DEBUG_L
        	        gettimeofday(&end_time, NULL);
        	        printf("Time debug: rReadSummary disk seek %f for DocID %u-%i (\"%s\")\n",getTimeDifference(&start_time,&end_time),DocID,rLotForDOCid(DocID),returnFilPathForLot(rLotForDOCid(DocID),subname));
        	#endif


       		#ifdef TIME_DEBUG_L
                	gettimeofday(&start_time, NULL);
        	#endif

		//leser både DocID og data i samme jafs, så kopierer DocID inn rikit etterpå.
		if ((n=read(fd,WorkBuff,rsize + sizeof(DocID_infile))) != (rsize + sizeof(DocID_infile))) {
        		printf("cant read. n = %i, rsize = %i\n",n,rsize);
        	        perror("read");
		}


        	#ifdef TIME_DEBUG_L
        	        gettimeofday(&end_time, NULL);
        	        printf("Time debug: rReadSummary disk read %f for DocID %u-%i, size %i (file: \"%s\")\n",getTimeDifference(&start_time,&end_time),DocID,rLotForDOCid(DocID),rsize + sizeof(DocID_infile),returnFilPathForLot(rLotForDOCid(DocID),subname));
        	#endif
	#endif

        #ifdef DISK_PROTECTOR
                dp_unlock(rLotForDOCid(DocID));
        #endif

	memcpy(&DocID_infile,WorkBuff_p,sizeof(DocID_infile));

	WorkBuff_p += sizeof(DocID_infile);

	if (DocID_infile != DocID) {
		printf("DocID_infile != DocID. Summery point to wrong summery\n");
		//return 0;
		goto rReadSummary_error;
	}

        #ifdef TIME_DEBUG_L
                gettimeofday(&start_time, NULL);
        #endif


	HtmlBufferSize = sizeof(HtmlBuffer);
	if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)&HtmlBufferSize,(Bytef *)WorkBuff_p,rsize)) != 0) {
        	printf("uncompress error. Code: %i for DocID %u-%i\n",nerror,DocID,rLotForDOCid(DocID));
		printf("HtmlBufferSize %i, rsize %i",HtmlBufferSize,rsize);

		//return 0;
		goto rReadSummary_error;

	}
        #ifdef TIME_DEBUG_L
                gettimeofday(&end_time, NULL);
                printf("Time debug: rReadSummary uncompress time %f for DocID %u\n",getTimeDifference(&start_time,&end_time),DocID);
        #endif

        #ifdef TIME_DEBUG_L
                gettimeofday(&start_time, NULL);
        #endif

	//er det ikke \0 med i buferren ??
       	HtmlBuffer[HtmlBufferSize] = '\0';

	//finner titel, meta og body

	HtmlBufferPtr = HtmlBuffer;


	if ((cptr = strchr(HtmlBufferPtr,'\n')) != NULL) {
	
		//title
		size = (unsigned int)((unsigned int)cptr - (unsigned int)HtmlBufferPtr);
		#ifdef DEBUG
		printf("size %i\n",size);
		#endif
		//printf("title size %i\n",size);
		*title = malloc(size +1);
		//strncpy(&(*title)[0],HtmlBufferPtr,size);
		for (i=0;i<size;i++) {
			(*title)[i] = HtmlBufferPtr[i];
		}
		(*title)[size] = '\0';

		#ifdef DEBUG
		printf("title %s, size %i, len %i, len2 %i\n",*title,size,strlen(*title),strlen(&(*title)[0]));
		#endif

		++cptr;
		HtmlBufferPtr = cptr;

	}
	else {
		printf("can't find title\n");
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


        #ifdef TIME_DEBUG_L
                gettimeofday(&end_time, NULL);
                printf("Time debug: rReadSummary unmarshall time %f for DocID %u\n",getTimeDifference(&start_time,&end_time),DocID);
        #endif


	#ifdef MMAP_REPO
		munmap(WorkBuff,mmap_size);
	#endif


        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_end_time, NULL);
        	printf("Time debug: rReadSummary total time %f for DocID %u-%i, size %i (file: \"%s\")\n",getTimeDifference(&tot_start_time,&tot_end_time),DocID,rLotForDOCid(DocID),rsize + sizeof(DocID_infile),returnFilPathForLot(rLotForDOCid(DocID),subname));
        #endif

	return 1;


	//hånterer error
	rReadSummary_error:
	#ifdef MMAP_REPO
		munmap(WorkBuff,mmap_size);
	#endif

	return 0;

}

int rReadSummary(const unsigned int DocID,char **metadesc, char **title, char **body ,unsigned int radress64bit,unsigned short rsize,char subname[]) {

	int ret = 0;
	int fd;

	if ((fd = lotOpenFileNoCashel(DocID,"summary","rb",'s',subname)) == -1) {
		return 0;
	}
	ret = rReadSummary_post(DocID,metadesc,title,body,radress64bit,rsize,subname,fd);

	close(fd);

	return ret;
}
int rReadSummary_l(const unsigned int DocID,char **metadesc, char **title, char **body ,unsigned int radress64bit,unsigned short rsize,char subname[], 	int fd) {

	int ret = 0;

	if (fd == -1) {
		printf("rReadSummary_l: fil was'n open, wil open it myself.\n");
		ret = rReadSummary(DocID,metadesc,title,body,radress64bit,rsize,subname);
	}
	else {
		ret = rReadSummary_post(DocID,metadesc,title,body,radress64bit,rsize,subname,fd);
	}

	return ret;
}


//#define DO_DIRECT

//leser en post
int rReadHtml (char HtmlBuffer[],unsigned int *HtmlBufferSize,unsigned int radress64bit,unsigned int 
		rsize,unsigned int DocID,char subname[],struct ReposetoryHeaderFormat *ReposetoryHeader,
		char **acl_allowbuffer,char **acl_deniedbuffer, unsigned int imagesize) {

        #ifdef TIME_DEBUG_L
                struct timeval start_time, end_time;
		// for totalt tid i funksjonen
                struct timeval tot_start_time, tot_end_time;
        #endif

        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_start_time, NULL);
        #endif

	int fd;
	off_t offset = radress64bit;
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


	#ifdef DISK_PROTECTOR
		dp_lock(rLotForDOCid(DocID));
	#endif

#ifndef DO_DIRECT
	#ifdef BLACK_BOKS
	fd = lotOpenFileNoCashel(DocID,"reposetory","rb",'n',subname);
	#else
	//og s
	fd = lotOpenFileNoCashel(DocID,"reposetory","rb",'s',subname);
	#endif

	//3 nov 2006: radress64bit = radress64bit + sizeof(struct ReposetoryHeaderFormat);
	if (fd == -1) {
		//return 0;
 
		forreturn = 0;
		goto rReadHtml_end;

	}
#else
	fd = lotOpenFileNoCache_direct(DocID, "reposetory", "r", 's', subname);	
#endif

	//printf("fseeko64\n");
       	#ifdef TIME_DEBUG_L
       	        gettimeofday(&start_time, NULL);
        #endif

	if (lseek64(fd,offset,SEEK_SET) == -1) {
		warn("fseeko64: DocID %u, fd %d, adress off_t %"PRId64", adress given %u, rsize %u",DocID, fd,offset,radress64bit,rsize);
		//return 0;
		forreturn = 0;
		goto rReadHtml_end;
	}		


       	#ifdef TIME_DEBUG_L
               	gettimeofday(&end_time, NULL);
       	        printf("Time debug: rReadHtml disk seek time %f for DocID %u\n",getTimeDifference(&start_time,&end_time),DocID);
        #endif


	#ifdef DO_DIRECT
		rReadPost2_fd(fd,ReposetoryHeader,WorkBuff,sizeof(WorkBuff),NULL,acl_allowbuffer,acl_deniedbuffer,
			recordseparator,rsize,imagesize);

	#else
		rReadPost2(fd,ReposetoryHeader,WorkBuff,sizeof(WorkBuff),NULL,acl_allowbuffer,acl_deniedbuffer,
			recordseparator,rsize,imagesize);
	#endif



	#ifdef DDEBUG
	printf("acl \"%s\"\n",(*aclbuffer));
	#endif

	
	if ( (error = uncompress((Bytef*)HtmlBuffer,(uLong *)HtmlBufferSize,(Bytef *)WorkBuff,rsize)) != 0) {
               	printf("uncompress error. Code: %i for DocID %u-%i\n",error,DocID,rLotForDOCid(DocID));
        		
		HtmlBuffer[0] = '\0';
		(*HtmlBufferSize) = 0;

		forreturn = 0;
	}
	else {

		//er det ikke \0 med i buferren ??
		HtmlBuffer[(*HtmlBufferSize)] = '\0';

		forreturn = 1;
	}
		//temp44: free(WorkBuff);



	rReadHtml_end:				
	if (fd != -1) {
		close(fd);
	}

	#ifdef DISK_PROTECTOR
		dp_unlock(rLotForDOCid(DocID));
	#endif


        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_end_time, NULL);
                printf("Time debug: rReadHtml total time %f for DocID %u\n\n",getTimeDifference(&tot_start_time,&tot_end_time),DocID);
        #endif

	return forreturn;
}

//copy a memory area, and return the size copyed
#ifdef DEBUG
	static size_t memcpyrc(void *s1, const void *s2, size_t n) {
#else
	static inline size_t memcpyrc(void *s1, const void *s2, size_t n) {
#endif

        memcpy(s1,s2,n);

        return n;
}


int rReadPost2_fd(int fd,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
			char imagebuffer[],char **acl_allowbuffer,char **acl_deniedbuffer,char recordseparator[],
			unsigned int rsize,unsigned int imagesize) {

        #ifdef TIME_DEBUG_L
                struct timeval start_time, end_time;
		// for totalt tid i funksjonen
                struct timeval tot_start_time, tot_end_time;
        #endif

        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_start_time, NULL);
        #endif


	if (htmlbufferSize < rsize) {
		printf("htmlSize lager then buffer. %i\n",htmlbufferSize);
		return 0;
	}
		

	#ifdef BLACK_BOKS
                unsigned int CurrentReposetoryVersionAsUInt;
                read(fd, &CurrentReposetoryVersionAsUInt,sizeof(unsigned int));
       	#endif

	//regner ut totalt hva vi skal lese
	int totalread;
	char *totalpost;
	char *totalpost_p;

	totalread = sizeof(struct ReposetoryHeaderFormat) + rsize + imagesize +3;
	if ((totalpost = malloc(totalread)) == NULL) {
		perror("malloc");
		return 0;
	}

	printf("totalpost: %p\n", totalpost);
	if (io_read_align(fd, totalpost, totalread) != totalread) {
		warn("cant read totalread %d", totalread);
	}
	printf("read everything... %p\n", totalpost);

	totalpost_p = totalpost;

	//hedder	
	totalpost_p += memcpyrc(ReposetoryHeader,totalpost_p,sizeof(struct ReposetoryHeaderFormat));


	if ((*ReposetoryHeader).htmlSize == 0) {
		#ifdef DEBUG
			printf("htmlSize is 0. Skipping to read it\n");
		#endif
	}
	else if (htmlbuffer == NULL) {
		//hvis vi ikke har en buffer å putte htmlen inn i søker vi bare over
		//fseek(LotFileOpen,(*ReposetoryHeader).htmlSize,SEEK_CUR);
	}
	else {
		totalpost_p += memcpyrc(htmlbuffer,totalpost_p,(*ReposetoryHeader).htmlSize);

	}

	//må ha #ifdef, slik at vi ikke kaller ftell unødvendig, når vi ikke er i debug modus
	#ifdef DEBUG
	//debug("image is at %u\n",(unsigned int)ftell(LotFileOpen));
	#endif

	if ((*ReposetoryHeader).imageSize == 0) {
		//printf("imageSize is 0. Skipping to read it\n");
	}
	else if (imagebuffer == NULL) {
		//hvis vi ikke har en buffer å putte bilde inn i søker vi bare over
		//fseek(LotFileOpen,(*ReposetoryHeader).imageSize,SEEK_CUR);
	}
	else {
		totalpost_p += memcpyrc(imagebuffer,totalpost_p,(*ReposetoryHeader).imageSize);

	}

	//leser acl
	#ifdef BLACK_BOKS

		//begrenser størelsen på en acl. Slik at en klikk ikke gjør at alt ikke fungerer. Må tenke på om 2000 er nokk størelse her
		if ((*ReposetoryHeader).acl_allowSize > 2000) {
			printf("bad acl_allowSize. size %i\n",(*ReposetoryHeader).acl_allowSize);
			return 0;
		}
		#ifdef IIACL
		if ((*ReposetoryHeader).acl_deniedSize > 2000) {
			printf("bad acl_deniedSize. size %i\n",(*ReposetoryHeader).acl_deniedSize);
			return 0;
		}
		#endif

			
		#ifdef DEBUG
		printf("acl_allow size %i\n",(*ReposetoryHeader).acl_allowSize);
		#endif
		(*acl_allowbuffer) = malloc((*ReposetoryHeader).acl_allowSize +1);
		if ((*ReposetoryHeader).acl_allowSize != 0) {
			if (read(fd, (*acl_allowbuffer),(*ReposetoryHeader).acl_allowSize) < 0) {
				printf("cant't read acl_allow. acl_allow size %i. at %s:%d\n",(*ReposetoryHeader).acl_allowSize,__FILE__,__LINE__);
				perror("");
			}
		}
		(*acl_allowbuffer)[(*ReposetoryHeader).acl_allowSize] = '\0';

		#ifdef IIACL
		#ifdef DEBUG
		printf("did read acl_allow %i b, that vas \"%s\"\n",(*ReposetoryHeader).acl_allowSize,(*acl_allowbuffer));
		#endif


		#ifdef DEBUG
		printf("acl_denied size %i\n",(*ReposetoryHeader).acl_deniedSize);
		#endif
		(*acl_deniedbuffer) = malloc((*ReposetoryHeader).acl_deniedSize +1);
		if ((*ReposetoryHeader).acl_deniedSize != 0) {
			if (read(fd, (*acl_deniedbuffer),(*ReposetoryHeader).acl_deniedSize) < 0) {
				printf("cant't read acl_denied. acl_denied size %i. At %s:%d\n",(*ReposetoryHeader).acl_deniedSize,__FILE__,__LINE__);
				perror("");
			}
		}
		(*acl_deniedbuffer)[(*ReposetoryHeader).acl_deniedSize] = '\0';

		#ifdef DEBUG
		printf("did read acl_denied %i b, that vas \"%s\"\n",(*ReposetoryHeader).acl_deniedSize,(*acl_deniedbuffer));
		#endif

		#endif

		if(read(fd, recordseparator,sizeof(char)*3) != 3) {
			perror("cant read recordseperator");
		}

	#else
		//(*aclbuffer) = NULL;

		//rart, ser ikke ut til at vi faktsik sjekker om disse er riktige		
		totalpost_p += memcpyrc(recordseparator,totalpost_p,3);

	#endif




	free(totalpost);


	if (strncmp(recordseparator,"***",3)) {
		printf("bad record separator %c%c%c\n",recordseparator[0],recordseparator[1],recordseparator[2]);
	}


	#ifdef DEBUG
		printf("ReposetoryHeader:\n");
		printf("\tDocID: %u\n",(*ReposetoryHeader).DocID);
		printf("\turl: \"%s\"\n",(*ReposetoryHeader).url);
		printf("\thtmlSize: %ho\n",(*ReposetoryHeader).htmlSize);
		printf("\timageSize: %ho\n",(*ReposetoryHeader).imageSize);
		printf("\n");
	#endif

	if ((*ReposetoryHeader).htmlSize != rsize) {
		printf("htmlsize %ho != rzise %ho\n",(*ReposetoryHeader).htmlSize,rsize);
		return 0;
	}


        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_end_time, NULL);
                printf("Time debug: rReadPost2 total time %f\n",getTimeDifference(&tot_start_time,&tot_end_time));
        #endif

	return 1;
}





int rReadPost2(int LotFileOpen,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
			char imagebuffer[],char **acl_allowbuffer,char **acl_deniedbuffer,char recordseparator[],
			unsigned int rsize,unsigned int imagesize) {

        #ifdef TIME_DEBUG_L
		// for totalt tid i funksjonen
                struct timeval tot_start_time, tot_end_time;
        #endif

        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_start_time, NULL);
        #endif
	
	int n;

	if (htmlbufferSize < rsize) {
		printf("htmlSize (%u) lager then buffer. %i\n",rsize,htmlbufferSize);
		return 0;
	}
		

	#ifdef BLACK_BOKS
                unsigned int CurrentReposetoryVersionAsUInt;
                read(LotFileOpen,&CurrentReposetoryVersionAsUInt,sizeof(unsigned int));
       	#endif

	//regner ut totalt hva vi skal lese
	int totalread;
	char *totalpost;
	char *totalpost_p;


	totalread = sizeof(struct ReposetoryHeaderFormat) + rsize + imagesize;
	#ifndef BLACK_BOKS //for bb skal vi lese mer, så lese record seperator
	 	totalread += 3;
	#endif
	

	if ((totalpost = malloc(totalread)) == NULL) {
		perror("malloc");
		return 0;
	}

	if ((n=read(LotFileOpen,totalpost,totalread)) != totalread) {
		fprintf(stderr,"rReadPost2: can't read totalread. Did read %i of %i\n",n,totalread);
		perror("read");
		free(totalpost);
		return 0;
	}

	totalpost_p = totalpost;

	//hedder	
	totalpost_p += memcpyrc(ReposetoryHeader,totalpost_p,sizeof(struct ReposetoryHeaderFormat));



	if ((*ReposetoryHeader).htmlSize == 0) {
		#ifdef DEBUG
			printf("htmlSize is 0. Skipping to read it\n");
		#endif
	}
	else if (htmlbuffer == NULL) {
		//hvis vi ikke har en buffer å putte htmlen inn i søker vi bare over
		//fseek(LotFileOpen,(*ReposetoryHeader).htmlSize,SEEK_CUR);
	}
	else {
		totalpost_p += memcpyrc(htmlbuffer,totalpost_p,(*ReposetoryHeader).htmlSize);

	}

	if ((*ReposetoryHeader).imageSize == 0) {
		//printf("imageSize is 0. Skipping to read it\n");
	}
	else if (imagebuffer == NULL) {
		//hvis vi ikke har en buffer å putte bilde inn i søker vi bare over
		//fseek(LotFileOpen,(*ReposetoryHeader).imageSize,SEEK_CUR);
	}
	else {
		totalpost_p += memcpyrc(imagebuffer,totalpost_p,(*ReposetoryHeader).imageSize);

	}

	//leser acl
	#ifdef BLACK_BOKS

		//begrenser størelsen på en acl. Slik at en klikk ikke gjør at alt ikke fungerer. Må tenke på om 2000 er nokk størelse her
		if ((*ReposetoryHeader).acl_allowSize > 2000) {
			printf("bad acl_allowSize. size %i\n",(*ReposetoryHeader).acl_allowSize);
			return 0;
		}
		#ifdef IIACL
		if ((*ReposetoryHeader).acl_deniedSize > 2000) {
			printf("bad acl_deniedSize. size %i\n",(*ReposetoryHeader).acl_deniedSize);
			return 0;
		}
		#endif

			
		#ifdef DEBUG
		printf("acl_allow size %i\n",(*ReposetoryHeader).acl_allowSize);
		#endif
		(*acl_allowbuffer) = malloc((*ReposetoryHeader).acl_allowSize +1);
		if ((*ReposetoryHeader).acl_allowSize != 0) {
			if (read(LotFileOpen,(*acl_allowbuffer),(*ReposetoryHeader).acl_allowSize) != (*ReposetoryHeader).acl_allowSize) {
				printf("cant't read acl_allow. acl_allow size %i at %s:%d\n",(*ReposetoryHeader).acl_allowSize,__FILE__,__LINE__);
				perror("");
			}
		}
		(*acl_allowbuffer)[(*ReposetoryHeader).acl_allowSize] = '\0';

		#ifdef IIACL
		#ifdef DEBUG
		printf("did read acl_allow %i b, that vas \"%s\"\n",(*ReposetoryHeader).acl_allowSize,(*acl_allowbuffer));
		#endif


		#ifdef DEBUG
		printf("acl_denied size %i\n",(*ReposetoryHeader).acl_deniedSize);
		#endif
		(*acl_deniedbuffer) = malloc((*ReposetoryHeader).acl_deniedSize +1);
		if ((*ReposetoryHeader).acl_deniedSize != 0) {
			if (read(LotFileOpen,(*acl_deniedbuffer),(*ReposetoryHeader).acl_deniedSize) != (*ReposetoryHeader).acl_deniedSize) {
				printf("cant't read acl_denied. acl_denied size %i at %s:%d\n",(*ReposetoryHeader).acl_deniedSize,__FILE__,__LINE__);
				perror("");
			}
		}
		(*acl_deniedbuffer)[(*ReposetoryHeader).acl_deniedSize] = '\0';

		#ifdef DEBUG
		printf("did read acl_denied %i b, that vas \"%s\"\n",(*ReposetoryHeader).acl_deniedSize,(*acl_deniedbuffer));
		#endif

		#endif

		if(read(LotFileOpen,recordseparator,sizeof(char) * 3) != 3) {
			perror("can't read recordseperator");
		}

	#else
		//(*aclbuffer) = NULL;

		//rart, ser ikke ut til at vi faktsik sjekker om disse er riktige		
		totalpost_p += memcpyrc(recordseparator,totalpost_p,3);

	#endif




	free(totalpost);



	if (strncmp(recordseparator,"***",3)) {
		printf("bad record separator %c%c%c\n",recordseparator[0],recordseparator[1],recordseparator[2]);
	}



	#ifdef DEBUG
		printf("ReposetoryHeader:\n");
		printf("\tDocID: %u\n",(*ReposetoryHeader).DocID);
		printf("\turl: \"%s\"\n",(*ReposetoryHeader).url);
		printf("\thtmlSize: %ho\n",(*ReposetoryHeader).htmlSize);
		printf("\timageSize: %ho\n",(*ReposetoryHeader).imageSize);
		printf("\n");
	#endif

	if ((*ReposetoryHeader).htmlSize != rsize) {
		printf("htmlsize %u != rzise %u\n",(*ReposetoryHeader).htmlSize,rsize);
		return 0;
	}


        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_end_time, NULL);
                printf("Time debug: rReadPost2 total time %f\n\n",getTimeDifference(&tot_start_time,&tot_end_time));
        #endif

	return 1;
}

int rReadPost(FILE *LotFileOpen,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
			char imagebuffer[],char **acl_allowbuffer,char **acl_deniedbuffer,char recordseparator[]) {

        #ifdef TIME_DEBUG_L
                struct timeval start_time, end_time;
		// for totalt tid i funksjonen
                struct timeval tot_start_time, tot_end_time;
        #endif

        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_start_time, NULL);
        #endif

		
		int n;

		#ifdef BLACK_BOKS
	                unsigned int CurrentReposetoryVersionAsUInt;
	                if (fread(&CurrentReposetoryVersionAsUInt,sizeof(unsigned int),1,LotFileOpen) != 1) {
				perror("can't read CurrentReposetoryVersionAsUInt");
			}
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
			printf("htmlSize (%hu) lager then buffer. %i\n",(*ReposetoryHeader).htmlSize,htmlbufferSize);
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

			//begrenser størelsen på en acl. Slik at en klikk ikke gjør at alt ikke fungerer. Må tenke på om 2000 er nokk størelse her
			if ((*ReposetoryHeader).acl_allowSize > 2000) {
				printf("bad acl_allowSize. size %i\n",(*ReposetoryHeader).acl_allowSize);
				return 0;
			}
			#ifdef IIACL
			if ((*ReposetoryHeader).acl_deniedSize > 2000) {
				printf("bad acl_deniedSize. size %i\n",(*ReposetoryHeader).acl_deniedSize);
				return 0;
			}
			#endif

			
			#ifdef DEBUG
			printf("acl_deniedSize size %i\n",(*ReposetoryHeader).acl_deniedSize);
			#endif
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


			#ifdef DEBUG
			printf("acl_denied size %i\n",(*ReposetoryHeader).acl_deniedSize);
			#endif

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


        #ifdef TIME_DEBUG_L
                gettimeofday(&tot_end_time, NULL);
                printf("Time debug: rReadPost total time %f\n\n",getTimeDifference(&tot_start_time,&tot_end_time));
        #endif


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

	return rGetNext_reponame(LotNr,ReposetoryHeader,htmlbuffer,htmlbufferSize,imagebuffer,radress,FilterTime,FileOffset,subname,acl_allowbuffer,acl_deniedbuffer,"reposetory");
}

int rGetNext_reponame (unsigned int LotNr, struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], 
int htmlbufferSize, char imagebuffer[], unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset,
char subname[], char **acl_allowbuffer,char **acl_deniedbuffer, char reponame[]) {

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
		//hvis den har vært open, lokker vi den. Hvis den er -1 er den ikke brukt enda, så ingen vits å å lokke den da :-)
		if (LotOpen != -1) {
			fclose(LotFileOpen);
		}
		
		GetFilPathForLot(FileName,LotNr,subname);
		strncat(FileName,reponame,128);

		printf("rGetNext: Opending lot %s\n",FileName);

		if ( (LotFileOpen = fopen(FileName,"rb")) == NULL) {
			perror(FileName);
			return 0;
			//exit(1);
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
	if (!found) {
		printf("ferdig. lokker filen\n");
		LotOpen = -1;
		fclose(LotFileOpen);

		printf("rGetNext: returnerer %i\n",found);
	}

	return found;
	
}
int runpack(char *ReposetoryData,uLong comprLen,char *inndata,int length) {

	
	int error;
	//char compressBuff[sizeof(ReposetoryData)];

	//uLong comprLen = sizeof((*ReposetoryData));

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
		return 0;
	}

	//printf("url: %s\n",(*ReposetoryData).url);

	//ReposetoryData.url[0] = 'e';
	//printf("%s\ncomprLen: %i\n",compressBuff,comprLen);
	
	//sscanf(compressBuff,ReposetoryStorageFormat,&ReposetoryData.DocID,&ReposetoryData.url,&ReposetoryData.content_type,&ReposetoryData.content);
	
	return 1;
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


/* AnchorIndex */
FILE *
anchorIndexOpen(unsigned int DocID, char type, char *subname)
{
	FILE *fp;
	int LotNr;
	char path[1024];

	LotNr = rLotForDOCid(DocID);
	GetFilPathForLot(path, LotNr, subname);

	strcat(path, "anchorIndex");

	if ((fp = fopen(path, "r+")) == NULL) {
		if (type == 'w') {
			fp = fopen(path, "w+");
		}
	}

	if (fp == NULL) {
		printf("anchorIndexOpen: can't open \"%s\"\n",path);
		perror("open()");
	}
	return fp;
}

int
anchorIndexPosition(FILE *fp, unsigned int DocID)
{
	int LotNr;

	LotNr = rLotForDOCid(DocID);
	if (fseek(fp, sizeof(struct anchorIndexFormat) * (DocID - LotDocIDOfset(LotNr)), SEEK_SET) != 0) {
		warn("fseek");
		return 0; 
	}

	return 1;
}

int
anchorIndexWrite(unsigned int DocID, char *subname, off_t offset)
{
	FILE *fp;
	struct anchorIndexFormat ai;

	if ((fp = anchorIndexOpen(DocID, 'w', subname)) == NULL)
		return 0;
	if (!anchorIndexPosition(fp, DocID)) {
		fclose(fp);
		return 0;
	}

	ai.offset = offset;
	if (fwrite(&ai, sizeof(ai), 1, fp) != 1) {
		fclose(fp);
		warn("fwrite");
		return 0;
	}
	fclose(fp);

	return 1;
}

int
anchorIndexRead(unsigned int DocID, char *subname, off_t *offset)
{
	FILE *fp;
	struct anchorIndexFormat ai;

	if ((fp = anchorIndexOpen(DocID, 'w', subname)) == NULL)
		return 0;
	if (!anchorIndexPosition(fp, DocID)) {
		fclose(fp);
		return 0;
	}

	if (fread(&ai, sizeof(ai), 1, fp) != 1) {
		fclose(fp);
		return 0;
	}

	*offset = ai.offset;

	fclose(fp);

	return 1;
}


//legger til en "anchor" (tekst på link)
void
anchoraddnew(unsigned int DocID, char *text, size_t textsize, char *subname, char *filename)
{
        FILE *ANCHORFILE;
        int i;
	struct anchorRepo anchor;
	struct DocumentIndexFormat docindex;
	off_t offset;
	char *newtext;
	char *p;
	int oldlen;
	int LotNr;
	char path[1024];

	LotNr = rLotForDOCid(DocID);

	oldlen = anchorRead(LotNr, subname, DocID, NULL, -1);
	if (oldlen > 0)
		oldlen++;
	if ((newtext = malloc(oldlen + textsize + 1)) == NULL) {
		warn("malloc");
		return;
	}

	if (anchorRead(LotNr, subname, DocID, newtext, oldlen)) {
		//printf("Got: %s\n", newtext);
		p = newtext + oldlen-1;
		strcpy(p, "\n");
		p++;
	} else {
		p = newtext;
	}
	strcpy(p, text);
	//printf("And: %s\n", newtext);


        //printf("textsize %i\n",textsize);

#if 0
        //fjerner eventuelle SPACE på slutten
        for(i=textsize-1; ((i>0) && ((text[i] == ' ') || (text[i] == '\0'))); i--) {
                //printf("i: %i, c: %c cn: %i\n",i,text[i],(int)text[i]);
        }
        text[i +1] = '\0';
        //fjerner eventuelle SPACE på slutten
#endif

        //printf("%i : -%s-\n",DocID,text);
        //skaf filhandler
        //FILE *lotOpenFile(int DocID,char resource[],char type[]);
	GetFilPathForLot(path, LotNr, subname);
	strcat(path, filename == NULL ? "anchors.new" : filename);
	if ((ANCHORFILE = fopen(path, "a")) == NULL) {
		printf("anchoraddnew: Can't open anchorfile\n");
		perror(path);
		exit(1);
	}

	offset = ftello64(ANCHORFILE);
	//printf("Foo: %x\n", offset);

	anchor.magic = ANCHORMAGIC;
	anchor.DocID = DocID;
	anchor.len = oldlen + textsize;
	fwrite(&anchor, sizeof(anchor), 1, ANCHORFILE);
	fwrite(newtext, oldlen + textsize, 1, ANCHORFILE);
	anchorIndexWrite(DocID, subname, offset);
	fclose(ANCHORFILE);
	free(newtext);
}


//legger til en "anchor" (tekst på link)
void anchoradd(unsigned int DocID,char *text,int textsize,char subname[], char *filename) {
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
        ANCHORFILE = lotOpenFile(DocID, filename == NULL ? "anchors" : filename,"ab",'s',subname);

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
		if (fread(DocID,sizeof(unsigned int),1,LotFileOpen) != 1) {
			perror("anchorGetNext can't read DocID");
			return 0;
		}

		
		//poper av teksten
                fpop(text,&textlength,LotFileOpen,'*',3);


                //finner adressen for denne recorden
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


int anchorGetNextnew(int LotNr,unsigned int *DocID,char *text,int textlength, unsigned int *radress,unsigned int *rsize,char *subname, off_t *offset) {
	static FILE *LotFileOpen;
	static int LotOpen = -1;
	int bufflength;
	char FileName[128];
	struct anchorRepo anchor;
	//char buff[sizeof(*ReposetoryData)];

	//tester om reposetoriet allerede er open, eller ikke
	if (LotOpen != LotNr) {
		//hvis den har vært open, lokker vi den. Hvi den er -1 er den ikke brukt enda, så ingen vits å å lokke den da :-)
		if (LotOpen != -1) {
			fclose(LotFileOpen);
		}
		GetFilPathForLot(FileName,LotNr,subname);
		strncat(FileName,"anchors.new",128);

		//printf("Opending lot %s\n",FileName);

		if ( (LotFileOpen = fopen(FileName,"rb")) == NULL) {
			perror(FileName);
			exit(1);
		}
		LotOpen = LotNr;
	}


	if (offset != NULL)
		*offset = ftello64(LotFileOpen);
	if (fread(&anchor, sizeof(anchor), 1, LotFileOpen) == 1) {

		*DocID = anchor.DocID;
		if (anchor.len+1 > textlength) { /* No room for the text */
			fread(text, textlength-1, 1, LotFileOpen);
			text[textlength-1] = '\0';
			/* Skip over the rest of the string */
			fseek(LotFileOpen, anchor.len-textlength+1, SEEK_CUR);
		} else {
			fread(text, anchor.len, 1, LotFileOpen);
			text[anchor.len] = '\0';
		}

		return 1;
	}

	return 0;
}

int
anchorRead(int LotNr, char *subname, unsigned int DocID, char *text, int len)
{
	struct DocumentIndexFormat docindex;
	FILE *fp;
	char path[512];
	struct anchorRepo anchor;
	off_t offset;

	if (!anchorIndexRead(DocID, subname, &offset)) {
		#ifdef DEBUG
			printf("anchorRead: can't anchorIndexRead\n");
		#endif
		return 0;
	}
	GetFilPathForLot(path, LotNr, subname);
	strncat(path, "anchors.new", 512);

	#ifdef DEBUG
		printf("anchors.new file \"%s\"\n",path);
	#endif

	if ((fp = fopen(path, "r")) == NULL) {
		#ifdef DEBUG
			printf("anchorRead: can't open \"%s\"\n",path);
		#endif
		return 0;
	}
	if (fseek(fp, offset, SEEK_SET) == -1) {
		fclose(fp);
		return 0;
	}

	if (fread(&anchor, sizeof(anchor), 1, fp) == 1) {
		#ifdef DEBUG
			printf("anchorRead: got candidate DocID %u\n",anchor.DocID);
		#endif

		if (anchor.DocID != DocID) {
			#ifdef DEBUG
                        	printf("anchorRead: candidate DocID %u is not correct DocID %u\n",anchor.DocID,DocID);
                	#endif
			fclose(fp);
			return 0;
		}

		if (len == -1) {
			fclose(fp);

			return anchor.len;
		} else if (anchor.len+1 > len) { /* No room for the text */
			fread(text, len-1, 1, fp);
			text[len-1] = '\0';
			/* Skip over the rest of the string */
			fseek(fp, anchor.len-len+1, SEEK_CUR);
		} else {
			fread(text, anchor.len, 1, fp);
			text[anchor.len] = '\0';
		}

		fclose(fp);
		return 1;
	}

	fclose(fp);
	return 0;
}

void
addResource(int LotNr, char *subname, unsigned int DocID, char *resource, size_t resourcelen)
{
	char FileName[1024];
	FILE *fp;
	off_t offset;
	struct DocumentIndexFormat docindex;
	int error;
	int WorkBuffSize = resourcelen;
	char *WorkBuff;

	WorkBuff = malloc(WorkBuffSize);
	int HtmlBufferSize = resourcelen;


	GetFilPathForLot(FileName,LotNr,subname);
	strcat(FileName,"resource");

	if ((fp = fopen(FileName, "a")) == NULL) {
		warn("fopen(resource)");
		return;
	}

	if ((error = compress((Bytef *)WorkBuff,(uLongf *)&WorkBuffSize,(Bytef *)resource,(uLongf)HtmlBufferSize)) != 0) {
		printf("compress error. Code: %i\n",error);
		printf("WorkBuffSize %i, HtmlBufferSize %i\n",WorkBuffSize,HtmlBufferSize);
		fclose(fp);
		return;
	}

	offset = ftello64(fp);
	printf("Writing: %d\n", DocID);
	printf("size: %d, %d\n", resourcelen, WorkBuffSize);
	if (fwrite(&DocID, sizeof(DocID), 1, fp) != 1)
		warn("fwrite1");
	if (fwrite(&resourcelen, sizeof(resourcelen), 1, fp) != 1)
		warn("fwrite1");
	if (fwrite(&WorkBuffSize, sizeof(WorkBuffSize), 1, fp) != 1)
		warn("fwrite1");
	if (fwrite(WorkBuff, WorkBuffSize, 1, fp) != 1)
		warn("fwrite2");
	fclose(fp);

	DIRead(&docindex, DocID, subname);
	docindex.ResourcePointer = offset;
	//printf("Offset: %ld\n", offset);
	docindex.ResourceSize = WorkBuffSize;
	DIWrite(&docindex, DocID, subname, NULL);
	free(WorkBuff);
}

size_t
getResource(int LotNr, char *subname, unsigned int DocID, char *resource, size_t resourcelen)
{
	char FileName[1024];
	FILE *fp;
	struct DocumentIndexFormat docindex;
	unsigned int rDocID;
	size_t len, clen;
	unsigned int HtmlBufferSize;
	char *workbuf;
	int error;

	DIRead(&docindex, DocID, subname);

	GetFilPathForLot(FileName,LotNr,subname);
	strcat(FileName,"resource");

	if ((fp = fopen(FileName, "r")) == NULL) {
		warn("fopen(%s)", FileName);
		return 0;
	}
	fseek(fp, docindex.ResourcePointer, SEEK_SET);
	fread(&rDocID, sizeof(rDocID), 1, fp);
	fread(&len, sizeof(len), 1, fp);
	fread(&clen, sizeof(clen), 1, fp);
	printf("len: %x clen: %x\n", len, clen);
	if (rDocID != DocID) {
		fclose(fp);
		if (resource != NULL)
			strcpy(resource, "");
		return 0;
	}
	if (resource != NULL) {
		workbuf = malloc(clen);
		len = fread(workbuf, clen, 1, fp);
		fclose(fp);
		printf("reslen: %d, \n", resourcelen);
		if ((error = uncompress((Bytef*)resource,(uLong *)&resourcelen,(Bytef *)workbuf,clen)) != 0) {
			printf("uncompress error. Code: %i for DocID %u\n", error, DocID);
			return 0;
		}
		len = resourcelen;
	}

	return len;
}


void addNewUrlOpen(struct addNewUrlhaFormat *addNewUrlha,int lotNr, char openmode[],char subname[], int nr) {
	(*addNewUrlha).OpenLot = -1;

	char filepath[512];

	sprintf(filepath,"nyeurler.%i",nr);

	#ifdef DEBUG
		printf("addNewUrlOpen: open \"%s\"\n",filepath);
	#endif

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

