#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


#include "poprank.h"
#include "define.h"

#include "lot.h" 
#include "lotlist.h" 

FILE *POPINDEX;
caddr_t POPINDEX_MMAP;
int ShortPopIndexSize;
//unsigned char *popMemArray;

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
	
	//static unsigned int DocID = 0;

	if (!feof((*popha).fh)) {

		fread(Rank,sizeof(int),1,(*popha).fh);


		*rDocID = (*popha).DocID++;
			
		return 1;
	}
	else {
		return 0;
	}
}

//opner popindex

void popopenMmap() {

 	int fd;
        struct stat inode;      // lager en struktur for fstat å returnere.



        if ((fd = open(SHORTPOPFILE, O_RDWR)) == -1) {
                perror("error");
        }

        fstat(fd,&inode);

	//oppdaterer global variabel slik at vi vet størelsen
	ShortPopIndexSize = inode.st_size;


        POPINDEX_MMAP = mmap(0,ShortPopIndexSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

        if ((int)POPINDEX_MMAP == -1) {
                perror("mmap");
        }
        close(fd);

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

				printf("loaded lot %i\n",i);

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

void popopenMemArray(char servername[], char subname[]) {

        FILE *FH;
	int i;
	unsigned char rank;
	int LocalLots;
	char LotFile[128];
	int branksize;

	lotlistLoad();
	lotlistMarkLocals(servername);

	//aloker minne for rank for de forskjelige lottene
	LocalLots=0;
	for (i=0;i<maxLots;i++) {
		//sjekekr om dette er en lokal lot
		

		if (lotlistIsLocal(i)) {

			GetFilPathForLot(LotFile,i,subname);
			strcat(LotFile,"Brank");

			// prøver å opne
			if ( (FH = fopen(LotFile,"rb")) == NULL ) {
                		perror(LotFile);
				popMemArray[i] = 0;
		        }
			else {

				printf("loaded lot %i\n",i);

				branksize = sizeof(unsigned char) * NrofDocIDsInLot;

				if ((popMemArray[i] = (unsigned char*)malloc(branksize)) == NULL) {
					printf("malloc eror for lot %i\n",i);
					perror("malloc");
					exit(1);
				}

				fread(popMemArray[i],branksize,1,FH);

				//debug: viser alle rankene vi laster
				//for(y=0;y<branksize;y++) {
				//	printf("DocID %i, rank %i\n",y,popMemArray[i][y]);
				//}

				fclose(FH);

				++LocalLots;

			}
		}		
		else {
			popMemArray[i] = 0;
		}
	}
	printf("have %i local lots\n",LocalLots);


}

int popRankForDocIDMemArray(unsigned int DocID) {
	int LotNr,DocIDPlace;


        //printf("DocID %i\n",DocID);
        //fohindrer at vi ber om en docid som er størrre en minne område og segfeiler.
        //burde nokk had noe felles vasking i iindex.c i steden for her. Andre liter sikkert med det samme
        //if ((DocID < ShortPopIndexSize) && (DocID > 0)) {
		//printf("ll: %i\n",popMemArray[DocID]);
                //temp: return (int) (popMemArray[DocID]);
		
		/*
		//bytet til å ha DocID som "unsigned int", fra bare "int"
		//hvis vi har en negativ DocID så er noe galt
		if (DocID < 0) {
			return -3;
		}
		*/

		//finner lot og offset
                LotNr = rLotForDOCid(DocID);
                DocIDPlace = (DocID - LotDocIDOfset(LotNr));


		if (popMemArray[LotNr] != 0) {
			//printf("have rank %u, i:%i, y:%i\n",(unsigned int)popMemArray[LotNr][DocIDPlace],LotNr,DocIDPlace);
			return popMemArray[LotNr][DocIDPlace];
		}
		else {
			return 0;
		}

}


int popRankForDocIDMmap(int DocID) {
	//printf("DocID %i\n",DocID);
	//fohindrer at vi ber om en docid som er størrre en minne område og segfeiler.
	//burde nokk had noe felles vasking i iindex.c i steden for her. Andre liter sikkert med det samme
	if ((DocID < ShortPopIndexSize) && (DocID > 0)) {
		return POPINDEX_MMAP[DocID];
	}
	else {
		return -1;
	}
}
int popopen (struct popl *popha, char *filname) {

	//printf("opening popfile %s\n",POPFILE);  

	(*popha).DocID = 0;

        if ((((*popha).fh = fopen(filname,"r+b")) == NULL) && (((*popha).fh = fopen(filname,"w+b"))  == NULL)) {
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

	// prøver å søker og lese filen, hvis det ikke går er det nokk 
	// for at vi ikke har noe rankk for denne filen enda, så vi retunerer 0
	if ((fseek((*popha).fh,DocID* sizeof(Rank),SEEK_SET) == 0) && (fread(&Rank,sizeof(Rank),1,(*popha).fh) != 0)){
		return Rank;
	}
	else {
		return -2;
	}
	
}


void popclose(struct popl *popha) {
	fclose((*popha).fh);
}

//setter verdien
void popset (struct popl *popha,int DocID,int pop) {
	//skriver ny verdi
        fseek((*popha).fh,DocID * sizeof(pop),SEEK_SET);

        fwrite(&pop,sizeof(pop),1,(*popha).fh);
}

//øker linkcounten med antall sider for en url
void popadd (struct popl *popha,int DocID,int increasement) {

	int pop = 0;
	

	//lser inn gammel verdi
	fseek((*popha).fh,DocID * sizeof(pop),SEEK_SET);
	
	fread(&pop,sizeof(pop),1,(*popha).fh);
	
	//printf("pop: %i\n",pop);

	//kalkulerer ny verdi
	pop = pop + increasement;

	//skriver ny verdi
	fseek((*popha).fh,DocID * sizeof(pop),SEEK_SET);
	
	fwrite(&pop,sizeof(pop),1,(*popha).fh);
	
	
}
