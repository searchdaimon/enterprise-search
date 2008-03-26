#ifndef _REPOSETORYNET_H_
#define _REPOSETORYNET_H_
#include <stdio.h>
#include <sys/file.h>
#include "define.h"

int rGetNextNET(char *HostName, unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffebuffer[], unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset,char subname[]);


void DIWriteNET (char *HostName, struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]);

int DIReadNET (char *HostName, struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]);
int DIReadNET2 ( struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]);

void closeNET ();

unsigned long int DIGetIp (char *HostName, unsigned int DocID,char subname[]);


void setLastIndexTimeForLotNET(char *HostName, int LotNr,char subname[]);
unsigned int GetLastIndexTimeForLotNET(char *HostName, int LotNr,char subname[]);

int rSendFile(char source[], char dest[], int LotNr, char opentype[],char subname[]);


int rSendFileByOpenHandler(FILE *FILEHANDLER, char dest[], int LotNr, char opentype[],char subname[]);
int rGetFileByOpenHandlerFromHostName(char source[],FILE *FILEHANDLER,int LotNr,char subname[],char HostName[]);

int rGetFileByOpenHandler(char source[],FILE *FILEHANDLER,int LotNr,char subname[]);

int rmkdir(char dest[], int LotNr,char subname[]);

int rComand(char dest[], int LotNr,char subname[]);

off_t rGetFileSize(char source[], int LotNr,char subname[]);

void anchoraddnewNET(char *hostname, unsigned int DocID, char *text, size_t textsize, char *subname);
void anchorReadNET(char *hostname, char *subname, unsigned int DocID, char *text, int len);

int readHTMLNET(char *subname, unsigned int DocID, char *text, size_t len);

int openUrlTODocIDNET(char *hostname);
int getUrlTODOcIDNET(int sock, char *url, unsigned int *DocID);
void closeUrlTODocIDNET(int sock);
int getLotToIndex(char subname[],char HostName[]);

#endif //_REPOSETORYNET_H_
