#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "integerindex.h"

#include "lot.h"
#include "define.h"

#include "lot.h"
#include "lotlist.h"

#define MMAP_INTI
#ifdef MMAP_INTI
       	#include <sys/types.h>
       	#include <sys/stat.h>
       	#include <fcntl.h>
	#include <sys/mman.h>
	#include <sys/resource.h>
#endif

int iintegerOpenForLot(struct iintegerFormat *iinteger,char index[],int elementsize ,int lotNr, char type[],char subname[]) {

	
	//if ((iinteger->FH = lotOpenFileNoCasheByLotNr(lotNr,index,type, 'e', subname)) == NULL) {
	//jayde
	if ((iinteger->FH = lotOpenFileNoCasheByLotNr(lotNr,index,type, 'd', subname)) == NULL) {
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
	off_t totsize = 0;

        lotlistLoad();
        lotlistMarkLocals(servername);
	
	for (i=0;i<maxLots;i++) {
		iintegerMemArray->MemArray[i] = NULL;
	}

	size = elementsize * NrofDocIDsInLot;

	for (i=0;i<maxLots;i++) {
                //sjekekr om dette er en lokal lot


                if (lotlistIsLocal(i)) {

			#ifdef DEBUG
			printf("iintegerLoadMemArray: local lot %i\n",i);
			#endif
			if (!iintegerOpenForLot(&iinteger,index,elementsize,i,"r+",subname)) {
				#ifdef DEBUG
				printf("don't have \"%s\" for lot %i\n",index,i);
				#endif
				continue;
			}

			fstat(fileno(iinteger.FH),&inode);
			#ifdef DEBUG
			printf("File size: %i\n",inode.st_size);
			#endif

			#ifdef MMAP_INTI

			if (inode.st_size < size) {
				fprintf(stderr,"iintegerLoadMemArray: file is smaler then size. file size %"PRId64", suposed to be %i\n",inode.st_size,size);

                                /*
                                Stretch the file size to the size of the (mmapped) array of ints
                                */
                                if (fseek(iinteger.FH, size +1, SEEK_SET) == -1) {
                                        perror("Error calling fseek() to 'stretch' the file");
                                        exit(EXIT_FAILURE);
                                }

                                /* Something needs to be written at the end of the file to
                                * have the file actually have the new size.
                                * Just writing an empty string at the current file position will do.
                                *
                                * Note:
                                *  - The current position in the file is at the end of the stretched
                                *    file due to the call to fseek().
                                *  - An empty string is actually a single '\0' character, so a zero-byte
                                *    will be written at the last byte of the file.
                                */
                                if (fwrite("", 1, 1, iinteger.FH) != 1) {
                                        perror("Error writing last byte of the file");
                                        exit(EXIT_FAILURE);
                                }

			}

			if ((iintegerMemArray->MemArray[i] = mmap(0,size,PROT_READ,MAP_SHARED,fileno(iinteger.FH),0) ) == NULL) {
				fprintf(stderr,"iintegerLoadMemArray: can't mmap for lot %i\n",i);
                        	perror("mmap");
                        }
			
			#else
			//if ((iintegerMemArray->MemArray[i] = malloc(inode.st_size)) == NULL) {
			if ((iintegerMemArray->MemArray[i] = malloc(size) ) == NULL) {
				perror("malloc");
				continue;
			}

			memset(iintegerMemArray->MemArray[i],'\0',size);

			fread(iintegerMemArray->MemArray[i],inode.st_size,1,iinteger.FH);
			#endif

			totsize += size;			

			iintegerClose(&iinteger);
		}

	}

	printf("did load a total of %"PRId64" into memory\n",totsize);
}

int iintegerLoadMemArray2(struct iintegerMemArrayFormat *iintegerMemArray,char index[], int elementsize,char subname[]) {

	int i, y ,z;
	struct iintegerFormat iinteger;
	struct stat inode;      // lager en struktur for fstat å returnere.
	int size;
	off_t totsize = 0;

        int locklimit;

        //finner grensen på antal sider vi kan låse i minne
        struct rlimit rlim;

        if (getrlimit(RLIMIT_MEMLOCK, &rlim) < 0) {
                printf("Warning: Cannot raise RLIMIT_MEMLOCK limits.");
                locklimit = 0;
        }
        else {
                locklimit = rlim.rlim_cur;
        }


	
	for (i=0;i<maxLots;i++) {
		iintegerMemArray->MemArray[i] = NULL;
	}

	size = elementsize * NrofDocIDsInLot;

	for (i=0;i<maxLots;i++) {
                //sjekekr om dette er en lokal lot



			if (!iintegerOpenForLot(&iinteger,index,elementsize,i,"r+",subname)) {
				#ifdef DEBUG
				printf("don't have \"%s\" for lot %i\n",index,i);
				#endif
				continue;
			}

			fstat(fileno(iinteger.FH),&inode);

			if (inode.st_size == 0) {
				#ifdef DEBUG
				printf("file is emty \"%s\" for lot %i\n",index,i);
				#endif
				continue;
			}

			//#ifdef DEBUG
			printf("iintegerLoadMemArray: local lot %i\n",i);
			//#endif


			#ifdef DEBUG
			printf("File size: %i\n",inode.st_size);
			#endif

			#ifdef MMAP_INTI

			if (inode.st_size < size) {
				fprintf(stderr,"iintegerLoadMemArray: file is smaler then size. file size %"PRId64", suposed to be %i\n",inode.st_size,size);

                                /*
                                Stretch the file size to the size of the (mmapped) array of ints
                                */
                                if (fseek(iinteger.FH, size +1, SEEK_SET) == -1) {
                                        perror("Error calling fseek() to 'stretch' the file");
                                        exit(EXIT_FAILURE);
                                }

                                /* Something needs to be written at the end of the file to
                                * have the file actually have the new size.
                                * Just writing an empty string at the current file position will do.
                                *
                                * Note:
                                *  - The current position in the file is at the end of the stretched
                                *    file due to the call to fseek().
                                *  - An empty string is actually a single '\0' character, so a zero-byte
                                *    will be written at the last byte of the file.
                                */
                                if (fwrite("", 1, 1, iinteger.FH) != 1) {
                                        perror("Error writing last byte of the file");
                                        exit(EXIT_FAILURE);
                                }

			}

			if ((iintegerMemArray->MemArray[i] = mmap(0,size,PROT_READ,MAP_SHARED,fileno(iinteger.FH),0) ) == NULL) {
				fprintf(stderr,"iintegerLoadMemArray: can't mmap for lot %i\n",i);
                        	perror("mmap");
                        }

                        //hvis vi kan låse uendelig med minne gjør vi det
                        if (locklimit == -1) {
                        	//laster all rankerings dataen fra minne, slik ad det går fort å leste den inn siden
                                z = 0;
                                for(y=0;y<NrofDocIDsInLot;y++) {
                                	z += iintegerMemArray->MemArray[i][y];
                               	}
                                //låser minne
                                if (mlock(iintegerMemArray->MemArray[i],size) != 0) {
                                	perror("iintegerLoadMemArray2: mlock array");
                                        //exit(1);
                                }
                        }

			#else
			//if ((iintegerMemArray->MemArray[i] = malloc(inode.st_size)) == NULL) {
			if ((iintegerMemArray->MemArray[i] = malloc(size) ) == NULL) {
				perror("malloc");
				continue;
			}

			memset(iintegerMemArray->MemArray[i],'\0',size);

			fread(iintegerMemArray->MemArray[i],inode.st_size,1,iinteger.FH);
			#endif

			totsize += size;			

			iintegerClose(&iinteger);

	}

	printf("did load a total of %"PRId64" into memory\n",totsize);
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

int iintegerGetValue(void *value,int valuesize,unsigned int DocID,char indexname[],char subname[]) {
	static FILE *fh;
	unsigned int DocIDPlace;

	#ifdef DEBUG
		printf("geting value of size %i for DocID %u for index \"%s\"\n",valuesize,DocID,indexname);
	#endif

	if ((fh = lotOpenFile(DocID,indexname,"rb",'r',subname)) == NULL) {
		return 0;
	}

	DocIDPlace = iintegerDocIDPlace(DocID,valuesize);

	#ifdef DEBUG
		printf("iintegerGetValue: DocIDPlace %u\n",DocIDPlace);
	#endif

	fseek(fh,DocIDPlace,SEEK_SET);
	fread(value,valuesize,1,fh);

	return 1;
}

