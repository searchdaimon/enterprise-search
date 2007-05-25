
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>


#include "define.h"
#include "lot.h"
#include "iindex.h"


struct DictionaryMemoryFormat {
	struct DictionaryFormat *Dictionary;
	int elements;
};

struct DictionaryMemoryFormat AthorDictionary[64];
struct DictionaryMemoryFormat MainDictionary[64];

int compare_DictionaryMemoryElements (const void *p1, const void *p2);

void IIndexInaliser() {
	int i;

	for (i=0;i<AntallBarrals;i++) {
		AthorDictionary[i].elements = 0;
		MainDictionary[i].elements = 0;
	}
}

//laster ordbken i minne
void IIndexLoad (char Type[], char lang[],char subname[]) {
	int i,y,x;
	char FilePath[255];
	char IndexPath[255];
	char IndexSprok[] = "aa";
	FILE *dictionaryha;
	struct stat inode;
	int max;
	struct DictionaryFormat DictionaryPost;
	int TermsForMemory, TotalTermsForMemory;

	printf("size : %i\n",sizeof(struct DictionaryFormat));

	TotalTermsForMemory = 0;

        for (i=0;i<AntallBarrals;i++) {
        	GetFilePathForIindex(FilePath,IndexPath,i,"Athor",IndexSprok,subname);
		//sprintf(IndexPath,"%s/iindex/Athor/dictionary/%s/%i.txt",FilePath,IndexSprok, i);

        	if ((dictionaryha = fopen(IndexPath,"rb")) == NULL) {
                	printf("Cant read Dictionary at %s\n",IndexPath);
        	        perror(IndexPath);
        	        //exit(1);
	        }
		else {
			fstat(fileno(dictionaryha),&inode);
	
			//printf("%s\n",IndexPath);
			max = inode.st_size / sizeof(DictionaryPost);

			//analyserer for å finne de vi skal lese inn
			TermsForMemory=0;
			for (y=0;y<max;y++) {
				fread(&DictionaryPost,sizeof(DictionaryPost),1,dictionaryha);

				if (DictionaryPost.SizeForTerm > mineAthorTermSizeForMemory) {
					//printf("%lu: %lu\n",DictionaryPost.WordID,DictionaryPost.SizeForTerm);
					++TermsForMemory;
				}
			}

			AthorDictionary[i].Dictionary = malloc(TermsForMemory * sizeof(DictionaryPost));

			AthorDictionary[i].elements = TermsForMemory;

			fseek(dictionaryha,0, SEEK_SET);
			//leser de vi skal ha inn i minne
			x=0;
			for (y=0;y<max;y++) {
                	        fread(&DictionaryPost,sizeof(DictionaryPost),1,dictionaryha);

                	        if (DictionaryPost.SizeForTerm > mineAthorTermSizeForMemory) {
					AthorDictionary[i].Dictionary[x] = DictionaryPost;
					++x;
					++TotalTermsForMemory;
                        	}
                	}

			printf("%i TermsForMemory %i\n",i,TermsForMemory);
			fclose(dictionaryha);




			qsort(AthorDictionary[i].Dictionary,AthorDictionary[i].elements,sizeof(struct DictionaryFormat),compare_DictionaryMemoryElements);


		}
	}

        for (i=0;i<AntallBarrals;i++) {
                GetFilePathForIindex(FilePath,IndexPath,i,"Main",IndexSprok,subname);
                //sprintf(IndexPath,"%s/iindex/Main/dictionary/%s/%i.txt",FilePath,IndexSprok, i);

                if ((dictionaryha = fopen(IndexPath,"rb")) == NULL) {
                        printf("Cant read Dictionary at %s\n",IndexPath);
                        perror(IndexPath);
                        //exit(1);
                }
                else {
                        fstat(fileno(dictionaryha),&inode);

                        //printf("%s\n",IndexPath);
                        max = inode.st_size / sizeof(DictionaryPost);

                        //analyserer for å finne de vi skal lese inn
                        TermsForMemory=0;
                        for (y=0;y<max;y++) {
                                fread(&DictionaryPost,sizeof(DictionaryPost),1,dictionaryha);

                                if (DictionaryPost.SizeForTerm > mineMainTermSizeForMemory) {
                                        //printf("%lu: %lu\n",DictionaryPost.WordID,DictionaryPost.SizeForTerm);
                                        ++TermsForMemory;
                                }
                        }

                        MainDictionary[i].Dictionary = malloc(TermsForMemory * sizeof(DictionaryPost));

                        MainDictionary[i].elements = TermsForMemory;

                        fseek(dictionaryha,0, SEEK_SET);
                        //leser de vi skal ha inn i minne
			x=0;
                        for (y=0;y<max;y++) {
                                fread(&DictionaryPost,sizeof(DictionaryPost),1,dictionaryha);

                                if (DictionaryPost.SizeForTerm > mineMainTermSizeForMemory) {
                                        MainDictionary[i].Dictionary[x] = DictionaryPost;
					++TotalTermsForMemory;
					++x;
                                }
                        }
                        printf("%i TermsForMemory %i\n",i,TermsForMemory);
                        fclose(dictionaryha);




                        qsort(MainDictionary[i].Dictionary,MainDictionary[i].elements,sizeof(struct DictionaryFormat),compare_DictionaryMemoryElements);

                }
        }

	printf("lodet %i element. Toatl of %f mb\n",TotalTermsForMemory,floor((TotalTermsForMemory * sizeof(DictionaryPost)) / 1000000));


	//exit(1);
}

int compare_DictionaryMemoryElements (const void *p1, const void *p2) {

        //int i1 = * (int *) p1;
        //int i2 = * (int *) p2;

        //return i1 - i2;
        struct DictionaryFormat *t1 = (struct DictionaryFormat*)p1;
        struct DictionaryFormat *t2 = (struct DictionaryFormat*)p2;

        if (t1->WordID > t2->WordID)
                return -1;
        else
                return t1->WordID < t2->WordID;

}

//ToDo: bruker ikke subname her enda. Må spesifisere det når vi loader
int ReadIIndexRecordFromMemeory (int *Adress, int *SizeForTerm, unsigned long Query_WordID,char *IndexType, char *IndexSprok,unsigned long WordIDcrc32,char subname[]) {

	int iindexfile;
	int i;
	struct DictionaryFormat *DictionaryPost;
	struct DictionaryFormat dummy;
	iindexfile = WordIDcrc32 % AntallBarrals;
	
	printf("ReadIIndexRecordFromMemeory: WordIDcrc32 %lu\n",WordIDcrc32);

	printf("FromMemeory iindexfile %i elements %i\n",iindexfile,AthorDictionary[iindexfile].elements);

	//for(i=0;i<AthorDictionary[iindexfile].elements;i++) {
        //        printf("%lu\n",AthorDictionary[iindexfile].Dictionary[i].WordID);
        //}

	dummy.WordID = WordIDcrc32;
	if (strcmp(IndexType,"Athor") == 0) {
		if ((DictionaryPost = bsearch(&dummy,AthorDictionary[iindexfile].Dictionary,AthorDictionary[iindexfile].elements,sizeof(struct DictionaryFormat),compare_DictionaryMemoryElements)) == NULL) {
			printf("fant ikke \n");

			return 0;
		}
		else {
			printf("funnet!\nmem term %lu Adress %lu, SizeForTerm %lu\n",(*DictionaryPost).WordID,(*DictionaryPost).Adress,(*DictionaryPost).SizeForTerm);

	                *Adress = (*DictionaryPost).Adress;
        	        *SizeForTerm = (*DictionaryPost).SizeForTerm;


			return 1;
		}
	}
	else if (strcmp(IndexType,"Main") == 0) {
		if ((DictionaryPost = bsearch(&dummy,MainDictionary[iindexfile].Dictionary,MainDictionary[iindexfile].elements,sizeof(struct DictionaryFormat),compare_DictionaryMemoryElements)) == NULL) {
			printf("fant ikke \n");

			return 0;
		}
		else {
			printf("funnet!\nmem term %lu Adress %lu, SizeForTerm %lu\n",(*DictionaryPost).WordID,(*DictionaryPost).Adress,(*DictionaryPost).SizeForTerm);

	                *Adress = (*DictionaryPost).Adress;
        	        *SizeForTerm = (*DictionaryPost).SizeForTerm;


			return 1;
		}

	}
	else {
		printf("Wrong IndexType \"%s\"\n",IndexType);
		return 0;
	}
}
/////////////////////////////////////////////////////////////////////
// Finner indeks adressen til en term, ved å binærsøke ordboken
/////////////////////////////////////////////////////////////////////
int ReadIIndexRecord (int *Adress, int *SizeForTerm, unsigned long Query_WordID,char *IndexType, char *IndexSprok,unsigned long WordIDcrc32,char subname[]) {


	FILE *dictionaryha;
	int max;
	int min;
	int fant;
	int halvert;
	int posisjon;
	int iindexfile;
	int count;

	char IndexPath[255];
	char FilePath[255];

	//int revindexFilNr;

	struct DictionaryFormat DictionaryPost;

	struct stat inode;	// lager en struktur for fstat å returnere.

	int DictionaryRecordSize;
	
	DictionaryRecordSize = sizeof(DictionaryPost);


	//DictionaryRecordSize = sizeof(DictionaryPost)
	//revindexFilNr = fmod(WordIDcrc32,AntallBarrals);
	//revindexFilNr = WordIDcrc32 % AntallBarrals;

	//printf("mod: %i WordIDcrc32 %lu AntallBarrals %i\n",revindexFilNr,WordIDcrc32,AntallBarrals);

	iindexfile = WordIDcrc32 % AntallBarrals;
	GetFilePathForIDictionary(FilePath,IndexPath,iindexfile,IndexType,IndexSprok,subname);

	printf("ReadIIndexRecord: From disk iindexfile %i WordIDcrc32 %lu\n",iindexfile,WordIDcrc32);

	//sprintf(IndexPath,"%s/iindex/%s/dictionary/%s/%i.txt",FilePath,IndexType,IndexSprok, iindexfile);

	//sprintf(IndexPath,"data/iindex/%s/dictionary/%s/%i.txt",IndexType,IndexSprok, WordIDcrc32 % AntallBarrals);

	#ifdef DEBUG
	printf("\ndictionary: %s\n",IndexPath);
	#endif

	if ((dictionaryha = fopen(IndexPath,"rb")) == NULL) {
		printf("Cant read Dictionary at %s\n",IndexPath);
		perror(IndexPath);
		//exit(1);

		//fik ikke til åopne filen. Setter slik at vi sier vi ikke fant noe
		//*Adress = -1;
                //*SizeForTerm = -1;
		return 0;
	}
	else {


		fstat(fileno(dictionaryha),&inode);

		printf("Stat: %i\n",inode.st_size);

		max = inode.st_size / DictionaryRecordSize;
		min = 0;

		printf("min %i, max %i\n",min,max);
	
		//debug: Viser alle forekomstene i indeksen
		/*
		printf("\n####################################################\n");
		printf("alle Dictionary forekomster:\n");
		int i;
		for (i=0;i<max;i++) {
			fread(&DictionaryPost,DictionaryRecordSize,1,dictionaryha);
			printf("did read %u\n",DictionaryPost.WordID);
		}
		
		printf("\n####################################################\n");
		*/
		//	

		//binersøker
		fant = 0;
		//to do. er >0 riktig her? Synes vi har fåt problem med forever looping før
		//while (((max - min) > 1) && (!fant)) { 
		count = 0;
		while (((max - min) > 0) && (!fant)) { 
			halvert = floor((((max - min) / 2) + min));
			//halvert = (int)((((max - min) / 2) + min) * 0.5);
			//halvert = (int)((((max - min) / 2)) * 0.5);


			posisjon = halvert * DictionaryRecordSize;

			printf("\tmax: %i, min: %i, halvert: %i, (max - min): %i\n",max,min,halvert,(max - min));
			//exit(1);
			fseek(dictionaryha,posisjon,0);
	
			fread(&DictionaryPost,DictionaryRecordSize,1,dictionaryha);

			printf("WordID: %lu = %lu ?\n",DictionaryPost.WordID,Query_WordID);
			if (Query_WordID == DictionaryPost.WordID) {
				fant = 1;
				//printf("Fant: WordID: %lu = %lu ?\n",DictionaryPost.WordID,Query_WordID);
			}
			else if (min == halvert) {
				//hvis vi ikke her mere halveringer igjen å gkøre. Litt usikker på 
				//hvorfår vi må ha dette, og den under. Men (max - min) blir aldri mindre en 1, hvis vi ikke finner
				//recorden, og vi evverlooper
				break;
			}
			else if (max == halvert) {
				break;
			}
			else if (Query_WordID > DictionaryPost.WordID) {
				min = halvert;
			}
			else if (Query_WordID < DictionaryPost.WordID) {
				max = halvert;
			}
			//temp:
			//if (count > 10) {
			//	exit(1);
			//}
			//v14 14 nov

			++count;
		}
		printf("line 365\n");
		//leser siste
		//toDo: hvorfor kommer ikke altid siste post med når vi halverer. Skyldes det bruk av floor() lengere opp?
		//leser manuelt får nå
		if (!fant) {
			posisjon = (halvert -1) * DictionaryRecordSize;
			fseek(dictionaryha,posisjon,0);
			fread(&DictionaryPost,DictionaryRecordSize,1,dictionaryha);
			if (Query_WordID == DictionaryPost.WordID) {
				fant = 1;
                	        printf("Fant: WordID: %lu = %lu ?\n",DictionaryPost.WordID,Query_WordID);			
			}
		}


		fclose(dictionaryha);

		if (fant) {
			*Adress = DictionaryPost.Adress;
			*SizeForTerm = DictionaryPost.SizeForTerm;
			printf("disk: Adress %lu, SizeForTerm %lu\n",DictionaryPost.Adress,DictionaryPost.SizeForTerm);
			return 1;
		}
		else {
			*Adress = -1;
			*SizeForTerm = -1;
			return 0;
		}
	}
	//printf("Adress %i,SizeForTerm %i\n",*Adress ,*SizeForTerm);

}

//copy a memory area, and return the size copyed
static inline size_t memcpyrc(void *s1, const void *s2, size_t n) {
//size_t memcpyrc(void *s1, const void *s2, size_t n) {
        memcpy(s1,s2,n);

        return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Returnerer en array med termner, forekomster og posisjoner for enkelt ord. 
//
//	Inn:
//	WordID
//	IndexSprok
//	IndexType
//	
//	Ut:
//	struct iindex
//	int antall //antal forekomster
/////////////////////////////////////////////////////////////////////////////////////////////////
void GetIndexAsArray (int *AntallTeff, struct iindexFormat *TeffArray, 
		unsigned long WordIDcrc32, char * IndexType, char *IndexSprok,
		struct subnamesFormat *subname, 
		int (*rank)(const unsigned short *,const int,const unsigned int DocID,struct subnamesFormat *subname, struct iindexFormat *TeffArray),
		int languageFilterNr, int languageFilterAsNr[] ) {



	FILE *fileha;
	char InnBuff[8]; //1040162
	int ReadOffsett = 0;
	char buff[512];
	unsigned long term;
	int Antall;
	int i,z;
	int y;
	int Element;



	int mergedn;
	char IndexPath[255];


	int Adress = 0;
	int SizeForTerm = 0;
	int iindexfile;
	char FilePath[255];

	//unsigned long DocID;
	//unsigned long TermAntall;
	unsigned short hit;


	int isv3 = 1;
/*
	//temp. FIkser at ikke alle indekser er v3
	if (strcmp(IndexType,"Main") == 0) {
		printf("vv: %s is v3\n",IndexType);
		isv3 = 1;
	}
	else if (strcmp(IndexType,"Url") == 0) {
		printf("vv: %s is v3\n",IndexType);
		isv3 = 1;
	}	
	else {
		printf("vv: %s is NOT v3\n",IndexType);
		isv3 = 0;
	}
*/
	printf("languageFilterNr: %i\n",languageFilterNr);
	//temp:
	//isv3 = 0;
	
	//5: printf("%s crc32: %lu\n",WordID,WordIDcrc32);

	//prøver førs å lese fra minne
	//temp:
	if ((!ReadIIndexRecordFromMemeory(&Adress, &SizeForTerm,WordIDcrc32,IndexType,IndexSprok,WordIDcrc32,(*subname).subname))
	&& (!ReadIIndexRecord(&Adress, &SizeForTerm,WordIDcrc32,IndexType,IndexSprok,WordIDcrc32,(*subname).subname))
	) {
		//*AntallTeff = 0;
	}
	else {


		//sprintf(IndexPath,"data/iindex/%s/index/%s/%i.txt",IndexType,IndexSprok, WordIDcrc32 % AntallBarrals);
		iindexfile = WordIDcrc32 % AntallBarrals;
		GetFilePathForIindex(FilePath,IndexPath,iindexfile,IndexType,IndexSprok,(*subname).subname);
		//sprintf(IndexPath,"%s/iindex/%s/index/%s/%i.txt",FilePath,IndexType,IndexSprok, iindexfile);

		printf("Åpner index %s\n",IndexPath);

		//fileha = fopen("data/iindex/Main/index/ENG/17.txt","rb");
		//if ((fileha = fopen("data/iindex/Main/index/aa/17.txt","rb")) == NULL) {
		if ((fileha = fopen(IndexPath,"rb")) == NULL) {
			perror(IndexPath);
		}
		else {

		fseek(fileha,Adress,0);



		fread(&term,sizeof(unsigned long),1,fileha);
		fread(&Antall,sizeof(unsigned long),1,fileha);


		#ifdef TIME_DEBUG
		gettimeofday(&end_time, NULL);
		printf("Time debug: read tern name and number %f\n",getTimeDifference(&start_time,&end_time));
		#endif

		#ifdef TIME_DEBUG
		gettimeofday(&start_time, NULL);
		#endif
			SizeForTerm -= (sizeof(term) + sizeof(Antall));
			Adress -= (sizeof(term) + sizeof(Antall));
			printf("size %i\n",SizeForTerm);
			char *allindex;

			if ((allindex = malloc(SizeForTerm)) == NULL) {
				perror("malloc");
				(*AntallTeff) = 0;
				return;
			}
			char *allindexp = allindex;
			fread(allindex,sizeof(char),SizeForTerm,fileha);

			//fseek(fileha,Adress,0);
		#ifdef TIME_DEBUG
		gettimeofday(&end_time, NULL);
		printf("Time debug: read all at ones %f\n",getTimeDifference(&start_time,&end_time));
		#endif


		//y=0;
		//for (i = 0; ((i < Antall) && (i < maxIndexElements)); i++) {
		y=(*AntallTeff);

		#ifdef TIME_DEBUG
		gettimeofday(&start_time, NULL);
		#endif

		void *allip;
		allip = allindex;

		for (i = 0; ((i < Antall) && (y < maxIndexElements)); i++) {


			TeffArray[y].phraseMatch = 0;

			//v3
			if (isv3) {
				
				allindex += memcpyrc(&TeffArray[y].DocID,allindex,sizeof(unsigned long));

				// v3 har langnr
				allindex += memcpyrc(&TeffArray[y].langnr,allindex,sizeof(char));

				allindex += memcpyrc(&TeffArray[y].TermAntall,allindex,sizeof(unsigned long));
				
			}
			else {
				allindex += memcpyrc(&TeffArray[y].DocID,allindex,sizeof(unsigned long));

				allindex += memcpyrc(&TeffArray[y].TermAntall,allindex,sizeof(unsigned long));

			}

			//printf("DociD %u, TermAntall %u\n",TeffArray[y].DocID,TeffArray[y].TermAntall);		
			if (TeffArray[y].TermAntall > MaxsHitsInIndex) {

				//allindex += memcpyrc( &TeffArray[y].hits,allindex,(sizeof(unsigned short) * TeffArray[y].TermAntall) );
				//Leser først maksimum vi kan ha, så flytter vi peggeren over de andre
				allindex += memcpyrc( &TeffArray[y].hits,allindex,(sizeof(unsigned short) * MaxsHitsInIndex) );
				allindex += ( sizeof(unsigned short) * (TeffArray[y].TermAntall - MaxsHitsInIndex) );

				//kan våre vi får for mange hits i athor, da vi ikke eher overholt noen grense. 
				//burde heller lagre en verdi på hvor mange hits vi har eller noe

		  		//printf("error. TermAntall to large at %i (max %i)\n",TeffArray[y].TermAntall,MaxsHitsInIndex);
				//printf("(TeffArray[y].TermAntall - maxIndexElements) %i, maxIndexElements %i\n",(TeffArray[y].TermAntall - maxIndexElements),maxIndexElements);
				//ToDo: denne blir hvis aldri kjørt, må undersøke det
				/*
				for (z = 0;(z < (TeffArray[y].TermAntall - maxIndexElements)) && (z < maxIndexElements); y++) {
					//allindex += memcpyrc(&hit,allindex,sizeof(unsigned short));
					allindex += sizeof(unsigned short);
					printf("z %i\n",z);
				}
				*/
				//allindex += ( sizeof(unsigned short) * (TeffArray[y].TermAntall - MaxsHitsInIndex) );
			}
			else {
				//fread(&TeffArray[y].hits,TeffArray[y].TermAntall,sizeof(unsigned short),fileha);
				allindex += memcpyrc(&TeffArray[y].hits,allindex,(sizeof(unsigned short) * TeffArray[y].TermAntall));				
				
			}

			TeffArray[y].subname = subname;

			#ifdef BLACK_BOKS
				//legger til en peker til subname
				TeffArray[y].deleted = 0;
				TeffArray[y].indexFiltered.filename = 0;
				TeffArray[y].indexFiltered.date = 0;
			#endif

                        TeffArray[y].TermRank = rank(TeffArray[y].hits,TeffArray[y].TermAntall,TeffArray[y].DocID,subname,&TeffArray[y]);
			//printf("TermRank: %i, subname \"%s\", DocID %u\n",TeffArray[y].TermRank,(*TeffArray[y].subname).subname,TeffArray[y].DocID);

			#ifndef BLACK_BOKS
				//midlertidig bug fiks. Ignorerer hit med DocID 0.
				//ser ut til at vi har noen bugger som lager DocID 0. Skal ikke være med i index
				if (TeffArray[y].DocID == 0) {
					continue;
				}
			#endif



			
			//int languageFilterNr, int languageFilterAsNr
			if (languageFilterNr == 0) {
				++y;
			}
			else if (languageFilterNr == 1) {
				if (languageFilterAsNr[0] == TeffArray[y].langnr) {
					printf("filter hit\n");
					++y;
				}
			}
			else {
				//søker os gjenom språkene vi skal filtrerte på
				//int h;
				//for(h=0;h<(languageFilterNr -1);h++) {
				//	languageFilterAsNr[h] = TeffArray[y].langnr
				//}
			}

		}


		free(allindexp);

/*
		//y=0;
		//for (i = 0; ((i < Antall) && (i < maxIndexElements)); i++) {
		y=(*AntallTeff);
		for (i = 0; ((i < Antall) && (y < maxIndexElements)); i++) {

			fread(&TeffArray[y].DocID,sizeof(unsigned long),1,fileha);


			//v3
			if (isv3) {
				fread(&TeffArray[y].langnr,sizeof(char),1,fileha);
			}
			//printf("lang %i\n",(int)TeffArray[y].langnr);
       			fread(&TeffArray[y].TermAntall,sizeof(unsigned long),1,fileha);


		
			if (TeffArray[y].TermAntall > MaxsHitsInIndex) {

				fread(&TeffArray[y].hits,TeffArray[y].TermAntall,sizeof(unsigned short),fileha);

				//kan våre vi får for mange hits i athor, da vi ikke eher overholt noen grense. 
				//burde heller lagre en verdi på hvor mange hits vi har eller noe

		  		//printf("error. TermAntall to large at %i\n",TeffArray[y].TermAntall);
				for (z = 0;(z < (TeffArray[y].TermAntall - maxIndexElements)) && (z < maxIndexElements); y++) {
					fread(&hit,sizeof(unsigned short),1,fileha);
				}
			}
			else {
				fread(&TeffArray[y].hits,TeffArray[y].TermAntall,sizeof(unsigned short),fileha);

			}

			//
               	        //lagger til poeng
                        //
                        TeffArray[y].TermRank = rank(TeffArray[y].hits,TeffArray[y].TermAntall);
			printf("TermRank: %i\n",TeffArray[y].TermRank);
                        //

			
			//if (TeffArray[y].DocID == 5630541) {
			//	printf("Ja, vi har 5630541, med %i - %i, rank %i\n",(int)TeffArray[y].TermAntall,MaxsHitsInIndex,(int)TeffArray[y].TermRank);
			//	
			//}
			

			//legger til en peker til subname
			TeffArray[y].subname = subname;

			//printf("languageFilterNr: %i, languageFilterAsNr[0]: %i, TeffArray[y].langnr: %i\n",languageFilterNr,languageFilterAsNr[0],TeffArray[y].langnr);
			
			//int languageFilterNr, int languageFilterAsNr
			if (languageFilterNr == 0) {
				++y;
			}
			else if (languageFilterNr == 1) {
				if (languageFilterAsNr[0] == TeffArray[y].langnr) {
					printf("filter hit\n");
					++y;
				}
			}
			else {
				//søker os gjenom språkene vi skal filtrerte på
				//int h;
				//for(h=0;h<(languageFilterNr -1);h++) {
				//	languageFilterAsNr[h] = TeffArray[y].langnr
				//}
			}

		}

*/				

		printf("jj: i=%i,y=%i\n",i,y);
		*AntallTeff = y;
		fclose(fileha);

	} // fopen
	}

	printf("GetIndexAsArray: AntallTeff = %i\n",(*AntallTeff));

}


void GetNForTerm(unsigned long WordIDcrc32, char *IndexType, char *IndexSprok, int *TotaltTreff, char subname[]) {

		int Adress;
        	int SizeForTerm;
		int iindexfile;
		char IndexPath[255],IndexFile[255];		    
		unsigned long term;
		unsigned long Antall;
		FILE *fileha;		    
		//////////////////////////////////////////////
		ReadIIndexRecord(&Adress, &SizeForTerm,WordIDcrc32,"Main","aa",WordIDcrc32,subname);
				    
				    
		iindexfile = WordIDcrc32 % AntallBarrals;
		GetFilePathForIindex(IndexPath,IndexFile,iindexfile,IndexType,IndexSprok,subname);
		sprintf(IndexPath,"%s/iindex/%s/index/%s/%i.txt",IndexPath,IndexType,IndexSprok, iindexfile);
		    
										    
	        if ((fileha = fopen(IndexFile,"rb")) == NULL) {
	    	    perror(IndexPath);
	        }
		fseek(fileha,Adress,0);
		
		fread(&term,sizeof(unsigned long),1,fileha);
		fread(&Antall,sizeof(unsigned long),1,fileha);
																						
										
		
		*TotaltTreff = 	(int)Antall;												
		//////////////////////////////////////////////
																									
}



