#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>

#define MMAP_POP
#ifdef MMAP_POP
       	#include <sys/types.h>
       	#include <sys/stat.h>
	#include <fcntl.h>
	#include <sys/resource.h>
#endif

#include "poprank.h"
#include "define.h"
#include "lot.h" 
#include "lotlist.h" 


#define mmap_MaxDocIDToAdd 100000000

FILE *POPINDEX;

unsigned char *popMemArray[maxLots];

unsigned int NrOfElementInPopIndex () {
	struct stat inode;      // lager en struktur for fstat å returnere.
	unsigned int popMemorySize;

	fstat(fileno(POPINDEX),&inode);

        popMemorySize = (inode.st_size / sizeof(int));
	
	printf("aa size %i\n",popMemorySize);

	return popMemorySize;
}

int popGetNext (struct popl *popha, int *Rank,unsigned int *rDocID) {

	if (!feof((*popha).fh)) {

		fread(Rank,sizeof(int),1,(*popha).fh);


		*rDocID = (*popha).DocID++;
			
		return 1;
	}
	else {
		return 0;
	}
}



void popopenMemArray_oneLot(char subname[], int i) {

        FILE *FH;
	unsigned char rank;
	int LocalLots;
	char LotFile[128];
	int branksize;


	GetFilPathForLot(LotFile,i,subname);
	strcat(LotFile,"Brank");

	// prøver å opne
	if ( (FH = fopen(LotFile,"rb")) == NULL ) {
		perror(LotFile);
		popMemArray[i] = 0;
	}
	else {
		#ifdef DEBUG
			printf("loaded lot %i\n",i);
		#endif

		branksize = sizeof(unsigned char) * NrofDocIDsInLot;

		if ((popMemArray[i] = (unsigned char*)malloc(branksize)) == NULL) {
			printf("malloc eror for lot %i\n",i);
			perror("malloc");
			exit(1);
		}

		fread(popMemArray[i],branksize,1,FH);

		//debug: viser alle rankene vi laster
		/*
		int y;
		for(y=0;y<NrofDocIDsInLot;y++) {
			printf("DocID %i, rank %u. i:%i, y:%i\n",(y + LotDocIDOfset(i)),(unsigned int)popMemArray[i][y],i,y);
		}
		*/				

		fclose(FH);

		++LocalLots;

	}

}

void popopenMemArray2(char subname[], char rankfile[]) {

        FILE *FH;
	int i, y, z;
	unsigned char rank;
	int LocalLots;
	char LotFile[128];
	int branksize;
	struct stat inode;
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

	//aloker minne for rank for de forskjelige lottene
	LocalLots=0;
	for (i=0;i<maxLots;i++) {
		//sjekekr om dette er en lokal lot
		
		popMemArray[i] = 0;

		GetFilPathForLot(LotFile,i,subname);
		strcat(LotFile,rankfile);

		// prøver å opne
		if ( (FH = fopen(LotFile,"rb")) == NULL ) {
			#ifdef DEBUG
				perror(LotFile);
			#endif
			
			continue;
		}


		fstat(fileno(FH),&inode);

		if (inode.st_size == 0) {
			printf("file is emty\n");
			continue;
		}

		#ifdef DEBUG
			printf("popopenMemArray2: loaded lot %i\n",i);
		#endif

		branksize = sizeof(unsigned char) * NrofDocIDsInLot;

		printf("branksize offset: %i\n",branksize % getpagesize());


		#ifdef MMAP_POP

			//vi må aligne dette med pagesize
			//branksize += branksize % getpagesize();

			if (inode.st_size != branksize) {
				fprintf(stderr,"popopenMemArray: file is smaler then size. file size %"PRId64", suposed to be %i\n",inode.st_size,branksize);
				continue;
			}
			
			//MAP_LOCKED (since Linux 2.5.37) 
			//	Lock the pages of the mapped region into memory in the manner of mlock(). This flag is ignored in older kernels. 

			if ((popMemArray[i] = mmap(0,branksize,PROT_READ,MAP_SHARED,fileno(FH),0) ) == MAP_FAILED) {
				fprintf(stderr,"popopenMemArray: can't mmap for lot %i\n",i);
				perror("mmap");
			}


			//hvis vi kan låse uendelig med minne gjør vi det
			if (locklimit == -1) {
				//laster all rankerings dataen fra minne, slik ad det går fort å leste den inn siden
				z = 0;
				for(y=0;y<NrofDocIDsInLot;y++) {
					z += popMemArray[i][y];	
				}
				//låser minne
				if (mlock(popMemArray[i],branksize) != 0) {
					perror("mlock");
				}
			}

			
		#else

			if ((popMemArray[i] = (unsigned char*)malloc(branksize)) == NULL) {
				printf("malloc eror for lot %i\n",i);
				perror("malloc");
				exit(1);
			}

			fread(popMemArray[i],branksize,1,FH);

		#endif

		//debug: viser alle rankene vi laster
		/*
		for(y=0;y<branksize;y++) {
			printf("DocID %i, rank %i\n",y,popMemArray[i][y]);
		}
		*/
		
		totsize += branksize;
	
		fclose(FH);

		++LocalLots;

	}

	printf("popopenMemArray: have %i local lots\n",LocalLots);
	printf("popopenMemArray: loaded a total of %"PRId64" bytes\n",totsize);

}

void popopenMemArray(char servername[], char subname[], char rankfile[]) {

        FILE *FH;
	int i;
	unsigned char rank;
	int LocalLots;
	char LotFile[128];
	int branksize;
	struct stat inode;
	off_t totsize = 0;	

	lotlistLoad();
	lotlistMarkLocals(servername);

	//aloker minne for rank for de forskjelige lottene
	LocalLots=0;
	for (i=0;i<maxLots;i++) {
		//sjekekr om dette er en lokal lot
		

		if (lotlistIsLocal(i)) {

			GetFilPathForLot(LotFile,i,subname);
			strcat(LotFile,rankfile);

			// prøver å opne
			if ( (FH = fopen(LotFile,"rb")) == NULL ) {
                		perror(LotFile);
				popMemArray[i] = 0;
		        }
			else {
				#ifdef DEBUG
					printf("loaded lot %i\n",i);
				#endif
				branksize = sizeof(unsigned char) * NrofDocIDsInLot;

				#ifdef MMAP_POP
					fstat(fileno(FH),&inode);

					if (inode.st_size != branksize) {
						fprintf(stderr,"popopenMemArray: file is smaler then size. file size %"PRId64", suposed to be %i\n",inode.st_size,branksize);
						continue;
					}

                        		if ((popMemArray[i] = mmap(0,branksize,PROT_READ,MAP_SHARED,fileno(FH),0) ) == NULL) {
                                		fprintf(stderr,"popopenMemArray: can't mmap for lot %i\n",i);
                                		perror("mmap");
                        		}
				#else

					if ((popMemArray[i] = (unsigned char*)malloc(branksize)) == NULL) {
						printf("malloc eror for lot %i\n",i);
						perror("malloc");
						exit(1);
					}

					fread(popMemArray[i],branksize,1,FH);

				#endif

				//debug: viser alle rankene vi laster
				/*
				for(y=0;y<branksize;y++) {
					printf("DocID %i, rank %i\n",y,popMemArray[i][y]);
				}
				*/

				totsize += branksize;
		
				fclose(FH);

				++LocalLots;

			}
		}		
		else {
			popMemArray[i] = 0;
		}
	}
	printf("popopenMemArray: have %i local lots\n",LocalLots);
	printf("popopenMemArray: loaded a total of %"PRId64" bytes\n",totsize);

}

int popRankForDocIDMemArray(unsigned int DocID) {
	int LotNr,DocIDPlace;

	//finner lot og offset
	LotNr = rLotForDOCid(DocID);
	DocIDPlace = (DocID - LotDocIDOfset(LotNr));

	if (popMemArray[LotNr] != 0) {
		#ifdef DEBUG
			printf("have rank %u, i:%i, y:%i\n",(unsigned int)popMemArray[LotNr][DocIDPlace],LotNr,DocIDPlace);
		#endif
		return popMemArray[LotNr][DocIDPlace];
	}
	else {
		return 0;
	}
}

/*
//opner popindex

void *mmap3264(void *addr, off_t len, int prot, int flags,int fildes, off_t off) {
	
	int i, bloacks,size;
	off_t offset;

	
	static void *mem[10];

	offset = off;

	bloacks = (int)ceil(len / 2147483647);

	printf("bloacks %i, len %"PRId64"\n",bloacks,len);

	for (i=0;i<bloacks;i++) {

		size = 2147483647;

        	mem[i] = mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fildes,0);
		printf("mmap ret %i\n",(unsigned int)mem[i]);

       		if ((int)mem[i] == -1) {
       		        perror("mmap");
			return 0;
        	}

		offset += size;

	}
	exit(1);
}
*/
int popopenMmap(struct popmemmapFormat *popmemmap,char *filname) {

        struct stat inode;      // lager en struktur for fstat å returnere.
	int i;
	popmemmap->largesDocID = 0;

        if ((popmemmap->fd = open(filname, O_RDWR)) == -1) {
                perror(filname);
		return 0;
        }

        fstat(popmemmap->fd,&inode);

	popmemmap->size = inode.st_size;
	printf("old file size %"PRId64"\n",popmemmap->size);

        popmemmap->size += (sizeof(unsigned int) * mmap_MaxDocIDToAdd);
	printf("new file size %"PRId64"\n",popmemmap->size);

	#ifdef DEBUG

	#else
		/*
		Stretch the file size to the size of the (mmapped) array of ints
		*/

		if (lseek(popmemmap->fd, popmemmap->size +1, SEEK_SET) == -1) {
			perror("Error calling lseek() to 'stretch' the file");
			exit(EXIT_FAILURE);
		}

		/* Something needs to be written at the end of the file to
		* have the file actually have the new size.
		* Just writing an empty string at the current file position will do.
		*
		* Note:
		*  - The current position in the file is at the end of the stretched
		*    file due to the call to lseek().
		*  - An empty string is actually a single '\0' character, so a zero-byte
		*    will be written at the last byte of the file.
		*/
		if (write(popmemmap->fd, "", 1) != 1) {
			perror("Error writing last byte of the file");
			exit(EXIT_FAILURE);
		}
	#endif



        popmemmap->ranks = mmap(0,popmemmap->size,PROT_READ|PROT_WRITE,MAP_SHARED,popmemmap->fd,0);
	printf("mmap ret %i\n",(unsigned int)popmemmap->ranks);


       	if ((int)popmemmap->ranks == -1) {
       	        perror("mmap");
		return 0;
        }


	return 1;

}


void popcloseMmap (struct popmemmapFormat *popmemmap) {
	ftruncate(popmemmap->fd,(popmemmap->largesDocID * sizeof(unsigned int)) );

	munmap(popmemmap->ranks,popmemmap->size);

        close(popmemmap->fd);
	
}
int popRankForDocIDMmap(struct popmemmapFormat *popmemmap,unsigned int DocID) {

	//fohindrer at vi ber om en docid som er størrre en minne område og segfeiler.
	if ((DocID * sizeof(unsigned int)) < popmemmap->size) {
		return popmemmap->ranks[DocID];
	}
	else {
		return -1;
	}
}
int popRankForDocIDMmapSet(struct popmemmapFormat *popmemmap,unsigned int DocID,int increasement) {

	//fohindrer at vi ber om en docid som er størrre en minne område og segfeiler.

	off_t size = (DocID * sizeof(unsigned int));

	if (size < popmemmap->size) {
		popmemmap->ranks[DocID] += increasement;

		if (DocID > popmemmap->largesDocID) {
			popmemmap->largesDocID = DocID;
		}

		return popmemmap->ranks[DocID];

	}
	else {
		printf("DocID %u out of mmap bounds\n",DocID);
		return -1;

	}


}
int popopen (struct popl *popha, char *filname) {  

	(*popha).DocID = 0;

        if ((((*popha).fh = (FILE *)fopen(filname,"r+b")) == NULL) && (((*popha).fh = fopen(filname,"w+b"))  == NULL)) {
                perror(filname);
		return 0;
        }
	else {
		return 1;
	}

}
/*
Henter ut filhontereren for direkte jobbing mot den
*/
FILE *GetFileHander() {

	return POPINDEX;
}

/*
Gir rank for en DocID
*/
int popRankForDocID(struct popl *popha,int DocID) {

	int Rank  = -1;

	off_t place;	

	place = DocID * sizeof(int);

	// prøver å søker og lese filen, hvis det ikke går er det nokk 
	// for at vi ikke har noe rankk for denne filen enda, så vi retunerer 0
	if (fseeko64((*popha).fh,place,SEEK_SET) != 0 ) {
		return -2;
	}
	else if (fread(&Rank,sizeof(Rank),1,(*popha).fh) != 1){
		printf("can't read at %"PRId64"\n",place);
		return -3;
	}
	else {
		return Rank;
	}
	
}


void popclose(struct popl *popha) {
	fclose((*popha).fh);
}

//setter verdien
void popset (struct popl *popha,int DocID,int pop) {
	//skriver ny verdi
        fseeko((*popha).fh,DocID * sizeof(pop),SEEK_SET);

        fwrite(&pop,sizeof(pop),1,(*popha).fh);
}

//øker linkcounten med antall sider for en url
void popadd (struct popl *popha,int DocID,int increasement) {

	int pop = 0;
	off_t place;	

	place = DocID * sizeof(pop);

	//lser inn gammel verdi
	if (fseeko((*popha).fh,place,SEEK_SET) != 0) {
		perror("popadd: fseek 1");
		return;
	}
	
	if (fread(&pop,sizeof(pop),1,(*popha).fh) != 1) {
		perror("popadd: read");
	}

	//kalkulerer ny verdi
	pop = pop + increasement;

	//skriver ny verdi
	if (fseeko((*popha).fh,place,SEEK_SET) != 0) {
		perror("popadd: fseek 2");
		return;		
	}
	
	if (fwrite(&pop,sizeof(pop),1,(*popha).fh) != 1) {
		perror("popadd: fwrite");
		return;			
	}
	
}
