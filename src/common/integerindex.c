#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/lot.h"

unsigned int iintegerDocIDPlace(unsigned int DocID,int valuesize) {

	int lotNr;
	unsigned int DocIDPlace;

	lotNr = rLotForDOCid(DocID);
        DocIDPlace = ((DocID - LotDocIDOfset(lotNr)) * valuesize) - valuesize;

	return DocIDPlace;
}

int iintegerSetValue(void *value,int valuesize,unsigned int DocID,char indexname[],char subname[]) {

	FILE *fh;
	unsigned int DocIDPlace;

	#ifdef DEBUG
	printf("set value %u for DocID %u\n",value,DocID);
	#endif

	fh = lotOpenFileNoCashe(DocID,indexname,">>",'w',subname);

	DocIDPlace = iintegerDocIDPlace(DocID,valuesize);

	fseek(fh,DocIDPlace,SEEK_SET);
	fwrite(value,valuesize,1,fh);


	fclose(fh);
}

int iintegerGetValueNoCashe(void *value,int valuesize,unsigned int DocID,char indexname[],char subname[]) {
	static FILE *fh;
	unsigned int DocIDPlace;

	//printf("get value %u for DocID %u\n",value,DocID);
	if ((fh = lotOpenFileNoCashe(DocID,indexname,"rb",'w',subname)) == NULL) {
		return 0;
	}

	DocIDPlace = iintegerDocIDPlace(DocID,valuesize);

	//printf("DocIDPlace %u\n",DocIDPlace);

	fseek(fh,DocIDPlace,SEEK_SET);
	fread(value,valuesize,1,fh);


	fclose(fh);

	return 1;
}
