#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>


#include "../common/crc32.h"
#include "../common/define.h"
#include "../common/reposetoryNET.h"
#include "../common/reposetory.h"
#include "../common/revindex.h"
#include "../common/langdetect.h"

#include "../common/DocumentIndex.h"
#include "../common/lot.h"

#include "../parser/html_parser.h"

#define maxWordForPage 4000
#define maxAdultWords 500
#define MaxAdultWordCount 50

//#define DEBUG_ADULT

//#define subname "www"

#define AdultWordsVektetFile "data/AdultWordsVektet.txt"
#define AdultFraserVektetFile "data/AdultFraserVektet.txt"

//global veriabel som sier fra om en alarm ble utløst. Trenger bare en da man bare kan ha en alarm av gangen
volatile sig_atomic_t alarm_got_raised;
unsigned int global_curentDocID;
int global_curentUrlIsDynamic;
char subname[maxSubnameLength];

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf );

struct adultWordFormat {
	char word[maxWordlLen +1];
	unsigned long crc32;
	int weight;
};

struct revIndexFomat {
        unsigned long WordID;
        unsigned long nr;
        unsigned short hits[MaxsHitsInIndex];
};
struct adultWordFraserFormat {
        char word[maxWordlLen +1];
        unsigned long crc32;
	struct adultWordFormat adultWord[MaxAdultWordCount];
	int adultWordCount;
        
};

struct adultFormat {
	struct adultWordFormat AdultWords[maxAdultWords];
	int adultWordnr;
	struct adultWordFraserFormat adultFraser[maxAdultWords];
	int adultWordFrasernr;
};


struct pagewordsFormat {
	int nr;
	int nextPosition;
	struct wordsFormat words[maxWordForPage];
	struct wordsFormat words_sorted[maxWordForPage];
	int revIndexnr;
	struct revIndexFomat revIndex[maxWordForPage];
};
struct pagewordsFormat pagewords;



void wordsReset() {

	pagewords.nr = 0;
	pagewords.nextPosition = 0;	
	pagewords.revIndexnr = 0;
}

void linkadd(char word[]) {

                struct updateFormat updatePost;
		static char lasturl[201] = {""};
		int lengt;
		char *cpnt;

		lengt = strlen(word);
		
		//finner siste bokstav
		if (word[lengt -1] == '/') {
			cpnt = &word[lengt -1];
		}
		else {
			if ((cpnt = strrchr(word,'.')) == NULL){
				cpnt = NULL;
			}
		}
			//printf("cpnt %s\n",cpnt);			


		//temp: for nå tar vi bare korte urler
		if (lengt > 50) {
	
		}
		else if (cpnt == NULL) {
			//printf("bad url %s\n",word);
		}
		else if (
			!(
			(strcmp(cpnt,"/") == 0) 
			|| (strcmp(cpnt,".html") == 0) 
			|| (strcmp(cpnt,".htm") == 0) 
			|| (strcmp(cpnt,".php") == 0) 
			|| (strcmp(cpnt,".asp") == 0)
			)
		) {
			//tar bar de filendelsene vi vil ha
			//printf("bad ending %s, cpnt %s\n",word,cpnt);
		}
		else if (strchr(word,'#') != NULL) {
			//printf("has #: %s\n",word);
		}
		else if (strcmp(word,lasturl) == 0) {
			//printf("last and ne the same %s == %s\n",word,lasturl);
		}
                else if ((strchr(word,'?') != NULL) && (global_curentUrlIsDynamic)) {
                        //printf("NO add %s\n",word);
                }
                else if (lengt > sizeof(updatePost.url)) {
                        //printf("url to long at %i char\n",strlen(word));
                }
                else {
                        //printf("link: %s\n",word);


                        strncpy(updatePost.url,word,sizeof(updatePost.url));
                        strncpy(updatePost.linktext,"",sizeof(updatePost.linktext));
                        updatePost.DocID_from = global_curentDocID;

                        //addNewUrl(&updatePost,"",subname);
                }

		strncpy(lasturl,word,sizeof(lasturl));

}


void wordsAdd(char word[],enum parsed_unit_flag puf) {
int i;
int wordlLength;
int wordTypeadd;
			if (pagewords.nr < maxWordForPage){

				switch (puf)
                        	{
                            		case puf_none: 
						//printf(" +p"); 
						wordTypeadd=1000;break;
                            		case puf_title: 
						//printf(" +title"); 
						wordTypeadd=100; break;
                            		case puf_h1: 
						//printf(" +h1"); 
						wordTypeadd=500; break;
                            		case puf_h2: 
						//printf(" +h2"); 
						wordTypeadd=500; break;
                            		case puf_h3: 
						//printf(" +h3"); 
						wordTypeadd=500; break;
                            		case puf_h4: 
						//printf(" +h4"); 
						wordTypeadd=500; break;
                            		case puf_h5: 
						//printf(" +h5"); 
						wordTypeadd=500; break;
                            		case puf_h6: 
						//printf(" +h6"); 
						wordTypeadd=500; break;
                        	}

				wordlLength = strlen(word);

				//gjør om til små bokstaver
				for(i=0;i<wordlLength;i++) {
					word[i] = (char)tolower(word[i]);
				}


				#ifdef DEBUG_ADULT
					strcpy(pagewords.words[pagewords.nr].word,word);
				#endif
				pagewords.words[pagewords.nr].WordID =  crc32boitho(word);
				pagewords.words[pagewords.nr].position = (pagewords.nextPosition + wordTypeadd);

				//printf("nextPosition %i, wordTypeadd %i, position %i\n",pagewords.nextPosition,wordTypeadd,pagewords.words[pagewords.nr].position);

				++pagewords.nextPosition;

				//printf("%s : %lu\n",word,pagewords.words[pagewords.nr]);

				++pagewords.nr;		
			}
			else {
				//printf("To many words in dokument\n");
			}
}

int compare_elements_words (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

        if (((struct wordsFormat*)p1)->WordID < ((struct wordsFormat*)p2)->WordID)
                return -1;
        else
                return ((struct wordsFormat*)p1)->WordID > ((struct wordsFormat*)p2)->WordID;

}



static int cmp1_crc32(const void *p, const void *q)
{
	printf("%lu %lu\n",*(const unsigned long *) p,*(const unsigned long *) q);

   return *(const unsigned long *) p - *(const unsigned long *) q;
}


void wordsMakeRevIndex(struct adultFormat *adult,unsigned short *adultWeight) {

	int i,y,adultpos,adultFraserpos;
	unsigned long oldcrc32;
	pagewords.revIndexnr = 0;
	int oldRevIndexnr;
	char lang[4];

	//kopierer over til den word arrayn som skal være sortert
	for(i=0;i<pagewords.nr;i++) {
		pagewords.words_sorted[i].WordID = pagewords.words[i].WordID;
		pagewords.words_sorted[i].position = pagewords.words[i].position;
		#ifdef DEBUG_ADULT
			strcpy(pagewords.words_sorted[i].word,pagewords.words[i].word);
		#endif
	}

	//sorter ordene
	qsort(&pagewords.words_sorted, pagewords.nr , sizeof(struct wordsFormat), compare_elements_words);



	langdetectDetect(pagewords.words_sorted,pagewords.nr,lang);



	oldcrc32 = 0;	
	adultpos = 0;
	adultFraserpos = 0;
	(*adultWeight) = 0;
	for(i=0;i<pagewords.nr;i++) {
		
		//printf("oo: %lu %i\n",pagewords.words_sorted[i].WordID,pagewords.words_sorted[i].position);

		
		if ((pagewords.words_sorted[i].WordID != oldcrc32) || (oldcrc32 == 0)) {
			// nytt ord, skal lages nytt
			pagewords.revIndex[pagewords.revIndexnr].WordID = pagewords.words_sorted[i].WordID;
			pagewords.revIndex[pagewords.revIndexnr].nr = 0;
			pagewords.revIndex[pagewords.revIndexnr].hits[pagewords.revIndex[pagewords.revIndexnr].nr] = pagewords.words_sorted[i].position;

			#ifdef DEBUG			
				printf("word %lu %s\n",pagewords.revIndex[pagewords.revIndexnr].WordID,pagewords.words_sorted[i].word);
			#endif

			///////////////////////////
			// adultWords


			while ((pagewords.revIndex[pagewords.revIndexnr].WordID > (*adult).AdultWords[adultpos].crc32) && (adultpos < (*adult).adultWordnr)) {
				#ifdef DEBUG
					//printf("s %lu %s\n",(*adult).AdultWords[adultpos].crc32,(*adult).AdultWords[adultpos].word);
				#endif
				++adultpos;
			}
			//printf("ll: adultpos %i < adultWordnr %i\n",adultpos,(*adult).adultWordnr);
			if ((*adult).AdultWords[adultpos].crc32 == pagewords.revIndex[pagewords.revIndexnr].WordID) {
				#if DEBUG_ADULT
					printf("adult hit %s %i\n",(*adult).AdultWords[adultpos].word,(*adult).AdultWords[adultpos].weight);
				#endif
				(*adultWeight) += (*adult).AdultWords[adultpos].weight;
				//exit(1);
			}
			///////////////////////////
			//adult fraser

			while ((pagewords.revIndex[pagewords.revIndexnr].WordID > (*adult).adultFraser[adultFraserpos].crc32) && (adultFraserpos < (*adult).adultWordFrasernr)) {
				#ifdef DEBUG
					//printf("s %lu %s\n",(*adult).adultFraser[adultFraserpos].crc32,(*adult).adultFraser[adultFraserpos].word);
				#endif
				++adultFraserpos;
			}
			if ((*adult).adultFraser[adultFraserpos].crc32 == pagewords.revIndex[pagewords.revIndexnr].WordID) {
				
				//har hitt. Lopper gjenom word2'ene på jakt etter ord nr to
				#ifdef DEBUG_ADULT
					printf("frase hit %s\n",(*adult).adultFraser[adultFraserpos].word);
					printf("next word is %s\n", pagewords.words[pagewords.words_sorted[i].position +1].word);
					printf("x words to try %i, pos %i\n",(*adult).adultFraser[adultFraserpos].adultWordCount,adultFraserpos);
				#endif

				for(y=0;y<(*adult).adultFraser[adultFraserpos].adultWordCount;y++) {

					if ((*adult).adultFraser[adultFraserpos].adultWord[y].crc32 == pagewords.words[pagewords.words_sorted[i].position +1].WordID) {
						#ifdef DEBUG_ADULT
							printf("frase hit %s\n",(*adult).adultFraser[adultFraserpos].adultWord[y].word);
						#endif
						//øker adult verdien med vekt
						(*adultWeight) += (*adult).adultFraser[adultFraserpos].adultWord[y].weight;
						break;
					}
				}

			}

			///////////////////////////

			oldRevIndexnr = pagewords.revIndexnr;
			//har fåt et til ord i revindex
			++pagewords.revIndex[pagewords.revIndexnr].nr;
			++pagewords.revIndexnr;
		}
		else {
			// ord er set før, skal legges til det tidligere
			// må passe på at vi ikke overskriver med flere hits en MaxsHits
			if (pagewords.revIndex[oldRevIndexnr].nr < MaxsHitsInIndex) {
				pagewords.revIndex[oldRevIndexnr].hits[pagewords.revIndex[oldRevIndexnr].nr] = pagewords.words_sorted[i].position;
				++pagewords.revIndex[oldRevIndexnr].nr;
			}
			else {
				//printf("to many hits\n");
			}
			//break;

		}

		oldcrc32 = pagewords.words_sorted[i].WordID;

	
	}

}


int compare_elements_adultWord (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

        if (((struct adultWordFormat*)p1)->crc32 < ((struct adultWordFormat*)p2)->crc32)
                return -1;
        else
                return ((struct adultWordFormat*)p1)->crc32 > ((struct adultWordFormat*)p2)->crc32;

}

int compare_elements_AdultFraser (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

        if (((struct adultWordFraserFormat*)p1)->crc32 < ((struct adultWordFraserFormat*)p2)->crc32)
                return -1;
        else
                return ((struct adultWordFraserFormat*)p1)->crc32 > ((struct adultWordFraserFormat*)p2)->crc32;

}



void adultLoad (struct adultFormat *adult) {

	FILE *FH;
	char buff[128];
	int i,y,x;
	char *cpoint;
	char word1[128];
	char word2[128];	
	int weight;
	unsigned long crc32tmp;

	//AdultWordsFile
	if ((FH = fopen(AdultWordsVektetFile,"r")) == NULL) {
                        perror(AdultWordsVektetFile);
                        exit(1);
	}

	i=0;
	while ((fgets(buff,sizeof(buff),FH) != NULL) && (i < maxAdultWords)) {
		//fjerner \n en på slutteten
		buff[strlen(buff) -1] = '\0';

		//gjør om til lite case
		for(x=0;x<strlen(buff);x++) {
			buff[x] = tolower(buff[x]);
		}

		//finner space, som er det som skiller
                cpoint = strchr(buff,' ');
		if (cpoint != NULL) {

			strncpy((*adult).AdultWords[i].word,buff,cpoint - buff);
			//vil ikke ha men spacen. Går et hakk vidre
			++cpoint;
			(*adult).AdultWords[i].weight = atoi(cpoint);
	
			(*adult).AdultWords[i].crc32 = crc32boitho((*adult).AdultWords[i].word);

		}		

		//(*adult).AdultWords[i].word[strlen((*adult).AdultWords[i].word) -1] = '\0';

		//printf("%i: -%s- %lu %i\n",i,(*adult).AdultWords[i].word,(*adult).AdultWords[i].crc32,(*adult).AdultWords[i].weight);
		++i;
	}
	(*adult).adultWordnr = i;

	fclose(FH);

	qsort((*adult).AdultWords, i , sizeof(struct adultWordFormat), compare_elements_adultWord);	

//debug: vis alle ordene, sortert
//	for(y=0;y<i;y++) {
//		printf("%i: -%s- %lu %i\n",y,(*adult).AdultWords[y].word,(*adult).AdultWords[y].crc32,(*adult).AdultWords[y].weight);
//	}

	for(i=0;i<maxAdultWords;i++) {
		(*adult).adultFraser[i].adultWordCount = 0;
	}


	//AdultFraserFile
	if ((FH = fopen(AdultFraserVektetFile,"r")) == NULL) {
                        perror(AdultFraserVektetFile);
                        exit(1);
	}

	i=-1;
	while ((fgets(buff,sizeof(buff) -1,FH) != NULL) && (i < maxAdultWords)) {
                //gjør om til lite case
                for(x=0;x<strlen(buff);x++) {
                        buff[x] = tolower(buff[x]);
                }

		//printf("buff %s\n",buff);
		if ((x=sscanf(buff,"%s %s %i\n",word1,word2,&weight))!=3) {
			
			printf("bad AdultFraserVektetFile format: \"%s\" . x: %i\n",buff, x);

		}
		else {

			//printf("%i: %s, %s, %i\n",i,word1,word2,weight);
	
			//finner crc32 verdeien for første ord
			crc32tmp = crc32boitho(word1);

			//hvsi dette er første så her vi ikke noen forige å legge den til i, så vi må opprette ny
			//hvsi dette derimot har samme word1 som forige så legger vi det til
			if ((i!=-1) && (crc32tmp == (*adult).adultFraser[i].crc32)) {
				//printf("nr to\n");
			}
			else {
				++i;
			}
		

			strcpy((*adult).adultFraser[i].word,word1);
			(*adult).adultFraser[i].crc32 = crc32boitho(word1);		

			(*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].weight = weight;
			strcpy((*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].word,word2);
			(*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].crc32 = crc32boitho(word2);

			if ((*adult).adultFraser[i].adultWordCount < MaxAdultWordCount -1) {
				++(*adult).adultFraser[i].adultWordCount;
			}
			else {
				printf("MaxAdultWordCount %i for %s\n",MaxAdultWordCount,buff);
			}

			



		}
	}
	fclose(FH);

	(*adult).adultWordFrasernr = i;
	qsort((*adult).adultFraser, (*adult).adultWordFrasernr , sizeof(struct adultWordFraserFormat), compare_elements_AdultFraser);

/*
	for(i=0;i<(*adult).adultWordFrasernr;i++) {
		printf("%i, -%s-, nr %i\n",i,(*adult).adultFraser[i].word,(*adult).adultFraser[i].adultWordCount);

		for(y=0;y<(*adult).adultFraser[i].adultWordCount;y++) {
			printf("\t %i: %s-%s: %i\n",y,(*adult).adultFraser[i].word,(*adult).adultFraser[i].adultWord[y].word,(*adult).adultFraser[i].adultWord[y].weight);
		}
		

	}
*/

}

void revindexFilesOpenNET(FILE *revindexFilesHa[]) {
	int i;

	for(i=0;i<NrOfDataDirectorys;i++) {
		if ((revindexFilesHa[i] = tmpfile()) == NULL) {
			perror("tmpfile");
			exit(1);
		}
	}
}
/**************************************************************************************
Sender revindex filene til en server som kjører boithold
***************************************************************************************/
void revindexFilesSendNET(FILE *revindexFilesHa[],int lotNr) {
	int i;
	char dest[64];

	for(i=0;i<NrOfDataDirectorys;i++) {

		sprintf(dest,"revindex/Main/%i.txt",i);

		rSendFileByOpenHandler(revindexFilesHa[i],dest,lotNr,"a",subname);
	}
}
/**************************************************************************************
Skriver reversert index til disk
***************************************************************************************/
void revindexFilesAppendWords(FILE *revindexFilesHa[],unsigned int DocID,char lang[]) {

	int i,y;
	int bucket;

	/*	
	//skriver først en hedder til alle filene
	for(i=0;i<NrOfDataDirectorys;i++) {
		fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[i]);
		// skriver 3 tegn av sprøket. Er det vi bruker
		//fwrite(&lang,sizeof(char),3,revindexFilesHa[i]);
		//temp, lattlige språkproblemer her :(
		fprintf(revindexFilesHa[i],"aa ");
	}
	*/

	for(i=0;i<pagewords.revIndexnr;i++) {
		//printf("%lu forekomster %i (+1)\n",pagewords.revIndex[i].WordID,pagewords.revIndex[i].nr);

		bucket = pagewords.revIndex[i].WordID % NrOfDataDirectorys;

		//printf("bucket %i\n",bucket);

		//++pagewords.revIndex[i].nr;
		fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[bucket]);
		fprintf(revindexFilesHa[bucket],"aa ");


		fwrite(&pagewords.revIndex[i].WordID,sizeof(unsigned long),1,revindexFilesHa[bucket]);	
		fwrite(&pagewords.revIndex[i].nr,sizeof(unsigned long),1,revindexFilesHa[bucket]);

		for(y=0;y<pagewords.revIndex[i].nr;y++) {
			//printf("\thits %i\n",pagewords.revIndex[i].hits[y]);
			fwrite(&pagewords.revIndex[i].hits[y],sizeof(unsigned short),1,revindexFilesHa[bucket]);
		}

	}	

	/*
	//skriver record terminator
	for(i=0;i<NrOfDataDirectorys;i++) {
		//ToDo: her bruker vi \n for linefeed, men bruker \cJ i perl. På andre platformer en *nix vil det føre til problmer
		//	erstatt \n med tegnet for linefeed
		fprintf(revindexFilesHa[i],"**\n");
	}
	*/
}

void copyRepToDi(struct DocumentIndexFormat *DocumentIndexPost,struct ReposetoryHeaderFormat *ReposetoryHeader) {
			strcpy((*DocumentIndexPost).Url,(*ReposetoryHeader).url);
			
			strcpy((*DocumentIndexPost).Dokumenttype,(*ReposetoryHeader).content_type);

			(*DocumentIndexPost).IPAddress 		= (*ReposetoryHeader).IPAddress;
			(*DocumentIndexPost).response 		= (*ReposetoryHeader).response;
			(*DocumentIndexPost).htmlSize 		= (*ReposetoryHeader).htmlSize;
			(*DocumentIndexPost).imageSize 		= (*ReposetoryHeader).imageSize;
			(*DocumentIndexPost).userID 		= (*ReposetoryHeader).userID;
			(*DocumentIndexPost).clientVersion 	= (*ReposetoryHeader).clientVersion;
			(*DocumentIndexPost).CrawleDato 	= (*ReposetoryHeader).time;

			(*DocumentIndexPost).ResourcePointer 	= 0;
			(*DocumentIndexPost).ResourceSize 	= 0;

}

void html_parser_timout( int signo )
{
    if ( signo == SIGALRM ) {
        printf("got alarm\n");
        alarm_got_raised = 1;
    }
 

}


void handelPage(char lotServer[], unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlcompressdbuffer[],char imagebuffebuffer[],FILE *revindexFilesHa[],struct DocumentIndexFormat *DocumentIndexPost, int DocID,int httpResponsCodes[], struct adultFormat *adult) {

        unsigned int HtmlBufferSize;
        char HtmlBuffer[524288];
	int nerror;
	//temp: må finne lang
	char lang[4] = " aa";

		if ((*ReposetoryHeader).response < nrOfHttpResponsCodes) {
			++httpResponsCodes[(*ReposetoryHeader).response];
		}


		//printf("%lu %s\n",(*ReposetoryHeader).DocID, (*ReposetoryHeader).url);

		if (((*ReposetoryHeader).response >= 200) && ((*ReposetoryHeader).response <= 299)) {

		

			HtmlBufferSize = sizeof(HtmlBuffer);
                	if ( (nerror = uncompress((Bytef*)&HtmlBuffer,(uLong *)&HtmlBufferSize,htmlcompressdbuffer,(*ReposetoryHeader).htmlSize)) != 0) {
	                	#ifndef NOWARNINGS
					printf("uncompress error. Code: %i\n",nerror);
				#endif
                	}
                	else {

			
				//if ((*ReposetoryHeader).DocID == 363093661) {
				//	FILE *FH;
				//	FH = fopen("/tmp/IndexerLotdump.html","wb");
				//	fwrite(HtmlBuffer,HtmlBufferSize,1,FH);
				//	fclose(FH);
				//	exit(1);
				//}

		
				
				//begynner på en ny side
				wordsReset();

				if (strcmp((*ReposetoryHeader).content_type,"htm") == 0) {

					//setter opp en alarm slik at run_html_parser blir avbrut hvis den henger seg 
					alarm_got_raised = 0;
					signal(SIGALRM, html_parser_timout);
					
					alarm( 5 );
					//parser htmlen
					run_html_parser( (*ReposetoryHeader).url, HtmlBuffer, HtmlBufferSize, fn );
					alarm( 0);
					if(alarm_got_raised) {
						printf("run_html_parser did time out. At DocID %lu\n",(*ReposetoryHeader).DocID);
					}
					else {

						wordsMakeRevIndex(adult,&(*DocumentIndexPost).AdultWeight);



						revindexFilesAppendWords(revindexFilesHa,(*ReposetoryHeader).DocID,lang);
					}
				
				}
				else if (strcmp((*ReposetoryHeader).content_type,"btx") == 0) {
					//ToDo: definer Boitho TeXt formatet
				}
				else {
					printf("unknown content_type \"%s\"\n",(*ReposetoryHeader).content_type);
				}


			}

		}
		else { //not in 200->299 range
			//radress må vel ikke nulles her, like går å ha den pekene til strukturen i reposetoryet 
			HtmlBufferSize = 0;
		}

			

}


int main (int argc, char *argv[]) {

        int lotNr;
	char lotServer[64];
	int pageCount;
	int i;

        unsigned int FiltetTime;
        unsigned int FileOffset;

        char htmlcompressdbuffer[524288];  //0.5 mb
        char imagebuffer[524288];  //0.5 mb
	
	int httpResponsCodes[nrOfHttpResponsCodes];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	struct DocumentIndexFormat DocumentIndexPost;
	unsigned long int radress;
	FILE *revindexFilesHa[NrOfDataDirectorys];
	struct adultFormat adult;
	unsigned int lastIndexTime;

        if (argc < 3) {
                printf("Dette programet indekserer en lot. Usage:\n\tIndexerLot lotNr subname\n");
                exit(0);
        }

	for(i=0;i<nrOfHttpResponsCodes;i++) {
		httpResponsCodes[i] = 0;
	}

	lotNr = atoi(argv[1]);
	strncpy(subname,argv[2],sizeof(subname) -1);


	//find server based on lotnr
	lotlistLoad();
	lotlistGetServer(lotServer,lotNr);


	printf("vil index lot nr %i at %s\n",lotNr,lotServer);

	adultLoad(&adult);

	langdetectInit();



	//temp: må hente dette fra slot server eller fil
	FiltetTime = 0;
	FileOffset = 0;

	pageCount = 0;

	if (0) {


		printf("will ges pages by net\n");

		revindexFilesOpenNET(revindexFilesHa);

		while (rGetNextNET(lotServer,lotNr,&ReposetoryHeader,htmlcompressdbuffer,imagebuffer,&radress,FiltetTime,FileOffset,subname)) {

                                global_curentDocID = ReposetoryHeader.DocID;
                                if (strchr(ReposetoryHeader.url,'?') == 0) {
                                        global_curentUrlIsDynamic = 0;
                                }
                                else {
                                        global_curentUrlIsDynamic = 1;
                                }
		
			
			
			handelPage(lotServer,lotNr,&ReposetoryHeader,htmlcompressdbuffer,imagebuffer,revindexFilesHa,&DocumentIndexPost,ReposetoryHeader.DocID,httpResponsCodes,&adult);
			//datta skal uansett kopieres over
			//kopierer over di data
			copyRepToDi(&DocumentIndexPost,&ReposetoryHeader);

			DocumentIndexPost.RepositoryPointer = radress;


			//skiver til DocumentIndex
			DIWriteNET(lotServer,&DocumentIndexPost,ReposetoryHeader.DocID,subname);


			++pageCount;
		
			//temp: 
			//if(pageCount > 999) {
			//	printf("Exeting after only %i docs\n",pageCount);
			//	break;
			//}

		}

		printf("Sending pages\n");

		revindexFilesSendNET(revindexFilesHa,lotNr);

	}
	else {
		printf("Wil acess files localy\n");

		//sjekker om vi har nokk palss
		if (!lotHasSufficientSpace(lotNr,4096,subname)) {
			printf("insufficient disk space\n");
			exit(1);
		}


		//finner siste indekseringstid
		lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);

		
		if(lastIndexTime != 0) {
			printf("lastIndexTime is not 0, but %i\n",lastIndexTime);
			exit(1);
		}
		
		
		revindexFilesOpenLocal(revindexFilesHa,lotNr,"Main","wb",subname);

		//temp:Søker til problemområdet
		//FileOffset = 334603785;		

		while (rGetNext(lotNr,&ReposetoryHeader,htmlcompressdbuffer,sizeof(htmlcompressdbuffer),imagebuffer,&radress,FiltetTime,FileOffset,subname)) {		
			//printf("D: %u, R: %lu\n",ReposetoryHeader.DocID, radress);


                                global_curentDocID = ReposetoryHeader.DocID;
                                if (strchr(ReposetoryHeader.url,'?') == 0) {
                                        global_curentUrlIsDynamic = 0;
                                }
                                else {
                                        global_curentUrlIsDynamic = 1;
                                }

								
				printf("%s\n",ReposetoryHeader.url);
				
				
				handelPage(lotServer,lotNr,&ReposetoryHeader,htmlcompressdbuffer,imagebuffer,revindexFilesHa,&DocumentIndexPost,ReposetoryHeader.DocID,httpResponsCodes,&adult);

				

				//datta skal kopieres over uanset hva som skjer
				//kopierer over di data
				copyRepToDi(&DocumentIndexPost,&ReposetoryHeader);

				DocumentIndexPost.RepositoryPointer = radress;

				//skiver til DocumentIndex
				DIWrite(&DocumentIndexPost,ReposetoryHeader.DocID,subname);
				

			++pageCount;
		
			//if(pageCount > 9999) {
			//	printf("Exeting after only %i docs\n",pageCount);
			//	//break;
			//	exit(1);
			//}

		}

		//skriver riktig indexstide til lotten
		setLastIndexTimeForLot(lotNr,httpResponsCodes,subname);

		// vi må ikke kopiere revindex filene da vi jobber på de lokale direkte
	}


	langdetectDestroy();

	printf("indexed %i pages\n\n\n",pageCount);

	return 0;
}


void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf )
{


	#ifdef DEBUG
    		printf("\t%s (%i) ", word, pos);
	#endif
    switch (pu)
        {
            case pu_word: 

			wordsAdd(word,puf);

			/*
    			switch (puf)
        		{
        		    case puf_none: break;
        		    case puf_title: printf(" +title"); break;
        		    case puf_h1: printf(" +h1"); break;
        		    case puf_h2: printf(" +h2"); break;
        		    case puf_h3: printf(" +h3"); break;
        		    case puf_h4: printf(" +h4"); break;
        		    case puf_h5: printf(" +h5"); break;
        		    case puf_h6: printf(" +h6"); break;
		        }
			*/

			#ifdef DEBUG
				printf("[word] is now %s ",word); 
			#endif


		break;
            case pu_linkword: 
			#ifdef DEBUG
				printf("[linkword]"); 
			#endif
		break;
            case pu_link:

			
			linkadd(word);
			#ifdef DEBUG 
				printf("[link]"); 
			#endif
		break;
            case pu_baselink:
			#ifdef DEBUG 
				printf("[baselink]");
			#endif 
		break;
            case pu_meta_keywords: 
			#ifdef DEBUG
				printf("[meta keywords]"); 
			#endif
			break;
            case pu_meta_description:
			#ifdef DEBUG 
				printf("[meta description]"); 
			#endif
			break;
            case pu_meta_author:
			#ifdef DEBUG 
				printf("[meta author]");
			#endif 
			break;
            default: printf("[...]");
        }

	#ifdef DEBUG
    		printf("\n");
	#endif

}

