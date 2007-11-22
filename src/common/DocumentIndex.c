/*
Rutinner for å jobbe på en Boitho DocumentIndex fil.

ToDo: trenger en "close" prosedyre for filhandlerene.
*/
//#include "define.h"
#include "DocumentIndex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WITHOUT_DIWRITE_FSYNC
	#include <unistd.h>
#endif

#include "dp.h"
//skrur av filcashn da vi deiver å segge feiler med den
//#define DI_FILE_CASHE

static int openDocumentIndex = -1;
#ifdef DI_FILE_CASHE
                FILE *DocumentIndexHA;

void closeDICache(void)
{
	if (openDocumentIndex != -1) {
		openDocumentIndex = -1;
		fclose(DocumentIndexHA);
	}
}
#endif


#define CurrentDocumentIndexVersion 4


/*
finner riktif fil og Søker seg frem til riktig adresse, slik at man bare kan lese/skrive
*/
FILE *GetFileHandler (unsigned int DocID,char type,char subname[], char *diname) {

	#ifndef DI_FILE_CASHE
		FILE *DocumentIndexHA;
	#endif
	int LotNr;

	char FileName[128];
	char FilePath[128];
	//unsigned int adress = -1;
	//off_t adress = -1;
	int adress = -1;
	
	//finner lot for denne DocIDen
	LotNr = rLotForDOCid(DocID);

	//printf("%u-%i\n",DocID,LotNr);

	//hvis filen ikke er open åpner vi den
	//segfeiler en skjelden gang
	#ifdef DI_FILE_CASHE
	if (LotNr != openDocumentIndex) {
	#else
	if(1) {
	#endif		
		#ifdef DI_FILE_CASHE
		//hvis vi har en open fil lukkes denne
		if (openDocumentIndex != -1) {
			//segfeiler her for searchkernel
			//18,okt segefeiler her igjen ????
			printf("open file for lot %i\n",openDocumentIndex);
			fclose(DocumentIndexHA);
		}
		#endif
				
		
	

		GetFilPathForLot(FilePath,LotNr,subname);
		
		strncpy(FileName,FilePath,128);
		strncat(FileName,diname == NULL ? "DocumentIndex" : diname,128);

		//debug:viser hvpathen til loten
		//printf("path: %s\n",FileName);

		//prøver først å åpne for lesing

		if (type == 'c') {
			//printf("opening file\n");
			//temp: setter filopning til r+ for å få til å samarbeid melom DIRead og DIwrite
			//dette gjør at søk ikke funker på web på grun av rettighter :-(
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				printf("cant open file %s\n",FileName);
				perror(FileName);
			    return NULL;
			}
		}
		else if (type == 'r') {
			//printf("opening file\n");
			//temp: setter filopning til r+ for å få til å samarbeid melom DIRead og DIwrite
			//dette gjør at søk ikke funker på web på grun av rettighter :-(
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				printf("cant open file %s\n",FileName);
				perror(FileName);
			    return NULL;
			}
		}
		else if (type == 'w'){
			//printf("opening file\n");
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				//hvis det ikke går lager vi og åpne filen
				makePath(FilePath);
				if ((DocumentIndexHA = fopen(FileName,"w+b")) == NULL) {
					perror(FileName);
					//exit(1);
					return NULL;
				}
			}
		}

		openDocumentIndex = LotNr;
	}

	#ifdef BLACK_BOKS
		adress = (sizeof(struct DocumentIndexFormat) + sizeof(unsigned int))* (DocID - LotDocIDOfset(LotNr));
	#else
		adress = sizeof(struct DocumentIndexFormat) * (DocID - LotDocIDOfset(LotNr));
	#endif
	//printf("tell: %i\n",ftell(DocumentIndexHA));
	


	//søker til riktig post
	if (fseek(DocumentIndexHA,adress,0) != 0) {
		perror("Can't seek");
		exit(1);
	}

//	printf("tell: %i\n",ftell(DocumentIndexHA));
		
	//file = DocumentIndexHA;

//	printf("tell åpen: %i\n",ftell(DocumentIndexHA));
//	printf("fa åpnet %i\n",(int)DocumentIndexHA);

	return DocumentIndexHA;

}

//sjeker om det fins en DocumentIndex fro denne loten

int DIHaveIndex (int lotNr,char subname[]) {

	char FilePath[512];
	FILE *FH;

        GetFilPathForLot(FilePath,lotNr,subname);

        strncat(FilePath,"DocumentIndex",sizeof(FilePath));

	if ((FH = fopen(FilePath,"r")) == NULL) {
		return 0;
	}
	else {
		fclose(FH);
		return 1;
	}
}

/*
skriver en post til DocumentIndex
*/
void DIWrite (struct DocumentIndexFormat *DocumentIndexPost, unsigned int DocID,char subname[], char *diname) {


	FILE *file;

	//printf("ha uinalisert %i\n",(int)file);
	
	if ((file = GetFileHandler(DocID,'w',subname, diname)) != NULL) {

		//printf("fa mottat %i\n",(int)file);

		//printf("motatt tell: %i\n",ftell(file));
		//printf("aa url: %s\n",(*DocumentIndexPost).Url);
		#ifdef BLACK_BOKS
			unsigned int CurrentDocumentIndexVersionAsUInt = CurrentDocumentIndexVersion;

			if (fwrite(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,file) != 1) {
	                        perror("Can't write");
        	                exit(1);
                	}
		#endif
		//skriver posten
		if (fwrite(DocumentIndexPost,sizeof(struct DocumentIndexFormat),1,file) != 1) {
			perror("Can't write");
			exit(1);
		}
#ifndef WITHOUT_DIWRITE_FSYNC
		fsync(fileno(file));
#endif
	}
	else {
		printf("Cant get fh\n");
	}

	//hvis vi ikke har på DI_FILE_CASHE må vi lokke filen
	#ifndef DI_FILE_CASHE
		fclose(file);
	#endif
}

/*
tar et lott nr inn og henter neste post

Den vil retunere 1 så lenge det er data og lese. slik at man kan ha en lopp slik

while (DIRGetNext(LotNr,DocumentIndexPost)) {

        ..gjør noe med ReposetoryData..

}
*/

int DIGetNext (struct DocumentIndexFormat *DocumentIndexPost, int LotNr,unsigned int *DocID,char subname[]) {


        static FILE *LotFileOpen;
        static int LotOpen = -1;
	static unsigned LastDocID;
	int n;

	char FileName[128];


        //tester om reposetoriet allerede er open, eller ikke
        if (LotOpen != LotNr) {
                //hvis den har vært open, lokker vi den. Hvi den er -1 er den ikke brukt enda, så ingen vits å å lokke den da :-)
                if (LotOpen != -1) {
                        fclose(LotFileOpen);
                }
                GetFilPathForLot(FileName,LotNr,subname);
                strncat(FileName,"DocumentIndex",128);

                printf("Opending lot %s\n",FileName);

                if ( (LotFileOpen = fopen(FileName,"rb")) == NULL) {
                        perror(FileName);
                        //exit(1);
			LotOpen = -1;
			return 0;
                }
                LotOpen = LotNr;
		LastDocID = GetStartDocIFForLot(LotNr);
        }
	else {
		++LastDocID;
	}

	(*DocID) = LastDocID;

	

        //hvis det det er data igjen i filen leser vi den
        if (!feof(LotFileOpen)) {
         

        	//leser posten
		//mystisk at vi får "Can't reed DocumentIndexPost: Success", ved eof her,
		//i steden for at vi  får eof lenger opp
		#ifdef BLACK_BOKS
                        unsigned int CurrentDocumentIndexVersionAsUInt;
			if ((fread(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,LotFileOpen)) != 1) {
				perror("CurrentDocumentIndexVersionAsUInt");
				return 0;
			}
		#endif

        	if ((n=fread(DocumentIndexPost,sizeof(struct DocumentIndexFormat),1,LotFileOpen)) != 1) {
			if (feof(LotFileOpen)) {
				printf("hit eof for DocumentIndex\n");
			}
			else {
                		printf("Can't reed DocumentIndexPost. n: %i, eof %i\n",n,feof(LotFileOpen));
				perror("fread() DocumentIndexPost");
			}
			//stnger ned filen
			fclose(LotFileOpen);
			LotOpen = -1;

                	//exit(1);
			return 0;
        	}
		else {
			//printf("Url: %s\n",(*DocumentIndexPost).Url);
	       		return 1;
		}

        }
        else {
        //hvis vi er tom for data stenger vi filen, og retunerer en 0 som sier at vi er ferdig.
                printf("ferdig\n");
                fclose(LotFileOpen);
		LotOpen = -1;
                return 0;
        }
}


int DIRead_fmode (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], char filemode) {

	FILE *file;
	int forReturn;

	#ifdef DEBUG
		printf("DIRead: reading for DocID %i, subname \"%s\"\n",DocID,subname);
	#endif

	#ifdef DISK_PROTECTOR
		dp_lock(rLotForDOCid(DocID));
	#endif

	if ((file = GetFileHandler(DocID,filemode,subname, NULL)) != NULL) {


		#ifdef BLACK_BOKS
                        unsigned int CurrentDocumentIndexVersionAsUInt;
                        if ((fread(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,file)) != 1) {
                                perror("CurrentDocumentIndexVersionAsUInt");
                                forReturn = 0;
                        }
                #endif


        	//lesr posten
        	if (fread(DocumentIndexPost,sizeof(*DocumentIndexPost),1,file) != 1) {
                	perror("Can't reed");

			//selv om vi ikke fikk lest fra filen må vi lokke den, så vi kan ikke kalle retun directe her
			forReturn =  0;
        	}
		else {
			forReturn  = 1;
		}
		
		//hvis vi ikke har på DI_FILE_CASHE må vi lokke filen
		#ifndef DI_FILE_CASHE
			fclose(file);
		#endif

		
        }
        else {
		printf("cant get GetFileHandler\n");
		forReturn =  0;

        }


	#ifdef DISK_PROTECTOR
		dp_unlock(rLotForDOCid(DocID));
	#endif

	return forReturn;

}

/*
leser en post fra DocumentIndex

runarb: 24.10.2007:
Tradisjonelt så har DIRead åpnet filen r+ for å kunne samarbeide med DIWrite.
Jeg har nå dåpt den åpningen om til c, og laget en DIRead_fmode der man kan spesifisere opningsmode selv, slik at man kan
få en ekte read
*/
int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]) {
	return DIRead_fmode(DocumentIndexPost,DocID,subname,'c');
}

//stenger ned filer
void DIClose(FILE *DocumentIndexHA) {
//	fclose(DocumentIndexHA);
}
