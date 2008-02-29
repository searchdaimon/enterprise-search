#ifndef _POPRANK__H_
#define _POPRANK__H_

#include <stdio.h>
#include <stdlib.h>

struct popl {
	FILE *fh;
	int DocID;
};

struct popmemmapFormat {
        int *ranks;
        off_t size;
	unsigned int largesDocID;
	int fd;
};

int popopen (struct popl *popha, char *filname);
void popclose(struct popl *popha);
void popadd (struct popl *popha,int DocID,int increasement);
void popset (struct popl *popha,int DocID,int pop);
FILE *GetFileHander();
int popRankForDocID(struct popl *popha,int DocID);
unsigned int NrOfElementInPopIndex ();
int popGetNext (struct popl *popha, int *Rank,unsigned int *rDocID);
void popopenMemArray(char servername[],char subname[], char rankfile[]);
int popRankForDocIDMemArray(unsigned int DocID);
void popopenMemArray_oneLot(char subname[], int i);
void popopenMemArray2(char subname[], char rankfile[]);


int popopenMmap(struct popmemmapFormat *popmemmap,char *filname);
int popRankForDocIDMmap(struct popmemmapFormat *popmemmap,unsigned int DocID);
int popRankForDocIDMmapSet(struct popmemmapFormat *popmemmap,unsigned int DocID,int increasement);
void popcloseMmap (struct popmemmapFormat *popmemmap);

#endif
