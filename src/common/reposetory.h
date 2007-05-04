#ifndef _REPOSETORY_H_
#define _REPOSETORY_H_

/*
reposetory.h

Subrutiner for å jobbe å Boitho Reposetory

Et Reposetory lagres i flere filer, i flere mapper. 

Det må være en fil kalt maplist.conf i samme mappe som 
programet som bruker denne filen, som indeholder en liste 
med 64 mapper som filer kan lagres i. Dette brtyr at man 
kan bruke opptil 64 disker å spre lagringen utover.


Open filhonterere cashes, og lagres i arrayen "OpenReposetoryFiles"


Kal "ropen" for å opne Reposetoryet, "rclose" for å stenge Reposetoryet.

*/


#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <sys/file.h>
#include <math.h>

struct addNewUrlhaFormat {
                int OpenLot;
                FILE *NYEURLER;
};


//#include "define.h"

//#define MaxReposetoryContent 30000

#define MaxOpenReposetoryFiles 3






//struct ReposetoryFormat {
//	int DocID;
//	char url[201];
//	char content_type[7];
//	char content[MaxReposetoryContent];
//};
//formatet som brues for å lagre en Reposetory post
#define ReposetoryStorageFormat "%i\n%s\n%s\n%s"
//#define ReposetoryStorageFormat "%i;%s;%s;%s;"



//formater på cashe over opne filhontarere
struct ReposetoryOpenFilesFormat {
	short int LotNr;
	FILE *FILEHANDLER;
};


//array med opne filhonterere
struct ReposetoryOpenFilesFormat OpenReposetoryFiles[MaxOpenReposetoryFiles];




//public subrutiner
void rapend (struct ReposetoryFormat ReposetoryData, long int radress);
//void ropen ();
void rclose ();

int rReadHtml (char HtmlBuffer[],unsigned int *HtmlBufferSize,unsigned int radress64bit,unsigned int rsize,unsigned
				int DocID,char subname[],struct ReposetoryHeaderFormat *ReposetoryHeader,char **aclbuffer);

int rGetNext (unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[],int htmlbufferSize, char imagebuffer[], unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset, char subname[], char **aclbuffer);

void runpack(struct ReposetoryFormat *ReposetoryData,char *inndata,int length);

int rReadSummary(unsigned int *DocID,char **metadesc, char **title, char **body ,unsigned int radress64bit,unsigned short rsize,char subname[]);

//Bilde rutiner
void risave (int DocID, char *image, int size,char subname[]);

//anchor
void anchoradd(unsigned int DocID,char *text,int textsize,char subname[]);
int anchorGetNext (int LotNr,unsigned int *DocID,char *text,int textlength, unsigned int *radress,unsigned int *rsize,char subname[]);

void addNewUrlOpen(struct addNewUrlhaFormat *addNewUrlha,int lotNr, char openmode[],char subname[], int nr);
void addNewUrl (struct addNewUrlhaFormat *addNewUrlha, struct updateFormat *updatePost);


int rApendPost (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl[]);
int rApendPostcompress (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl[]);
void setLastIndexTimeForLot(int LotNr,int httpResponsCodes[],char subname[]);
unsigned int GetLastIndexTimeForLot(int LotNr,char subname[]);
int rReadPost(FILE *LotFileOpen,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], int htmlbufferSize,
                        char imagebuffer[],char **aclbuffer,char recordseparator[]);

off_t getImagepFromRadres(unsigned int radress64bit,unsigned int htmlbufferSize);
unsigned int rLastDocID(char subname[]);

unsigned int rGeneraeADocID(char subname[]);

#endif //_REPOSETORY_H_
