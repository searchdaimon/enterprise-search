#ifndef _LOT__H_
#define _LOT__H_

#include <stdio.h>
#include <math.h>
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
void GetFilPathForLot(char *FilePath,int LotNr,char subname[]);
void GetFilPathForLotFile(char *FilePath,char lotfile[],int LotNr,char subname[]);
void GetFilPathForLotByDocID(char *FilePath,int DocID,char subname[]);
void GetFilePathForIindex (char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]);
void GetFilePathForIDictionary(char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]);
void GetFilPathForThumbnaleByDocID(char *FileName,int DocID,char subname[]);
FILE *lotOpenFile(unsigned int DocID,char resource[],char type[], char lock,char subname[]);
FILE *lotOpenFileNoCasheByLotNr(int LotNr,char resource[],char type[], char lock,char subname[]);
FILE *lotOpenFileNoCashe(unsigned int DocID,char resource[],char type[], char lock,char subname[]);
int GetStartDocIFForLot (int LotNr);
void makePath (char path[]);
void lotCloseFiles();
FILE *openMaplist();
int lotHasSufficientSpace(int lot, int needSpace,char subname[]);
int HasSufficientSpace(char FilePath[], int needSpace);

#endif //_LOT__H_
