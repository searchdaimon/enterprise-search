#ifndef _INTEGERINDEX__H_
#define _INTEGERINDEX__H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "define.h"



struct iintegerFormat {
        FILE *FH;
        int lotNr;
	int elementsize;
};

struct iintegerMemArrayFormat {
	//unsigned short *MemArray[maxLots];
	unsigned char *MemArray[maxLots];
};

int iintegerOpenForLot(struct iintegerFormat *iinteger, char index[], int elementsize, int lotNr, char type[],char subname[]);

void iintegerClose(struct iintegerFormat *iinteger);

int iintegerSetValueNoCashe(void *value,int valuesize,unsigned int DocID,char indexname[],char subname[]);
int iintegerGetValueNoCashe(void *value,int valuesize,unsigned int DocID,char indexname[],char subname[]);

void iintegerSetValue(struct iintegerFormat *iinteger,void *value,int valuesize,unsigned int DocID,char subname[]);

int iintegerLoadMemArray(struct iintegerMemArrayFormat *iintegerMemArray,char index[], int elementsize,char servername[], char subname[]);
int iintegerLoadMemArray2(struct iintegerMemArrayFormat *iintegerMemArray,char index[], int elementsize,char subname[]);

int iintegerMemArrayGet (struct iintegerMemArrayFormat *iintegerMemArray,void **value,int valuesize,unsigned int DocID);

int iintegerGetValue(void *value,int valuesize,unsigned int DocID,char indexname[],char subname[]);
#endif //_INTEGERINDEX__H_
