/*
Rutinner for å jobbe på en Boitho DocumentIndex fil.

ToDo: trenger en "close" prosedyre for filhandlerene.
*/
#include "DocumentIndex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "bstr.h"
#include "dp.h"

// turn off the file cache since we keep getting segfaults with it
//#define DI_FILE_CASHE

#ifdef DI_FILE_CASHE

	static int openDocumentIndex = -1;
	static char openName[64] = {'\0'};

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

// DocumentIndex struct, these are routines that work on a di struct
void DIS_delete(struct DocumentIndexFormat *DocumentIndexPost) {
	DocumentIndexPost->htmlSize2 = 0;
	DocumentIndexPost->htmlSize = 0;
	DocumentIndexPost->imageSize = 0;
	DocumentIndexPost->RepositoryPointer = 0;
	DocumentIndexPost->ResourcePointer = 0;
	DocumentIndexPost->SummaryPointer = 0;
	DocumentIndexPost->SummarySize = 0;

}

int DIS_isDeleted(struct DocumentIndexFormat *DocumentIndexPost) {

	// the document is deleted if all the following pointers and size are 0
	if (
	   ((DocumentIndexPost->htmlSize2 == 0) && (DocumentIndexPost->htmlSize == 0))
	&& (DocumentIndexPost->imageSize == 0)
	&& (DocumentIndexPost->RepositoryPointer == 0)
	&& (DocumentIndexPost->ResourcePointer == 0)
	&& (DocumentIndexPost->SummaryPointer == 0)
	&& (DocumentIndexPost->SummarySize == 0)
	) {
		return 1;
	}
	else {
		return 0;
	}
}
int DIPostAdress(unsigned int DocID) {

	int adress = -1;

	int LotNr;

	// find the lot for this DocID
	LotNr = rLotForDOCid(DocID);

	#ifdef BLACK_BOX
		adress = (sizeof(struct DocumentIndexFormat) + sizeof(unsigned int))* (DocID - LotDocIDOfset(LotNr));
	#else
		adress = sizeof(struct DocumentIndexFormat) * (DocID - LotDocIDOfset(LotNr));
	#endif

	return adress;


}
/*
finner riktig fil og Søker seg frem til riktig adresse, slik at man bare kan lese/skrive
*/
FILE *GetFileHandler (unsigned int DocID,char type,char subname[], char *diname) {

	#ifndef DI_FILE_CASHE
		FILE *DocumentIndexHA = NULL;
	#endif
	int LotNr;
	char FileName[128];
	char FilePath[128];
	
	// find the lot for this DocID
	LotNr = rLotForDOCid(DocID);

	// if the file is not open we open it
	// segfaults once in a rare while
	#ifdef DI_FILE_CASHE
	if ((LotNr == openDocumentIndex) && (diname == NULL || strcmp(openName,diname) == 0)) {

	}
	#else
	if(0) {

	}
	#endif
	else {		

		GetFilPathForLot(FilePath,LotNr,subname);
		
		strncpy(FileName,FilePath,128);
		strncat(FileName,diname == NULL ? "DocumentIndex" : diname,128);

		#ifdef DI_FILE_CASHE
			printf("openig di file \"%s\"\n",FileName);
		#endif

		#ifdef DI_FILE_CASHE

			// if we have an open file we close it
			if (openDocumentIndex != -1) {
				// segfaults here for searchkernel
				// 18 Oct segfaults here again ????
				fclose(DocumentIndexHA);
			}
		#endif
				
		
		// first try to open for reading
		if (type == 'c') {
			// temp: set file opening to r+ to make DIRead and DIwrite cooperate
			// this makes search not work on web due to permissions :-(
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				printf("%d: cant open file %s for c\n", __LINE__, FileName);
				perror(FileName);
			    return NULL;
			}
		}
		else if (type == 'r') {
			// temp: set file opening to r+ to make DIRead and DIwrite cooperate
			// this makes search not work on web due to permissions :-(
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				printf("%d: cant open file %s for r\n",__LINE__, FileName);
				perror(FileName);
			    return NULL;
			}
		}
		else if (type == 's') {
			// a real r read
			if ((DocumentIndexHA = fopen(FileName,"rb")) == NULL) {
				printf("%d: cant open file %s for rb\n", __LINE__, FileName);
				perror(FileName);
			    return NULL;
			}
		}

		else if (type == 'w'){
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				// if that does not work we create and open the file
				makePath(FilePath);
				if ((DocumentIndexHA = fopen(FileName,"w+b")) == NULL) {
					perror(FileName);

					return NULL;
				}
			}
		}

		#ifdef DI_FILE_CASHE
			openDocumentIndex = LotNr;
			strscpy(openName,diname == NULL ? "DocumentIndex" : diname,sizeof(openName));
		#endif
	}

	// seek to the correct record
	if (fseek(DocumentIndexHA,DIPostAdress(DocID),0) != 0) {
		perror("Can't seek");
		exit(1);
	}


	return DocumentIndexHA;

}

// check whether a DocumentIndex exists for this lot
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
	
	if ((file = GetFileHandler(DocID,'w',subname, diname)) != NULL) {

		#ifdef BLACK_BOX
			unsigned int CurrentDocumentIndexVersionAsUInt = CurrentDocumentIndexVersion;

			if (fwrite(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,file) != 1) {
	                        perror("Can't write");
        	                exit(1);
                	}
		#endif
		// write the record
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

	// if we do not have DI_FILE_CASHE on we must lock the file
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



        // test whether the repository is already open or not
        if (LotOpen != LotNr) {
                // if it has been open, we close it. If it is -1 it has not been used yet, so no point in closing it then :-)
                if (LotOpen != -1) {
                        fclose(LotFileOpen);
                }
                GetFilPathForLot(FileName,LotNr,subname);
                strncat(FileName,"DocumentIndex",128);

		#ifdef DEBUG
	                printf("%s:%i: Opending lot %s\n",__FILE__,__LINE__,FileName);
		#endif

                if ( (LotFileOpen = fopen(FileName,"rb")) == NULL) {
			#ifdef DEBUG
                        	perror(FileName);
			#endif

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

	

        // if there is still data left in the file we read it
        if (!feof(LotFileOpen)) {
         

        	// read the record
		// mysterious that we get "Can't reed DocumentIndexPost: Success" at eof here,
		// instead of getting eof further up
		#ifdef BLACK_BOX
                        unsigned int CurrentDocumentIndexVersionAsUInt;
			if ((fread(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,LotFileOpen)) != 1) {
				#ifdef DEBUG
				perror("CurrentDocumentIndexVersionAsUInt");
				#endif

				// close down the file
        	                fclose(LotFileOpen);
	                        LotOpen = -1;

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
			// close down the file
			fclose(LotFileOpen);
			LotOpen = -1;

			return 0;
        	}
		else {
	       		return 1;
		}

        }
        else {
		// if we are out of data we close the file and return a 0 to say we are done.
		#ifdef DEBUG
                	printf("ferdig\n");
		#endif
                fclose(LotFileOpen);
		LotOpen = -1;
                return 0;
        }
}

int DIRead_post_i(struct DocumentIndexFormat *DocumentIndexPost, int file) {

	int forReturn = 0;

		#ifdef BLACK_BOX
                        unsigned int CurrentDocumentIndexVersionAsUInt;
                        if ((read(file,&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int))) < 0) {
				#ifdef DEBUG
                                perror("CurrentDocumentIndexVersionAsUInt");
				#endif
                                forReturn = 0;
                        }
                #endif


        	// read the record
        	if (read(file,DocumentIndexPost,sizeof(*DocumentIndexPost)) < 0) {
			#ifdef DEBUG
                	perror("Can't reed");
			#endif
			// even if we did not get to read from the file we must lock it, so we cannot call return directly here
			forReturn =  0;
        	}
		else {
			forReturn  = 1;
		}

        if ((*DocumentIndexPost).htmlSize != 0) {
                (*DocumentIndexPost).htmlSize2 = (*DocumentIndexPost).htmlSize;
        }

	return forReturn;
		
}

int DIRead_post_fh(struct DocumentIndexFormat *DocumentIndexPost, FILE *file) {

	#ifdef DEBUG
		printf("DIRead_post_fh()\n");
	#endif

	int forReturn = 0;

		#ifdef BLACK_BOX
                        unsigned int CurrentDocumentIndexVersionAsUInt;
                        if ((fread(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,file)) != 1) {
				#ifdef DEBUG
                                perror("CurrentDocumentIndexVersionAsUInt");
				#endif
                                forReturn = 0;
                        }
                #endif


        	// read the record
        	if (fread(DocumentIndexPost,sizeof(*DocumentIndexPost),1,file) != 1) {
			#ifdef DEBUG
				perror("Can't reed");
			#endif
			// even if we did not get to read from the file we must lock it, so we cannot call return directly here
			forReturn =  0;
        	}
		else {
			forReturn  = 1;
		}


	        if ((*DocumentIndexPost).htmlSize != 0) {
	                (*DocumentIndexPost).htmlSize2 = (*DocumentIndexPost).htmlSize;
	        }


	return forReturn;
		
}

int DIRead_fmode (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], char filemode) {

	FILE *file;
	int forReturn = 0;

	#ifdef DEBUG
		printf("DIRead_fmode(DocID=%i, subname=\"%s\")\n",DocID,subname);
	#endif

	#ifdef DISK_PROTECTOR
		dp_lock(rLotForDOCid(DocID));
	#endif

	if ((file = GetFileHandler(DocID,filemode,subname, NULL)) != NULL) {

		if (DIRead_post_fh(DocumentIndexPost,file)) {
			forReturn = 1;
		}

		// if we do not have DI_FILE_CASHE on we must lock the file
		#ifndef DI_FILE_CASHE
			fclose(file);
		#endif
		
        }
        else {
		printf("can't open DocumentIndexPost for DocID %u.\n",DocID);
        }


        if ((*DocumentIndexPost).htmlSize != 0) {
                (*DocumentIndexPost).htmlSize2 = (*DocumentIndexPost).htmlSize;
        }


	#ifdef DISK_PROTECTOR
		dp_unlock(rLotForDOCid(DocID));
	#endif

	return forReturn;

}

int DIRead_fh(struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], FILE *file) {

	int forReturn = 0;


	if (file == NULL) {
		#ifdef DEBUG
			printf("DIRead_fh: file isent open.\n");
		#endif
		forReturn = DIRead_fmode(DocumentIndexPost,DocID,subname,'r');
	}
	else {
		#ifdef DISK_PROTECTOR
			dp_lock(rLotForDOCid(DocID));
		#endif

		

		// seek to the correct record
		if (fseek(file,DIPostAdress(DocID),0) != 0) {
			perror("Can't seek");
			exit(1);
		}

		if (DIRead_post_fh(DocumentIndexPost,file)) {
			forReturn = 1;
		}
		#ifdef DISK_PROTECTOR
			dp_unlock(rLotForDOCid(DocID));
		#endif

	}

        if ((*DocumentIndexPost).htmlSize != 0) {
                (*DocumentIndexPost).htmlSize2 = (*DocumentIndexPost).htmlSize;
        }


	return forReturn;
}

int DIRead_i(struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], int file) {

	int forReturn = 0;


	if (file == -1) {
		#ifdef DEBUG
			printf("DIRead_fh: file isent open.\n");
		#endif
		forReturn = DIRead_fmode(DocumentIndexPost,DocID,subname,'r');
	}
	else {
		#ifdef DISK_PROTECTOR
			dp_lock(rLotForDOCid(DocID));
		#endif

		

		// seek to the correct record
		if (lseek(file,DIPostAdress(DocID),SEEK_SET) < 0) {
			perror("Can't lseek");
			forReturn = 0;
		}
		else if (DIRead_post_i(DocumentIndexPost,file)) {
			forReturn = 1;
		}
		#ifdef DISK_PROTECTOR
			dp_unlock(rLotForDOCid(DocID));
		#endif

	}

        if ((*DocumentIndexPost).htmlSize != 0) {
                (*DocumentIndexPost).htmlSize2 = (*DocumentIndexPost).htmlSize;
        }


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

// close down files
void DIClose(FILE *DocumentIndexHA) {
	fclose(DocumentIndexHA);
}
