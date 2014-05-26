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
#include "../common/attributes.h"

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

void wordsReset_part(struct pagewordsFormatPartFormat *wordsPart) {

	wordsPart->nr = 0;
	wordsPart->revIndexnr = 0;
	wordsPart->nextPosition = 0;	

}
void wordsReset(struct pagewordsFormat *pagewords,unsigned int DocID) {

	pagewords->nrOfOutLinks = 0;
	pagewords->lasturl[0] = '\0';
	pagewords->DocID = DocID;

	wordsReset_part(&pagewords->normalWords);
	wordsReset_part(&pagewords->linkWords);
	wordsReset_part(&pagewords->spamWords);

	#ifdef BLACK_BOX
		#ifdef IIACL
			(*pagewords).acl_allow.aclnr = 0;
			(*pagewords).acl_denied.aclnr = 0;
		#endif

		#ifdef ATTRIBUTES 
			(*pagewords).attrib.attribnr = 0;
		#endif

	#endif


}

#ifndef BLACK_BOX
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

				//bruker sidens DocID til å velge hvilken fil. Usikker på om det er lurt, men det er lett og implementere for nå
				addNewUrl(&addNewUrlha[(*pagewords).curentDocID % NEWURLFILES_NR],&updatePost);			

			}
		}
	}

}
#endif

#ifndef BLACK_BOX
void linkwordadd(struct pagewordsFormat *pagewords, char word[]) {


	int wordlLength;

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
#endif

#ifdef BLACK_BOX
void acladd(struct IndexerRes_acls *acl, char word[]) {


	convert_to_lowercase((unsigned char *)word);

	#ifdef DEBUG
	printf("acladd: got \"%s\"\n",word);
	#endif

	if ((*acl).aclnr > maxAclForPage){
        	#ifdef DEBUG
                	printf("mor then maxAclForPage words\n");
               	#endif
        }
        else {

		(*acl).acls[(*acl).aclnr].WordID =  crc32boitho(word);
		(*acl).acls[(*acl).aclnr].position = 0;

		++(*acl).aclnr;
	}
}

void attribadd(struct IndexerRes_attrib *attrib, char word[]) {

	convert_to_lowercase((unsigned char *)word);

	#ifdef DEBUG
	printf("attribadd: got \"%s\"\n",word);
	#endif

	if ((*attrib).attribnr > maxAttribForPage){
        	#ifdef DEBUG
                	printf("more than maxAttribForPage words\n");
               	#endif
        }
        else {

		(*attrib).attrib[(*attrib).attribnr].WordID =  crc32boitho(word);
		(*attrib).attrib[(*attrib).attribnr].position = 0;

		++(*attrib).attribnr;
	}
}
#endif

void linkadd(struct pagewordsFormat *pagewords, char word[]) {

		char url[201];

		strscpy(url,word,sizeof(url));


		if (IndexerMaxLinks > (*pagewords).nrOfOutLinks) {

			//inaliserer den til å våre dårlig
			(*pagewords).outlinks[(*pagewords).nrOfOutLinks].good = 0;

			url_normalization (url,sizeof(url));

			if (!globalIndexerLotConfig.collectUrls) {
				//skal ikke lagge til urler
			}
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

				 strscpy((char *)(*pagewords).outlinks[(*pagewords).nrOfOutLinks].url,url,sizeof((*pagewords).outlinks[(*pagewords).nrOfOutLinks].url));
				(*pagewords).outlinks[(*pagewords).nrOfOutLinks].linktext[0] = '\0';
				(*pagewords).outlinks[(*pagewords).nrOfOutLinks].linktextlen = 0;
				(*pagewords).outlinks[(*pagewords).nrOfOutLinks].good = 1;
			}
		}

		//øker oversikten over antall utgående linker
		++(*pagewords).nrOfOutLinks;

}


void wordsAdd(struct pagewordsFormatPartFormat *wordsPart, char word[],enum parsed_unit_flag puf) {

			int wordlLength;
			int wordTypeadd = 0;

			if (wordsPart->nr > maxWordForPage){
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
				convert_to_lowercase((unsigned char *)word);


				#ifdef PRESERVE_WORDS
					strcpy(wordsPart->words[wordsPart->nr].word,word);
				#endif

				wordsPart->words[wordsPart->nr].WordID =  crc32boitho(word);

				#ifdef DEBUG
					printf(" (crc %s -> %u) ",word,wordsPart->words[wordsPart->nr].WordID);
				#endif

				wordsPart->words[wordsPart->nr].position = (wordsPart->nextPosition + wordTypeadd);
				// må ha en index posisjon her. Slik at vi kan finne ord før og etter. Posisjon er kodet
				wordsPart->words[wordsPart->nr].unsortetIndexPosition = wordsPart->nr;


				++wordsPart->nextPosition;

				//printf("%s : %u\n",word,wordsPart->words[wordsPart->nr]);

				++wordsPart->nr;		
			}
}

void IndexerRes_fn( char* word, int __attribute__((unused))pos, enum parsed_unit pu, enum parsed_unit_flag puf, void *pagewords )
{


	#ifdef DEBUG
    		printf("\t%s (%i) ", word, pos);
	#endif
    switch (pu)
        {
            case pu_word: 

			wordsAdd(&((struct pagewordsFormat *)pagewords)->normalWords,word,puf);

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
			#ifdef BLACK_BOX
				wordsAdd(&((struct pagewordsFormat *)pagewords)->normalWords,word,puf_none);
			#else
				linkwordadd(pagewords,word);
				wordsAdd(&((struct pagewordsFormat *)pagewords)->linkWords,word,puf_none);

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
            case pu_cloaked_word:
			#ifdef DEBUG 
				printf("[pu_cloaked_word]");
			#endif 
				wordsAdd(&((struct pagewordsFormat *)pagewords)->spamWords,word,puf_none);
			break;
            case pu_cloaked_linkword:
			#ifdef DEBUG 
				printf("[pu_cloaked_linkword]");
			#endif 
				wordsAdd(&((struct pagewordsFormat *)pagewords)->spamWords,word,puf_none);
			break;
            default: printf("fn: Unknown type of word\n");
        }

	#ifdef DEBUG
    		printf("\n");
	#endif

}



int compare_elements_AdultFraserRecordFormat (const void *p1, const void *p2) {

	int n;

	n = strcmp(((struct AdultFraserRecordFormat *)p1)->word1,((struct AdultFraserRecordFormat *)p2)->word1);

	//hvis ordene ikke er like kan vi bare returnere
	if (n != 0) {
		return n;
	}
	else {
		//sorterer på ord 2
		return (strcmp(((struct AdultFraserRecordFormat *)p1)->word2,((struct AdultFraserRecordFormat *)p2)->word2));
	}

}
int compare_elements_words (const void *p1, const void *p2) {


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
	#ifdef DEBUG
	printf("%u %u\n",*(const unsigned int *) p,*(const unsigned int *) q);
	#endif

	return *(const unsigned int *) p - *(const unsigned int *) q;
}

#ifdef BLACK_BOX
void aclsMakeRevIndex(struct IndexerRes_acls *acl) {

	int i;
	unsigned int lastWodID;
	
	#ifdef DEBUG
	printf("aclsMakeRevIndex: aclns %i\n",(*acl).aclnr);
	#endif

	for(i=0;i<(*acl).aclnr;i++) {
		#ifdef DEBUG
		printf("aclsMakeRevIndex: crc32: %u\n",(*acl).acls[i].WordID);
		#endif
		(*acl).acls_sorted[i] = (*acl).acls[i];
	}

	//sorter ordene
	qsort(&(*acl).acls_sorted, (*acl).aclnr , sizeof(struct wordsFormat), compare_elements_words);

	(*acl).aclIndexnr = 0;
	lastWodID = 0;
	for(i=0;i<(*acl).aclnr;i++) {

		if (lastWodID == (*acl).acls_sorted[i].WordID) {
			#ifdef DEBUG
			printf("aclsMakeRevIndex: is the same as last WordId: %u\n",(*acl).acls_sorted[i].WordID);
			#endif
		}
		else {
			#ifdef DEBUG
			printf("aclsMakeRevIndex: adding crc32: %u\n",(*acl).acls_sorted[i].WordID);
			#endif

			(*acl).aclIndex[(*acl).aclIndexnr].WordID = (*acl).acls_sorted[i].WordID;
			(*acl).aclIndex[(*acl).aclIndexnr].nr = 0;
			lastWodID = (*acl).aclIndex[(*acl).aclIndexnr].WordID;
			++(*acl).aclIndexnr;
		}
	}
}


void attribMakeRevIndex(struct IndexerRes_attrib *attrib) {

	int i;
	unsigned int lastWordID;
	
	#ifdef DEBUG
	printf("attribMakeRevIndex: attribnr %i\n",(*attrib).attribnr);
	#endif

	for(i=0;i<(*attrib).attribnr;i++) {
		#ifdef DEBUG
		printf("attribMakeRevIndex: crc32: %u\n",(*attrib).attrib[i].WordID);
		#endif
		(*attrib).attrib_sorted[i] = (*attrib).attrib[i];
	}

	//sorter ordene
	qsort(&(*attrib).attrib_sorted, (*attrib).attribnr , sizeof(struct wordsFormat), compare_elements_words);

	(*attrib).attribIndexnr = 0;
	lastWordID = 0;
	for(i=0;i<(*attrib).attribnr;i++) {

		if (lastWordID == (*attrib).attrib_sorted[i].WordID) {
			#ifdef DEBUG
			printf("attribMakeRevIndex: is the same as last WordId: %u\n",(*attrib).attrib_sorted[i].WordID);
			#endif
		}
		else {
			#ifdef DEBUG
			printf("attribMakeRevIndex: adding crc32: %u\n",(*attrib).attrib_sorted[i].WordID);
			#endif

			(*attrib).attribIndex[(*attrib).attribIndexnr].WordID = (*attrib).attrib_sorted[i].WordID;
			(*attrib).attribIndex[(*attrib).attribIndexnr].nr = 0;
			lastWordID = (*attrib).attribIndex[(*attrib).attribIndexnr].WordID;
			++(*attrib).attribIndexnr;
		}
	}
}
#endif
/****************************************************/

void wordsMakeRevIndex_part(struct pagewordsFormatPartFormat *wordsPart,struct adultFormat __attribute__((unused))*adult,int __attribute__((unused))*adultWeight) {

	int i,adultpos,adultFraserpos;
	unsigned int oldcrc32;
	int oldRevIndexnr = 0;

	//kopierer over til den word arrayn som skal være sortert
	for(i=0;i<wordsPart->nr;i++) {

		wordsPart->words_sorted[i] = wordsPart->words[i];
		#ifdef PRESERVE_WORDS
			strcpy(wordsPart->words_sorted[i].word,wordsPart->words[i].word);
		#endif
	}

	//sorter ordene
	qsort(&wordsPart->words_sorted, wordsPart->nr , sizeof(struct wordsFormat), compare_elements_words);


	oldcrc32 = 0;	
	adultpos = 0;
	adultFraserpos = 0;
	wordsPart->revIndexnr = 0;
	for(i=0;i<wordsPart->nr;i++) {
		

		if ((wordsPart->words_sorted[i].WordID != oldcrc32) || (oldcrc32 == 0)) {
			// nytt ord, skal lages nytt
			#ifdef DEBUG_ADULT
				//printf("new word. Word \"%s\"\n",wordsPart->words_sorted[i].word);
			#endif
			#ifdef DEBUG
				printf("new word. WordID \"%u\"\n",wordsPart->words_sorted[i].WordID);
			#endif

			wordsPart->revIndex[wordsPart->revIndexnr].WordID = wordsPart->words_sorted[i].WordID;
			wordsPart->revIndex[wordsPart->revIndexnr].nr = 0;
			wordsPart->revIndex[wordsPart->revIndexnr].hits[ wordsPart->revIndex[wordsPart->revIndexnr].nr ].pos = wordsPart->words_sorted[i].position;
			wordsPart->revIndex[wordsPart->revIndexnr].hits[ wordsPart->revIndex[wordsPart->revIndexnr].nr ].realpos = wordsPart->words_sorted[i].unsortetIndexPosition;


			#ifdef PRESERVE_WORDS
				strcpy(wordsPart->revIndex[wordsPart->revIndexnr].word,wordsPart->words_sorted[i].word);
				wordsPart->revIndex[wordsPart->revIndexnr].wordnr = 1;
				//printf("word %u %s\n",wordsPart->revIndex[wordsPart->revIndexnr].WordID,wordsPart->words_sorted[i].word);
			#endif


			#ifndef BLACK_BOX
			///////////////////////////
			// adultWords

			//printf("adultpos %i, (*adult).adultWordnr %i\n",adultpos,(*adult).adultWordnr);
			while ((
				(adultpos < (*adult).adultWordnr)
				&& wordsPart->revIndex[wordsPart->revIndexnr].WordID > (*adult).AdultWords[adultpos].crc32) 
				) {
				#ifdef DEBUG
					//printf("testing for %u %s\n",(*adult).AdultWords[adultpos].crc32,(*adult).AdultWords[adultpos].word);
				#endif
				++adultpos;
			}
			//printf("ll: adultpos %i < adultWordnr %i\n",adultpos,(*adult).adultWordnr);
			if (adultpos == (*adult).adultWordnr) {
				//nåd enden
				//ToDo: vi får fortsat med siste ord her, ikke sant?
				//printf("adultpos %i, (*adult).adultWordnr %i\n",adultpos,(*adult).adultWordnr);
			}
			else if ((*adult).AdultWords[adultpos].crc32 == wordsPart->revIndex[wordsPart->revIndexnr].WordID) {
				#if DEBUG_ADULT
					printf("adult hit \"%s\" %i\n",(*adult).AdultWords[adultpos].word,(*adult).AdultWords[adultpos].weight);
				#endif
				(*adultWeight) += (*adult).AdultWords[adultpos].weight;
				//printf("adultWeight1 %u\n",(*adultWeight));
				//printf("weight %i\n",(*adult).AdultWords[adultpos].weight);

				//exit(1);
			}

			#endif

			oldRevIndexnr = wordsPart->revIndexnr;
			//har fåt et til ord i revindex
			++wordsPart->revIndex[wordsPart->revIndexnr].nr;

			//printf("b%i\n\n",wordsPart->revIndexnr);
			++wordsPart->revIndexnr;
		}
		else {
			#ifdef DEBUG
				printf("word seen befor. Adding. WordID \"%u\"\n",wordsPart->words_sorted[i].WordID);
			#endif

			#ifdef PRESERVE_WORDS
				++wordsPart->revIndex[oldRevIndexnr].wordnr;
			#endif
			// ord er set før, skal legges til det tidligere
			// må passe på at vi ikke overskriver med flere hits en MaxsHits
			if (wordsPart->revIndex[oldRevIndexnr].nr < MaxsHitsInIndex) {
				wordsPart->revIndex[oldRevIndexnr].hits[ wordsPart->revIndex[oldRevIndexnr].nr ].pos = wordsPart->words_sorted[i].position;
				wordsPart->revIndex[oldRevIndexnr].hits[ wordsPart->revIndex[oldRevIndexnr].nr ].realpos = wordsPart->words_sorted[i].unsortetIndexPosition;

				++wordsPart->revIndex[oldRevIndexnr].nr;
			}
			else {
				#ifdef DEBUG
				printf("to many hits for WordID \"%u\"\n",wordsPart->words_sorted[i].WordID);
				#endif
			}
			//break;

		}

		oldcrc32 = wordsPart->words_sorted[i].WordID;

	
	}



}

#ifndef BLACK_BOX
void adultPhrases(struct pagewordsFormatPartFormat *wordsPart, struct adultFormat *adult,int *adultWeight, unsigned char *langnr) {

	int i,y,z,adultFraserpos;

	//går gjenom alle hitene og ser etter adult fraser
	//toDo: denne er ikke helt optimal, da vi teller samme frase igjen og igjen. Så hvis vi har x z y x z, der "x z" er bannnet får vi nå to banninger, som gjør at verdien blir dobelt så stor.
	adultFraserpos = 0;
	for (i=0;i<wordsPart->revIndexnr;i++) {

		#ifdef DEBUG_ADULT
			//printf("word \"%s\", nr %i\n",wordsPart->revIndex[i].word,wordsPart->revIndex[i].nr);
		#endif



		while ((wordsPart->revIndex[i].WordID > (*adult).adultFraser[adultFraserpos].crc32) && (adultFraserpos < (*adult).adultWordFrasernr)) {
			#ifdef DEBUG
				printf("searching to next adult word, skipping: s %u %s\n",(*adult).adultFraser[adultFraserpos].crc32,(*adult).adultFraser[adultFraserpos].word);
			#endif
			++adultFraserpos;
		}
		if ((*adult).adultFraser[adultFraserpos].crc32 == wordsPart->revIndex[i].WordID) {

			
			//har hitt. Lopper gjenom word2'ene på jakt etter ord nr to
			#ifdef DEBUG_ADULT
				printf("word is first in frase %s\n",(*adult).adultFraser[adultFraserpos].word);
				printf("x words to try %i, pos %i\n",(*adult).adultFraser[adultFraserpos].adultWordCount,
							adultFraserpos);
			#endif

			//nulstiller oversikten over de ordene vi allerede har lagt til
			for(z=0;z<(*adult).adultFraser[adultFraserpos].adultWordCount;z++) {
				(*adult).adultFraser[adultFraserpos].adultWord[z].addedAllReady = 0;
			}

			for(y=0;y<wordsPart->revIndex[i].nr;y++) {

				for(z=0;z<(*adult).adultFraser[adultFraserpos].adultWordCount;z++) {

					if ((*adult).adultFraser[adultFraserpos].adultWord[z].crc32 
						== wordsPart->words[ wordsPart->revIndex[i].hits[y].realpos +1 ].WordID) {
						
						//setter at vi allerede har sette dette ordet
						++(*adult).adultFraser[adultFraserpos].adultWord[z].addedAllReady;
						
						if ((*adult).adultFraser[adultFraserpos].adultWord[z].addedAllReady != 1) {

							#ifdef DEBUG_ADULT

								printf("frase hit, but have bean added: \"%s %s\", weight %i, sean for the %i. time\n",
									wordsPart->revIndex[i].word,
									wordsPart->words[ wordsPart->revIndex[i].hits[y].realpos +1 ].word,
									(*adult).adultFraser[adultFraserpos].adultWord[z].weight,
									(*adult).adultFraser[adultFraserpos].adultWord[z].addedAllReady);
							#endif

							continue;
						}

						#ifdef DEBUG_ADULT
							printf("frase hit: \"%s %s\", weight %i\n",
								wordsPart->revIndex[i].word,
								wordsPart->words[ wordsPart->revIndex[i].hits[y].realpos +1 ].word,
								(*adult).adultFraser[adultFraserpos].adultWord[z].weight);
						#endif
						//øker adult verdien med vekt
						(*adultWeight) += (*adult).adultFraser[adultFraserpos].adultWord[z].weight;

					}
					else {
						#ifdef DEBUG_ADULT
						//	printf("NOT frase hit \"%s %s\", weight %i\n",wordsPart->words_sorted[i].word,(*adult).adultFraser[adultFraserpos].adultWord[y].word,(*adult).adultFraser[adultFraserpos].adultWord[y].weight);
						#endif 
					}
				}
			}

		}
		
		

	}

}
#endif

void wordsMakeRevIndex(struct pagewordsFormat *pagewords, struct adultFormat *adult,int *adultWeight, unsigned char *langnr) {

	(*adultWeight) = 0;

	wordsMakeRevIndex_part(&pagewords->normalWords,adult,adultWeight);


	//finner språk
	char lang[4];
	langdetectDetect(pagewords->normalWords.words_sorted,pagewords->normalWords.nr,lang);
	*langnr = getLangNr(lang);

	#ifdef DEBUG
	printf("lang: \"%s\"\n",lang);
	#endif

	// normal words
	#ifndef BLACK_BOX
	adultPhrases(&pagewords->normalWords,adult,adultWeight,langnr);
	#endif

	#ifdef DEBUG
	printf("wordsMakeRevIndex: adult after normalWords %i\n",(*adultWeight));
	#endif

	// link word
	wordsMakeRevIndex_part(&pagewords->linkWords,adult,adultWeight);
	#ifndef BLACK_BOX
	adultPhrases(&pagewords->linkWords,adult,adultWeight,langnr);
	#endif

	#ifdef DEBUG
	printf("wordsMakeRevIndex: adult after linkWords %i\n",(*adultWeight));
	#endif

	// spam word
	wordsMakeRevIndex_part(&pagewords->spamWords,adult,adultWeight);
	#ifndef BLACK_BOX
	adultPhrases(&pagewords->spamWords,adult,adultWeight,langnr);
	#endif

	#ifdef DEBUG
	printf("wordsMakeRevIndex: adult after spamWords %i\n",(*adultWeight));
	#endif

}




int compare_elements_adultWord (const void *p1, const void *p2) {


        if (((struct adultWordFormat*)p1)->crc32 < ((struct adultWordFormat*)p2)->crc32)
                return -1;
        else
                return ((struct adultWordFormat*)p1)->crc32 > ((struct adultWordFormat*)p2)->crc32;

}

int compare_elements_AdultFraser (const void *p1, const void *p2) {


        if (((struct adultWordFraserFormat*)p1)->crc32 < ((struct adultWordFraserFormat*)p2)->crc32)
                return -1;
        else
                return ((struct adultWordFraserFormat*)p1)->crc32 > ((struct adultWordFraserFormat*)p2)->crc32;

}


#ifndef BLACK_BOX
void adultLoad (struct adultFormat *adult) {

	FILE *FH;
	char buff[128];
	int i,y,x;
	char *cpoint;
	unsigned int crc32tmp;

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

			strscpy((*adult).AdultWords[i].word,buff,cpoint - buff +1);

			//vil ikke ha men spacen. Går et hakk vidre
			++cpoint;
			(*adult).AdultWords[i].weight = atoi(cpoint);
			

			(*adult).AdultWords[i].crc32 = crc32boitho((*adult).AdultWords[i].word);

		}		
		else {
			printf("bad AdultWords \"%s\"\n",buff);
		}

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
//		printf("%i: -%s- %u %i\n",y,(*adult).AdultWords[y].word,(*adult).AdultWords[y].crc32,(*adult).AdultWords[y].weight);
//	}


	crc32tmp = 0;	
	for(y=0;y<i;y++) {
		if ( ((*adult).AdultWords[y].crc32 == crc32tmp) && (crc32tmp != 0) ) {
			printf("Dublicate adult word: %s\n",(*adult).AdultWords[y].word);
		}

		crc32tmp = (*adult).AdultWords[y].crc32;
	}

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
	if (maxAdultWords == y) {
		printf("Adult frase list is larger then maxAdultWords (%i)\n",maxAdultWords);
		exit(1);
	}


	qsort(AdultFraserRecords, y , sizeof(struct AdultFraserRecordFormat), compare_elements_AdultFraserRecordFormat);
	
	i=-1;	
	for (x=0;x<y;x++) {

			if (x != 0) {
				if ((strcmp(AdultFraserRecords[x].word1,AdultFraserRecords[x -1].word1) == 0) && (strcmp(AdultFraserRecords[x].word2,AdultFraserRecords[x -1].word2) == 0)) {
					printf("dublicate adult frase: %i: %s %s %i\n",i,AdultFraserRecords[x].word1,AdultFraserRecords[x].word2,AdultFraserRecords[x].weight);
					continue;
				}
			}

			#ifdef DEBUG
				printf("adult frase: %i: %s, %s, %i\n",i,AdultFraserRecords[x].word1,AdultFraserRecords[x].word2,AdultFraserRecords[x].weight);
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
	

}
#endif

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
			strscpy((*DocumentIndexPost).Url,(*ReposetoryHeader).url,sizeof((*DocumentIndexPost).Url));
			
			strcpy((*DocumentIndexPost).Dokumenttype,(*ReposetoryHeader).content_type);

			(*DocumentIndexPost).IPAddress 		= (*ReposetoryHeader).IPAddress;
			(*DocumentIndexPost).response 		= (*ReposetoryHeader).response;
			(*DocumentIndexPost).htmlSize2 		= (*ReposetoryHeader).htmlSize2;
			(*DocumentIndexPost).imageSize 		= (*ReposetoryHeader).imageSize;
			(*DocumentIndexPost).userID 		= (*ReposetoryHeader).userID;
			(*DocumentIndexPost).clientVersion 	= (*ReposetoryHeader).clientVersion;
			(*DocumentIndexPost).CrawleDato 	= (*ReposetoryHeader).time;

			(*DocumentIndexPost).ResourcePointer 	= 0;
			(*DocumentIndexPost).ResourceSize 	= 0;
#ifdef BLACK_BOX
			//runarb: 31 mars 2008: gjør dette bare hvis vi ikke har en tid fra før. Hvis ikke risikerer 
			//vi og tilbakedatere tiden, og dermed slettes den av garbage collection.
			if (DocumentIndexPost->lastSeen == 0) {
				DocumentIndexPost->lastSeen = ReposetoryHeader->storageTime;
			}
#endif
}




//copy a memory area, and return the size copyed
static inline size_t memcpyrc(void *s1, const void *s2, size_t n) {
//size_t memcpyrc(void *s1, const void *s2, size_t n) {
        memcpy(s1,s2,n);

        return n;
}

#ifdef BLACK_BOX

void aclsMakeRevIndexBucket (struct IndexerRes_acls *acl,unsigned int DocID,unsigned char *langnr) {



	int i,y;

	for(i=0;i<NrOfDataDirectorys;i++) {
		(*acl).nrofAclBucketElements[i].records = 0;
		(*acl).nrofAclBucketElements[i].hits = 0;
	}

	//printf("aclIndexnr %i\n",(*acl).aclIndexnr);
	for(i=0;i<(*acl).aclIndexnr;i++) {

		(*acl).aclIndex[i].bucket = (*acl).aclIndex[i].WordID % NrOfDataDirectorys;

		++(*acl).nrofAclBucketElements[(*acl).aclIndex[i].bucket].records;
		(*acl).nrofAclBucketElements[(*acl).aclIndex[i].bucket].hits += (*acl).aclIndex[i].nr;
	}

	for(i=0;i<NrOfDataDirectorys;i++) {
		(*acl).nrofAclBucketElements[i].bucketbuffsize = ((sizeof(unsigned int) + sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int)) * (*acl).nrofAclBucketElements[i].records) + ((*acl).nrofAclBucketElements[i].hits * sizeof(unsigned short));
		//printf("bucketbuffsize %i\n",(*acl).nrofaclBucketElements[i].bucketbuffsize);

		(*acl).nrofAclBucketElements[i].bucketbuff = malloc((*acl).nrofAclBucketElements[i].bucketbuffsize);
	}

	//setter pekeren til begyndelsen. Siden vil vi jo flytte denne etter hvert som vi kommer lenger ut
	for(i=0;i<NrOfDataDirectorys;i++) {
		(*acl).nrofAclBucketElements[i].p = (*acl).nrofAclBucketElements[i].bucketbuff;
	}

	//bruker en temperær p peker her som erstatning for (*acl).nrofaclBucketElements[(*acl).aclIndex[i].bucket].p, 
	//så koden ikke blir så uoversiktelig
	void *p;
	for(i=0;i<(*acl).aclIndexnr;i++) {
		
			p = (*acl).nrofAclBucketElements[(*acl).aclIndex[i].bucket].p;

			p += memcpyrc(p,&DocID,sizeof(unsigned int));
			p += memcpyrc(p,langnr,sizeof(char));
			p += memcpyrc(p,&(*acl).aclIndex[i].WordID,sizeof(unsigned int));
			p += memcpyrc(p,&(*acl).aclIndex[i].nr,sizeof(unsigned int));
			for(y=0;y<(*acl).aclIndex[i].nr;y++) {
				p += memcpyrc(p,&(*acl).aclIndex[i].hits[y].pos,sizeof(unsigned short));
			}

			(*acl).nrofAclBucketElements[(*acl).aclIndex[i].bucket].p = p;
		
	}

}


void attribMakeRevIndexBucket (struct IndexerRes_attrib *attrib,unsigned int DocID,unsigned char *langnr) {



	int i,y;

	for(i=0;i<NrOfDataDirectorys;i++) {
		(*attrib).nrofAttribBucketElements[i].records = 0;
		(*attrib).nrofAttribBucketElements[i].hits = 0;
	}

	//printf("attribIndexnr %i\n",(*attrib).attribIndexnr);
	for(i=0;i<(*attrib).attribIndexnr;i++) {

		(*attrib).attribIndex[i].bucket = (*attrib).attribIndex[i].WordID % NrOfDataDirectorys;

		++(*attrib).nrofAttribBucketElements[(*attrib).attribIndex[i].bucket].records;
		(*attrib).nrofAttribBucketElements[(*attrib).attribIndex[i].bucket].hits += (*attrib).attribIndex[i].nr;
	}

	for(i=0;i<NrOfDataDirectorys;i++) {
		(*attrib).nrofAttribBucketElements[i].bucketbuffsize = ((sizeof(unsigned int) + sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int)) * (*attrib).nrofAttribBucketElements[i].records) + ((*attrib).nrofAttribBucketElements[i].hits * sizeof(unsigned short));

		(*attrib).nrofAttribBucketElements[i].bucketbuff = malloc((*attrib).nrofAttribBucketElements[i].bucketbuffsize);
	}

	//setter pekeren til begyndelsen. Siden vil vi jo flytte denne etter hvert som vi kommer lenger ut
	for(i=0;i<NrOfDataDirectorys;i++) {
		(*attrib).nrofAttribBucketElements[i].p = (*attrib).nrofAttribBucketElements[i].bucketbuff;
	}

	//bruker en temperær p peker her som erstatning for (*attrib).nrofattribBucketElements[(*attrib).attribIndex[i].bucket].p, 
	//så koden ikke blir så uoversiktelig
	void *p;
	for(i=0;i<(*attrib).attribIndexnr;i++) {
		
			p = (*attrib).nrofAttribBucketElements[(*attrib).attribIndex[i].bucket].p;

			p += memcpyrc(p,&DocID,sizeof(unsigned int));
			p += memcpyrc(p,langnr,sizeof(char));
			p += memcpyrc(p,&(*attrib).attribIndex[i].WordID,sizeof(unsigned int));
			p += memcpyrc(p,&(*attrib).attribIndex[i].nr,sizeof(unsigned int));
			for(y=0;y<(*attrib).attribIndex[i].nr;y++) {
				p += memcpyrc(p,&(*attrib).attribIndex[i].hits[y].pos,sizeof(unsigned short));
			}

			(*attrib).nrofAttribBucketElements[(*attrib).attribIndex[i].bucket].p = p;
		
	}

}

#endif
void wordsMakeRevIndexBucket_part(struct pagewordsFormatPartFormat *wordsPart,unsigned int DocID,unsigned char *langnr) {

	int i,y;


	for(i=0;i<NrOfDataDirectorys;i++) {
		wordsPart->nrofBucketElements[i].records = 0;
		wordsPart->nrofBucketElements[i].hits = 0;
	}


	for(i=0;i<wordsPart->revIndexnr;i++) {

		wordsPart->revIndex[i].bucket = wordsPart->revIndex[i].WordID % NrOfDataDirectorys;

		++wordsPart->nrofBucketElements[wordsPart->revIndex[i].bucket].records;
		wordsPart->nrofBucketElements[wordsPart->revIndex[i].bucket].hits += wordsPart->revIndex[i].nr;
	}
	
	for(i=0;i<NrOfDataDirectorys;i++) {
		wordsPart->nrofBucketElements[i].bucketbuffsize = ((sizeof(unsigned int) + sizeof(char) + sizeof(unsigned int) + sizeof(unsigned int)) * wordsPart->nrofBucketElements[i].records) + (wordsPart->nrofBucketElements[i].hits * sizeof(unsigned short));

		if((wordsPart->nrofBucketElements[i].bucketbuff = malloc(wordsPart->nrofBucketElements[i].bucketbuffsize)) == NULL) {
			perror("malloc nrofBucketElements");
			exit(-1);
		}
	}

	//setter pekeren til begyndelsen. Siden vil vi jo flytte denne etter hvert som vi kommer lenger ut
	for(i=0;i<NrOfDataDirectorys;i++) {
		wordsPart->nrofBucketElements[i].p = wordsPart->nrofBucketElements[i].bucketbuff;
	}

	//bruker en temperær p peker her som erstatning for wordsPart->nrofBucketElements[wordsPart->revIndex[i].bucket].p, 
	//så koden ikke blir så uoversiktelig
	void *p;
	for(i=0;i<wordsPart->revIndexnr;i++) {

			if(wordsPart->revIndex[i].nr > MaxsHitsInIndex) {
				fprintf(stderr,"have more then MaxsHitsInIndex (%i). Have %u\n",MaxsHitsInIndex,wordsPart->revIndex[i].nr);
				exit(-1);
			}

			if (wordsPart->revIndex[i].nr == 0) {
				fprintf(stderr,"Number of hits for DocID is 0.\n");
			}

			p = wordsPart->nrofBucketElements[wordsPart->revIndex[i].bucket].p;

			p += memcpyrc(p,&DocID,sizeof(unsigned int));
			p += memcpyrc(p,langnr,sizeof(char));
			p += memcpyrc(p,&wordsPart->revIndex[i].WordID,sizeof(unsigned int));
			p += memcpyrc(p,&wordsPart->revIndex[i].nr,sizeof(unsigned int));
			for(y=0;y<wordsPart->revIndex[i].nr;y++) {
				p += memcpyrc(p,&wordsPart->revIndex[i].hits[y].pos,sizeof(unsigned short));
			}

			wordsPart->nrofBucketElements[wordsPart->revIndex[i].bucket].p = p;
		
	}



}

void wordsMakeRevIndexBucket (struct pagewordsFormat *pagewords,unsigned int DocID,unsigned char *langnr) {

	wordsMakeRevIndexBucket_part(&pagewords->normalWords,DocID,langnr);

}

void dictionaryWordsWrite_part (struct pagewordsFormatPartFormat *wordsPart,FILE *FH, char *acl_allow, char *acl_denied) {

	int i;

	#ifdef PRESERVE_WORDS
		for(i=0;i<wordsPart->revIndexnr;i++) {
			#ifdef DEBUG
			printf("word: \"%s\": %i (acl: allow: %s denied: %s)\n",wordsPart->revIndex[i].word, 
			       wordsPart->revIndex[i].wordnr, acl_allow, acl_denied);

			#endif
			fprintf(FH,"%s %i %s %s\n",wordsPart->revIndex[i].word,wordsPart->revIndex[i].wordnr, acl_allow, acl_denied ? acl_denied : "");
		}
	#endif

}

void dictionaryWordsWrite (struct pagewordsFormat *pagewords,FILE *FH, char *acl_allow, char *acl_denied) {
	dictionaryWordsWrite_part(&pagewords->normalWords,FH,acl_allow,acl_denied);
}

/**************************************************************************************
Skriver acl index til disk
***************************************************************************************/
#ifdef BLACK_BOX
void aclindexFilesAppendWords(struct IndexerRes_acls *acl,FILE *aclindexFilesHa[]) {


	int i;

	for(i=0;i<NrOfDataDirectorys;i++) {
		if ((*acl).nrofAclBucketElements[i].bucketbuffsize != 0) {
			fwrite((*acl).nrofAclBucketElements[i].bucketbuff,(*acl).nrofAclBucketElements[i].bucketbuffsize,1,aclindexFilesHa[i]);
		}
	}


	for(i=0;i<NrOfDataDirectorys;i++) {
			free((*acl).nrofAclBucketElements[i].bucketbuff);
		
	}

}

void attribindexFilesAppendWords(struct IndexerRes_attrib *attrib,FILE *attribindexFilesHa[]) {


	int i;

	for(i=0;i<NrOfDataDirectorys;i++) {
		if ((*attrib).nrofAttribBucketElements[i].bucketbuffsize != 0) {
			fwrite((*attrib).nrofAttribBucketElements[i].bucketbuff,(*attrib).nrofAttribBucketElements[i].bucketbuffsize,1,attribindexFilesHa[i]);
		}
	}


	for(i=0;i<NrOfDataDirectorys;i++) {
			free((*attrib).nrofAttribBucketElements[i].bucketbuff);
		
	}

}
#endif
/**************************************************************************************
Skriver reversert index til disk
***************************************************************************************/
void revindexFilesAppendWords_part(struct pagewordsFormatPartFormat *wordsPart,FILE *revindexFilesHa[]) {

	int i;

	for(i=0;i<NrOfDataDirectorys;i++) {
		if (wordsPart->nrofBucketElements[i].bucketbuffsize != 0) {
			fwrite(wordsPart->nrofBucketElements[i].bucketbuff,wordsPart->nrofBucketElements[i].bucketbuffsize,1,revindexFilesHa[i]);
		}
	}


	for(i=0;i<NrOfDataDirectorys;i++) {
			free(wordsPart->nrofBucketElements[i].bucketbuff);
		
	}


}

void revindexFilesAppendWords(struct pagewordsFormat *pagewords,FILE *revindexFilesHa[]) {

	revindexFilesAppendWords_part(&pagewords->normalWords,revindexFilesHa);
}

void html_parser_timout( int signo )
{
    if ( signo == SIGALRM ) {
        printf("got alarm\n");
        alarm_got_raised = 1;
    }
 

}



void handelPage(struct pagewordsFormat *pagewords,struct ReposetoryHeaderFormat *ReposetoryHeader, 
		char HtmlBuffer[],int HtmlBufferLength, 
		char **title, char **body) {

		html_parser_run((*ReposetoryHeader).url,HtmlBuffer, HtmlBufferLength,title, body,IndexerRes_fn,pagewords );
}


