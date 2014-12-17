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


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <sys/file.h>
#include <math.h>
#include <zlib.h>

#ifdef BLACK_BOX
	#include "../ds/dcontainer.h"
#endif

struct addNewUrlhaFormat {
                int OpenLot;
                FILE *NYEURLER;
};


#define MaxOpenReposetoryFiles 3


//formatet som brues for å lagre en Reposetory post
#define ReposetoryStorageFormat "%i\n%s\n%s\n%s"




//formater på cashe over opne filhontarere
struct ReposetoryOpenFilesFormat {
	short int LotNr;
	FILE *FILEHANDLER;
};


//array med opne filhonterere
struct ReposetoryOpenFilesFormat OpenReposetoryFiles[MaxOpenReposetoryFiles];


#ifdef BLACK_BOX
	container* ropen();
	void rclose(container*);
#endif

int rReadHtml (char *HtmlBuffer[],uLong *HtmlBufferSize,unsigned int radress64bit,unsigned int rsize,unsigned
				int DocID,char subname[],struct ReposetoryHeaderFormat *ReposetoryHeader,
				char **acl_allowbuffer,char **acl_deniedbuffer, unsigned int imagesize, char **url,
				char **attributes
				);

int rGetNext_reponame (unsigned int LotNr, struct ReposetoryHeaderFormat *ReposetoryHeader, Bytef *htmlbuffer,
		int htmlbufferSize, char imagebuffer[], unsigned int *radress, unsigned int FilterTime, unsigned int FileOffset,
		char subname[], char **acl_allowbuffer,char **acl_deniedbuffer, char *reponame, char **url, char **attributes);

int rGetNext (unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, Bytef *htmlbuffer,int htmlbufferSize, char imagebuffer[], unsigned int *radress, unsigned int FilterTime, unsigned int FileOffset, char subname[], char **acl_allowbuffer,char **acl_deniedbuffer, char **url, char **attributes);


int rGetNext_fh (unsigned int LotNr, struct ReposetoryHeaderFormat *ReposetoryHeader, Bytef *htmlbuffer,
		int htmlbufferSize, char imagebuffer[], unsigned int *radress, unsigned int FilterTime, 
		char **acl_allowbuffer,char **acl_deniedbuffer, FILE *LotFileOpen, char **url, char **attributes);

int runpack(char *ReposetoryData,uLong comprLen,char *inndata,int length);
int rReadSummary(unsigned int DocID,char **metadesc, char **title, char **body ,unsigned int radress64bit,unsigned short rsize,char subname[]);
int rReadSummary_l(const unsigned int DocID,char **metadesc, char **title, char **body ,unsigned int radress64bit,unsigned short rsize,char subname[],  int fd);
//Bilde rutiner
void risave (int DocID, char *image, int size,char subname[]);

//anchor
#ifndef BLACK_BOX
void anchoradd(unsigned int DocID,char *text,int textsize,char subname[], char *);
int anchorGetNext (int LotNr,unsigned int *DocID,char *text,int textlength, unsigned int *radress,unsigned int *rsize,char subname[]);

void addNewUrlOpen(struct addNewUrlhaFormat *addNewUrlha,int lotNr, char openmode[],char subname[], int nr);
void addNewUrl (struct addNewUrlhaFormat *addNewUrlha, struct updateFormat *updatePost);
int anchorRead(int, char *, unsigned int, char *, int);
int anchorGetNextnew(int,unsigned int *,char *,int, unsigned int *,unsigned int *,char *, off_t *);
void anchoraddnew(unsigned int, char *, size_t, char *, char *);
int anchorIndexWrite(unsigned int, char *, off_t);
int anchorIndexRead(unsigned int, char *, off_t *);
#endif

/* Resource */
void addResource(int LotNr, char *subname, unsigned int DocID, char *resource, size_t resourcelen);
size_t getResource(int LotNr, char *subname, unsigned int DocID, char *resource, size_t resourcelen);



unsigned int rApendPost (struct ReposetoryHeaderFormat *ReposetoryHeader, Bytef *htmlbuffer, char imagebuffer[],char subname[], char acl_allow[], char acl_denied[], char *reponame, char *url, char *attributes, container *attrkeys);
int rApendPostcompress (struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffer[],char subname[], char acl_allow[], char acl_denied[], char *reponame, char *url, char *attributes, container *attrkeys, size_t HtmlBufferSize);
void setLastIndexTimeForLot(int LotNr,int httpResponsCodes[],char subname[]);
unsigned int GetLastIndexTimeForLot(int LotNr,char subname[]);

int rReadPost(FILE *LotFileOpen,struct ReposetoryHeaderFormat *ReposetoryHeader, Bytef *htmlbuffer, int htmlbufferSize,
                        char imagebuffer[],char **acl_allowbuffer,char **acl_deniedbuffer,char recordseparator[], char **url,
			char **attributes, int LotNr);

off_t getImagepFromRadres(unsigned int radress64bit,unsigned int htmlbufferSize);
unsigned int rLastDocID(char subname[]);

unsigned int rGeneraeADocID(char subname[]);
int findLotToIndex(char subname[], int dirty);

#endif //_REPOSETORY_H_
