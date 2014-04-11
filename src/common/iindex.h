#ifndef _IINDEX__H_
#define _IINDEX__H_


#include "define.h"

struct IndekserOptFormat {
        int optAllowDuplicates;
        int optMustBeNewerThen;
	char *optValidDocIDs;
        int sequenceMode;
        int garbareCollection;
};


void GetIndexAsArray (int *AntallTeff, struct iindexFormat *TeffArray,
                unsigned int WordIDcrc32, char * IndexType, char *IndexSprok,
                struct subnamesFormat *subname,
                int languageFilterNr, int languageFilterAsNr[] );

void _GetIndexAsArray (int *AntallTeff, struct iindexFormat *TeffArray,
                unsigned int WordIDcrc32, char * IndexType, char *IndexSprok,
                struct subnamesFormat *subname,
                int languageFilterNr, int languageFilterAsNr[], void *(filemap)(char *, size_t *) );


void GetNForTerm(unsigned int WordIDcrc32, char *IndexType, char *IndexSprok, int  *TotaltTreff, struct subnamesFormat *subname);
void _GetNForTerm(unsigned int WordIDcrc32, char *IndexType, char *IndexSprok, int  *TotaltTreff, struct subnamesFormat *subname, void *(filemap)(char *, size_t *));



void IIndexLoad (char subname[]);

void IIndexInaliser();

int Indekser(int lotNr,char type[],int part,char subname[], struct IndekserOptFormat *IndekserOpt);

void Indekser_deleteGcedFile(int lotNr, char subname[]);

void mergei (int bucket,int startIndex,int stoppIndex,char *type,char *lang,char *subname, int *DocIDcount);


#endif // _IINDEX__H_
