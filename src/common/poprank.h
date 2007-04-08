#ifndef _POPRANK__H_
#define _POPRANK__H_

#include <stdio.h>

struct popl {
	FILE *fh;
	int DocID;
};

int popopen (struct popl *popha, char *filname);
void popclose(struct popl *popha);
void popadd (struct popl *popha,int DocID,int increasement);
void popset (struct popl *popha,int DocID,int pop);
FILE *GetFileHander();
int popRankForDocID(struct popl *popha,int DocID);
unsigned int NrOfElementInPopIndex ();
int popGetNext (struct popl *popha, int *Rank,unsigned int *rDocID);
int popRankForDocIDMmap(int DocID);
void popopenMmap();
void popopenMemArray(char servername[],char subname[]);
int popRankForDocIDMemArray(unsigned int DocID);
void popopenMemArray_oneLot(char subname[], int i);

#endif
