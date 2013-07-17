#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../common/define.h"
#include "../common/ir.h"
#include "../common/revindex.h"
#include "../common/url.h"
#include "../common/bstr.h"
#include "../common/lot.h"
#include "../common/utf8-strings.h"

#include "../common/reposetory.h"

#define DEBUG

int main (int argc, char *argv[]) {

	int lotNr;
	int i;
	unsigned int DocID;
	char text[50];
	unsigned int radress;
	unsigned int rsize;
	char **Data;
  	int Count, TokCount;
	unsigned short hits;
	unsigned long WordID;
	int bucket;
	int y;
	int nr;
	FILE *revindexFilesHa[NrOfDataDirectorys];
	unsigned char lang;
	FILE *FH;
	unsigned int DocIDPlace;

	int *nrOfLinkWordsToDocID = malloc(sizeof(int) * NrofDocIDsInLot);

	for (i=0;i<NrofDocIDsInLot;i++) {
		//begynner på 2000 så det skal være lett og skille de visuelt fra andre hits
		nrOfLinkWordsToDocID[i] = 2000;
	}
        //tester for at vi har fåt hvilken lot vi skal bruke
        if (argc < 3) {
                printf("Usage: ./anchorread lotnr subname\n\n");
		exit(1);
        }

	lotNr = atoi(argv[1]);
	char *subname = argv[2];

	if ( (FH = lotOpenFileNoCasheByLotNr(lotNr,"anchors","rb", 's',subname)) == NULL) {
		printf("lot dont have a anchors file\n");
		exit(1);
	}	
	fclose(FH);

	revindexFilesOpenLocal(revindexFilesHa,lotNr,"Anchor","wb",subname);

	//int anchorGetNext (int LotNr,unsigned int *DocID,char *text,unsigned int *radress,unsigned int *rsize)
	while (anchorGetNext(lotNr,&DocID,text,sizeof(text),&radress,&rsize,subname) ) {	

			DocIDPlace = (DocID - LotDocIDOfset(rLotForDOCid(DocID)));	
			++nrOfLinkWordsToDocID[DocIDPlace];



			convert_to_lowercase((unsigned char *)text);


			#ifdef DEBUG
			if (DocID == 4999999) {
				printf("DocID %i, text: \"%s\", DocIDPlace %i, nrOfLinkWordsToDocID %i\n",DocID,text,DocIDPlace,nrOfLinkWordsToDocID[DocIDPlace]);
			}
			#endif

  			if ((TokCount = split(text, " ", &Data)) == -1) {
				printf("canæt splitt \"%s\"\n",text);
			}

			//for (i=(TokCount-1);i>=0;i--) {
			i=0;
			while (Data[i] != NULL) {

				/*
				if (nrOfLinkWordsToDocID[DocIDPlace] > 65505) {
					#ifdef DEBUG
						if (DocID == 4999999) {
							printf("reach max nr of words for DocID %u. Hav %i+ words\n",DocID,nrOfLinkWordsToDocID[DocIDPlace]);
						}
					#endif
					break;
				}
				*/

				if (Data[i][0] == '\0') {
					#ifdef DEBUG
						if (DocID == 4999999) {

							printf("emty data element\n");
						}
					#endif
				} 
				else if (strcmp(Data[i],"www") == 0) {
					#ifdef DEBUG
						if (DocID == 4999999) {
							printf("www\n");
						}
					#endif
					++nrOfLinkWordsToDocID[DocIDPlace];
				} 
				else if (isStoppWord(Data[i])) {
					#ifdef DEBUG
						if (DocID == 4999999) {
							printf("stopword \"%s\"\n",Data[i]);
						}
					#endif
					//++nrOfLinkWordsToDocID[DocIDPlace];
				}
				else {
				
					#ifdef DEBUG
						if (DocID == 4999999) {
							printf("\t\"%s\" %i\n",Data[i],nrOfLinkWordsToDocID[DocIDPlace]);
						}
					#endif


			

					WordID = crc32boitho(Data[i]);

					if (WordID == 0) {
						printf("got 0 as word id for \"%s\". Somthing may be wrong.\n",Data[i]);
					}

                			bucket = WordID % NrOfDataDirectorys;

					if (nrOfLinkWordsToDocID[DocIDPlace] > 65535) {
						hits = 65535;
					}
					else {
						hits = nrOfLinkWordsToDocID[DocIDPlace];

					}

					#ifdef DEBUG
						if (DocID == 4999999) {
	    		       				printf("\thits %i: \"%s\": %hu, bucket %i\n",i,Data[i],hits,bucket);
						}
					#endif

                
        	        		if (fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[bucket]) != 1) {
						perror("fwrite DocID");
					}
					//runarb: 13 mai 2007. vi har byttet til å bruke et tal for språk.
					//burde da dette fra DocumentIndex hvis det finnes, men lagres ikke der
					//må si i IndexRes på hvordan vi gjør det der
        	        		//fprintf(revindexFilesHa[bucket],"aa ");
					lang = 0;
					nr = 1;
					if(fwrite(&lang,sizeof(unsigned char),1,revindexFilesHa[bucket]) != 1) {
						perror("fwrite lang");
					}


        	        		if(fwrite(&WordID,sizeof(unsigned long),1,revindexFilesHa[bucket]) != 1) {
						perror("fwrite WordID");
					}

        	        		if(fwrite(&nr,sizeof(unsigned long),1,revindexFilesHa[bucket]) != 1) {
						perror("fwrite nr");
					}


        		        	if(fwrite(&hits,sizeof(unsigned short),1,revindexFilesHa[bucket]) != 1) {
						perror("fwrite hits");
					}

			                
        	        		++nrOfLinkWordsToDocID[DocIDPlace];
				
			
				}


				++i;
			}
  			FreeSplitList(Data);


			#ifdef DEBUG
				if (DocID == 4999999) {
				printf("\n");
				}
			#endif
	}

	free(nrOfLinkWordsToDocID);

}

/*


*/
