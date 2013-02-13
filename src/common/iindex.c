
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <inttypes.h>

#include "define.h"
#include "lot.h"
#include "iindex.h"
#include "mgsort.h"
#include "revindex.h"
#include "bstr.h"
#include "atomicallyio.h"

#include "../3pLibs/keyValueHash/hashtable.h"

#define MaxRevIndexArraySize 2000000


#define MMAP_IINDEX

#ifdef MMAP_IINDEX
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#endif

#ifdef TIME_DEBUG
	#include "timediff.h"
#endif

struct DictionaryMemoryFormat {
	struct DictionaryFormat *Dictionary;
	int elements;
};

struct revIndexArrayFomat {
	unsigned int DocID;
        unsigned long WordID;
	unsigned char langnr;
        unsigned long nrOfHits;
        unsigned short hits[MaxsHitsInIndex];
	char tombstone;
};

struct DictionaryMemoryFormat AthorDictionary[64];
struct DictionaryMemoryFormat MainDictionary[64];

struct hashtable * loadGced(int lotNr, char subname[]);
int Indekser_compare_elements (const void *p1, const void *p2);
int compare_DictionaryMemoryElements (const void *p1, const void *p2);
int mergei_compare_elements (const void *p1, const void *p2);
int mergei_compare_elements_eof (const void *p1, const void *p2);
int mergei_compare_filenr (const void *p1, const void *p2);


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
                	printf("Cant read Dictionary for %s at %s:%d\n",IndexPath,__FILE__,__LINE__);
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
					//printf("%u: %u\n",DictionaryPost.WordID,DictionaryPost.SizeForTerm);
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
                        printf("Cant read Dictionary for %s at %s:%d\n",IndexPath,__FILE__,__LINE__);
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
                                        //printf("%u: %u\n",DictionaryPost.WordID,DictionaryPost.SizeForTerm);
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
int ReadIIndexRecordFromMemeory (unsigned int *Adress, unsigned int *SizeForTerm, unsigned int Query_WordID,char *IndexType, char *IndexSprok,unsigned int WordIDcrc32,char subname[], void *(filemap)(char *, size_t *)) {

	int iindexfile;
	int i;
	struct DictionaryFormat *DictionaryPost;
	struct DictionaryFormat dummy;
	iindexfile = WordIDcrc32 % AntallBarrals;
	char path[2048], name[2048];
	size_t dictsize;
	void *ptr;
	
	#ifdef DEBUG
	printf("ReadIIndexRecordFromMemeory: WordIDcrc32 %u\n",WordIDcrc32);

	printf("FromMemeory iindexfile %i elements %i\n",iindexfile,AthorDictionary[iindexfile].elements);
	#endif

	//for(i=0;i<AthorDictionary[iindexfile].elements;i++) {
        //        printf("%u\n",AthorDictionary[iindexfile].Dictionary[i].WordID);
        //}
	//
	//
	//
	
	//printf("With filemap: %p %s\n", filemap, IndexType);
	
	if (filemap == NULL)
		return 0;
	
	iindexfile = WordIDcrc32 % AntallBarrals;
	GetFilePathForIDictionary(path,name,iindexfile,IndexType,IndexSprok,subname);
	ptr = filemap(name, &dictsize);
	if (ptr == MAP_FAILED) {
		//printf("Empty file... No match no matter what.\n");
		return 1;
	}

	dummy.WordID = WordIDcrc32;
	if (ptr != NULL) {
		if ((DictionaryPost = bsearch(&dummy,ptr,dictsize/sizeof(dummy),sizeof(dummy),compare_DictionaryMemoryElements)) == NULL) {
#ifdef DEBUG
			printf("fant ikke \n");
#endif
			return 0;
		}
		else {
			printf("funnet!\nmem term %u Adress %u, SizeForTerm %u\n",(*DictionaryPost).WordID,(*DictionaryPost).Adress,(*DictionaryPost).SizeForTerm);

			*Adress = (*DictionaryPost).Adress;
			*SizeForTerm = (*DictionaryPost).SizeForTerm;

			return 1;
		}

	}
	else if (strcmp(IndexType,"Athor") == 0) {
		if ((DictionaryPost = bsearch(&dummy,AthorDictionary[iindexfile].Dictionary,AthorDictionary[iindexfile].elements,sizeof(struct DictionaryFormat),compare_DictionaryMemoryElements)) == NULL) {
			#ifdef DEBUG
			printf("fant ikke \n");
			#endif
			return 0;
		}
		else {
			printf("funnet!\nmem term %u Adress %u, SizeForTerm %u\n",(*DictionaryPost).WordID,(*DictionaryPost).Adress,(*DictionaryPost).SizeForTerm);

	                *Adress = (*DictionaryPost).Adress;
        	        *SizeForTerm = (*DictionaryPost).SizeForTerm;


			return 1;
		}
	}
	else {
		#ifdef DEBUG
		printf("ReadIIndexRecordFromMemeory: Wrong IndexType \"%s\"\n",IndexType);
		#endif
		return 0;
	}
}
/////////////////////////////////////////////////////////////////////
// Finner indeks adressen til en term, ved å binærsøke ordboken
/////////////////////////////////////////////////////////////////////
int ReadIIndexRecord (unsigned int *Adress, unsigned int *SizeForTerm, unsigned int Query_WordID,char *IndexType, char *IndexSprok,unsigned int WordIDcrc32,char subname[]) {


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

	//int DictionaryRecordSize;
	
	//DictionaryRecordSize = sizeof(DictionaryPost);

	#ifdef TIME_DEBUG
                struct timeval start_time, end_time;
	#endif

        #ifdef TIME_DEBUG
                gettimeofday(&start_time, NULL);
        #endif

	//DictionaryRecordSize = sizeof(DictionaryPost)
	//revindexFilNr = fmod(WordIDcrc32,AntallBarrals);
	//revindexFilNr = WordIDcrc32 % AntallBarrals;

	//printf("mod: %i WordIDcrc32 %u AntallBarrals %i\n",revindexFilNr,WordIDcrc32,AntallBarrals);

	iindexfile = WordIDcrc32 % AntallBarrals;
	GetFilePathForIDictionary(FilePath,IndexPath,iindexfile,IndexType,IndexSprok,subname);

	#ifdef DEBUG
	printf("ReadIIndexRecord: From disk iindexfile %i WordIDcrc32 %u\n",iindexfile,WordIDcrc32);
	#endif
	//sprintf(IndexPath,"%s/iindex/%s/dictionary/%s/%i.txt",FilePath,IndexType,IndexSprok, iindexfile);

	//sprintf(IndexPath,"data/iindex/%s/dictionary/%s/%i.txt",IndexType,IndexSprok, WordIDcrc32 % AntallBarrals);

	#ifdef DEBUG
	printf("\ndictionary: %s\n",IndexPath);
	#endif

	if ((dictionaryha = fopen(IndexPath,"rb")) == NULL) {
		#ifdef DEBUG
		//viser ikke denne da vi ofte har subname som ikke er på den serveren med i queryet
		printf("Can't read Dictionary for index %i, path \"%s\" at %s:%d\n",iindexfile,IndexPath,__FILE__,__LINE__);
		perror(IndexPath);
		#endif
		return 0;
	}


	fstat(fileno(dictionaryha),&inode);

	#ifdef DEBUG
	printf("Stat: %u / %i\n",(unsigned int)inode.st_size,sizeof(struct DictionaryFormat));
	#endif

	if (inode.st_size == 0) {
		*Adress = 0;
		*SizeForTerm = 0;
		fclose(dictionaryha);
		return 0;
	}

	min = 0;
	max = inode.st_size / sizeof(struct DictionaryFormat);

	#ifdef DEBUG
	printf("min %i, max %i\n",min,max);
	#endif

	//debug: Viser alle forekomstene i indeksen
	/*
	printf("\n####################################################\n");
	printf("alle Dictionary forekomster:\n");
	int i;
	for (i=0;i<max;i++) {
		fread(&DictionaryPost,sizeof(struct DictionaryFormat),1,dictionaryha);
		printf("did read %u\n",DictionaryPost.WordID);
	}
		
	printf("\n####################################################\n");
	*/
	//	

	//binersøker
	fant = 0;
	halvert = 0;
	//to do. er >0 riktig her? Synes vi har fåt problem med forever looping før
	//while (((max - min) > 1) && (!fant)) { 
	count = 0;
	while (((max - min) > 0) && (!fant)) { 
		halvert = floor((((max - min) / 2) + min));
		//halvert = (int)((((max - min) / 2) + min) * 0.5);
		//halvert = (int)((((max - min) / 2)) * 0.5);

		posisjon = halvert * sizeof(struct DictionaryFormat);

		#ifdef DEBUG
		printf("\tmax: %i, min: %i, halvert: %i, (max - min): %i. posisjon %i\n",max,min,halvert,(max - min),posisjon);
		#endif
		//exit(1);
		if (fseek(dictionaryha,posisjon,0) != 0) {
			printf("can't seek to post\n");
			break;
		}
	
		if (fread(&DictionaryPost,sizeof(struct DictionaryFormat),1,dictionaryha) != 1) {
			printf("can't read post\n");
			break;
		}
		#ifdef DEBUG
		printf("WordID: %u = %u ?\n",DictionaryPost.WordID,Query_WordID);
		#endif
		if (Query_WordID == DictionaryPost.WordID) {
			fant = 1;
			//printf("Fant: WordID: %u = %u ?\n",DictionaryPost.WordID,Query_WordID);
		}
		else if (min == halvert) {
			//hvis vi ikke her mere halveringer igjen å kjøre. Litt usikker på 
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

	//leser siste
	//toDo: hvorfor kommer ikke altid siste post med når vi halverer. Skyldes det bruk av floor() lengere opp?
	//leser manuelt får nå
	if (!fant) {
		posisjon = (halvert -1) * sizeof(struct DictionaryFormat);
		fseek(dictionaryha,posisjon,0);
		if (fread(&DictionaryPost,sizeof(struct DictionaryFormat),1,dictionaryha) != 1) {
			#ifdef DEBUG
			printf("can't read last post\n");
			#endif
		}
		else {
			if (Query_WordID == DictionaryPost.WordID) {
				fant = 1;
               		        printf("Fant: WordID: %u = %u ?\n",DictionaryPost.WordID,Query_WordID);			
			}
		}
	}


	fclose(dictionaryha);

        #ifdef TIME_DEBUG
               gettimeofday(&end_time, NULL);
                printf("Time debug: ReadIIndexRecord total time %f\n",getTimeDifference(&start_time,&end_time));
        #endif
	if (!fant) {
		*Adress = 0;
		*SizeForTerm = 0;
		return 0;

	}
	else if (DictionaryPost.SizeForTerm == 0) {
		printf("###################################\nBug: DictionaryPost SizeForTerm is 0!\nWordIDcrc32: %u\nIndexPath: \"%s\"\n###################################\n",WordIDcrc32,IndexPath);
		*Adress = 0;
		*SizeForTerm = 0;
		return 0;

	}
	else {
		*Adress = DictionaryPost.Adress;
		*SizeForTerm = DictionaryPost.SizeForTerm;
		#ifdef DEBUG
		printf("disk: Adress %u, SizeForTerm %u\n",DictionaryPost.Adress,DictionaryPost.SizeForTerm);
		#endif
		return 1;
	}

	//printf("Adress %u,SizeForTerm %i\n",*Adress ,*SizeForTerm);

}

#define DEBUG_MEMCPYRC

//copy a memory area, and return the size copyed
#ifdef DEBUG_MEMCPYRC
static size_t memcpyrc(void *s1, const void *s2, size_t n) {
#else
static inline size_t memcpyrc(void *s1, const void *s2, size_t n) {
#endif
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
void _GetIndexAsArray (int *AntallTeff, struct iindexFormat *TeffArray, 
		unsigned int WordIDcrc32, char * IndexType, char *IndexSprok,
		struct subnamesFormat *subname, 
		int languageFilterNr, int languageFilterAsNr[], void *(filemap)(char *, size_t *) ) {

	#ifdef TIME_DEBUG
		struct timeval start_time, end_time;
		struct timeval start_time_total, end_time_total;
		gettimeofday(&start_time_total, NULL);
	#endif

	FILE *fileha;

	unsigned int term;
	int Antall;
	int i,z;
	int y;

	off_t mmap_size;

	char IndexPath[255];


	unsigned int Adress = 0;
	unsigned int SizeForTerm = 0;
	int iindexfile;
	char FilePath[255];

	unsigned short hit;
	//void *allip;
	char *allindex;
	char *allindexp;
	void *filemapptr;



	int isv3 = 1;

	#ifdef HAVE_IS3_BUG

	//temp. FIkser at ikke alle indekser er v3
	if (strcmp(IndexType,"Main") == 0) {
		printf("vv: %s is v3\n",IndexType);
		isv3 = 0;
	}
	else if (strcmp(IndexType,"Url") == 0) {
		printf("vv: %s is v3\n",IndexType);
		isv3 = 0;
	}	
	else {
		printf("vv: %s is NOT v3\n",IndexType);
		isv3 = 0;
	}
	#endif

	#ifdef EXPLAIN_RANK
		printf("_GetIndexAsArray( AntallTeff: %i, IndexType: \"%s\", IndexSprok:\"%s\", subname: \"%s\", languageFilterNr: %i )\n",*AntallTeff,IndexType,IndexSprok,subname,languageFilterNr);
	#endif

	//temp:
	//isv3 = 0;
	
	//5: printf("%s crc32: %u\n",WordID,WordIDcrc32);

	//setter denne til 0, slik at hvis i ikke fr til å opne filen, eller hente ordboken er den 0
	//runarb: 19 aug 2007: gjør om slik at vi kan ha elementer fra før i indeksen
	//tror vi ikke kan nulle den ut da
	//må sjekkes opp
	//ser ut til at dette er antall treff i indexsen, inklysive de vi har fra før, ikke bare nye treff for denne index
	//(*AntallTeff) = 0;

	//prøver førs å lese fra minne
	//temp:
	if ((!ReadIIndexRecordFromMemeory(&Adress, &SizeForTerm,WordIDcrc32,IndexType,IndexSprok,WordIDcrc32,(*subname).subname,filemap))
	&& (!ReadIIndexRecord(&Adress, &SizeForTerm,WordIDcrc32,IndexType,IndexSprok,WordIDcrc32,(*subname).subname))
	) {

	}
	else {


		//sprintf(IndexPath,"data/iindex/%s/index/%s/%i.txt",IndexType,IndexSprok, WordIDcrc32 % AntallBarrals);
		iindexfile = WordIDcrc32 % AntallBarrals;
		GetFilePathForIindex(FilePath,IndexPath,iindexfile,IndexType,IndexSprok,(*subname).subname);
#ifdef MMAP_IINDEX
		if (filemap == NULL) {
			filemapptr = NULL;
		} else {
			filemapptr = filemap(IndexPath, NULL);
			if (filemapptr == MAP_FAILED) {
				#ifdef DEBUG
					fprintf(stderr,"IndexPath: \"%s\"\n",IndexPath);
					warnx("Empty file");
				#endif
				return;
			}
		}
#endif
		//sprintf(IndexPath,"%s/iindex/%s/index/%s/%i.txt",FilePath,IndexType,IndexSprok, iindexfile);

		#ifdef EXPLAIN_RANK
			printf("Åpner index %s\n",IndexPath);
			printf("size %u\n",SizeForTerm);
		#endif

		#ifdef TIME_DEBUG
			gettimeofday(&start_time, NULL);
		#endif

		if (
#ifdef MMAP_IINDEX
			!filemapptr && 
#endif
			(fileha = fopen(IndexPath,"rb")) == NULL) {
			perror(IndexPath);
		}
		else {

			#ifdef TIME_DEBUG
				gettimeofday(&end_time, NULL);
				printf("Time debug: open %f\n",getTimeDifference(&start_time,&end_time));
			#endif



		#ifdef MMAP_IINDEX


				if (filemapptr) {
					allindex = allindexp = filemapptr+Adress;
					printf("### Using cached indexpath: %s %p\n", IndexPath, filemapptr);
				} else {
		
					//lseek(filed,Adress,0);
					off_t mmap_offset;

					mmap_offset = Adress % getpagesize();

					#ifdef DEBUG
					printf("Adress %i, page size %i, mmap_offset %i\n",Adress,getpagesize(),mmap_offset);
					#endif

					//Adress = 0;
					//mmap_offset = 0;

					mmap_size = SizeForTerm + mmap_offset;
					if ((allindex = mmap(0,mmap_size,PROT_READ,MAP_SHARED,fileno(fileha),Adress - mmap_offset) ) == MAP_FAILED) {
						fprintf(stderr,"can't mmap file \"%s\", Adress: %u, SizeForTerm: %u\n",IndexPath,Adress,SizeForTerm);
						perror("mmap");
						return;
					}

					#ifdef DEBUG
					printf("mmap respons: %i\n",(int)allindex);
					#endif

					allindexp = allindex;

					allindex += mmap_offset;
				}

		#else


				fseek(fileha,Adress,0);

				//forhindrer at vi leser inn en for stor index, og bruker mye tid på det. Kan dog skje at vi buffer owerflover her
				maxIIindexSize = maxIndexElements * 209;

				if (SizeForTerm > maxIIindexSize) { // DocID 4b +  langnr 1b + TermAntall 4b + (100 hit * 2b)
					#ifdef DEBUG
					printf("size it to large. Will only read first %i bytes\n",maxIIindexSize);
					#endif
					SizeForTerm = maxIIindexSize;
				}



				if ((allindex = malloc(SizeForTerm)) == NULL) {
					perror("malloc");
					(*AntallTeff) = 0;
					return;
				}
				#ifdef TIME_DEBUG
				gettimeofday(&start_time, NULL);
				#endif

				fread(allindex,sizeof(char),SizeForTerm,fileha);

				allindexp = allindex;

				#ifdef TIME_DEBUG
					gettimeofday(&end_time, NULL);
					printf("Time debug: read all at ones %f\n",getTimeDifference(&start_time,&end_time));
				#endif
		#endif





		#ifdef TIME_DEBUG
		gettimeofday(&start_time, NULL);
		#endif

		y=(*AntallTeff);

		allindex += memcpyrc(&term,allindex,sizeof(unsigned int));
		allindex += memcpyrc(&Antall,allindex,sizeof(unsigned int));

		#ifdef EXPLAIN_RANK
			printf("term: %u, Antall: %i, nrofHits %i\n",term,Antall,TeffArray->nrofHits);
		#endif

		//fører til overskrivning av hits. Må inaliseres før vi begynner å lese fra index
		//TeffArray->nrofHits = 0;

		for (i = 0; ((i < Antall) && (y < maxIndexElements)); i++) {


			TeffArray->iindex[y].phraseMatch = 0;

			//v3
			if (isv3) {
				
				allindex += memcpyrc(&TeffArray->iindex[y].DocID,allindex,sizeof(unsigned int));

				// v3 har langnr
				allindex += memcpyrc(&TeffArray->iindex[y].langnr,allindex,sizeof(char));

				allindex += memcpyrc(&TeffArray->iindex[y].TermAntall,allindex,sizeof(unsigned int));
				
			}
			else {
				allindex += memcpyrc(&TeffArray->iindex[y].DocID,allindex,sizeof(unsigned int));

				allindex += memcpyrc(&TeffArray->iindex[y].TermAntall,allindex,sizeof(unsigned int));

			}

			if (TeffArray->iindex[y].TermAntall < 0) {
				printf("Bug: TermAntall is below 0! Was %i\n",TeffArray->iindex[y].TermAntall);
				exit(-1);
			}


			if (TeffArray->iindex[y].TermAntall != 0) {

				//slutter hvis vi har tat for mange hits
				if ((TeffArray->nrofHits + TeffArray->iindex[y].TermAntall) > maxTotalIindexHits) {
					printf("Har max hits. Har nå %i, TermAntall: %i, maxTotalIindexHits: %i\n",TeffArray->nrofHits,TeffArray->iindex[y].TermAntall,maxTotalIindexHits);
					TeffArray->iindex[y].TermAntall = 0;
					break;
				}

				TeffArray->iindex[y].hits = &TeffArray->hits[TeffArray->nrofHits];

				//vi må koppiere de inn 1 og 1 slik at posisjon blir riktig, og phrase blir 0
				for (z=0;z<TeffArray->iindex[y].TermAntall;z++) {
					allindex += memcpyrc( &TeffArray->hits[TeffArray->nrofHits].pos,allindex,sizeof(unsigned short) );
					TeffArray->hits[TeffArray->nrofHits].phrase = 0;

					++TeffArray->nrofHits;
				}


				#ifdef EXPLAIN_RANK
					printf("inserting into place %i DocID %u nrofHits in this array: %i, TermAntall: %i, maxTotalIindexHits %i, p %u. Hits: ",y,TeffArray->iindex[y].DocID,TeffArray->nrofHits,TeffArray->iindex[y].TermAntall,maxTotalIindexHits,(unsigned int )TeffArray->iindex[y].hits);			
					for (z=0;z<TeffArray->iindex[y].TermAntall;z++) {
						printf("%hu ",TeffArray->iindex[y].hits[z].pos);
					}
					printf("\n");
				#endif


			}

			TeffArray->iindex[y].subname = subname;

			#ifdef BLACK_BOKS
				//legger til en peker til subname
				TeffArray->iindex[y].deleted = 0;
				TeffArray->iindex[y].indexFiltered.filename = 0;
				TeffArray->iindex[y].indexFiltered.date = 0;
				TeffArray->iindex[y].indexFiltered.subname = 0;
			#endif

			//seter disse til 0 da vi ikke altid kjører body søk, og dermed ikke inaliserer disse.
			//dog noe uefektift og altid sette disse til 0, tar tid
			#ifdef EXPLAIN_RANK
                		TeffArray->iindex[y].rank_explaind.rankBody = 0;
                		TeffArray->iindex[y].rank_explaind.rankHeadline = 0;
                		TeffArray->iindex[y].rank_explaind.rankTittel = 0;
                		TeffArray->iindex[y].rank_explaind.rankAthor = 0;

                		TeffArray->iindex[y].rank_explaind.rankUrl_mainbody = 0;
                		TeffArray->iindex[y].rank_explaind.rankUrlDomain = 0;
                		TeffArray->iindex[y].rank_explaind.rankUrlSub = 0;

                		TeffArray->iindex[y].rank_explaind.nrAthorPhrase = 0;
                		TeffArray->iindex[y].rank_explaind.nrAthor = 0;

                		TeffArray->iindex[y].rank_explaind.nrBody = 0;
                		TeffArray->iindex[y].rank_explaind.nrHeadline = 0;
                		TeffArray->iindex[y].rank_explaind.nrTittel = 0;
                		TeffArray->iindex[y].rank_explaind.nrUrl_mainbody = 0;
                		TeffArray->iindex[y].rank_explaind.nrUrlDomain = 0;
                		TeffArray->iindex[y].rank_explaind.nrUrlSub = 0;

                		TeffArray->iindex[y].rank_explaind.maxBody = 0;
                		TeffArray->iindex[y].rank_explaind.maxHeadline = 0;
                		TeffArray->iindex[y].rank_explaind.maxTittel = 0;
                		TeffArray->iindex[y].rank_explaind.maxAthor = 0;
                		TeffArray->iindex[y].rank_explaind.maxUrl_mainbody = 0;
                		TeffArray->iindex[y].rank_explaind.maxUrlDomain = 0;
                		TeffArray->iindex[y].rank_explaind.maxUrlSub = 0;

        		#endif



			#ifndef BLACK_BOKS
				//midlertidig bug fiks. Ignorerer hit med DocID 0.
				//ser ut til at vi har noen bugger som lager DocID 0. Skal ikke være med i index
				if (TeffArray->iindex[y].DocID == 0) {
					#ifdef EXPLAIN_RANK
						printf("Bug: Hva docid=0!\n");
					#endif
					continue;
				}
			#endif

			
			if (languageFilterNr == 0) {
				++y;
			}
			else if (languageFilterNr == 1) {
				if (languageFilterAsNr[0] == TeffArray->iindex[y].langnr) {
					printf("filter hit\n");
					++y;
				}
			}
			else {
				//søker os gjenom språkene vi skal filtrerte på
				//int h;
				//for(h=0;h<(languageFilterNr -1);h++) {
				//	languageFilterAsNr[h] = TeffArray->iindex[y].langnr
				//}
			}
		}

		#ifdef TIME_DEBUG
		gettimeofday(&end_time, NULL);
		printf("Time debug: memcopy index into place %f\n",getTimeDifference(&start_time,&end_time));
		#endif

		#ifdef MMAP_IINDEX
			if (filemapptr == NULL)
				munmap(allindexp,mmap_size);
		#else
			free(allindexp);
		#endif
		if (filemapptr == NULL)
			fclose(fileha);

		*AntallTeff = y;
			
		

	} // fopen
	}

	#ifdef TIME_DEBUG
                gettimeofday(&end_time_total, NULL);
                printf("Time debug: GetIndexAsArray total %f\n",getTimeDifference(&start_time_total,&end_time_total));
      	#endif
	#ifdef DEBUG
	printf("GetIndexAsArray: AntallTeff = %i\n",(*AntallTeff));
	#endif
}

void GetIndexAsArray (int *AntallTeff, struct iindexFormat *TeffArray, 
		unsigned int WordIDcrc32, char * IndexType, char *IndexSprok,
		struct subnamesFormat *subname, 
		int languageFilterNr, int languageFilterAsNr[] ) {
	_GetIndexAsArray(AntallTeff, TeffArray, WordIDcrc32, IndexType, IndexSprok, subname, languageFilterNr, languageFilterAsNr, NULL);
}




void GetNForTerm(unsigned int WordIDcrc32, char *IndexType, char *IndexSprok, int *TotaltTreff, struct subnamesFormat *subname) {
	return _GetNForTerm(WordIDcrc32, IndexType, IndexSprok, TotaltTreff, subname, NULL);
}
void _GetNForTerm(unsigned int WordIDcrc32, char *IndexType, char *IndexSprok, int *TotaltTreff, struct subnamesFormat *subname, void *(filemap)(char *, size_t *)) {

		unsigned int Adress;
        	unsigned int SizeForTerm;
		int iindexfile;
		char IndexPath[256],IndexFile[255];		    
		char tmpPath[256];
		unsigned int term;
		unsigned int Antall;
		FILE *fileha;		    
		//////////////////////////////////////////////
		if ((!ReadIIndexRecordFromMemeory(&Adress, &SizeForTerm,WordIDcrc32,IndexType,IndexSprok,WordIDcrc32,(*subname).subname, filemap))
		&& (!ReadIIndexRecord(&Adress, &SizeForTerm,WordIDcrc32,IndexType,IndexSprok,WordIDcrc32,(*subname).subname))
		) {
			//*AntallTeff = 0;
			(*TotaltTreff) = 0;

		}
		else {

		//ReadIIndexRecord(&Adress, &SizeForTerm,WordIDcrc32,"Main","aa",WordIDcrc32,subname->subname);
				    
				    
		iindexfile = WordIDcrc32 % AntallBarrals;
		GetFilePathForIindex(IndexPath,IndexFile,iindexfile,IndexType,IndexSprok,subname->subname);
		snprintf(tmpPath,sizeof(tmpPath)-1,"%s/iindex/%s/index/%s/%i.txt",IndexPath,IndexType,IndexSprok, iindexfile);
		tmpPath[sizeof(IndexPath)-1] = '\0';
		strcpy(IndexPath, tmpPath);

		    
										    
	        if ((fileha = fopen(IndexFile,"rb")) == NULL) {
	    	    perror(IndexPath);
	        }
			fseek(fileha,Adress,0);
		
			fread(&term,sizeof(unsigned int),1,fileha);
			fread(&Antall,sizeof(unsigned int),1,fileha);
																						
			fclose(fileha);							
		
			(*TotaltTreff) = 	Antall;												
			//////////////////////////////////////////////
		}
																									
}




static unsigned int Indekser_hashfromkey(void *ky)
{
        return(*(unsigned int *)ky);
}

static int Indekser_equalkeys(void *k1, void *k2)
{
    return (*(unsigned int *)k1 == *(unsigned int *)k2);
}




void Indekser_deleteGcedFile(int lotNr, char subname[]) {
	FILE *GCEDFH;

	if ((GCEDFH =  lotOpenFileNoCasheByLotNr(lotNr,"gced","w", 'e',subname)) == NULL) {
		perror("can't open gced file");
		return;
	}

	if (ftruncate(fileno(GCEDFH), 0) != 0) {
		perror("can't truncate gced index");
	}

	fclose(GCEDFH);

}

struct hashtable * loadGced(int lotNr, char subname[]) {

	unsigned int *gcedArray;
	int nrofGced;
	struct stat inode;      // lager en struktur for fstat å returnere.
	FILE *GCEDFH;
	int i, y;

	struct hashtable *h;
	unsigned int *filesKey;
	int *filesValue;

	h = create_hashtable(200, Indekser_hashfromkey, Indekser_equalkeys);

	//get the list of garbage collected pages.
	if ((GCEDFH =  lotOpenFileNoCasheByLotNr(lotNr,"gced","r", 'e',subname)) == NULL) {
		perror("can't open gced file");
		return h;
	}

	fstat(fileno(GCEDFH),&inode);

	nrofGced = (inode.st_size / sizeof(unsigned int));

	#ifdef DEBUG
		printf("have %u gced DocID's\n",nrofGced);
	#endif

	if (nrofGced != 0) {

		if ((gcedArray = malloc(inode.st_size)) == NULL) {
			perror("malloc");
			return NULL;
		}

		fread(gcedArray,inode.st_size,1,GCEDFH);



		for(i=0;i<nrofGced;i++) {
			#ifdef DEBUG
			printf("gced DocID: %u\n",gcedArray[i]);
			#endif
	                if (NULL == (filesValue = hashtable_search(h,&gcedArray[i]) )) {
                                #ifdef DEBUG
				printf("filtyper: not found!. Vil insert first\n");
				#endif

                                filesValue = malloc(sizeof(int));
                                (*filesValue) = 1;

				filesKey = malloc(sizeof(gcedArray[i]));
				*filesKey = gcedArray[i];

                                if (! hashtable_insert(h,filesKey,filesValue) ) {
                                        printf("cant insert\n");
                                        exit(-1);
                                }

                        }
		}


	

		free(gcedArray);
	}

	fclose(GCEDFH);

	return h;
}


int Indekser(int lotNr,char type[],int part,char subname[], struct IndekserOptFormat *IndekserOpt) {

	struct hashtable *h = NULL;
	int i,y;
	unsigned int u, uy;
	int mgsort_i,mgsort_k;
	FILE *REVINDEXFH = NULL;
	FILE *IINDEXFH = NULL;
	unsigned int nrOfHits;
	unsigned short hit;
	char recordSeperator[4];
	char iindexPath[512];
	char iindexPathNew[512];
	char iindexPathOld[512];
	int count;
	char c;
	unsigned int lastWordID;
	unsigned int lastDocID;
        //char lang[4];
	int forekomstnr;
	struct stat inode;      // lager en struktur for fstat å returnere.
	char path[256];
	unsigned int *nrofDocIDsForWordID;
	struct revIndexArrayFomat *revIndexArray; 

        unsigned long term;
        unsigned long Antall;
        unsigned char langnr;
	int revIndexArraySize;

	#ifdef DEBUG
		printf("starting on an new index of part %i\n",part);
	#endif

	if (IndekserOpt->garbareCollection) {
		if ((h = loadGced(lotNr, subname)) == NULL) {
			perror("loadGced");
			return 0;
		}

	}


	//"$revindexPath/$revindexFilNr.txt";
	GetFilPathForLot(path,lotNr,subname);
	//ToDo: må sette språk annen plass
	sprintf(iindexPath,"%siindex/%s/index/aa/",path,type);

	//oppretter paths
	makePath(iindexPath);			

	sprintf(iindexPathOld,"%s%i.txt",iindexPath,part);
	sprintf(iindexPathNew,"%s%i.txt.new",iindexPath,part);

	if ((IndekserOpt->optMustBeNewerThen != 0)) {
		if (fopen(iindexPathOld,"r") != NULL) {
			printf("we all ready have a iindex.\n");
			return 0;
		}
	}

	revIndexArraySize = 0;


	if ((REVINDEXFH = revindexFilesOpenLocalPart(lotNr,type,"r+b",subname,part)) == NULL) {
		perror("revindexFilesOpenLocalPart");
		//exit(1);
		return 0;
	}

	fstat(fileno(REVINDEXFH),&inode);

	//ToDo: runarb 29.03.2008
	//veldig usikker på om dette er ret, antall DocId'er må være en del mindre en størelsen. Kansje 1/3 ?
	//må etterforske
	revIndexArraySize += (inode.st_size / 2);


	if ((IINDEXFH = fopen(iindexPathOld,"rb")) == NULL) {
		#ifdef DEBUG
			fprintf(stderr,"Can't open iindex file.\n");
			perror(iindexPathOld);
		#endif
	}
	else {
		flock(fileno(IINDEXFH),LOCK_EX);
		fstat(fileno(IINDEXFH),&inode);

		revIndexArraySize += (inode.st_size / 2);
	}

	if (revIndexArraySize > MaxRevIndexArraySize) {
		revIndexArraySize = MaxRevIndexArraySize;
	}

	if ((revIndexArray = malloc(sizeof(struct revIndexArrayFomat) * revIndexArraySize)) == NULL) {
		perror("malloc revIndexArray");
		exit(1);
	}

	
	if ((nrofDocIDsForWordID = malloc(sizeof(unsigned int) * revIndexArraySize)) == NULL) {
		perror("malloc nrofDocIDsForWordID");
		exit(1);
	}

	/*
	vi må fortsatt kjøre garbarge collection. En ytelsesforbedring i frmtiden vi være å bare gjøre det når det er 
	DocID'er i gced filen

	//kjører 
	if (inode.st_size == 0) {
		printf("rev index is emty. We dont have to do any thing.");
		return 0;
	}
	*/

	count = 0;

	//last iindex
	if (IINDEXFH == NULL) {
		//ingen vits å å gjøre noe hvis vi ikke fikk til å åpne filen	
	}
	else if (!IndekserOpt->sequenceMode) {

	}
	else {
		printf("Syatying to load iindex \"%s\" of size %d\n", iindexPathOld, inode.st_size);

		while ((!feof(IINDEXFH)) && (count < revIndexArraySize)) {
        	        //wordid hedder
                	if (fread(&term,sizeof(unsigned long),1,IINDEXFH) != 1) {
				//skriver ut feilmelding hvis vi fik en feil, men ikke eof
				if (!feof(IINDEXFH)) {
                        		printf("can't read term\n");
                        		perror(iindexPathOld);
				}
                        	//continue;
				break;
                	}
			fread(&Antall,sizeof(unsigned long),1,IINDEXFH);

			#ifdef DEBUG
			printf("term: %u antall: %u\n",term,Antall);
			#endif

			for (u=0;u<Antall;u++) {

				revIndexArray[count].WordID = term;

				if (fread(&revIndexArray[count].DocID,sizeof(unsigned long),1,IINDEXFH) != 1) {
                        	        printf("can't read DocID for nr %i\n",u);
                        	        perror("");
                        	        continue;
                        	}

				fread(&revIndexArray[count].langnr,sizeof(char),1,IINDEXFH);
                        	fread(&revIndexArray[count].nrOfHits,sizeof(unsigned long),1,IINDEXFH);

				revIndexArray[count].tombstone = 0;

				#ifdef DEBUG
					printf("\tcount: %i\n",count);
					printf("\tDocID %u lang %i\n",revIndexArray[count].DocID,(int)revIndexArray[count].langnr);
					printf("\tread WordID: %u, nrOfHits %u\n",revIndexArray[count].WordID,revIndexArray[count].nrOfHits);
				#endif

				if (revIndexArray[count].nrOfHits > MaxsHitsInIndex) {
					printf("iindex: nrOfHits lager then MaxsHitsInIndex (%i). Nr was %i\n",MaxsHitsInIndex ,revIndexArray[count].nrOfHits );
					goto IndekserError;			
				}


				for (uy = 0;uy < revIndexArray[count].nrOfHits; uy++) {
                                        if (fread(&revIndexArray[count].hits[uy],sizeof(unsigned short),1,IINDEXFH) != 1) {
						fprintf(stderr,"Can't read hit. DocID %u, nr of hits %d\n",revIndexArray[count].DocID,revIndexArray[count].nrOfHits);
						perror(iindexPathOld);
						goto IndekserError;
					}
                        	}


				if (NULL == hashtable_search(h,&revIndexArray[count].DocID) ) {
					++count;
					#ifdef DEBUG
						printf("old iindex DocID %u is ok\n",revIndexArray[count].DocID);
					#endif
				}
				else {
					#ifdef DEBUG
						printf("old iindex DocID %u is deleted\n",revIndexArray[count].DocID);
					#endif
				}

			}
		}

		#ifdef DEBUG
		printf("got %i good index elements from before\n",count);
		#endif
	}

	if (IINDEXFH != NULL) {
		fclose(IINDEXFH);
		IINDEXFH = NULL;
	}

	#ifdef DEBUG
	printf("rev index:\n");
	#endif

	while ((!feof(REVINDEXFH)) && (count < revIndexArraySize)) {
	

		if (fread(&revIndexArray[count].DocID,sizeof(revIndexArray[count].DocID),1,REVINDEXFH) != 1) {
			#ifdef DEBUG
			//har kommer vi til eof, det er helt normalt
			printf("can't read any more data\n");
			perror("revindex");
			#endif
			break;
		}


		//v3
		if (fread(&revIndexArray[count].langnr,sizeof(char),1,REVINDEXFH) != 1) {
			printf("fread langnr");
			goto IndekserError;
		}
		//printf("lang1 %i\n",(int)revIndexArray[count].langnr);


		if (fread(&revIndexArray[count].WordID,sizeof(revIndexArray[count].WordID),1,REVINDEXFH) != 1) {
			printf("fread WordID");
			goto IndekserError;

		}
		if (fread(&revIndexArray[count].nrOfHits,sizeof(revIndexArray[count].nrOfHits),1,REVINDEXFH) != 1) {
			printf("fread nrOfHits");
			goto IndekserError;
		}

		#ifdef DEBUG
			printf("%i\n",count);
			printf("\tread WordID: %u, nrOfHits %u\n",revIndexArray[count].WordID,revIndexArray[count].nrOfHits);
			printf("\tDocID %u lang %i\n",revIndexArray[count].DocID,(int)revIndexArray[count].langnr);
		#endif

		if (revIndexArray[count].nrOfHits > MaxsHitsInIndex) {
			printf("revinde: nrOfHits lager then MaxsHitsInIndex (%i). Nr was %i\n",MaxsHitsInIndex ,revIndexArray[count].nrOfHits );
			goto IndekserError;			
		}

		//leser antall hist vi skulle ha
		if (fread(&revIndexArray[count].hits,1,revIndexArray[count].nrOfHits * sizeof(short),REVINDEXFH) != (revIndexArray[count].nrOfHits * sizeof(short)) ) {
			perror("read hits");
			goto IndekserError;
		}

		revIndexArray[count].tombstone = 0;
			
		//debug:  hits
		#ifdef DEBUG
			printf("\tread hits: ");
			for (i=0;i<revIndexArray[count].nrOfHits;i++) {
				printf("%hu, ",revIndexArray[count].hits[i]);
			}
			printf("\n");
		#endif

		
		//Runarb  3 juni 2008: 	Dette er ikke rikig, vi skal slette gamle forekomster, men ikke nye, som har oppstått i ettertid.
		//Runarb 16 juni 2008: 	Vi bytter om, slik at vi bare analyserer DocumentIndex for å finne gcede sider. Hvis den er slettet der, så er den pr deg slettet. 
		//			Slik hånterer vi at det kan ligge nydata1->nydata2->nydata3 i reposetoriet, og v1 er ok, v2 er ok, men siste, v3 er ikke ok, og skal lsettes
		//			Da skal de ut av alle indekser.
		//Runarb 14 jan 2009:   Forstår ikke hvorfor dettte er slik. Nå kan vi ikke legge til dokumentet i gced filen hvis det er endret. Og alle 
		//			tidligere ord i et dokument vil være søkbar for det. Tror v1,v2,v3 problemet gjelder på web, der vi av og til
		//			sletter dokumenter vi ikke liker. Har ikke endret noe nåe.
		//Runarb 23 main 2011: 	Hadde ret i 2009. Gc må gjøres når vi laster ii index dokumentenem, hvis ikke blir oppdateringer av dokumenter feil.
		//garbare collection
		/* if ((IndekserOpt->garbareCollection) && (NULL != hashtable_search(h,&revIndexArray[count].DocID)) ) {
			#ifdef DEBUG
			printf("DocID %u is deleted\n",revIndexArray[count].DocID);
			#endif

		}
		else */ 
		if ((IndekserOpt->optValidDocIDs != NULL) && (IndekserOpt->optValidDocIDs[(revIndexArray[count].DocID - LotDocIDOfset(lotNr))] != 1)) {
			//#ifdef DEBUG
				printf("aaaaaaa: DocID %u is not in valid list\n",revIndexArray[count].DocID);
			//#endif

		}
		else {
			++count;

		}

	}

	#ifdef DEBUG
	printf("Documents in index: %i\n",count);
	#endif
	
	//runarb: 17 aug 2007: hvorfor har vi med -- her. Ser ut til at vi da mksiter siste dokumentet. haker ut for nå
	//--count;


	mgsort(revIndexArray, count , sizeof(struct revIndexArrayFomat),Indekser_compare_elements);


	if ((IINDEXFH = fopen(iindexPathNew,"wb")) == NULL) {
		fprintf(stderr,"can't open iindex for wb\n");
		perror(iindexPathNew);
		goto IndekserError;		
	}
	flock(fileno(IINDEXFH),LOCK_EX);

	//teller forkomster av DocID's pr WordID
	lastWordID = 0;
	forekomstnr = -1;
	lastDocID = 0;
	for(i=0;i<count;i++) {
		#ifdef DEBUG
		printf("WordID: %u, DocID %u\n",revIndexArray[i].WordID,revIndexArray[i].DocID);
		#endif

		if (lastWordID != revIndexArray[i].WordID) {
			++forekomstnr;			
			nrofDocIDsForWordID[forekomstnr] = 0;
			lastDocID = 0;
		}

		if ((IndekserOpt->optAllowDuplicates == 0) && (revIndexArray[i].DocID == lastDocID)) {
			#ifdef DEBUG
			printf("DocID %u is same as last\n",revIndexArray[i].DocID);
			#endif

			revIndexArray[i -1].tombstone = 1;
		}
		else {
			++nrofDocIDsForWordID[forekomstnr];
		}

		lastWordID = revIndexArray[i].WordID;
		lastDocID = revIndexArray[i].DocID;
	}

	lastWordID = 0;
	forekomstnr = 0;
	for(i=0;i<count;i++) {

		#ifdef DEBUG
		printf("looking at  WordID %u, nr %u\n",revIndexArray[i].WordID,nrofDocIDsForWordID[forekomstnr]);
		#endif

		//sjekker at dette ikke er en slettet DocID
		if (revIndexArray[i].tombstone) {
			#ifdef DEBUG
				fprintf(stderr,"DocID %u is tombstoned.\n",revIndexArray[i].DocID);
			#endif
			//toDo kan vi bare kalle continue her. Blir det ikke fil i noe antall?
			//Runarb: 1 juli 2008: ser ut til at vi henter antallet fra 
			//nrofDocIDsForWordID[forekomstnr], så det går bra
			continue;
		}

		if (lastWordID != revIndexArray[i].WordID) {

			#ifdef DEBUG
				printf("write WordID %u, nr %u\n",revIndexArray[i].WordID,nrofDocIDsForWordID[forekomstnr]);
			#endif
		
			fwrite(&revIndexArray[i].WordID,sizeof(revIndexArray[i].WordID),1,IINDEXFH);
			fwrite(&nrofDocIDsForWordID[forekomstnr],sizeof(int),1,IINDEXFH);

			++forekomstnr;
		}
		lastWordID = revIndexArray[i].WordID;

		if (revIndexArray[i].nrOfHits > MaxsHitsInIndex) {
			printf("Writing iindex: nrOfHits lager then MaxsHitsInIndex (%i). Nr was %i\n",MaxsHitsInIndex ,revIndexArray[i].nrOfHits );
			goto IndekserError;			
		}


		//printf("\tDocID %u, nrOfHits %u\n",revIndexArray[i].DocID,revIndexArray[i].nrOfHits);

		//skrive DocID og antall hit vi har
		fwrite(&revIndexArray[i].DocID,sizeof(revIndexArray[i].DocID),1,IINDEXFH);
		//v3
		fwrite(&revIndexArray[i].langnr,sizeof(char),1,IINDEXFH);

		fwrite(&revIndexArray[i].nrOfHits,sizeof(revIndexArray[i].nrOfHits),1,IINDEXFH);

		//skriver alle hittene		
		for(uy=0;uy<revIndexArray[i].nrOfHits;uy++) {
			#ifdef DEBUG		
				printf("\t\thit %hu\n",revIndexArray[i].hits[uy]);
			#endif
			fwrite(&revIndexArray[i].hits[uy],sizeof(short),1,IINDEXFH);
		}
		#ifdef DEBUG
		printf("write: DocID %u, WordID: %u, %u\n",revIndexArray[i].DocID,revIndexArray[i].WordID,revIndexArray[i].nrOfHits);		
		#endif
	}

	fclose(IINDEXFH);
	IINDEXFH = NULL;

	if (IndekserOpt->sequenceMode) {
		//trunkerer rev index. i LotInvertetIndexMaker3 er det bare en oppdateringsfil
		if (ftruncate(fileno(REVINDEXFH), 0) != 0) {
			perror("can't truncate rev index");
		}
	}

	fclose(REVINDEXFH);
	REVINDEXFH = NULL;

	free(revIndexArray);
	free(nrofDocIDsForWordID);


	if (rename(iindexPathNew,iindexPathOld) != 0) {
		fprintf(stderr,"rename(\"%s\", \"%s\")\n",iindexPathNew,iindexPathOld);
		perror("rename");
	}

	if (IndekserOpt->garbareCollection) {
		hashtable_destroy(h,1);
	}

	return 1;

	//hvis vi ikke fik til å indeksere skal vi free gi resursene våre, så returnere 0
	IndekserError:
		free(revIndexArray);
		free(nrofDocIDsForWordID);
		if (h != NULL) {
			hashtable_destroy(h,1);
		}
		if (REVINDEXFH != NULL) {
			fclose(REVINDEXFH);
		}
		if (IINDEXFH != NULL) {
			fclose(IINDEXFH);
		}
		return 0;

}

//sortere først på WordID, så DocID
//krever en stabil algoritme
int Indekser_compare_elements (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

	
	if (((struct revIndexArrayFomat*)p1)->WordID == ((struct revIndexArrayFomat*)p2)->WordID) {

		if (((struct revIndexArrayFomat*)p1)->DocID < ((struct revIndexArrayFomat*)p2)->DocID) {
        	        return -1;
        	}
        	else {
        	        return ((struct revIndexArrayFomat*)p1)->DocID > ((struct revIndexArrayFomat*)p2)->DocID;
	        }


	}
        else if (((struct revIndexArrayFomat*)p1)->WordID < ((struct revIndexArrayFomat*)p2)->WordID) {
                return -1;
	}
        else {
                return ((struct revIndexArrayFomat*)p1)->WordID > ((struct revIndexArrayFomat*)p2)->WordID;
	}
}



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




int mergei (int bucket,int startIndex,int stoppIndex,char *type,char *lang,char *subname, int *DocIDcount) {

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
        unsigned short hits[MaxsHitsInIndex];

	unsigned long currentTerm;

	unsigned char langnr;

	unsigned long term;
	unsigned long Antall;

	off_t startAdress;
	off_t stoppAdress;

	char FinalIindexFileName[128];
	char FinalDictionaryFileName[128];


	char FinalIindexFilePath[128];
        char FinalDictionaryFilePath[128];
	struct stat inode;      // lager en struktur for fstat å returnere.

	int count;

	FILE *FinalIindexFileFA;
	FILE *FinalDictionaryFileFA;

	char PathForIindex[128];
	char PathForLotIndex[128];

	if (startIndex == 0) {
		startIndex = 1;
	}
	if (stoppIndex == 0) {
		stoppIndex = maxLots;
	}

	#ifdef DEBUG
		printf("Merge index %i\n",bucket);
	#endif

	GetFilePathForIindex(FinalIindexFilePath,FinalIindexFileName,bucket,type,lang,subname);


	//sprintf(FinalIindexFilePath,"%s/iindex/%s/index/%s",PathForIindex,type,lang);
	//sprintf(FinalIindexFileName,"%s/%i.txt",FinalIindexFilePath,bucket);
	#ifdef DEBUG
		printf("FinalIindexFileName %s\nFinalIindexFilePath %s\n",FinalIindexFileName,FinalIindexFilePath);
	#endif

	makePath(FinalIindexFilePath);

	//sjekker om vi har nokk palss for dette
	if (!HasSufficientSpace(FinalIindexFilePath,4096)) {
                printf("insufficient disk space\n");
		exit(1);
        }


	if ((FinalIindexFileFA = batomicallyopen(FinalIindexFileName,"wb")) == NULL) {
		perror(FinalIindexFileName);
		exit(1);
	}
	flock(fileno(FinalIindexFileFA),LOCK_EX);

	//sprintf(FinalDictionaryFilePath,"%s/iindex/%s/dictionary/%s",PathForIindex,type,lang);
	//sprintf(FinalDictionaryFileName,"%s/%i.txt",FinalDictionaryFilePath,bucket);
	//printf("FinalDictionaryFileName %s\n",FinalDictionaryFileName);
	
	GetFilePathForIDictionary(FinalDictionaryFilePath,FinalDictionaryFileName,bucket,type,lang,subname);


	makePath(FinalDictionaryFilePath);

	if ((FinalDictionaryFileFA = batomicallyopen(FinalDictionaryFileName,"wb")) == NULL) {
                perror(FinalDictionaryFileName);
		exit(1);
        }
	flock(fileno(FinalDictionaryFileFA),LOCK_EX);

	#ifdef DEBUG
	printf("FinalDictionaryFileName \"%s\"\n",FinalDictionaryFileName);
	#endif

	#ifdef DEBUG
	printf("mallocing iindexfile of size %i\n", ((stoppIndex - startIndex) * sizeof(struct iindexfileFormat)) );
	#endif

	if ((iindexfile = malloc((stoppIndex - startIndex) * sizeof(struct iindexfileFormat))) == NULL) {
		perror("malloc iindexfile");
		exit(-1);
	}

	//printf("out file %s\nnrOffIindexFiles %i\n",FinalIindexFileName,nrOffIindexFiles);

	//setter alle som ikke åpnet	
        count=0;
	for (i=startIndex;i<stoppIndex;i++) {
		iindexfile[count].fileha = NULL;
		iindexfile[count].lastTerm = 0;
		++count;
	}

	count=0;
	for (i=startIndex;i<stoppIndex;i++) {
		//printf("iindex file nr %i\n",count);

		GetFilPathForLot(iindexfile[count].PathForLotIndex,i,subname);
		///iindex/Main/index/aa/63.txt
		
		sprintf(iindexfile[count].PathForLotIndex,"%siindex/%s/index/%s/%i.txt",iindexfile[count].PathForLotIndex,type,lang,bucket);

		//printf("aa iindex file %s\n",iindexfile[count].PathForLotIndex);

                if ((iindexfile[count].fileha = fopen(iindexfile[count].PathForLotIndex,"rb")) == NULL) {
			//ENOENT = No such file or directory
			if (errno == ENOENT) {
				printf("No such file or directory: %s\n",iindexfile[count].PathForLotIndex);
			}
			else {
				printf("errno %i, ENOENT %i\n",errno,ENOENT);
                        	perror(PathForLotIndex);
				exit(1);
			}
			//debug: viser hvilkene filer vi IKKE fikk åpnet
			//printf("can't open %s\n",iindexfile[count].PathForLotIndex);
			continue;
                }
		flock(fileno(iindexfile[count].fileha),LOCK_EX);

		fstat(fileno(iindexfile[count].fileha),&inode);

		if (inode.st_size == 0) {
			#ifdef DEBUG
			printf("File %s is emty, vill ignore it.\n",iindexfile[count].PathForLotIndex);
			#endif
			fclose(iindexfile[count].fileha);
			iindexfile[count].fileha = NULL;
		}
               	//leser inn første term
               	else if (fread(&iindexfile[count].lastTerm,sizeof(unsigned long),1,iindexfile[count].fileha) != 1) {
			printf("can't read first lastTerm for %s. Ignoring it\n",iindexfile[count].PathForLotIndex);
                        perror("read");
		}
            	else if (fread(&iindexfile[count].lastAntall,sizeof(unsigned long),1,iindexfile[count].fileha) != 1) {
			printf("can't read first lastAntall for %s. Ignoring it\n",iindexfile[count].PathForLotIndex);
                        perror("read");
		}
		else {

			//finner størelsen på filen
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

	#ifdef DEBUG
	printf("nrOffIindexFiles %i\n",nrOffIindexFiles);
	#endif

	gotEof = 0;
	//for (y=0;y<nrOffIindexFiles;y++) {
	while (nrOffIindexFiles != 0) {
	//while (nrOffIindexFiles > 0) {
		//hvis vi har fått en endoffile siden sist sorterer vi slik at den kommer nederst
		if (gotEof) {


			//+gotEof da vi skal også ha med de sidene vi sorterer ut nå i sorteringen
			qsort(iindexfile, nrOffIindexFiles + gotEof, sizeof(struct iindexfileFormat), mergei_compare_elements_eof);


			//printf("got eof %i\n",gotEof);

			//nrOffIindexFiles = nrOffIindexFiles - gotEof;


			
			gotEof = 0;

		}

		//sorter de etter term
		//ToDo: dette er nokk ikke optimalt når man tenker på hastighet. Må se på dette
		qsort(iindexfile, nrOffIindexFiles , sizeof(struct iindexfileFormat), mergei_compare_elements);

		//finner elementene som skal kopieres inn
		mergeTerm = iindexfile[0].lastTerm;
		i = 0;
		totaltAntall = 0;
		while ((mergeTerm == iindexfile[i].lastTerm) && (i<stoppIndex)) {
			//printf("mergeTerm=%d, lastTerm=%d, stoppIndex=%d, i=%d\n",mergeTerm,iindexfile[i].lastTerm,stoppIndex,i);
			totaltAntall += iindexfile[i].lastAntall;
			i++;
		}
		//printf("sort\n");
		//sorterer etter fil nr slik at vi får minste docid først
		qsort(iindexfile, i , sizeof(struct iindexfileFormat), mergei_compare_filenr);
	
		#ifdef DEBUG
			printf("totaltAntall %lu for term %lu\n",totaltAntall,iindexfile[0].lastTerm);
		#endif

		currentTerm = iindexfile[0].lastTerm;

		startAdress = batomicallyftell(FinalIindexFileFA);
		fwrite(&iindexfile[0].lastTerm,sizeof(iindexfile[0].lastTerm),1,FinalIindexFileFA);
		fwrite(&totaltAntall,sizeof(totaltAntall),1,FinalIindexFileFA);

		
		i = 0;
		while ( (mergeTerm == iindexfile[i].lastTerm) && (i<stoppIndex) ) {
		//kjører gjenom alle filene og skriver ut de som har index
		//for (i=0;i<nrOffIindexFiles;i++) {

			#ifdef DEBUG
			printf("i: %i skriv lastTerm %lu : lastAntall %lu fra nr %i\n",i,iindexfile[i].lastTerm,iindexfile[i].lastAntall,iindexfile[i].nr);
			#endif
			
			/////////////////////////////////////////////////

			for (x=0;x<iindexfile[i].lastAntall;x++) {
                        	//side hedder

				//if (iindexfile[i].lastAntall > 100000) {
 				//	printf("trying to read from %i, open %i, file %s\n",iindexfile[i].nr,iindexfile[i].open,iindexfile[i].PathForLotIndex);
				//}

				if ((n=fread(&DocID,sizeof(DocID),1,iindexfile[i].fileha)) != 1) {
					printf("can't read DocID for %s\n",iindexfile[i].PathForLotIndex);
					printf("iindex eof %i\n",iindexfile[i].eof);
					printf("i: %i\n",i);
					printf("nrOffIindexFiles %i\n",nrOffIindexFiles);
                        		perror("read");
					exit(1);
                        	}

				++(*DocIDcount);

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


				if (TermAntall > MaxsHitsInIndex) {
					fprintf(stderr,"TermAntall %d is lager then MaxsHitsInIndex %d. file \"%s\"\n",TermAntall,MaxsHitsInIndex,iindexfile[i].PathForLotIndex);
					goto iindexfileReadError;
				}

                        	for (z = 0;z < TermAntall; z++) {
					if ((n=fread(&hits[z],sizeof(unsigned short),1,iindexfile[i].fileha)) != 1) {
						fprintf(stderr,"can't read hit for %s. z: %i, TermAntall: %i. DocID %u\n",iindexfile[i].PathForLotIndex,z,TermAntall, DocID);
		                       		perror(iindexfile[i].PathForLotIndex);
                                        	//exit(1);
						//ToDo: dette er ikke 100% lurt, break her går ut av den første for loppen, men ikke den viktige hoved loopen
						goto iindexfileReadError;
                                	}
                        	}

				//skriver til final index
				fwrite(&DocID,sizeof(unsigned long),1,FinalIindexFileFA);
				fwrite(&langnr,sizeof(char),1,FinalIindexFileFA);
				fwrite(&TermAntall,sizeof(unsigned long),1,FinalIindexFileFA);

				//skriver hits
                        	for (z = 0;z < TermAntall; z++) {
					//skriver til final index
					fwrite(&hits[z],sizeof(unsigned short),1,FinalIindexFileFA);
				}
                	}


			////////////////////////////////////////////////
						
			if (feof(iindexfile[i].fileha)) {
				//printf("got eof %s\n",iindexfile[i].PathForLotIndex);
				//error hånterer som eof
				iindexfileReadError:	
		
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

		stoppAdress = batomicallyftell(FinalIindexFileFA);
                DictionaryPost.WordID = currentTerm;
                DictionaryPost.Adress = startAdress;
                DictionaryPost.SizeForTerm = stoppAdress - startAdress;
		if (DictionaryPost.SizeForTerm == 0) {
			fprintf(stderr,"BUG: DictionaryPost.SizeForTerm is 0!\n");
			fprintf(stderr,"\tWordID %u\n\tAdress: %u\n\tSizeForTerm: %u\n",DictionaryPost.WordID,DictionaryPost.Adress,DictionaryPost.SizeForTerm);
		}

                if (fwrite(&DictionaryPost,sizeof(DictionaryPost),1,FinalDictionaryFileFA) != 1) {
			printf("Cant't DictionaryPost to Dictionary");
			perror("fwrite");
			exit(-1);
		}

		//printf("WordID %ul, Adress: %ul, SizeForTerm: %ul\n",DictionaryPost.WordID,DictionaryPost.Adress,DictionaryPost.SizeForTerm);

		//printf("\n");
	}

	fclose(FinalDictionaryFileFA);
	fclose(FinalIindexFileFA);

	//for (i=0;i<nrOffIindexFiles;i++) {
	count=0;
	for (i=startIndex;i<stoppIndex;i++) {
		if (iindexfile[count].fileha != NULL) {
			fclose(iindexfile[count].fileha);
		}
		++count;
	}

	free(iindexfile);

}








int mergei_compare_elements (const void *p1, const void *p2) {

        //return i1 - i2;
        struct iindexfileFormat *t1 = (struct iindexfileFormat*)p1;
        struct iindexfileFormat *t2 = (struct iindexfileFormat*)p2;

	if (t1->lastTerm < t2->lastTerm)
                return -1;
        else
                return t1->lastTerm > t2->lastTerm;

}

int mergei_compare_elements_eof (const void *p1, const void *p2) {

        //return i1 - i2;
        struct iindexfileFormat *t1 = (struct iindexfileFormat*)p1;
        struct iindexfileFormat *t2 = (struct iindexfileFormat*)p2;

        if (t1->eof < t2->eof)
                return -1;
        else
                return t1->eof > t2->eof;

}

int mergei_compare_filenr (const void *p1, const void *p2) {

        //return i1 - i2;
        struct iindexfileFormat *t1 = (struct iindexfileFormat*)p1;
        struct iindexfileFormat *t2 = (struct iindexfileFormat*)p2;

        if (t1->nr < t2->nr)
                return -1;
        else
                return t1->nr > t2->nr;

}

