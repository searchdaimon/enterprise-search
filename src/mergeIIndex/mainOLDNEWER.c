#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//temp
#include <sys/types.h>
#include <sys/stat.h>
//

#include "../common/lot.h"
#include "../common/define.h"
#include "../common/bstr.h"

//#define subname "www"

struct iindexfileFormat {
                FILE *fileha;
                unsigned long lastTerm;
                unsigned long lastAntall;
		int nr;
		int eof;
		int open;
		char PathForLotIndex[128];
		off_t filesize;
};

//struct DictionaryFormat {
//        unsigned long WordID;
//        unsigned long Adress;
//        unsigned long SizeForTerm;
//};

int compare_elements (const void *p1, const void *p2);
int compare_elements_eof (const void *p1, const void *p2);
int compare_filenr (const void *p1, const void *p2);

int main (int argc, char *argv[]) {

	int i,y,x,z,n;

	FILE *fileha;

	struct iindexfileFormat *iindexfile;
	struct DictionaryFormat DictionaryPost;

	int nrOffIindexFiles;
	int gotEof;

	unsigned long mergeTerm;
	unsigned long totaltAntall;

        unsigned long DocID;

        unsigned long TermAntall;
        unsigned short hit;

	unsigned long currentTerm;

	unsigned char langnr;

	unsigned long term;
	unsigned long Antall;

	unsigned long startAdress;
	unsigned long stoppAdress;

	char FinalIindexFileName[128];
	char FinalDictionaryFileName[128];

	char subname[maxSubnameLength];

	char FinalIindexFilePath[128];
        char FinalDictionaryFilePath[128];
	struct stat inode;      // lager en struktur for fstat å returnere.

	int DocIDcount = 0;
	int count;

	FILE *FinalIindexFileFA;
	FILE *FinalDictionaryFileFA;

	char PathForIindex[128];
	char PathForLotIndex[128];

	int startIndex;
	int stoppIndex;
printf("argc %i\n",argc);
        if (argc != 7) {
                printf("Dette programet printer ut en iindex.\n\n");
		printf("\tUse:\n\n\t./mergeIIndex lot fralot tillot type (Main | Athor) språk subname\n\n");
                exit(0);
        }

	strncpy(subname,argv[6],sizeof(subname) -1);

	printf("Merge index %s\n",argv[1]);

	
	GetFilePathForIindex(FinalIindexFilePath,FinalIindexFileName,atol(argv[1]),argv[4],argv[5],subname);


	//sprintf(FinalIindexFilePath,"%s/iindex/%s/index/%s",PathForIindex,argv[4],argv[5]);
	//sprintf(FinalIindexFileName,"%s/%i.txt",FinalIindexFilePath,atol(argv[1]));
	printf("FinalIindexFileName %s\nFinalIindexFilePath %s\n",FinalIindexFileName,FinalIindexFilePath);

	makePath(FinalIindexFilePath);

	//sjekker om vi har nokk palss for dette
	if (!HasSufficientSpace(FinalIindexFilePath,4096)) {
                printf("insufficient disk space\n");
		exit(1);
        }


	if ((FinalIindexFileFA = fopen(FinalIindexFileName,"wb")) == NULL) {
		perror(FinalIindexFileName);
		exit(1);
	}

	//sprintf(FinalDictionaryFilePath,"%s/iindex/%s/dictionary/%s",PathForIindex,argv[4],argv[5]);
	//sprintf(FinalDictionaryFileName,"%s/%i.txt",FinalDictionaryFilePath,atol(argv[1]));
	//printf("FinalDictionaryFileName %s\n",FinalDictionaryFileName);
	GetFilePathForIDictionary(FinalDictionaryFilePath,FinalDictionaryFileName,atol(argv[1]),argv[4],argv[5],subname);


	makePath(FinalDictionaryFilePath);
	if ((FinalDictionaryFileFA = fopen(FinalDictionaryFileName,"wb")) == NULL) {
                perror(FinalDictionaryFileName);
		exit(1);
        }

	//nrOffIindexFiles = argc -2;

	startIndex = atoi(argv[2]);
        stoppIndex = atoi(argv[3]) +1;

	iindexfile = malloc((stoppIndex - startIndex) * sizeof(struct iindexfileFormat));

	//printf("out file %s\nnrOffIindexFiles %i\n",FinalIindexFileName,nrOffIindexFiles);
	

	count=0;
	for (i=startIndex;i<stoppIndex;i++) {
		//printf("iindex file nr %i\n",count);

		GetFilPathForLot(iindexfile[count].PathForLotIndex,i,subname);
		///iindex/Main/index/aa/63.txt
		
		sprintf(iindexfile[count].PathForLotIndex,"%siindex/%s/index/%s/%s.txt",iindexfile[count].PathForLotIndex,argv[4],argv[5],argv[1]);

		//printf("aa iindex file %s\n",iindexfile[count].PathForLotIndex);

                if ((iindexfile[count].fileha = fopen(iindexfile[count].PathForLotIndex,"rb")) == NULL) {
                        //perror(PathForLotIndex);
			//debug: viser hvilkene filer vi IKKE fikk åpnet
			//printf("cant open %s\n",iindexfile[count].PathForLotIndex);
                }
               	//leser inn første term
               	else if (fread(&iindexfile[count].lastTerm,sizeof(unsigned long),1,iindexfile[count].fileha) != 1) {
			printf("cant read first lastTerm for %s. Ignoring it\n",iindexfile[count].PathForLotIndex);
                        perror("read");
		}
            	else if (fread(&iindexfile[count].lastAntall,sizeof(unsigned long),1,iindexfile[count].fileha) != 1) {
			printf("cant read first lastAntall for %s. Ignoring it\n",iindexfile[count].PathForLotIndex);
                        perror("read");
		}
		else {

			//finner størelsen på filen
			fstat(fileno(iindexfile[count].fileha),&inode);
			iindexfile[count].filesize = inode.st_size;

			//debug: viser hvilkene filer vi fikk åpnet
			//printf("did open %i - %s\n",count,iindexfile[count].PathForLotIndex);


                	iindexfile[count].eof = 0;

                	iindexfile[count].nr = count;

			iindexfile[count].open = 4;

			count++;		
		}
	}	

	nrOffIindexFiles = count;
	printf("nrOffIindexFiles %i\n",nrOffIindexFiles);

	gotEof = 0;
	//for (y=0;y<nrOffIindexFiles;y++) {
	while (nrOffIindexFiles != 0) {
	//while (nrOffIindexFiles > 0) {
		//hvis vi har fått en endoffile siden sist sorterer vi slik at den kommer nederst
		if (gotEof) {


			//+gotEof da vi skal også ha med de sidene vi sorterer ut nå i sorteringen
			qsort(iindexfile, nrOffIindexFiles + gotEof, sizeof(struct iindexfileFormat), compare_elements_eof);


			//printf("got eof %i\n",gotEof);

			//nrOffIindexFiles = nrOffIindexFiles - gotEof;


			
			gotEof = 0;

		}

		//sorter de etter term
		//ToDo: dette er nokk ikke optimalt når man tenker på hastighet. Må se på dette
		qsort(iindexfile, nrOffIindexFiles , sizeof(struct iindexfileFormat), compare_elements);

		for (i=0;i<nrOffIindexFiles;i++) {
		
			//printf("%i: t %lu : %lu eof %i\n",iindexfile[i].nr,iindexfile[i].lastTerm,iindexfile[i].lastAntall,iindexfile[i].eof);
			
		}

		//finner elementene som skal kopieres inn
		mergeTerm = iindexfile[0].lastTerm;
		i = 0;
		totaltAntall = 0;
		while (mergeTerm == iindexfile[i].lastTerm) {
			totaltAntall += iindexfile[i].lastAntall;
			i++;
		}
		//printf("sort\n");
		//sorterer etter fil nr slik at vi får minste docid først
		qsort(iindexfile, i , sizeof(struct iindexfileFormat), compare_filenr);
	
		#ifdef DEBUG
			printf("totaltAntall %lu for term %lu\n",totaltAntall,iindexfile[0].lastTerm);
		#endif

		currentTerm = iindexfile[0].lastTerm;

		startAdress = (unsigned long)ftell(FinalIindexFileFA);
		fwrite(&iindexfile[0].lastTerm,sizeof(iindexfile[0].lastTerm),1,FinalIindexFileFA);
		fwrite(&totaltAntall,sizeof(totaltAntall),1,FinalIindexFileFA);

		
		i = 0;
		while ((mergeTerm == iindexfile[i].lastTerm)) {
		//kjører gjenom alle filene og skriver ut de som har index
		//for (i=0;i<nrOffIindexFiles;i++) {

			//printf("aa: %i skriv %lu : %lu fra nr %i\n",i,iindexfile[i].lastTerm,iindexfile[i].lastAntall,iindexfile[i].nr);

			
			/////////////////////////////////////////////////

			for (x=0;x<iindexfile[i].lastAntall;x++) {
                        	//side hedder

				//if (iindexfile[i].lastAntall > 100000) {
				//	printf("trying to read from %i, open %i, file %s\n",iindexfile[i].nr,iindexfile[i].open,iindexfile[i].PathForLotIndex);
				//}

				if ((n=fread(&DocID,sizeof(DocID),1,iindexfile[i].fileha)) != 1) {
					printf("cant read DocID for %s\n",iindexfile[i].PathForLotIndex);
					printf("iindex eof %i\n",iindexfile[i].eof);
					printf("i: %i\n",i);
					printf("nrOffIindexFiles %i\n",nrOffIindexFiles);
                        		perror("read");
					exit(1);
                        	}

				++DocIDcount;

				if ((n=fread(&langnr,sizeof(char),1,iindexfile[i].fileha)) != 1) {
					printf("cant read lang for %s\n",iindexfile[i].PathForLotIndex);
                        		perror("read");
					exit(1);
				}


				if ((n=fread(&TermAntall,sizeof(TermAntall),1,iindexfile[i].fileha)) != 1) {
					printf("cant read TermAntall for %s\n",iindexfile[i].PathForLotIndex);
                        		perror("read");
					exit(1);
				}
			
				//if (iindexfile[i].lastAntall > 100000) {
				//	printf("DocID %lu %lu: \n",DocID,TermAntall);
				//}


				//skriver til final index
				fwrite(&DocID,sizeof(unsigned long),1,FinalIindexFileFA);
				fwrite(&langnr,sizeof(char),1,FinalIindexFileFA);
				fwrite(&TermAntall,sizeof(unsigned long),1,FinalIindexFileFA);


                        	for (z = 0;z < TermAntall; z++) {
					if ((n=fread(&hit,sizeof(unsigned short),1,iindexfile[i].fileha)) != 1) {
						printf("cant read hit for %s\n",iindexfile[i].PathForLotIndex);
		                       		perror(iindexfile[i].PathForLotIndex);
                                        	exit(1);
                                	}

					//skriver til final index
					fwrite(&hit,sizeof(unsigned short),1,FinalIindexFileFA);


                                        //printf("%i,",hit);

                        	}
                	}


			////////////////////////////////////////////////
						
			if (feof(iindexfile[i].fileha)) {
				//printf("got eof %s\n",iindexfile[i].PathForLotIndex);
				iindexfile[i].eof = 1;
				++gotEof;
				nrOffIindexFiles--;
			}
			else if (iindexfile[i].filesize == ftell(iindexfile[i].fileha)) {
				//printf("is at end without eof\n");
				iindexfile[i].eof = 1;
				++gotEof;
				nrOffIindexFiles--;
			}
			else {
				//leser inn neste term
                        	if ((n=fread(&iindexfile[i].lastTerm,sizeof(unsigned long),1,iindexfile[i].fileha)) != 1) { 
					printf("cant read lastTerm for %s\n",iindexfile[i].PathForLotIndex);
                       			perror("read");
					exit(1);			
				}
                        	if ((n=fread(&iindexfile[i].lastAntall,sizeof(unsigned long),1,iindexfile[i].fileha)) != 1) {
					printf("cant read lastAntall for %s\n",iindexfile[i].PathForLotIndex);
                        		perror("read");
                                	exit(1);
				}
			}
			
			i++;
		}

		stoppAdress = (unsigned long)ftell(FinalIindexFileFA);;
                DictionaryPost.WordID = currentTerm;
                DictionaryPost.Adress = startAdress;
                DictionaryPost.SizeForTerm = stoppAdress - startAdress;

                fwrite(&DictionaryPost,sizeof(DictionaryPost),1,FinalDictionaryFileFA);

		//printf("WordID %ul, Adress: %ul, SizeForTerm: %ul\n",DictionaryPost.WordID,DictionaryPost.Adress,DictionaryPost.SizeForTerm);

		//printf("\n");
	}

	fclose(FinalDictionaryFileFA);
	fclose(FinalIindexFileFA);

	for (i=2;i<nrOffIindexFiles;i++) {
		fclose(iindexfile[i].fileha);
	}

	printf("DocIDcount: %i\n",DocIDcount);

}








int compare_elements (const void *p1, const void *p2) {

        //return i1 - i2;
        struct iindexfileFormat *t1 = (struct iindexfileFormat*)p1;
        struct iindexfileFormat *t2 = (struct iindexfileFormat*)p2;

	if (t1->lastTerm < t2->lastTerm)
                return -1;
        else
                return t1->lastTerm > t2->lastTerm;

}

int compare_elements_eof (const void *p1, const void *p2) {

        //return i1 - i2;
        struct iindexfileFormat *t1 = (struct iindexfileFormat*)p1;
        struct iindexfileFormat *t2 = (struct iindexfileFormat*)p2;

        if (t1->eof < t2->eof)
                return -1;
        else
                return t1->eof > t2->eof;

}

int compare_filenr (const void *p1, const void *p2) {

        //return i1 - i2;
        struct iindexfileFormat *t1 = (struct iindexfileFormat*)p1;
        struct iindexfileFormat *t2 = (struct iindexfileFormat*)p2;

        if (t1->nr < t2->nr)
                return -1;
        else
                return t1->nr > t2->nr;

}

