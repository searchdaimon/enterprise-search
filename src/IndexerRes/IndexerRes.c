/*****************************************/
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "../common/langdetect.h"
#include "../common/bstr.h"
#include "../common/langToNr.h"
#include "../IndexerRes/IndexerRes.h"
#include "../common/utf8-strings.h"

#include "../common/boithohome.h"

struct AdultFraserRecordFormat {
	char word1[128];
	char word2[128];	
	int weight;
};

void wordsInit(struct pagewordsFormat *pagewords) {

	(*pagewords).outlinks = malloc(sizeof(struct outlinksFormat) * IndexerMaxLinks);	

}	

//kalles sist når man skal slutte og bruke en pagewords. Free'er det som er alokert
void wordsEnd(struct pagewordsFormat *pagewords) {
	free((*pagewords).outlinks);
}
void wordsReset(struct pagewordsFormat *pagewords,unsigned int DocID) {

	(*pagewords).nr = 0;
	(*pagewords).nextPosition = 0;	
	(*pagewords).revIndexnr = 0;
	(*pagewords).nrOfOutLinks = 0;
	(*pagewords).lasturl[0] = '\0';
	(*pagewords).DocID = DocID;

	#ifdef BLACK_BOKS
		(*pagewords).aclnr = 0;
	#endif
}

void linksWrite(struct pagewordsFormat *pagewords,struct addNewUrlhaFormat addNewUrlha[]) {
	int i, len;

	struct updateFormat updatePost;

	for(i=0;i<(*pagewords).nrOfOutLinks;i++) {

		if ((IndexerMaxLinks)> i) {

			if ((*pagewords).outlinks[i].good) {

				len = (*pagewords).outlinks[i].linktextlen;
				//fjerner space på slutten
				if (len > 0) {
					(*pagewords).outlinks[i].linktext[len -1] = '\0';
				}	

				//tømmer minne for å gjøre filene mer komprimeringsvendlige
				memset(updatePost.url,'\0',sizeof(updatePost.url));
				memset(updatePost.linktext,'\0',sizeof(updatePost.linktext));

				strscpy((char *)updatePost.url,(*pagewords).outlinks[i].url,sizeof((*pagewords).outlinks[i].url));
				strscpy((char *)updatePost.linktext,(*pagewords).outlinks[i].linktext,sizeof( (*pagewords).outlinks[i].linktext ));
				updatePost.DocID_from = (*pagewords).curentDocID;

				//printf("linksWrite: \"%s\", text \"%s\", len %i\n",updatePost.url,updatePost.linktext,len);
				//bruker sidens DocID til å velge hvilken fil. Usikker på om det er lurt, men det er lett og implementere for nå
				addNewUrl(&addNewUrlha[(*pagewords).curentDocID % NEWURLFILES_NR],&updatePost);			

			}
		}
	}

}


void linkwordadd(struct pagewordsFormat *pagewords, char word[]) {


	int wordlLength;

	//printf("link word \"%s\"\n",word);

	wordlLength = strlen(word);

	convert_to_lowercase((unsigned char *)word);
	
	//-1 så vi ikke appender alt på slutten av siste linken hvis vi har mr en IndexerMaxLinks
	if ((IndexerMaxLinks)> (*pagewords).nrOfOutLinks) {

		if (!(*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].good) {
			//dårlig link
		}
		else if (((*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktextlen + wordlLength +2 ) > sizeof((*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktext)) {
			//har ikke mer plass til link tekst
		}
		else {
			//må ha -1 her da det er den nåverende vi jobber på, ikke den neste
			strcpy((*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktext + (*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktextlen,word);
			(*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktextlen += wordlLength;
			strcpy((*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktext + (*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktextlen," ");
			(*pagewords).outlinks[(*pagewords).nrOfOutLinks -1].linktextlen += 1;
		}
	}	
	else {
		#ifdef DEBUG
		printf("note: to many links. Has %i sow far\n",(*pagewords).nrOfOutLinks);
		#endif
	}

}

#ifdef BLACK_BOKS
void acladd(struct pagewordsFormat *pagewords, char word[]) {

	convert_to_lowercase((unsigned char *)word);
	printf("acladd: got \"%s\"\n",word);

	if ((*pagewords).aclnr > maxAclForPage){
        	#ifdef DEBUG
                	printf("mor then maxAclForPage words\n");
               #endif
        }
        else {

		(*pagewords).acls[(*pagewords).aclnr].WordID =  crc32boitho(word);
		(*pagewords).acls[(*pagewords).aclnr].position = 0;

		++(*pagewords).aclnr;
	}
}
#endif

void linkadd(struct pagewordsFormat *pagewords, char word[]) {

		int i;
		char url[201];

		strscpy(url,word,sizeof(url));

		//printf("linkadd. DocID %u, link \"%s\"\n",(*pagewords).DocID,url);


		if (IndexerMaxLinks > (*pagewords).nrOfOutLinks) {

			//inaliserer den til å våre dårlig
			(*pagewords).outlinks[(*pagewords).nrOfOutLinks].good = 0;

			url_normalization (url,sizeof(url));

			//temp: butt ut linje 1 med linje 2 når wiki bugen er reparset
			//if (!globalIndexerLotConfig.collectUrls) {
			if ((!globalIndexerLotConfig.collectUrls) && (!isWikiUrl(word))) {
				//skal ikke lagge til urler
			}
			//else if (strcmp((*pagewords).lasturl,url) == 0) {
			//	//gider ikke legge til like url. Men hva hvis dette er på en ny side. Denne burde bli resettet			
			//}
			else if ((strchr(url,'?') != NULL) && ((*pagewords).curentUrlIsDynamic)) {
                	        //printf("NO add, dynamic -> dunamic %s\n",(*pagewords).updatePost[i].url);
                	}
			else if (!gyldig_url(url)) {
				//printf("bad url: \"%s\"\n",url);
			}
			else if (!isOkTttl(url)) {
				//printf("bad ttl: \"%s\"\n",url);
			}
			else {



				//(*pagewords).updatePost[(*pagewords).nrOfOutLinks].DocID_from = (*pagewords).curentDocID;
				 strscpy((char *)(*pagewords).outlinks[(*pagewords).nrOfOutLinks].url,url,sizeof((*pagewords).outlinks[(*pagewords).nrOfOutLinks].url));
				(*pagewords).outlinks[(*pagewords).nrOfOutLinks].linktext[0] = '\0';
				(*pagewords).outlinks[(*pagewords).nrOfOutLinks].linktextlen = 0;
				(*pagewords).outlinks[(*pagewords).nrOfOutLinks].good = 1;
			}
		}

		//øker oversikten over antall utgående linker
		++(*pagewords).nrOfOutLinks;



	

		/*

                struct updateFormat updatePost;

		if (!globalIndexerLotConfig.collectUrls) {
			//skal ikke lagge til urler
		}
		else if (strcmp((*pagewords).lasturl,word) == 0) {
			//gider ikke legge til like url. Men hva hvis dette er på en ny side. Denne burde bli resettet			
		}
		else if ((strchr(word,'?') != NULL) && ((*pagewords).curentUrlIsDynamic)) {
                        //printf("NO add %s\n",word);
                }
		else if (gyldig_url(word)) {

			
			//printf("ADD %s\n",word);
                      
			strncpy((char *)updatePost.url,word,sizeof(updatePost.url));
			//ToDo: se fn() også. Ser ut til å mangle håntering av anker tekst her??
                        strncpy((char *)updatePost.linktext,"",sizeof(updatePost.linktext));
                        updatePost.DocID_from = (*pagewords).curentDocID;

			if (globalIndexerLotConfig.urlfilter == NULL) {
                        	addNewUrl(&global_addNewUrlha_pri1,&updatePost);
			}
			else {
				//tester filterer om vi skal legge til en url
				i=0;
				while( (globalIndexerLotConfig.urlfilter[i] != NULL) ) {
	                                //printf("\t\t%i\tis ttl \"%s\" in url \"%s\"\n", i, globalIndexerLotConfig.urlfilter[i],word);
					if (url_isttl(word,globalIndexerLotConfig.urlfilter[i])) {
						//printf("added url\n");
						addNewUrl(&global_addNewUrlha_pri1,&updatePost);
						break;
					}
					++i;
                                }				
			}
			

			//if (url_havpri1(word) || global_source_url_havpri) {
			//	//printf("hav pri %s\n",word);
                        //	addNewUrl(&global_addNewUrlha_pri1,&updatePost,"_pri1",subname);
			//}
			//
			//temp: skrur av crawling av ikke pri sider
			//else if (url_havpri2(word)) {
			//	//printf("source_url_havpri %s\n",word);
                        //        addNewUrl(&global_addNewUrlha_pri2,&updatePost,"_pri2",subname);
			//}
			//else {
			//	//printf("normal %s\n",word);
                        //	addNewUrl(&global_addNewUrlha,&updatePost,"",subname);
			//
			//}
			

                }
		else {
			//printf("NO ADD %s\n",word);
		}

		strncpy((*pagewords).lasturl,word,sizeof((*pagewords).lasturl));

		*/
}


void wordsAdd(struct pagewordsFormat *pagewords, char word[],enum parsed_unit_flag puf) {

int i;
int wordlLength;
int wordTypeadd;
			if ((*pagewords).nr > maxWordForPage){
				#ifdef DEBUG
					printf("mor then maxWordForPage words\n");
				#endif
			}
			else {

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
					default:
						printf(" no catsh\n"); break;
                        	}

				wordlLength = strlen(word);

				//gjør om til små bokstaver
				//for(i=0;i<wordlLength;i++) {
				//	word[i] = (char)btolower(word[i]);
				//}
				convert_to_lowercase((unsigned char *)word);


				//#ifdef DEBUG_ADULT
				#ifdef PRESERVE_WORDS
					strcpy((*pagewords).words[(*pagewords).nr].word,word);
				#endif
				(*pagewords).words[(*pagewords).nr].WordID =  crc32boitho(word);
					#ifdef DEBUG
						printf(" (crc %s -> %u) ",word,(*pagewords).words[(*pagewords).nr].WordID);
					#endif				
				(*pagewords).words[(*pagewords).nr].position = ((*pagewords).nextPosition + wordTypeadd);
				// må ha en index posisjon her. Slik at vi kan finne ord før og etter. Posisjon er kodet
				(*pagewords).words[(*pagewords).nr].unsortetIndexPosition = (*pagewords).nr;

				//printf("nextPosition %i, wordTypeadd %i, position %i\n",(*pagewords).nextPosition,wordTypeadd,(*pagewords).words[(*pagewords).nr].position);

				++(*pagewords).nextPosition;

				//printf("%s : %lu\n",word,(*pagewords).words[(*pagewords).nr]);

				++(*pagewords).nr;		
			}
}

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf, void* pagewords )
{


	#ifdef DEBUG
    		printf("\t%s (%i) ", word, pos);
	#endif
    switch (pu)
        {
            case pu_word: 

			wordsAdd(pagewords,word,puf);

			#ifdef DEBUG
	    			switch (puf)
        			{
        			    	case puf_none: printf(" none"); break;
        			    	case puf_title: printf(" +title"); break;
        		    		case puf_h1: printf(" +h1"); break;
        		    		case puf_h2: printf(" +h2"); break;
        		    		case puf_h3: printf(" +h3"); break;
        		    		case puf_h4: printf(" +h4"); break;
        		    		case puf_h5: printf(" +h5"); break;
        		    		case puf_h6: printf(" +h6"); break;
		        	}

				printf("[word] is now %s ",word); 
			#endif


		break;
            case pu_linkword: 
			//på vep hånterer vi link tekst som ankertekst. På bb er det en del av dokumentet
			//ToDo: Vi ser ut til å mangle håntering av link ord her???
			//har det falt ut ??
			#ifdef BLACK_BOKS
				wordsAdd(pagewords,word,puf_none);
			#else
				linkwordadd(pagewords,word);
			#endif

			#ifdef DEBUG
				printf("[linkword]"); 
			#endif
		break;
            case pu_link:
					
			linkadd(pagewords,word);
			
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
            default: printf("fn: Unknown type of word\n");
        }

	#ifdef DEBUG
    		printf("\n");
	#endif

}



/**************************************/


int compare_elements_AdultFraserRecordFormat (const void *p1, const void *p2) {
	return (strcmp(((struct AdultFraserRecordFormat *)p1)->word1,((struct AdultFraserRecordFormat *)p2)->word1));
}
int compare_elements_words (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

        if (((struct wordsFormat*)p1)->WordID < ((struct wordsFormat*)p2)->WordID)
                return -1;
        else
                return ((struct wordsFormat*)p1)->WordID > ((struct wordsFormat*)p2)->WordID;

}

int compare_elements_nr (const void *p1, const void *p2) {


        if (((struct revIndexFomat*)p1)->nr < ((struct revIndexFomat*)p2)->nr)
                return -1;
        else
                return ((struct revIndexFomat*)p1)->nr > ((struct revIndexFomat*)p2)->nr;

}



static int cmp1_crc32(const void *p, const void *q)
{
	printf("%lu %lu\n",*(const unsigned long *) p,*(const unsigned long *) q);

   return *(const unsigned long *) p - *(const unsigned long *) q;
}

//void pagewordsSortOnOccurrence(){
//	qsort(&pagewords.revIndex, pagewords.revIndexnr , sizeof(struct revIndexFomat), compare_elements_nr);
//
//}
#ifdef BLACK_BOKS
void aclsMakeRevIndex(struct pagewordsFormat *pagewords) {

	int i;
	
	printf("aclsMakeRevIndex: aclns %i\n",(*pagewords).aclnr);
	for(i=0;i<(*pagewords).aclnr;i++) {
		printf("aclsMakeRevIndex: crc32: %u\n",(*pagewords).acls[i].WordID);
		(*pagewords).acls_sorted[i] = (*pagewords).acls[i];
	}

	//sorter ordene
	qsort(&(*pagewords).acls_sorted, (*pagewords).aclnr , sizeof(struct wordsFormat), compare_elements_words);

	(*pagewords).aclIndexnr = 0;
	for(i=0;i<(*pagewords).aclnr;i++) {

		(*pagewords).aclIndex[(*pagewords).aclIndexnr].WordID = (*pagewords).acls_sorted[i].WordID;
		(*pagewords).aclIndex[(*pagewords).aclIndexnr].nr = 0;
		//(*pagewords).aclIndex[(*pagewords).aclIndexnr].hits[(*pagewords).aclIndex[(*pagewords).aclIndexnr].nr] = (*pagewords).acls_sorted[i].position;

		++(*pagewords).aclIndexnr;
	}
}
#endif
/****************************************************/
void wordsMakeRevIndex(struct pagewordsFormat *pagewords, struct adultFormat *adult,int *adultWeight, unsigned char *langnr) {

	int i,y,adultpos,adultFraserpos;
	unsigned long oldcrc32;
	int oldRevIndexnr;

	//kopierer over til den word arrayn som skal være sortert
	for(i=0;i<(*pagewords).nr;i++) {
		//(*pagewords).words_sorted[i].WordID = (*pagewords).words[i].WordID;
		//(*pagewords).words_sorted[i].position = (*pagewords).words[i].position;
		(*pagewords).words_sorted[i] = (*pagewords).words[i];
		//#ifdef DEBUG_ADULT
		#ifdef PRESERVE_WORDS
			strcpy((*pagewords).words_sorted[i].word,(*pagewords).words[i].word);
		#endif
	}

	//sorter ordene
	qsort(&(*pagewords).words_sorted, (*pagewords).nr , sizeof(struct wordsFormat), compare_elements_words);

	//finner språk
	char lang[4];
	langdetectDetect((*pagewords).words_sorted,(*pagewords).nr,lang);

	*langnr = getLangNr(lang);

	oldcrc32 = 0;	
	adultpos = 0;
	adultFraserpos = 0;
	(*adultWeight) = 0;
	(*pagewords).revIndexnr = 0;
	for(i=0;i<(*pagewords).nr;i++) {
		
		//printf("oo: %lu %i\n",(*pagewords).words_sorted[i].WordID,(*pagewords).words_sorted[i].position);

		//printf("t%i\n",(*pagewords).revIndexnr);

		if (((*pagewords).words_sorted[i].WordID != oldcrc32) || (oldcrc32 == 0)) {
			// nytt ord, skal lages nytt
			#ifdef DEBUG_ADULT
				printf("new word. Word \"%s\"\n",(*pagewords).words_sorted[i].word);
			#endif
			#ifdef DEBUG
				printf("new word. WordID \"%u\"\n",(*pagewords).words_sorted[i].WordID);
			#endif

			(*pagewords).revIndex[(*pagewords).revIndexnr].WordID = (*pagewords).words_sorted[i].WordID;
			(*pagewords).revIndex[(*pagewords).revIndexnr].nr = 0;
			(*pagewords).revIndex[(*pagewords).revIndexnr].hits[(*pagewords).revIndex[(*pagewords).revIndexnr].nr] = (*pagewords).words_sorted[i].position;

			//#ifdef DEBUG_ADULT			
			#ifdef PRESERVE_WORDS
				strcpy((*pagewords).revIndex[(*pagewords).revIndexnr].word,(*pagewords).words_sorted[i].word);
				(*pagewords).revIndex[(*pagewords).revIndexnr].wordnr = 1;
				//printf("word %lu %s\n",(*pagewords).revIndex[(*pagewords).revIndexnr].WordID,(*pagewords).words_sorted[i].word);
			#endif


			#ifndef BLACK_BOKS
			///////////////////////////
			// adultWords


			while (((*pagewords).revIndex[(*pagewords).revIndexnr].WordID 
				> (*adult).AdultWords[adultpos].crc32) 
				&& (adultpos < (*adult).adultWordnr)) {
				#ifdef DEBUG
					//printf("testing for %lu %s\n",(*adult).AdultWords[adultpos].crc32,(*adult).AdultWords[adultpos].word);
				#endif
				++adultpos;
			}
			//printf("ll: adultpos %i < adultWordnr %i\n",adultpos,(*adult).adultWordnr);
			if (adultpos == (*adult).adultWordnr) {
				//nåd enden
				//ToDo: vi får fortsat med siste ord her, ikke sant?
				//printf("adultpos %i, (*adult).adultWordnr %i\n",adultpos,(*adult).adultWordnr);
			}
			else if ((*adult).AdultWords[adultpos].crc32 == (*pagewords).revIndex[(*pagewords).revIndexnr].WordID) {
				#if DEBUG_ADULT
					printf("adult hit \"%s\" %i\n",(*adult).AdultWords[adultpos].word,(*adult).AdultWords[adultpos].weight);
				#endif
				(*adultWeight) += (*adult).AdultWords[adultpos].weight;
				//printf("adultWeight1 %u\n",(*adultWeight));
				//printf("weight %i\n",(*adult).AdultWords[adultpos].weight);

				//exit(1);
			}
			///////////////////////////
			//adult fraser

			while (((*pagewords).revIndex[(*pagewords).revIndexnr].WordID > (*adult).adultFraser[adultFraserpos].crc32) && (adultFraserpos < (*adult).adultWordFrasernr)) {
				#ifdef DEBUG
					//printf("s %lu %s\n",(*adult).adultFraser[adultFraserpos].crc32,(*adult).adultFraser[adultFraserpos].word);
				#endif
				++adultFraserpos;
			}
			if ((*adult).adultFraser[adultFraserpos].crc32 == (*pagewords).revIndex[(*pagewords).revIndexnr].WordID) {
				
				//har hitt. Lopper gjenom word2'ene på jakt etter ord nr to
				#ifdef DEBUG_ADULT
					printf("word is first in frase %s\n",(*adult).adultFraser[adultFraserpos].word);
					printf("word pos %i\n",(*pagewords).words_sorted[i].unsortetIndexPosition);
					printf("bb nex word is \"%s\"\n",(*pagewords).words[((*pagewords).words_sorted[i].unsortetIndexPosition +1)].word);
					printf("x words to try %i, pos %i\n",(*adult).adultFraser[adultFraserpos].adultWordCount,
								adultFraserpos);
				#endif

				for(y=0;y<(*adult).adultFraser[adultFraserpos].adultWordCount;y++) {

					//if ((*adult).adultFraser[adultFraserpos].adultWord[y].crc32 == (*pagewords).words[(*pagewords).words_sorted[i].position +1].WordID) {
					if ((*adult).adultFraser[adultFraserpos].adultWord[y].crc32 == (*pagewords).words[((*pagewords).words_sorted[i].unsortetIndexPosition +1)].WordID) {
						#ifdef DEBUG_ADULT
							printf("frase hit \"%s %s\", weight %i\n",(*pagewords).words_sorted[i].word,(*adult).adultFraser[adultFraserpos].adultWord[y].word,(*adult).adultFraser[adultFraserpos].adultWord[y].weight);
						#endif
						//øker adult verdien med vekt
						(*adultWeight) += (*adult).adultFraser[adultFraserpos].adultWord[y].weight;
						//printf("adultWeight2 %u\n",(*adultWeight));
						// 1 juli. Blir feil og gå ut av loopen her da vi kan ha treff på flere fraser.
						//break;
					}
				}

			}

			///////////////////////////
			#endif

			oldRevIndexnr = (*pagewords).revIndexnr;
			//har fåt et til ord i revindex
			++(*pagewords).revIndex[(*pagewords).revIndexnr].nr;

			//printf("b%i\n\n",(*pagewords).revIndexnr);
			++(*pagewords).revIndexnr;
		}
		else {
			#ifdef DEBUG
				printf("word seen befor. Adding. WordID \"%u\"\n",(*pagewords).words_sorted[i].WordID);
			#endif

			#ifdef PRESERVE_WORDS
				++(*pagewords).revIndex[oldRevIndexnr].wordnr;
			#endif
			// ord er set før, skal legges til det tidligere
			// må passe på at vi ikke overskriver med flere hits en MaxsHits
			if ((*pagewords).revIndex[oldRevIndexnr].nr < MaxsHitsInIndex) {
				(*pagewords).revIndex[oldRevIndexnr].hits[(*pagewords).revIndex[oldRevIndexnr].nr] = (*pagewords).words_sorted[i].position;
				++(*pagewords).revIndex[oldRevIndexnr].nr;
			}
			else {
				#ifdef DEBUG
				printf("to many hits for WordID \"%u\"\n",(*pagewords).words_sorted[i].WordID);
				#endif
			}
			//break;

		}

		oldcrc32 = (*pagewords).words_sorted[i].WordID;

	
	}


}




/**************************/

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

	#ifdef BLACK_BOKS
		printf("Note: Wont load adult filter. Is abb\n");
	#else

	FILE *FH;
	char buff[128];
	int i,y,x;
	char *cpoint;
	unsigned long crc32tmp;

	//AdultWordsFile
	if ((FH = bfopen(AdultWordsVektetFile,"r")) == NULL) {
                        perror(AdultWordsVektetFile);
                        exit(1);
	}

	i=0;
	while ((fgets(buff,sizeof(buff),FH) != NULL) && (i < maxAdultWords)) {
		//fjerner \n en på slutteten
		buff[strlen(buff) -1] = '\0';

		if ((buff[0] == '#') || (buff[0] == '\0')) {
			//printf("bad line or comment: %s\n",buff);
			continue;
		}

		//gjør om til lite case
		for(x=0;x<strlen(buff);x++) {
			buff[x] = btolower(buff[x]);
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
	if (maxAdultWords == i) {
		printf("Adult list is larger then maxAdultWords (%i)\n",maxAdultWords);
		exit(1);
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
	if ((FH = bfopen(AdultFraserVektetFile,"r")) == NULL) {
                        perror(AdultFraserVektetFile);
                        exit(1);
	}

	y=0;
	struct AdultFraserRecordFormat AdultFraserRecords[maxAdultWords];

	while ((fgets(buff,sizeof(buff) -1,FH) != NULL) && (y < maxAdultWords)) {
                //gjør om til lite case
                for(x=0;x<strlen(buff);x++) {
                        buff[x] = btolower(buff[x]);
                }

		if (buff[0] == '#' || buff[0] == '\n') {
			//printf("comment \"%s\"\n",buff);
			continue;
		}

		//printf("buff %s\n",buff);
		if ((x=sscanf(buff,"%s %s %i\n",AdultFraserRecords[y].word1,AdultFraserRecords[y].word2,&AdultFraserRecords[y].weight))!=3) {
			
			printf("bad AdultFraserVektetFile format: \"%s\" . x: %i\n",buff, x);
			exit(1);
		}
		++y;
	}


	qsort(AdultFraserRecords, y , sizeof(struct AdultFraserRecordFormat), compare_elements_AdultFraserRecordFormat);
	
	i=-1;	
	for (x=0;x<y;x++) {

			#ifdef DEBUG
			printf("%i: %s, %s, %i\n",i,AdultFraserRecords[x].word1,AdultFraserRecords[x].word2,AdultFraserRecords[x].weight);
			#endif	
			//finner crc32 verdeien for første ord
			crc32tmp = crc32boitho(AdultFraserRecords[x].word1);

			//hvsi dette er første så her vi ikke noen forige å legge den til i, så vi må opprette ny
			//hvsi dette derimot har samme word1 som forige så legger vi det til
			if ((i!=-1) && (crc32tmp == (*adult).adultFraser[i].crc32)) {
				//printf("nr to\n");
			}
			else {
				++i;
			}
		

			strcpy((*adult).adultFraser[i].word,AdultFraserRecords[x].word1);
			(*adult).adultFraser[i].crc32 = crc32boitho(AdultFraserRecords[x].word1);		

			(*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].weight = AdultFraserRecords[x].weight;
			strcpy((*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].word,AdultFraserRecords[x].word2);
			(*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].crc32 = crc32boitho(AdultFraserRecords[x].word2);

			if ((*adult).adultFraser[i].adultWordCount < MaxAdultWordCount -1) {
				++(*adult).adultFraser[i].adultWordCount;
			}
			else {
				printf("MaxAdultWordCount %i for %s\n",MaxAdultWordCount,AdultFraserRecords[x].word1);
				exit(1);
			}

			



	}
	fclose(FH);

	(*adult).adultWordFrasernr = i;
	qsort((*adult).adultFraser, (*adult).adultWordFrasernr , sizeof(struct adultWordFraserFormat), compare_elements_AdultFraser);

	#ifdef DEBUG
	for(i=0;i<(*adult).adultWordFrasernr;i++) {
		printf("nr %i, word: \"%s\", subwords %i\n",i,(*adult).adultFraser[i].word,(*adult).adultFraser[i].adultWordCount);

		for(y=0;y<(*adult).adultFraser[i].adultWordCount;y++) {
			printf("\t %i: %s-%s: %i\n",y,(*adult).adultFraser[i].word,(*adult).adultFraser[i].adultWord[y].word,(*adult).adultFraser[i].adultWord[y].weight);
		}
		

	}
	#endif
	

	#endif
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




//copy a memory area, and return the size copyed
static inline size_t memcpyrc(void *s1, const void *s2, size_t n) {
//size_t memcpyrc(void *s1, const void *s2, size_t n) {
        memcpy(s1,s2,n);

        return n;
}

#ifdef BLACK_BOKS
void aclsMakeRevIndexBucket (struct pagewordsFormat *pagewords,unsigned int DocID,unsigned char *langnr) {


	int i,y;

	for(i=0;i<NrOfDataDirectorys;i++) {
		(*pagewords).nrofAclBucketElements[i].records = 0;
		(*pagewords).nrofAclBucketElements[i].hits = 0;
	}

	//printf("aclIndexnr %i\n",(*pagewords).aclIndexnr);
	for(i=0;i<(*pagewords).aclIndexnr;i++) {

		(*pagewords).aclIndex[i].bucket = (*pagewords).aclIndex[i].WordID % NrOfDataDirectorys;

		++(*pagewords).nrofAclBucketElements[(*pagewords).aclIndex[i].bucket].records;
		(*pagewords).nrofAclBucketElements[(*pagewords).aclIndex[i].bucket].hits += (*pagewords).aclIndex[i].nr;
	}

	for(i=0;i<NrOfDataDirectorys;i++) {
		(*pagewords).nrofAclBucketElements[i].bucketbuffsize = ((sizeof(unsigned int) + sizeof(char) + sizeof(unsigned long) + sizeof(unsigned long)) * (*pagewords).nrofAclBucketElements[i].records) + ((*pagewords).nrofAclBucketElements[i].hits * sizeof(unsigned short));
		//printf("bucketbuffsize %i\n",(*pagewords).nrofaclBucketElements[i].bucketbuffsize);

		(*pagewords).nrofAclBucketElements[i].bucketbuff = malloc((*pagewords).nrofAclBucketElements[i].bucketbuffsize);
	}

	//setter pekeren til begyndelsen. Siden vil vi jo flytte denne etter hvert som vi kommer lenger ut
	for(i=0;i<NrOfDataDirectorys;i++) {
		(*pagewords).nrofAclBucketElements[i].p = (*pagewords).nrofAclBucketElements[i].bucketbuff;
	}

	//bruker en temperær p peker her som erstatning for (*pagewords).nrofaclBucketElements[(*pagewords).aclIndex[i].bucket].p, 
	//så koden ikke blir så uoversiktelig
	void *p;
	for(i=0;i<(*pagewords).aclIndexnr;i++) {
		
			p = (*pagewords).nrofAclBucketElements[(*pagewords).aclIndex[i].bucket].p;

			p += memcpyrc(p,&DocID,sizeof(unsigned int));
			p += memcpyrc(p,langnr,sizeof(char));
			p += memcpyrc(p,&(*pagewords).aclIndex[i].WordID,sizeof(unsigned long));
			p += memcpyrc(p,&(*pagewords).aclIndex[i].nr,sizeof(unsigned long));
			for(y=0;y<(*pagewords).aclIndex[i].nr;y++) {
				p += memcpyrc(p,&(*pagewords).aclIndex[i].hits[y],sizeof(unsigned short));
			}

			(*pagewords).nrofAclBucketElements[(*pagewords).aclIndex[i].bucket].p = p;
		
	}

}

#endif
void wordsMakeRevIndexBucket (struct pagewordsFormat *pagewords,unsigned int DocID,unsigned char *langnr) {

	int i,y;


	for(i=0;i<NrOfDataDirectorys;i++) {
		(*pagewords).nrofBucketElements[i].records = 0;
		(*pagewords).nrofBucketElements[i].hits = 0;
	}

	//printf("revIndexnr %i\n",(*pagewords).revIndexnr);
	for(i=0;i<(*pagewords).revIndexnr;i++) {

		(*pagewords).revIndex[i].bucket = (*pagewords).revIndex[i].WordID % NrOfDataDirectorys;

		++(*pagewords).nrofBucketElements[(*pagewords).revIndex[i].bucket].records;
		(*pagewords).nrofBucketElements[(*pagewords).revIndex[i].bucket].hits += (*pagewords).revIndex[i].nr;
	}
	
	for(i=0;i<NrOfDataDirectorys;i++) {
		(*pagewords).nrofBucketElements[i].bucketbuffsize = ((sizeof(unsigned int) + sizeof(char) + sizeof(unsigned long) + sizeof(unsigned long)) * (*pagewords).nrofBucketElements[i].records) + ((*pagewords).nrofBucketElements[i].hits * sizeof(unsigned short));
		//printf("bucketbuffsize %i\n",(*pagewords).nrofBucketElements[i].bucketbuffsize);

		(*pagewords).nrofBucketElements[i].bucketbuff = malloc((*pagewords).nrofBucketElements[i].bucketbuffsize);
	}

	//setter pekeren til begyndelsen. Siden vil vi jo flytte denne etter hvert som vi kommer lenger ut
	for(i=0;i<NrOfDataDirectorys;i++) {
		(*pagewords).nrofBucketElements[i].p = (*pagewords).nrofBucketElements[i].bucketbuff;
	}

	//bruker en temperær p peker her som erstatning for (*pagewords).nrofBucketElements[(*pagewords).revIndex[i].bucket].p, 
	//så koden ikke blir så uoversiktelig
	void *p;
	for(i=0;i<(*pagewords).revIndexnr;i++) {
		
			p = (*pagewords).nrofBucketElements[(*pagewords).revIndex[i].bucket].p;

			p += memcpyrc(p,&DocID,sizeof(unsigned int));
			p += memcpyrc(p,langnr,sizeof(char));
			p += memcpyrc(p,&(*pagewords).revIndex[i].WordID,sizeof(unsigned long));
			p += memcpyrc(p,&(*pagewords).revIndex[i].nr,sizeof(unsigned long));
			for(y=0;y<(*pagewords).revIndex[i].nr;y++) {
				p += memcpyrc(p,&(*pagewords).revIndex[i].hits[y],sizeof(unsigned short));
			}

			(*pagewords).nrofBucketElements[(*pagewords).revIndex[i].bucket].p = p;
		
	}



}
void dictionaryWordsWrite (struct pagewordsFormat *pagewords,FILE *FH) {

	int i;

	#ifdef PRESERVE_WORDS
		for(i=0;i<(*pagewords).revIndexnr;i++) {
			#ifdef DEBUG
			printf("word: \"%s\": %i\n",(*pagewords).revIndex[i].word,(*pagewords).revIndex[i].wordnr);
			#endif
			fprintf(FH,"%s %i\n",(*pagewords).revIndex[i].word,(*pagewords).revIndex[i].wordnr);
		}
	#endif

}

/**************************************************************************************
Skriver acl index til disk
***************************************************************************************/
#ifdef BLACK_BOKS
void aclindexFilesAppendWords(struct pagewordsFormat *pagewords,FILE *aclindexFilesHa[],unsigned int DocID,unsigned char *langnr) {

	int i,y;
	int bucket;


	for(i=0;i<NrOfDataDirectorys;i++) {
		if ((*pagewords).nrofAclBucketElements[i].bucketbuffsize != 0) {
			fwrite((*pagewords).nrofAclBucketElements[i].bucketbuff,(*pagewords).nrofAclBucketElements[i].bucketbuffsize,1,aclindexFilesHa[i]);
		}
	}


	for(i=0;i<NrOfDataDirectorys;i++) {
			free((*pagewords).nrofAclBucketElements[i].bucketbuff);
		
	}

}
#endif
/**************************************************************************************
Skriver reversert index til disk
***************************************************************************************/
void revindexFilesAppendWords(struct pagewordsFormat *pagewords,FILE *revindexFilesHa[],unsigned int DocID,unsigned char *langnr) {

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


	for(i=0;i<NrOfDataDirectorys;i++) {
		if ((*pagewords).nrofBucketElements[i].bucketbuffsize != 0) {
			fwrite((*pagewords).nrofBucketElements[i].bucketbuff,(*pagewords).nrofBucketElements[i].bucketbuffsize,1,revindexFilesHa[i]);
		}
	}


	for(i=0;i<NrOfDataDirectorys;i++) {
			free((*pagewords).nrofBucketElements[i].bucketbuff);
		
	}



/*
	for(i=0;i<(*pagewords).revIndexnr;i++) {


		bucket = (*pagewords).revIndex[i].WordID % NrOfDataDirectorys;

		#ifdef DEBUG
			printf("WordID %lu forekomster %i (+1). bucket %i\n",(*pagewords).revIndex[i].WordID,(*pagewords).revIndex[i].nr,bucket);
		#endif

		//++(*pagewords).revIndex[i].nr;
		fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[bucket]);
		//fprintf(revindexFilesHa[bucket],"aa ");
		fwrite(langnr,sizeof(char),1,revindexFilesHa[bucket]);
		//printf("lang1 %i%\n",(int)*langnr);

		fwrite(&(*pagewords).revIndex[i].WordID,sizeof(unsigned long),1,revindexFilesHa[bucket]);	
		fwrite(&(*pagewords).revIndex[i].nr,sizeof(unsigned long),1,revindexFilesHa[bucket]);

		for(y=0;y<(*pagewords).revIndex[i].nr;y++) {
			//printf("\thits %i\n",(*pagewords).revIndex[i].hits[y]);
			fwrite(&(*pagewords).revIndex[i].hits[y],sizeof(unsigned short),1,revindexFilesHa[bucket]);
		}

	}	
*/
	/*
	//skriver record terminator
	for(i=0;i<NrOfDataDirectorys;i++) {
		//ToDo: her bruker vi \n for linefeed, men bruker \cJ i perl. På andre platformer en *nix vil det føre til problmer
		//	erstatt \n med tegnet for linefeed
		fprintf(revindexFilesHa[i],"**\n");
	}
	*/
}


void html_parser_timout( int signo )
{
    if ( signo == SIGALRM ) {
        printf("got alarm\n");
        alarm_got_raised = 1;
    }
 

}



void handelPage(struct pagewordsFormat *pagewords, unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, 
		char HtmlBuffer[],int HtmlBufferLength,struct DocumentIndexFormat *DocumentIndexPost, 
		int DocID,int httpResponsCodes[], struct adultFormat *adult,
		char **title, char **body) {

		//int AdultWeight;
		//char *title = NULL;
		//char *body = NULL;


		//printf("%lu %s\n",(*ReposetoryHeader).DocID, (*ReposetoryHeader).url);


		

					//setter opp en alarm slik at run_html_parser blir avbrut hvis den henger seg 
					//alarm_got_raised = 0;
					//signal(SIGALRM, html_parser_timout);
					
					//alarm( 5 );
					//parser htmlen
					//printf("html: %s\n\nUrl \"%s\"\nHtmlBufferLength %i\n",HtmlBuffer,(*ReposetoryHeader).url,HtmlBufferLength);
					//run_html_parser( (*ReposetoryHeader).url, HtmlBuffer, HtmlBufferLength, fn );
					//html_parser_run( "http://YAHOOgroups.com/svada/index.html", buf, size, &title, &body, fn, NULL );
					html_parser_run((*ReposetoryHeader).url,HtmlBuffer, HtmlBufferLength,title, body,fn,pagewords );
					//printf("title %s\n",(*title));
					//alarm( 0);
					//if(alarm_got_raised) {
					//	printf("run_html_parser did time out. At DocID %lu\n",(*ReposetoryHeader).DocID);
					//}
					//else {


						/*
						wordsMakeRevIndex(pagewords,adult,&AdultWeight,langnr);

						if (AdultWeight > 255) {
							(*DocumentIndexPost).AdultWeight = 255;
						}
						else {
							(*DocumentIndexPost).AdultWeight = AdultWeight;
						}

						revindexFilesAppendWords(pagewords,revindexFilesHa,(*ReposetoryHeader).DocID,langnr);
						*/
					//}
				

		
		//free(body);
		//free(title);
}


/******************************************/
