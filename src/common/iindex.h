#include "define.h"

void GetIndexAsArray (int *AntallTeff, struct iindexFormat *TeffArray,
                unsigned int WordIDcrc32, char * IndexType, char *IndexSprok,
                struct subnamesFormat *subname,
                int languageFilterNr, int languageFilterAsNr[] );

void GetNForTerm(unsigned int WordIDcrc32, char *IndexType, char *IndexSprok, int  *TotaltTreff, struct subnamesFormat *subname);

void IIndexLoad (char Type[], char lang[],char subname[]);

void IIndexInaliser();

