#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct ReposetoryHeaderFormat_v13 {
        unsigned int DocID;
        char url[200];
        char content_type[4];
        unsigned long int IPAddress;
        unsigned short response;
        unsigned short htmlSize;
        unsigned short imageSize;
        unsigned long int time;
        unsigned short userID;
        double clientVersion;
        #ifdef BLACK_BOKS
                int aclSize;
                //time_t storageTime; //3 now
                //char doctype[4]; //3 now
                //char reservedSpace[64]; //3 now
        #endif
};



int rGetNext_v13 (unsigned int LotNr, struct ReposetoryHeaderFormat_v13 *ReposetoryHeader, char htmlbuffer[],
int htmlbufferSize, char imagebuffer[], unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset,
char subname[], char **aclbuffer);

int rReadPost_v13(FILE *LotFileOpen,struct ReposetoryHeaderFormat_v13 *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
                        char imagebuffer[],char **aclbuffer,char recordseparator[]);


main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];

	struct ReposetoryHeaderFormat_v13 ReposetoryHeader_v13;
	struct ReposetoryHeaderFormat ReposetoryHeader_v14;
	unsigned long int radress;

	char htmlbuffer[524288];
	char imagebuffer[524288];
	char *acl;

	if (argc < 3) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www\n");
		exit(1);
	}

	LotNr = atoi(argv[1]);
	char *subname = argv[2];

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);

	char oldreposetory[512];
	char newreposetory[512];
	sprintf(oldreposetory,"%sreposetory",lotPath);
	sprintf(newreposetory,"%sreposetory_v13",lotPath);


	printf("%s -> %s\n",oldreposetory,newreposetory);

	rename(oldreposetory,newreposetory);

	//loppergjenom alle
	while (rGetNext_v13(LotNr,&ReposetoryHeader_v13,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&radress,0,0,subname,&acl)) {

		//printf("DocId: %i url: %s res %hi htmls %hi time %lu\n",ReposetoryHeader_v13.DocID,ReposetoryHeader_v13.url,ReposetoryHeader_v13.response,ReposetoryHeader_v13.htmlSize,ReposetoryHeader_v13.time);

		//printf("################################\n%s##############################\n",htmlbuffer);

		
		ReposetoryHeader_v14.DocID = ReposetoryHeader_v13.DocID;
		strcpy(ReposetoryHeader_v14.url,ReposetoryHeader_v13.url);
		strcpy(ReposetoryHeader_v14.content_type,ReposetoryHeader_v13.content_type);
		ReposetoryHeader_v14.IPAddress = ReposetoryHeader_v13.IPAddress;
		ReposetoryHeader_v14.response = ReposetoryHeader_v13.response;
		ReposetoryHeader_v14.htmlSize = ReposetoryHeader_v13.htmlSize;
		ReposetoryHeader_v14.imageSize = ReposetoryHeader_v13.imageSize;
		ReposetoryHeader_v14.time = ReposetoryHeader_v13.time;
		ReposetoryHeader_v14.userID = ReposetoryHeader_v13.userID;
		ReposetoryHeader_v14.clientVersion = 1.3;
		ReposetoryHeader_v14.aclSize = ReposetoryHeader_v13.aclSize;
		//ReposetoryHeader_v14.storageTime
		strncpy(ReposetoryHeader_v14.doctype,"mail",4); 
		//ReposetoryHeader_v14.reservedSpace
		//int rApendPostcompress (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl[])
		rApendPost(&ReposetoryHeader_v14,htmlbuffer,imagebuffer,subname,acl);		
	}
	
	
}

int rReadPost_v13(FILE *LotFileOpen,struct ReposetoryHeaderFormat_v13 *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
                        char imagebuffer[],char **aclbuffer,char recordseparator[]) {


                #ifdef BLACK_BOKS
                        //unsigned int CurrentReposetoryVersionAsUInt;
                        //fread(&CurrentReposetoryVersionAsUInt,sizeof(unsigned int),1,LotFileOpen);
                #endif

                //leser hedder
                if (fread(ReposetoryHeader,sizeof(struct ReposetoryHeaderFormat_v13),1,LotFileOpen) != 1) {
                        perror("cant read ReposetoryHeader");
                }


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

                if ((*ReposetoryHeader).imageSize == 0) {
                        //printf("imageSize is 0. Skipping to read it\n");
                }
                else if (imagebuffer == NULL) {
                        //hvis vi ikke har en buffer å putte bilde inn i søker vi bare over
                        fseek(LotFileOpen,(*ReposetoryHeader).imageSize,SEEK_CUR);

                }
                else {
                        if (fread(imagebuffer,(*ReposetoryHeader).imageSize,1,LotFileOpen) != 1) {
                                printf("can't read image. ImageSize %hu \n",(*ReposetoryHeader).imageSize);
                        }
                }

                //leser acl
                #ifdef BLACK_BOKS

                        printf("acl sise %i\n",(*ReposetoryHeader).aclSize);
                        (*aclbuffer) = malloc((*ReposetoryHeader).aclSize +1);
                        if (fread((*aclbuffer),(*ReposetoryHeader).aclSize,1,LotFileOpen) != 1) {
                                printf("cant't read acl:\n");
                                perror("");
                        }
                        (*aclbuffer)[(*ReposetoryHeader).aclSize] = '\0';

                        #ifdef DEBUG
                        printf("did read acl %i b, that vas \"%s\"\n",(*ReposetoryHeader).aclSize,(*aclbuffer));
                        #endif
                #else
                        //(*aclbuffer) = NULL;
                #endif

                if(fread(recordseparator,sizeof(char),3,LotFileOpen) != 3) {
                        perror("cant read recordseperator");
                }

}


int rGetNext_v13 (unsigned int LotNr, struct ReposetoryHeaderFormat_v13 *ReposetoryHeader, char htmlbuffer[],
int htmlbufferSize, char imagebuffer[], unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset,
char subname[], char **aclbuffer) {

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
                //hvis den har vært open, lokker vi den. Hvi den er -1 er den ikke brukt enda, så ingen vits å å lokke d$
                if (LotOpen != -1) {
                        fclose(LotFileOpen);
                }
                GetFilPathForLot(FileName,LotNr,subname);
                strncat(FileName,"reposetory_v13",128);

                printf("Opending lot %s\n",FileName);

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
        //      printf("stoppOffset == inode.st_size\n");
        //      return 0;
        //}
        /************************************/

        #ifdef DEBUG
        printf("\n\nstart rGetNext()\n");
        #endif

        while (!feof(LotFileOpen) && (!found)) {


                //finner hvor i filen vi starter
                startOffset = ftell(LotFileOpen);
                //rReadPost(struct ReposetoryHeaderFormat_v13 *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
                //        char imagebuffer[],char **aclbuffer,char recordseparator[])


                rReadPost_v13(LotFileOpen,ReposetoryHeader,htmlbuffer,htmlbufferSize,imagebuffer,aclbuffer,recordseparator);

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
                                *radress = startOffset;
                        //*rsize = bufflength;

                        //hvis dette er en ny nokk rekord retunerer vi denne
			#ifdef BLACK_BOKS
                        //if ((*ReposetoryHeader).storageTime >= FilterTime){
                        #else
                        //if ((*ReposetoryHeader).time >= FilterTime){
                        #endif
			if (1) {
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


