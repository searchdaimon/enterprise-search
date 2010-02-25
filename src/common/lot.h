#ifndef _LOT__H_
#define _LOT__H_

#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <dirent.h>

#include "define.h"

#define MaxOpenFiles  10

#ifdef DEFLOT
	#define rLotForDOCid(x) (int)((x / NrofDocIDsInLot) +1) 
	#define LotDocIDOfset(x) ((NrofDocIDsInLot * x) - NrofDocIDsInLot)
#else
	int rLotForDOCid (unsigned int DocID);
	int LotDocIDOfset (int LotNr);

#endif
//FILE *bfopen(char name[],char flags[]);
char *returnFilPathForLot(int LotNr,char subname[]);
void GetFilPathForLot(char *FilePath,int LotNr,char subname[]);
void GetFilPathForLotFile(char *FilePath,char lotfile[],int LotNr,char subname[]);
void GetFilPathForLotByDocID(char *FilePath,int DocID,char subname[]);
void GetFilePathForIindex (char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]);
void GetFilePathForIDictionary(char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]);
void GetFilPathForThumbnaleByDocID(char *FileName,int DocID,char subname[]);
FILE *lotOpenFile(unsigned int DocID,char resource[],char type[], char lock,char subname[]);
FILE *lotOpenFileNoCasheByLotNr(int LotNr,char resource[],char type[], char lock,char subname[]);
int lotDeleteFile(char File[], int LotNr,char subname[]);
#ifdef DO_DIRECT
int lotOpenFileNoCache_direct(unsigned int DocID, char *resource, char *type, char lock, char *subname);
#endif
FILE *lotOpenFileNoCashe(unsigned int DocID,char resource[],char type[], char lock,char subname[]);
int GetStartDocIFForLot (int LotNr);
void makePath (char path[]);
void lotCloseFiles();
FILE *openMaplist();
int lotHasSufficientSpace(int lot, int needSpace,char subname[]);
int HasSufficientSpace(char FilePath[], int needSpace);

int lotOpenFileNoCashel(unsigned int DocID,char resource[],char type[], char lock,char subname[]);
int lotOpenFileNoCasheByLotNrl(int LotNr,char resource[],char type[], char lock,char subname[]);

int GetDevIdForLot(int LotNr);
void makeLotPath(int lotNr,char folder[],char subname[]);

//rutiner for å lsite ut alle lokale subnames
DIR *listAllColl_start();
char *listAllColl_next(DIR * ll);
void listAllColl_close(DIR * ll);

void lot_get_closed_collections_file(char *buf);

void lot_recache_collection(char subname[]);

#endif //_LOT__H_


