#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "integerindex.h"

#include "lot.h"
#include "define.h"

#include "lot.h"
#include "lotlist.h"


int iintegerOpenForLot(struct iintegerFormat *iinteger,char index[],int elementsize ,int lotNr, char type[],char subname[]) {

	
	if ((iinteger->FH = lotOpenFileNoCasheByLotNr(lotNr,index,type, 'e', subname)) == NULL) {
		return 0;
	}
	
	iinteger->lotNr = lotNr;;
	iinteger->elementsize = elementsize;

	return 1;
}

void iintegerClose(struct iintegerFormat *iinteger) {

	#ifdef DEBUG
	printf("closinf iinteger\n");
	#endif

	iinteger->lotNr = 0;
	fclose(iinteger->FH);
}

unsigned int iintegerDocIDPlace(unsigned int DocID,int valuesize) {

	int lotNr;
	unsigned int DocIDPlace;

	lotNr = rLotForDOCid(DocID);

	#ifdef BLACK_BOKS
	        DocIDPlace = ((DocID - LotDocIDOfset(lotNr)) * valuesize) - valuesize;
	#else
	        DocIDPlace = ((DocID - LotDocIDOfset(lotNr)) * valuesize);
	#endif

	return DocIDPlace;
}

int iintegerMemArrayGet (struct iintegerMemArrayFormat *iintegerMemArray,void **value,int valuesize,unsigned int DocID) {

	int lotNr;
	unsigned int DocIDPlace;

	lotNr = rLotForDOCid(DocID);


	DocIDPlace = iintegerDocIDPlace(DocID,valuesize);

	if (iintegerMemArray->MemArray[lotNr] == NULL) {
		return 0;
	}

	//(*value) = &iintegerMemArray->MemArray[lotNr][DocIDPlace];
	(*value) = (iintegerMemArray->MemArray[lotNr] + DocIDPlace);

	#ifdef DEBUG
	printf("iintegerMemArrayGet: value %ho, DocID %u-%i DocIDPlace %u, size %i\n",iintegerMemArray->MemArray[lotNr][DocIDPlace],DocID,lotNr,DocIDPlace,valuesize);
	#endif

	return 1;
}

int iintegerLoadMemArray(struct iintegerMemArrayFormat *iintegerMemArray,char index[], int elementsize,char servername[], char subname[]) {

	int i;
	struct iintegerFormat iinteger;
	struct stat inode;      // lager en struktur for fstat å returnere.
	int size;

        lotlistLoad();
        lotlistMarkLocals(servername);
	
	for (i=0;i<maxLots;i++) {
		iintegerMemArray->MemArray[i] = NULL;
	}

	size = elementsize * NrofDocIDsInLot;

	for (i=0;i<maxLots;i++) {
                //sjekekr om dette er en lokal lot


                if (lotlistIsLocal(i)) {

			printf("iintegerLoadMemArray: local lot %i\n",i);
			
			if (!iintegerOpenForLot(&iinteger,index,elementsize,i,"r",subname)) {
				printf("don't have \"%s\" for ot %i\n",index,i);
				continue;
			}

			fstat(fileno(iinteger.FH),&inode);
			printf("File size: %i\n",inode.st_size);


			//if ((iintegerMemArray->MemArray[i] = malloc(inode.st_size)) == NULL) {
			if ((iintegerMemArray->MemArray[i] = malloc(size) ) == NULL) {
				perror("malloc");
				continue;
			}

			memset(iintegerMemArray->MemArray[i],'\0',size);

			fread(iintegerMemArray->MemArray[i],inode.st_size,1,iinteger.FH);

			iintegerClose(&iinteger);
		}

	}
}


void iintegerSetValue(struct iintegerFormat *iinteger,void *value,int valuesize,unsigned int DocID,char subname[]) {

	unsigned int DocIDPlace;

	DocIDPlace = iintegerDocIDPlace(DocID,valuesize);

	#ifdef DEBUG
	printf("DocID %u, DocIDPlace %u, valuesize %i, fh %i, value %ho\n",DocID,DocIDPlace,valuesize,(unsigned int)iinteger->FH,(*(unsigned short *)value));
	#endif
	if (fseek(iinteger->FH,DocIDPlace,SEEK_SET) != 0) {
		perror("fseek");
	}
	if (fwrite(value,valuesize,1,iinteger->FH) != 1) {
		perror("fseek");		
	}
	
}



int iintegerSetValueNoCashe(void *value,int valuesize,unsigned int DocID,char indexname[],char subname[]) {

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
