#ifndef _DOCUMENTINDEX_H_
#define _DOCUMENTINDEX_H_

#include "define.h"
#include <stdio.h>
#include "lot.h"


int DIHaveIndex (int lotNr,char subname[]);
int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]);
int DIRead_fmode (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], char filemode);
void DIWrite (struct DocumentIndexFormat *DocumentIndexPost, unsigned int DocID,char subname[], char *);
int DIGetNext (struct DocumentIndexFormat *DocumentIndexPost, int LotNr,unsigned int *DocID,char subname[]);
void DIClose(FILE *DocumentIndexHA);
int DIRead_fh(struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], FILE *file);
int DIRead_i(struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[], int file);

void DIS_delete(struct DocumentIndexFormat *DocumentIndexPost);
int DIS_isDeleted(struct DocumentIndexFormat *DocumentIndexPost);


#ifdef DI_FILE_CASHE
	void closeDICache(void);
#endif

#endif //_DOCUMENTINDEX_H_
