#include <stdio.h>
#include "../common/lot.h"
#include "../common/define.h"

struct iindexfileFormat {
                FILE *fileha;
                unsigned long lastTerm;
                unsigned long lastAntall;
		int nr;
		int eof;
		int Have;
};

//struct DictionaryFormat {
//        unsigned long WordID;
//        unsigned long Adress;
//        unsigned long SizeForTerm;
//};

int compare_elements (const void *p1, const void *p2);
int compare_elements_eof (const void *p1, const void *p2);

int main (int argc, char *argv[]) {

	int i,y,x,z,n;

	FILE *fileha;

	struct iindexfileFormat *iindexfileArray;

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

	unsigned long term;
	unsigned long Antall;

	unsigned long startAdress;
	unsigned long stoppAdress;

	char FinalIindexFileName[128];
	char FinalDictionaryFileName[128];

	char FinalIindexFilePath[128];
        char FinalDictionaryFilePath[128];

	int DocIDcount = 0;
	int count;

	FILE *FinalIindexFileFA;
	FILE *FinalDictionaryFileFA;

	char PathForIindex[128];
	char PathForLotIndex[128];

	int startIndex;
	int stoppIndex;

        if (argc < 2) {
                printf("Dette programet printer ut en iindex. Gi den en fil inn\n\n");
                exit(0);
        }

	GetFilePathForIindex(PathForIindex,atol(argv[1]));


	sprintf(FinalIindexFilePath,"%s/iindex/%s/index/%s",PathForIindex,"Main","aa");
	sprintf(FinalIindexFileName,"%s/%i.txt",FinalIindexFilePath,atol(argv[1]));
	printf("FinalIindexFileName %s\n",FinalIindexFileName);
	makePath(FinalIindexFilePath);
	if ((FinalIindexFileFA = fopen(FinalIindexFileName,"wb")) == NULL) {
		perror(FinalIindexFileName);
		exit(1);
	}

	sprintf(FinalDictionaryFilePath,"%s/iindex/%s/dictionary/%s",PathForIindex,"Main","aa");
	sprintf(FinalDictionaryFileName,"%s/%i.txt",FinalDictionaryFilePath,atol(argv[1]));
	printf("FinalDictionaryFileName %s\n",FinalDictionaryFileName);
	makePath(FinalDictionaryFilePath);
	if ((FinalDictionaryFileFA = fopen(FinalDictionaryFileName,"wb")) == NULL) {
                perror(FinalDictionaryFileName);
		exit(1);
        }

	nrOffIindexFiles = argc -2;

	iindexfile = malloc(nrOffIindexFiles * sizeof(struct iindexfileFormat));
	iindexfileArray = malloc(nrOffIindexFiles * sizeof(struct iindexfileFormat));

	printf("out file %s\nnrOffIindexFiles %i\n",FinalIindexFileName,nrOffIindexFiles);
	
	startIndex = atoi(argv[2]);
        stoppIndex = atoi(argv[3]) +1;

	count=0;
	for (i=startIndex;i<stoppIndex;i++) {
		//printf("iindex file nr %i\n",i);

		GetFilPathForLot(PathForLotIndex,i);
		///iindex/Main/index/aa/63.txt
		sprintf(PathForLotIndex,"%siindex/%s/index/%s/%s.txt",PathForLotIndex,argv[4],argv[5],argv[1]);

		printf("iindex file %s\n",PathForLotIndex);

                if ((iindexfile[count].fileha = fopen(PathForLotIndex,"rb")) == NULL) {
                        perror(PathForLotIndex);
                }
		else {

                	iindexfile[count].eof = 0;

                	//leser inn første term
                	fread(&iindexfile[count].lastTerm,sizeof(unsigned long),1,iindexfile[count].fileha);
                	fread(&iindexfile[count].lastAntall,sizeof(unsigned long),1,iindexfile[count].fileha);

                	iindexfile[count].nr = count;
			iindexfile[count].Have =0;

			iindexfileArray[count] = iindexfile[count];

			count++;
		}
	}	
	nrOffIindexFiles = count;
	/*
	//inaliserer iindeksene
	for (i=0;i<nrOffIindexFiles;i++) {
		printf("iindex file %s\n",argv[i +2]);

		if ((iindexfile[i].fileha = fopen(argv[i +2],"rb")) == NULL) {
                	perror(argv[i +2]);
			exit(1);
        	}

		iindexfile[i].eof = 0;

		//leser inn første term
		fread(&iindexfile[i].lastTerm,sizeof(unsigned long),1,iindexfile[i].fileha);
                fread(&iindexfile[i].lastAntall,sizeof(unsigned long),1,iindexfile[i].fileha);

		iindexfile[i].nr = i;
		
	}
	*/


	gotEof = 0;
	//for (y=0;y<nrOffIindexFiles;y++) {
	while (nrOffIindexFiles != 0) {
		//hvis vi har fåt en en of file siden sist sorterer vi slik at den kommer nederst
		if (gotEof) {

			qsort(iindexfile, nrOffIindexFiles , sizeof(struct iindexfileFormat), compare_elements_eof);

			gotEof = 0;
			nrOffIindexFiles--;
		}

		//sorter de etter term
		qsort(iindexfile, nrOffIindexFiles , sizeof(struct iindexfileFormat), compare_elements);

		for (i=0;i<nrOffIindexFiles;i++) {
		
			printf("%i: t %lu : %lu eof %i\n",iindexfile[i].nr,iindexfile[i].lastTerm,iindexfile[i].lastAntall,iindexfile[i].eof);
			
		}

		//finner elementene som skal kopieres inn
		mergeTerm = iindexfile[0].lastTerm;
		i = 0;
		totaltAntall = 0;
		while (mergeTerm == iindexfile[i].lastTerm) {
			totaltAntall += iindexfile[i].lastAntall;
			iindexfile[i].Have = 1;
			i++;
		}

		printf("totaltAntall %lu for term %ul\n",totaltAntall,iindexfile[0].lastTerm);

		currentTerm = iindexfile[0].lastTerm;

		startAdress = (unsigned long)ftell(FinalIindexFileFA);
		fwrite(&iindexfile[0].lastTerm,sizeof(iindexfile[0].lastTerm),1,FinalIindexFileFA);
		fwrite(&totaltAntall,sizeof(totaltAntall),1,FinalIindexFileFA);

		

		i = 0;
		//while (mergeTerm == iindexfile[i].lastTerm) {
		//kj�rer gjenom alle filene og skriver ut de som har index
		for (i=0;i<nrOffIindexFiles;i++) {

			printf("trying %i, Have %i, term %lu\n",iindexfileArray[i].nr,iindexfileArray[i].Have,iindexfile[i].lastTerm);

			if (iindexfileArray[i].Have) {
			iindexfileArray[i].Have = 0;

			printf("aa: %i skriv %lu : %lu fra nr %i\n",i,iindexfile[i].lastTerm,iindexfile[i].lastAntall,iindexfile[i].nr);

			
			/////////////////////////////////////////////////

			for (x=0;x<iindexfile[i].lastAntall;x++) {
                        	//side hedder
				if ((n=fread(&DocID,sizeof(unsigned long),1,iindexfile[i].fileha)) == -1) {
                        		perror("cant read");
					exit(1);
                        	}
				DocIDcount++;

				if ((n=fread(&TermAntall,sizeof(unsigned long),1,iindexfile[i].fileha)) == -1) {
					perror("cant read");
					exit(1);
				}
			
				//printf("DocID %lu %lu: \n",DocID,TermAntall);

				//skriver til final index
				fwrite(&DocID,sizeof(unsigned long),1,FinalIindexFileFA);
				fwrite(&TermAntall,sizeof(unsigned long),1,FinalIindexFileFA);


                        	for (z = 0;z < TermAntall; z++) {
					if ((n=fread(&hit,sizeof(unsigned short),1,iindexfile[i].fileha)) == -1) {
						perror("cant read");
                                        	exit(1);
                                	}

					//skriver til final index
					fwrite(&hit,sizeof(unsigned short),1,FinalIindexFileFA);


                                        //printf("%i,",hit);

                        	}
                        	//printf("\n");
                	}


			////////////////////////////////////////////////
			

			//leser inn neste term
                        fread(&iindexfile[i].lastTerm,sizeof(unsigned long),1,iindexfile[i].fileha);
                        fread(&iindexfile[i].lastAntall,sizeof(unsigned long),1,iindexfile[i].fileha);

			if (feof(iindexfile[i].fileha)) {
				iindexfile[i].eof = 1;
				gotEof = 1;
			}
			
			//i++;
			}
		}

		stoppAdress = (unsigned long)ftell(FinalIindexFileFA);;
                DictionaryPost.WordID = currentTerm;
                DictionaryPost.Adress = startAdress;
                DictionaryPost.SizeForTerm = stoppAdress - startAdress;

                fwrite(&DictionaryPost,sizeof(DictionaryPost),1,FinalDictionaryFileFA);

		printf("WordID %ul, Adress: %ul, SizeForTerm: %ul\n",DictionaryPost.WordID,DictionaryPost.Adress,DictionaryPost.SizeForTerm);

		printf("\n");
	}

	fclose(FinalDictionaryFileFA);
	fclose(FinalIindexFileFA);

	for (i=2;i<nrOffIindexFiles;i++) {
		fclose(iindexfile[i].fileha);
	}

	printf("DocIDcount: %i\n",DocIDcount);

exit(1);
        if ((fileha = fopen(argv[1],"rb")) == NULL) {
                perror(argv[1]);
        }



	while (!feof(fileha)) {
		//wordid hedder
        	fread(&term,sizeof(unsigned long),1,fileha);
        	fread(&Antall,sizeof(unsigned long),1,fileha);

		printf("term %u antall: %u: \n",term,Antall);

		for (i=0;i<Antall;i++) {
			//side hedder
			fread(&DocID,sizeof(unsigned long),1,fileha);
        		fread(&TermAntall,sizeof(unsigned long),1,fileha);

			//printf("DocID %u %u: ",DocID,TermAntall);

			for (y = 0;y < TermAntall; y++) {
                		        fread(&hit,sizeof(unsigned short),1,fileha);

					//printf("%i,",hit);

			}
			//printf("\n");
		}	
		//printf("\n\n");
	}
	fclose(fileha);
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

