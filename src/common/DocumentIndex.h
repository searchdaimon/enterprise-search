#ifndef _DOCUMENTINDEX_H_
#define _DOCUMENTINDEX_H_

#include "define.h"
#include <stdio.h>
#include "lot.h"


int DIHaveIndex (int lotNr,char subname[]);
int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]);
void DIWrite (struct DocumentIndexFormat *DocumentIndexPost, unsigned int DocID,char subname[], char *);
int DIGetNext (struct DocumentIndexFormat *DocumentIndexPost, int LotNr,unsigned int *DocID,char subname[]);
void DIClose(FILE *DocumentIndexHA);
void closeDICache(void);



#endif //_DOCUMENTINDEX_H_
