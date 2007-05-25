#include "define.h"

void GetIndexAsArray (int *AntallTeff, struct iindexFormat *TeffArray,
                unsigned long WordIDcrc32, char * IndexType, char *IndexSprok,
                struct subnamesFormat *subname,
                int (*rank)(const unsigned short *,const int,const unsigned int DocID,struct subnamesFormat *subname,struct iindexFormat *TeffArray),
                int languageFilterNr, int languageFilterAsNr[] );

void GetNForTerm(unsigned long WordIDcrc32, char *IndexType, char *IndexSprok, int  *TotaltTreff, char subname[]);

void IIndexLoad (char Type[], char lang[],char subname[]);

void IIndexInaliser();

