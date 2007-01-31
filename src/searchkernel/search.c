#include <sys/time.h>

//#include "../common/define.h"
#include "../common/poprank.h"
#include "../common/iindex.h"
#include "../common/debug.h"
#include "../common/crc32.h"
//#include "../query/query_parser.h"
#include "searchkernel.h"
#include "../common/timediff.h"
#include "../common/integerindex.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../getdate/dateview.h"

#include <string.h>
#include <limits.h>

#define popvekt 0.6
#define termvekt 0.4

#include <math.h>
#include <stdlib.h>

#include "search.h"

//rangeringsfilter er der vi har et hvist antall treff, definert som RFC (Rank Filter Cutoff).
//har vi så mnage treff filtrerer vi ut de under en hvis rankt
#define AthorRFC1 	10000
#define MainRFC1 	200000
#define MainRFC2 	10000

int compare_elements (const void *,const void *);
int compare_filetypes (const void *p1, const void *p2);


struct filesKeyFormat {
	struct subnamesFormat *subname;
	char filename[4];
};

static unsigned int fileshashfromkey(void *ky)
{
    struct filesKeyFormat *k = (struct filesKeyFormat *)ky;
    return ((unsigned int)k->subname + (unsigned int)k->filename[0] + (unsigned int)k->filename[1] + (unsigned int)k->filename[2] + (unsigned int)k->filename[3]);
}

static int filesequalkeys(void *k1, void *k2)
{
    return (0 == memcmp(k1,k2,sizeof(struct filesKeyFormat)));
}


int rankUrl(const unsigned short *hits, int nrofhit) {
	int rank, i;

	rank = 0;
	for (i = 0;i < nrofhit; i++) {
	
		if (hits[i] == 2) {
        		rank =+ poengForUrlMain;
        	}
        	else {
        		rank =+  poengForUrlSub;
        	}
	}
	
	return rank;
}

int rankAthor(const unsigned short *hits, int nrofhit) {
	int rank, i;

	rank = 0;
	for (i = 0;i < nrofhit; i++) {

		rank++;
	}


	if (rank > maxPoengAthor) {
		rank = maxPoengAthor;
	}

	return rank;
}

int rankMain(const unsigned short *hits, int nrofhit) {

	int i;
	int poengBody, poengHeadline, poengTittel, poengUrl; 
	double poengDouble;

                               poengBody = 0;
                                poengHeadline = 0;
                                poengTittel = 0;
                                poengUrl = 0;

                                // kjører gjenom anttall hit
				for (i = 0;i < nrofhit; i++) {

                                        /*************************************************
                                        lagger til poeng
                                        *************************************************/
                                        if (hits[i] >= 1000) {              //Body
                                                poengBody += poengForBody;
                                        }
                                        else if (hits[i] >= 500) {  //Headline
                                                poengHeadline += poengForHeadline;
                                        }
                                        else if (hits[i] >= 100) {  //Tittel
                                                poengTittel += poengForTittel;
                                        }
                                        else if (hits[i] == 2) {
                                                //poengUrl += poengForMainUrlWord;
                                        }
                                        else if (hits[i] >= 1) {    //url
                                                // ingen urler i body mere
                                                //poengUrl += poengForUrl;
                                        }
                                        else {
                                                //printf("Error, fikk inn 0 eller mindre\n");
                                        }
                                        /**************************************************/
                                }

                                //Kalkulerer total poeng
                                poengDouble = 0;

                                if (poengBody != 0) {
                                        //poengDouble += log(poengBody);
                                        //temp: 16 now:
                                        if (poengBody > maxPoengBody) {
                                                poengBody = maxPoengBody;
                                        }
                                        else {
                                                poengDouble += poengBody;
                                        }
                                }
                                if (poengHeadline != 0) {
                                        //poengDouble += log(poengHeadline);
                                        if (poengHeadline > MaxPoengHeadline) {
                                                poengDouble += MaxPoengHeadline;
                                        }
                                        else {
                                                poengDouble += poengHeadline;
                                        }
                                }
                                if (poengTittel != 0) {
                                        //poengDouble += log(poengTittel);
                                        if (poengTittel > MaxPoengTittel) {
                                                poengDouble += MaxPoengTittel;
                                        }
                                        else {
                                                poengDouble += poengTittel;
                                        }
                                }
                                if (poengUrl != 0) {
                                        //poengDouble += log(poengUrl);
                                        poengDouble += poengUrl;
                                }

	return (int)ceil(poengDouble);

}


void or_merge(struct iindexFormat *c, int *baselen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
	int i=0;
	int j=0;
	int k=0;

	(*baselen) = 0;

	while ((i<alen) && (j<blen) && (k < maxIndexElements))
	{

	//if (b[j].DocID == 2788928) {
	//	printf("b: %i , %i\n",a[i].DocID,b[j].DocID);
	//	printf("b TermAntall %i\n",b[j].TermAntall);
	//}
		if (a[i].DocID == b[j].DocID) {
			//printf("%i == %i\n",a[i].DocID,b[j].DocID);
			//c[k] = a[i];

			//c[k].TermRank = a[i].TermRank + b[j].TermRank;		

			//TermRank = a[i].TermRank + b[j].TermRank;
                        //c[k] = a[i];
                        //c[k].TermRank = TermRank;

			c[k] = a[i];
			c[k].TermRank = a[i].TermRank + b[j].TermRank;
			c[k].phraseMatch = a[i].phraseMatch + b[j].phraseMatch;

			++k; ++j; ++i;
			++(*baselen);
		}
 		else if( a[i].DocID < b[j].DocID ) {
                	c[k] = a[i];
			
			++i; 
			++k;
			++(*baselen);
		}
 		else {
	                c[k] = b[j];

			++j; 
			++k;
			++(*baselen);
		}
	}

	printf("i %i, alen %i, j %i, blen %i. k %i\n",i,alen,j,blen,k);

	while (i<alen && (k < maxIndexElements)){

                c[k] = a[i];

		++k; ++i;
		++(*baselen);
	}
	
	while (j<blen && (k < maxIndexElements)) {

                c[k] = b[j];

		++k; ++j;
		++(*baselen);
	}

	printf("or_merge a and b of length %i %i. Into %i\n",alen,blen,(*baselen));
	printf("end or merge\n");
}
void and_merge(struct iindexFormat *c, int *baselen, int originalLen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
	int i=0,j=0;
	int TermRank;

	int k=originalLen;

	(*baselen) = 0;

	printf("and_merge a and b of length %i %i\n",alen,blen);


	while (i<alen && j<blen)
	{
		if (a[i].DocID == b[j].DocID) {
			//printf("%i == %i\n",a[i].DocID,b[j].DocID);
			//c[k] = a[i];

			//c[k].TermRank = a[i].TermRank + b[j].TermRank;		

			TermRank = a[i].TermRank + b[j].TermRank;
                        c[k] = a[i];
                        c[k].TermRank = TermRank;
			c[k].phraseMatch = 0;

			k++; j++; i++;
			(*baselen)++;
		}
 		else if( a[i].DocID < b[j].DocID ) {
			//printf("%i < %i\n",a[i].DocID,b[j].DocID);
	
   			//c[k++] = a[i++];
			i++;
		}
 		else {
			//printf("%i > %i\n",a[i].DocID,b[j].DocID);
   			//c[k++] = b[j++];
			j++;
		}
	}
}

//and søk med progsymasjon () mere vekt  hvis ordene er nerme en fjernt.
void andprox_merge(struct iindexFormat *c, int *baselen, int originalLen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
        int i=0,j=0;
	int k=originalLen;
	int y;
	int distance;
	int ah,bh;
	int TermRank;
        (*baselen) = 0;
	int found;
	
	#ifdef DEBUG
	struct timeval start_time, end_time;

	gettimeofday(&start_time, NULL);
	#endif

        while (i<alen && j<blen)
        {
                if (a[i].DocID == b[j].DocID) {

			//TermRank = a[i].TermRank + b[j].TermRank;
			//termrank blir den verdien som er minst. Det gjør at det lønner seg å ha 
			//høye verdier for begge. Ikke slik at et dokument som er "ord1, ord1, ord1 ord2"
			//komer bra ut på søk for både ord1 og ord2. Da ord2 forekommer skjelden.
			if (a[i].TermRank < b[j].TermRank) {
				TermRank = a[i].TermRank;
			}
			else {
				TermRank = b[j].TermRank;
			}
			

			c[k] = a[i];
			c[k].TermRank = TermRank;
			c[k].phraseMatch = 0;

			ah = bh = 0;
			/*
			//lopper gjenom alle hitene og finner de som er rett etter hverandre
			while (ah<a[i].TermAntall && bh <b[j].TermAntall) {

				
				if (a[i].hits[ah] < b[j].hits[bh]) {
					distance = b[j].hits[bh] - a[i].hits[ah];

					//printf("%i < %i = %i\n",a[i].hits[ah],b[j].hits[bh],distance);

					ah++;
				}
				else {
					distance =  a[i].hits[ah] - b[j].hits[bh];

					//printf("%i > %i = %i\n",a[i].hits[ah],b[j].hits[bh],distance);
					bh++;
				}

				//bør også se på å belønne fraser som er nermere en X og X, kansje i intervalene 1, 10, 100 ?
				if (distance == 1) {
					//ToDo: må så på en bedre møte å øke TermRank når man har sammenfallende hits
					c[k].TermRank = c[k].TermRank * 2;					
				}
				

			};
			*/
			found = 0;
			//lopper gjenom alle hitene og finner de som er rett etter hverandre
			while (ah<a[i].TermAntall && bh <b[j].TermAntall) {

				//sjekker om dette er en frase. Altså at "ord2" kommer ret etter "ord1"
				if ((b[j].hits[bh] - a[i].hits[ah]) == 1) {
					found = 1;
				}
				
				if (b[j].hits[bh] > a[i].hits[ah]) {
				//else if (a[i].hits[ah] < b[j].hits[bh]) {
					//går videre
					ah++;
				}
				else {
					//går videre
					bh++;
				}

						

			}

			if (found) {
				c[k].phraseMatch = 1;
			}


                        k++; j++; i++;
                        (*baselen)++;
                }
                else if( a[i].DocID < b[j].DocID ) {
                        //printf("%i < %i\n",a[i].DocID,b[j].DocID);

                        //c[k++] = a[i++];
                        i++;
                }
                else {
                        //printf("%i > %i\n",a[i].DocID,b[j].DocID);
                        //c[k++] = b[j++];
                        j++;
                }
        }
	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: andprox_merge %u\n",getTimeDifference(&start_time,&end_time));
	#endif

	printf("andprox_merge a and b of length %i %i. Into %i\n",alen,blen,(*baselen));

}

//hopper over orde. Må da flytte alle ordene et hakk bortover
frase_stopword(struct iindexFormat *c, int *clen) {
	int i,y;

	for (i=0;i<*clen;i++) {

		for(y=0;y<c[i].TermAntall;y++) {
			c[i].hits[y]++;
		}
	
	}

}

//frasesk. Denne er dog ikke bra, egentlig en versjon av andprox_merge der bare de sidene med distanse 1 blir med
void frase_merge(struct iindexFormat *c, int *baselen,int Originallen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
        int i=0,j=0;
	int k=Originallen;
	int y;
	int ah,bh;
	int hitcount,found;
	int TermRank;
	unsigned short hits[MaxTermHit];
        (*baselen) = 0;

	debug("frase_merge: start");
	debug("frase_merge: merging array a of len %i to b of len %i",alen,blen);
	debug("frase_merge: arrays %u %u",(unsigned int)a,(unsigned int)b);
        while (i<alen && j<blen)
        {
                if (a[i].DocID == b[j].DocID) {



			#ifdef DEBUG
                        	printf("Have DocID match %u == %u\n",a[i].DocID,b[j].DocID);
                        
				printf("a: ");
				for (y=0; (y < a[i].TermAntall) && (y < MaxTermHit); y++) {
                        		printf("%hu ",a[i].hits[y]);
        	        	}
				printf("\n");
				
				printf("b: ");
                        	for (y=0; (y < b[j].TermAntall) && (y < MaxTermHit); y++) {
                        	        printf("%hu ",b[j].hits[y]);
                        	}
				printf("\n");
			#endif

			ah = bh = 0;
			hitcount = 0;
			found = 0;
			//lopper gjenom alle hitene og finner de som er rett etter hverandre
			while (ah<a[i].TermAntall && bh <b[j].TermAntall) {

				//sjekker om dette er en frase. Altså at "ord2" kommer ret etter "ord1"
				if ((b[j].hits[bh] - a[i].hits[ah]) == 1) {
					found = 1;
					hits[hitcount] = b[j].hits[bh];
					hitcount++;
				}


				//går videre
				if (a[i].hits[ah] < b[j].hits[bh]) {
					ah++;
				}
				else {
					bh++;
				}

						

			}

			if (found) {
				//printf("hit! %i\n",a[i].DocID);
                        	//c[k] = a[i];
				//
                        	//c[k].TermRank = a[i].TermRank + b[j].TermRank;
				//
                        	//c[k].hits[hitcount] = a[i].hits[ah];
				

				//TermRank = a[i].TermRank + b[j].TermRank;
				//
 				//termrank blir den verdien som er minst. Det gjør at det lønner seg å ha
                        	//høye verdier for begge. Ikke slik at et dokument som er "ord1, ord1, ord1 ord2"
                        	//komer bra ut på søk for både ord1 og ord2. Da ord2 forekommer skjelden.
                        	if (a[i].TermRank < b[j].TermRank) {
                                	TermRank = a[i].TermRank;
                        	}
                        	else {
                                	TermRank = b[j].TermRank;
                        	}
                        	c[k] = a[i];
                        	c[k].TermRank = TermRank;
				c[k].phraseMatch = 1;

				for(y=0;y<hitcount;y++) {
                                        c[k].hits[y] = hits[y];
                                }


                        	(*baselen)++;
				k++;
			}

			//c[k].TermAntall = hitcount;

                        j++; i++;
                }
                else if( a[i].DocID < b[j].DocID ) {
                        //printf("%i < %i\n",a[i].DocID,b[j].DocID);

                        //c[k++] = a[i++];
                        i++;
                }
                else {
                        //printf("%i > %i\n",a[i].DocID,b[j].DocID);
                        //c[k++] = b[j++];
                        j++;
                }
        }

	debug("frase_merge: end");

}


void searchIndex (char *indexType, int *TeffArrayElementer, struct iindexFormat *TeffArray,query_array *queryParsed,struct iindexFormat *TmpArray,struct subnamesFormat *subname, int (*rank)(const unsigned short *,const int), int languageFilterNr, int languageFilterAsNr[]){

	int i, y, j,k;
	char queryelement[128];
	unsigned long WordIDcrc32;
	int baseArrayLen;
        int TmpArrayLen;
	int TeffArrayOriginal;

	#ifdef DEBUG
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	#endif

	debug("######################################################################");
	debug("searchIndex: start");

	printf("\nsearchIndex \"%s\", subname \"%s\"\n",indexType,(*subname).subname);
	TeffArrayOriginal = (*TeffArrayElementer);


//for (i=0; i<(*queryParsed).size; i++)
for (i=0; i<(*queryParsed).n; i++)
        {
            //struct text_list *t_it = (*queryParsed).elem[i];

            printf("Search type %c\n", (*queryParsed).query[i].operand );


		switch ((*queryParsed).query[i].operand) {

			case '+':

					//(*TeffArrayElementer) = 0;

					queryelement[0] = '\0';
		                	//while ( t_it!=NULL )
                			//{
					for (j=0; j<(*queryParsed).query[i].n; j++) {
                    				printf("aa_ søker på \"%s\"\n", (*queryParsed).query[i].s[j]);
                    				strncat(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));
                    			                			
                				printf("queryelement:  %s\n", queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						//hvis vi ikke har noen elementer i base arrayen, legger vi inn direkte
						//ToDo: kan ikke gjøre det da dette kansje ikke er første element
						//må skille her
						//if (*TeffArrayElementer == 0) {
						if (i == 0) {
							TmpArrayLen = (*TeffArrayElementer);
							GetIndexAsArray(TeffArrayElementer,TeffArray,&WordIDcrc32,indexType,"aa",subname,rank,languageFilterNr, languageFilterAsNr);
							(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;
						}
						else {
							TmpArrayLen = 0;
							GetIndexAsArray(&TmpArrayLen,TmpArray,&WordIDcrc32,indexType,"aa",subname,rank,languageFilterNr, languageFilterAsNr);
						
							printf("did find %i pages\n",TmpArrayLen);
												
						
													
							andprox_merge(TeffArray,&baseArrayLen,TeffArrayOriginal,TeffArray,(*TeffArrayElementer),TmpArray,TmpArrayLen);
							printf("baseArrayLen %i\n",baseArrayLen);
							(*TeffArrayElementer) = baseArrayLen;

						}

					//	t_it = t_it->next;
					}

					
				break;



			case '"':
					


			                queryelement[0] = '\0';
                			//while ( t_it!=NULL )
					debug("will frases search for:");
					for (j=0; j<(*queryParsed).query[i].n; j++) {
						debug("\t%s",(*queryParsed).query[i].s[j]);
					}

					//Vi må først frasesøke de ordene vi skal, og lagre dete i en temp array. Så merge dette med resten
					struct iindexFormat *tmpResult = (struct iindexFormat *)malloc(maxIndexElements * sizeof(struct iindexFormat));
					int tmpResultElementer = 0;

					for (j=0; j<(*queryParsed).query[i].n; j++) {

						WordIDcrc32 = crc32boitho((*queryParsed).query[i].s[j]);
                			
                    				printf("\nelement %s\n", (*queryParsed).query[i].s[j]);
                    				debug("crc32: %u",WordIDcrc32);
                    				
						//printf("word %s is st %i\n",t_it->text,t_it->stopword);

						//hvsi dette er første element leger vi de inn
						if (j == 0) {
			                                //TmpArrayLen = (*TeffArrayElementer);
							tmpResultElementer = 0;
							GetIndexAsArray(&tmpResultElementer,tmpResult,&WordIDcrc32,indexType,"aa",subname,rank,languageFilterNr, languageFilterAsNr);

                                                        //(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;        	        
							//printf("TeffArrayElementer %i\n",(*TeffArrayElementer));
						}
						else {

							
                                        		//if (*TeffArrayElementer == 0) {
                                        		//}
							//else
							//støtter ikke stopord 
							//if (t_it->stopword) {
							if (0) {
								//printf("er sttopword 2\n");
								frase_stopword(TeffArray,TeffArrayElementer);
							}
                                        		else {
							
								TmpArrayLen = 0;
								GetIndexAsArray(&TmpArrayLen,TmpArray,&WordIDcrc32,indexType,"aa",subname,rank,languageFilterNr, languageFilterAsNr);

								printf("\t dddd: frase_merge %i %i\n",(*TeffArrayElementer),TmpArrayLen);

								//dette er ikke en ekts frasesøk, kan bare ha en frase
								frase_merge(tmpResult,&tmpResultElementer,TeffArrayOriginal,tmpResult,tmpResultElementer,TmpArray,TmpArrayLen);
								//ToDO: burde kansje bruke noe mem move eller slik her
								//for (y=0;y<baseArrayLen;y++) {
                                        	        	//        TeffArray[y] = baseArray[y];
                                        	        	//}
                                               						
								(*TeffArrayElementer) = baseArrayLen;
							}
                				}
						//t_it = t_it->next;

					}
						
					//så må vi and merge frasene inn i queryet
					//hvis dette er første forekomst så kopierer vi bare inn
					//hvis ikke må vi and merge
					if (i == 0) {
						debug("er første fraseelement");
						k=TeffArrayOriginal;
						for (j=0;j<tmpResultElementer;j++) {
							//memcpy(TeffArray[j],tmpResult[j],sizeof(struct iindexFormat));
							TeffArray[k] = tmpResult[j];
							++k;
						}
						(*TeffArrayElementer) = tmpResultElementer;
					}
					else {
						and_merge(TeffArray,&baseArrayLen,TeffArrayOriginal,TeffArray,(*TeffArrayElementer),tmpResult,tmpResultElementer);
						(*TeffArrayElementer) = baseArrayLen;
					}

					free(tmpResult);


                                break;

		}


 
        }

	printf("(*TeffArrayElementer) %i, TeffArrayOriginal %i\n",(*TeffArrayElementer),TeffArrayOriginal);
//toDo: trenger vi denne nå???
//tror ikke vi trenger denne mere, da vi har merget queryet inn i den
	(*TeffArrayElementer) = (*TeffArrayElementer) + TeffArrayOriginal;
	TeffArrayOriginal = (*TeffArrayElementer);
	printf("new len is %i\n",(*TeffArrayElementer));

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: searchIndex %f\n",getTimeDifference(&start_time,&end_time));

	#endif

	debug("searchIndex: end");
	debug("######################################################################");

}


void searchSimple (int *TeffArrayElementer, struct iindexFormat *TeffArray,int *TotaltTreff, query_array *queryParsed, struct queryTimeFormat *queryTime, struct subnamesFormat subnames[], int nrOfSubnames,int languageFilterNr, int languageFilterAsNr[], char orderby[], int dates[]) {

	int i,y;
	//int x=0,j=0,k=0;
	int PopRank;

	//int M,N;
	int tmpint;

	struct timeval start_time, end_time;
		
	//double TermRankTemp;
       	//double PopRankTemp;

	struct iindexFormat *TmpArray; 
			
	struct iindexFormat *AthorArray;
	struct iindexFormat *MainArray;
	struct iindexFormat *UrlArray;

	int MainArrayLen;
	int UrlArrayLen;
	int AthorArrayLen;

	//char *WordID;




	
     
        int baseArrayLen,MainArrayHits;
	//int TmpArrayLen;	
	*TeffArrayElementer = 0;
	//unsigned long WordIDcrc32;


	gettimeofday(&start_time, NULL);

	TmpArray 	= (struct iindexFormat *)malloc(maxIndexElements * sizeof(struct iindexFormat));
	printf("TmpArray p %u, aaaa %i\n",(unsigned int)TmpArray,sizeof(struct iindexFormat));

	
	AthorArray 	= (struct iindexFormat *)malloc(maxIndexElements * sizeof(struct iindexFormat));
	printf("AthorArray %u\n",(unsigned int)AthorArray);

	AthorArrayLen = 0;

	#ifndef BLACK_BOKS
	for(i=0;i<nrOfSubnames;i++) {
		subnames[i].hits = AthorArrayLen;
		searchIndex("Athor",&AthorArrayLen,AthorArray,queryParsed,TmpArray,&subnames[i],rankAthor,languageFilterNr, languageFilterAsNr);
		subnames[i].hits = AthorArrayLen - subnames[i].hits;
	}
		printf("TmpArray p %u\n",(unsigned int)TmpArray);

		printf("AthorArrayLen %i\n",AthorArrayLen);

		//gjør rangering og filtrering
		#ifdef WITH_RANK_FILTER
		if (AthorArrayLen > 20000) {
	
			y=0;
                	for (i = 0; i < AthorArrayLen; i++) {
                	        PopRank = popRankForDocIDMemArray(AthorArray[i].DocID);


				if (PopRank > 34) {
					AthorArray[y] = AthorArray[i];
					AthorArray[y].PopRank = PopRank;
					++y;
				}
                	}
			printf("oo: filtrerte fra %i, til %i\n",AthorArrayLen,y);
			AthorArrayLen = y--; //ToDo: usikker om vi trenger -- her
		}
		else {
                	for (i = 0; i < AthorArrayLen; i++) {
                	        AthorArray[i].PopRank = popRankForDocIDMemArray(AthorArray[i].DocID);
                	}

		}
		#else 
			for (i = 0; i < AthorArrayLen; i++) {
                                AthorArray[i].PopRank = popRankForDocIDMemArray(AthorArray[i].DocID);
                        }
		#endif
	#endif

	UrlArray = (struct iindexFormat *)malloc(maxIndexElements * sizeof(struct iindexFormat));
	UrlArrayLen = 0;

	#ifndef BLACK_BOKS
	for(i=0;i<nrOfSubnames;i++) {
		subnames[i].hits = UrlArrayLen;
		searchIndex("Url",&UrlArrayLen,UrlArray,queryParsed,TmpArray,&subnames[i],rankUrl,languageFilterNr, languageFilterAsNr);
		subnames[i].hits = UrlArrayLen - subnames[i].hits;
	}
			printf("TmpArray p %u\n",(unsigned int)TmpArray);



		//gjør rangering og filtrering
		#ifdef WITH_RANK_FILTER
                if (UrlArrayLen > 20000) {
                        //gjør rnagering og filtrering
                        y=0;
                        for (i = 0; i < UrlArrayLen; i++) {
                                PopRank = popRankForDocIDMemArray(UrlArray[i].DocID);

                                if (PopRank > 34) {
					UrlArray[y] = UrlArray[i];
					UrlArray[y].PopRank = PopRank;
                                        ++y;
                                }
                        }
			printf("ll: merget url from %i ot %i\n",UrlArrayLen,y);
                        UrlArrayLen = y--; //ToDo: usikker om vi trenger -- her
                }
		else {
			for (i = 0; i < UrlArrayLen; i++) {
                                UrlArray[i].PopRank = popRankForDocIDMemArray(UrlArray[i].DocID);

                        }
		}
		#else
			for (i = 0; i < UrlArrayLen; i++) {
                                UrlArray[i].PopRank = popRankForDocIDMemArray(UrlArray[i].DocID);

                        }
                #endif

	#endif

	printf("\turl merger. UrlArrayLen %i, AthorArrayLen %i\n",UrlArrayLen,AthorArrayLen);
	or_merge(TeffArray,TeffArrayElementer,AthorArray,AthorArrayLen,UrlArray,UrlArrayLen);
	printf("AthorArray %u\n",(unsigned int)AthorArray);

	free(AthorArray);
	free(UrlArray);
	gettimeofday(&end_time, NULL);
	(*queryTime).AthorSearch = getTimeDifference(&start_time,&end_time);

	printf("TeffArrayElementer: %i\n",*TeffArrayElementer);

	//for testing kan vi spesifisere at vi bare skal ha treff fra ATHOR 
	#ifdef ATHOR_ONLY
		if (1) {
	#else
		if (*TeffArrayElementer > 20000) {
	#endif
		/*
		//gjør rnagering
		for (i = 0; i < (*TeffArrayElementer); i++) {
                        TeffArray[i].PopRank = popRankForDocIDMemArray(TeffArray[i].DocID);
		}
		*/

		(*TotaltTreff) = (*TeffArrayElementer);
		

		printf("ATHOR: TotaltTreff %i, TeffArrayElementer: %i\n",(*TotaltTreff),(*TeffArrayElementer));
	}
	else {


	
		MainArray = (struct iindexFormat *)malloc(maxIndexElements * sizeof(struct iindexFormat));
		AthorArray = (struct iindexFormat *)malloc(maxIndexElements * sizeof(struct iindexFormat));

		//for oring kan vi ikke ha en ev erayene som merges som resultatarry, slik som på oring
		//må ha en egen til dette. Legger derfor Athor svarene i en egen array.
		//kopierer Athor da den som regel vil være minst
		#ifdef WITH_RANK_FILTER
		if ((*TeffArrayElementer) > AthorRFC1) {
			
			gettimeofday(&start_time, NULL);
				y = 0;
				for (i=0;i<(*TeffArrayElementer);i++) {

					TeffArray[i].PopRank = popRankForDocIDMemArray(TeffArray[i].DocID);

                      			if (TeffArray[i].PopRank > 12) {
						AthorArray[y] = TeffArray[i];
						++y;
					}
		
				}
			gettimeofday(&end_time, NULL);
                        (*queryTime).AthorRank = getTimeDifference(&start_time,&end_time);

			printf("aa: filtrerte Athor array fra %i til %i\n",(*TeffArrayElementer),y);

			(*TeffArrayElementer) = y;
		}
		else {
			gettimeofday(&start_time, NULL);
				for (i=0;i<(*TeffArrayElementer);i++) {
					TeffArray[i].PopRank = popRankForDocIDMemArray(TeffArray[i].DocID);
					AthorArray[i] = TeffArray[i];
				}
			gettimeofday(&end_time, NULL);
                        (*queryTime).AthorRank = getTimeDifference(&start_time,&end_time);
			
			printf("filtrerte ikke Athor array\n");
			printf("aa: Athor array har %i elementer\n",(*TeffArrayElementer));

		}
		#else

			gettimeofday(&start_time, NULL);
                        	for (i=0;i<(*TeffArrayElementer);i++) {
                                	TeffArray[i].PopRank = popRankForDocIDMemArray(TeffArray[i].DocID);
                                	AthorArray[i] = TeffArray[i];
                        	}
			gettimeofday(&end_time, NULL);
			(*queryTime).AthorRank = getTimeDifference(&start_time,&end_time);

                        printf("aa: Athor array har %i elementer\n",(*TeffArrayElementer));	
		#endif


		gettimeofday(&start_time, NULL);
		MainArrayLen=0;	
		for(i=0;i<nrOfSubnames;i++) {
			tmpint = MainArrayLen;
			printf("\nhit pre i: %i %i %i\n",i,tmpint,MainArrayLen);
			searchIndex("Main",&MainArrayLen,MainArray,queryParsed,TmpArray,&subnames[i],rankMain,languageFilterNr, languageFilterAsNr);
			printf("hit aa %i - %i\n",MainArrayLen,subnames[i].hits);

			subnames[i].hits = MainArrayLen - tmpint;
		}
			
		printf("TmpArray p %u\n",(unsigned int)TmpArray);


		gettimeofday(&end_time, NULL);
                (*queryTime).MainSearch = getTimeDifference(&start_time,&end_time);

		//må ta vare på antall hists også, MainArrayLen er lengden, og den forandres
		MainArrayHits = MainArrayLen;


		#ifdef WITH_RANK_FILTER
		if (MainArrayLen > MainRFC1) {
			//filtrerer bort små forekomster i main array
			gettimeofday(&start_time, NULL);

                        y = 0;
                        for (i=0;i<MainArrayLen;i++) {
                                MainArray[i].PopRank = popRankForDocIDMemArray(MainArray[i].DocID);

                                if (MainArray[i].PopRank > 30) {
                                        MainArray[y] = MainArray[i];
                                        ++y;
                                }
                        }
                        printf("filtrerte main array fra %i til %i\n",MainArrayLen,y);

                        MainArrayLen = y;

                        gettimeofday(&end_time, NULL);
                        (*queryTime).MainRank = getTimeDifference(&start_time,&end_time);


		}
		else if (MainArrayLen > MainRFC2) {
			//filtrerer bort små forekomster i main array
			gettimeofday(&start_time, NULL);
			y = 0;
                	for (i=0;i<MainArrayLen;i++) {
				MainArray[i].PopRank = popRankForDocIDMemArray(MainArray[i].DocID);

				if (MainArray[i].PopRank > 12) {
					MainArray[y] = MainArray[i];
					++y;
				}
			}
                        gettimeofday(&end_time, NULL);
                        (*queryTime).MainRank = getTimeDifference(&start_time,&end_time);

			printf("filtrerte main array fra %i til %i\n",MainArrayLen,y);

			MainArrayLen = y;
		}
		else {
			gettimeofday(&start_time, NULL);
				for (i=0;i<MainArrayLen;i++) {
					MainArray[i].PopRank = popRankForDocIDMemArray(MainArray[i].DocID);
					//printf("%lu: %i\n",MainArray[i].DocID,MainArray[i].PopRank);
				}
                        gettimeofday(&end_time, NULL);
                        (*queryTime).MainRank = getTimeDifference(&start_time,&end_time);
			
			printf("filtrerte ikke main array\n");
		}
		#else

			gettimeofday(&start_time, NULL);
                        	for (i=0;i<MainArrayLen;i++) {
                                	MainArray[i].PopRank = popRankForDocIDMemArray(MainArray[i].DocID);
                                	//printf("%lu: %i\n",MainArray[i].DocID,MainArray[i].PopRank);
                        	}

			gettimeofday(&end_time, NULL);
                        (*queryTime).MainRank = getTimeDifference(&start_time,&end_time);

			printf("filtrerte ikke main array\n");

			printf("aa: Main array har %i elementer\n",MainArrayLen);

		#endif

		//(*TotaltTreff) = (*TeffArrayElementer) + MainArrayLen;
		(*TotaltTreff) = (*TeffArrayElementer) + MainArrayHits;

		printf("TotaltTreff: %i, TeffArrayElementer %i, MainArrayLen %i\n",(*TotaltTreff),(*TeffArrayElementer),MainArrayLen);

		gettimeofday(&start_time, NULL);
			or_merge(TeffArray,&baseArrayLen,AthorArray,*TeffArrayElementer,MainArray,MainArrayLen);
                gettimeofday(&end_time, NULL);
                (*queryTime).MainAthorMerge = getTimeDifference(&start_time,&end_time);

		printf("baseArrayLen %i\n",baseArrayLen);

		free(MainArray);
		free(AthorArray);

		(*TeffArrayElementer) = baseArrayLen;

		//er +i riktig her??
		//(*TotaltTreff) = (*TeffArrayElementer) +i;	
		//(*TotaltTreff) = (*TeffArrayElementer) + MainArrayLen;
	}

	
		printf("TmpArray p %u\n",(unsigned int)TmpArray);
	free(TmpArray);


	#ifdef BLACK_BOKS
		//lager en oversikt over filformater
		/*
			Dette her blit en ganske komplisert sak. Vi har hash nøkler på fårmatet "subname-filtype", i en strukt.
			
			Vi har også med en peker til subname strukten, slik at vi kan manipulere denne direkte. Slik har vi
			bare en gjenomgang av dataene får å legge inn filtype infoen. Så en gjenomgang av alle subname-filetype
			som finnes for å lagge de inn i subname struktuen.

		*/

		gettimeofday(&start_time, NULL);

		char filetype[5];
		struct filesKeyFormat *filesKey;
		int *filesValue;
		struct hashtable *h;

		h = create_hashtable(200, fileshashfromkey, filesequalkeys);

		for (i = 0; i < (*TeffArrayElementer); i++) {
			printf("i = %i, subname \"%s\"\n",i,(*TeffArray[i].subname).subname);
			if (iintegerGetValueNoCashe(&filetype,4,TeffArray[i].DocID,"filtypes",(*TeffArray[i].subname).subname) == 0) {
				printf("woldent get integerindex\n");
			}
			else {
				#ifdef DEBUG
				printf("file \"%c%c%c%c\"\n",filetype[0],filetype[1],filetype[2],filetype[3]);
				#endif
				filesKey = malloc(sizeof(struct filesKeyFormat));
				(*filesKey).subname = TeffArray[i].subname;
				memcpy((*filesKey).filename,filetype,sizeof((*filesKey).filename)); 	

				if (NULL == (filesValue = hashtable_search(h,filesKey) )) {    
					printf("not found!. Vil insert first");
					filesValue = malloc(sizeof(int));
					(*filesValue) = 1;
					if (! hashtable_insert(h,filesKey,filesValue) ) {
						printf("cant insert\n");     
						exit(-1);
					}

		                }
				else {
					++(*filesValue);
				}
			}
		}

		for(i=0;i<nrOfSubnames;i++) {
                	subnames[i].nrOfFiletypes = 0;
		}
		/* Iterator constructor only returns a valid iterator if
		* the hashtable is not empty */

		if (hashtable_count(h) > 0)
		{
			struct hashtable_itr *itr;

       			itr = hashtable_iterator(h);
       			do {
       				filesKey = (struct filesKeyFormat *)hashtable_iterator_key(itr);
       				filesValue = (int *)hashtable_iterator_value(itr);

				printf("files \"%s-%s\": %i\n",(*(*filesKey).subname).subname,(*filesKey).filename,*filesValue);

				if ((*(*filesKey).subname).nrOfFiletypes < MAXFILTYPES) {
					memcpy((*(*filesKey).subname).filtypes[(*(*filesKey).subname).nrOfFiletypes].name,(*filesKey).filename,4);
					(*(*filesKey).subname).filtypes[(*(*filesKey).subname).nrOfFiletypes].name[5] = '\0';
					(*(*filesKey).subname).filtypes[(*(*filesKey).subname).nrOfFiletypes].nrof = (*filesValue);
					++(*(*filesKey).subname).nrOfFiletypes;
				}

       			} while (hashtable_iterator_advance(itr));
    			free(itr);

		}

		hashtable_destroy(h,1); 

		// sort by nrof
		for(i=0;i<nrOfSubnames;i++) {
			printf("qsort filtypes %i\n",subnames[i].nrOfFiletypes);
			qsort(subnames[i].filtypes,subnames[i].nrOfFiletypes,sizeof(struct subnamesFiltypesFormat),compare_filetypes);
		}

		gettimeofday(&end_time, NULL);
		(*queryTime).filetypes = getTimeDifference(&start_time,&end_time);


		#ifdef DEBUG
		//show subnames and hits in then
		for(i=0;i<nrOfSubnames;i++) {
                        printf("nrOfFiletypes %s: %i\n",subnames[i].subname,subnames[i].nrOfFiletypes);
			for(y=0;y<subnames[i].nrOfFiletypes;y++) {
				printf("files %s %i\n",subnames[i].filtypes[y].name,subnames[i].filtypes[y].nrof);
			}
                }
		#endif


	#endif

	// BLACK_BOKS har ikke pop rank, bare term rank
	#ifdef BLACK_BOKS


		gettimeofday(&start_time, NULL);


		printf("looking opp dates\n");

		//slår opp alle datoene
		for (i = 0; i < *TeffArrayElementer; i++) {
			iintegerGetValueNoCashe(&TeffArray[i].date,sizeof(int),TeffArray[i].DocID,"dates",(*TeffArray[i].subname).subname);
		}
		printf("looking opp dates end\n");

		gettimeofday(&end_time, NULL);
		(*queryTime).iintegerGetValueDate = getTimeDifference(&start_time,&end_time);


		gettimeofday(&start_time, NULL);
		//kjører dateview
		dateview dv;
		dateview *dvo;

		printf("dateview start\n");
		date_info_start(&dv, 0, -1);
		
		printf("for all dates\n");		
		for (i = 0; i < *TeffArrayElementer; i++) {
			date_info_add(&dv, (time_t)TeffArray[i].date);
		}
		printf("for all dates end\n");

		dvo = date_info_end(&dv);


		//inaliserer
		for (i=0;i<10;i++) {
			dates[i] = 0;
		}

		enum dateview_output_type type = TWO_YEARS_PLUS;
		for (i = 0; i < type; i++) {
		                printf("Type: %d Count: %d\n", i, dvo->output[i]);
				dates[i] = dvo->output[i];
	        }
		
		//for (; dvo != NULL; dvo = dvo->next) {
		//
		//	dates[(int)dvo->type] = dvo->length;
		//}
		printf("dateview end\n");

		gettimeofday(&end_time, NULL);
		(*queryTime).dateview = getTimeDifference(&start_time,&end_time);


		gettimeofday(&start_time, NULL);

		printf("order by \"%s\"\n",orderby);

		if (strcmp(orderby,"ddesc") == 0) {
			for (i = 0; i < *TeffArrayElementer; i++) {
				TeffArray[i].allrank = TeffArray[i].date;
			}
		}
		else if (strcmp(orderby,"dasc") == 0) {
			printf("do dasc sort\n");
			for (i = 0; i < *TeffArrayElementer; i++) {
				//4294967295 unsigned int (long) max
				//TeffArray[i].allrank = 4294967295 - TeffArray[i].allrank;
				TeffArray[i].allrank = ULONG_MAX - TeffArray[i].date;
			}			
		}
		else {
			printf("do normal sort\n");
			for (i = 0; i < *TeffArrayElementer; i++) {
				TeffArray[i].allrank = TeffArray[i].TermRank;
			}
		}
		gettimeofday(&end_time, NULL);
                (*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);
	#else
	//hvis vi har få traff bruker vi en annen rangering
	//if ((*TeffArrayElementer) < 2000) {
	if ((*TotaltTreff) < 2000) {

		gettimeofday(&start_time, NULL);
			printf("do term ranking\n");
			for (i = 0; i < *TeffArrayElementer; i++) {
				//TeffArray[i].PopRank = popRankForDocIDMemArray(TeffArray[i].DocID);
				//TeffArray[i].PopRank = TeffArray[i].PopRank * 4;
				//if (TeffArray[i].TermRank < 0) {
                        	//	TeffArray[i].allrank = 0;
                		//}
               			//else if (TeffArray[i].PopRank < 0) {
                		//        TeffArray[i].allrank = 0;
        	        	//}
        	        	//else {
				//legger til en her da vi kan ha 0, og vi har * med 0. Bør fikkses en anne plass. da 255++ rt 0
				++TeffArray[i].PopRank;

				//hvis vi har en treff som er i frase øker vi betydligheten av den siden
				//ToDo: bør ha noe annt en .PopRank * 2 her. Kansje en verdi som er 0, eller 
				//xx hvis vi har en frasetreff?
				if (TeffArray[i].phraseMatch) {
					TeffArray[i].PopRank = TeffArray[i].PopRank * 2;
				}

				TeffArray[i].allrank = floor((TeffArray[i].TermRank * TeffArray[i].TermRank) * TeffArray[i].PopRank);
				//TeffArray[i].allrank = TeffArray[i].TermRank + TeffArray[i].PopRank;
				//}
			}
			gettimeofday(&end_time, NULL);
			(*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);

	}
	else {

		gettimeofday(&start_time, NULL);

			for (i = 0; i < *TeffArrayElementer; i++) {
				//if (TeffArray[i].TermRank > 18) {
				//	TeffArray[i].allrank = TeffArray[i].PopRank;
				//	
				//}
				if (TeffArray[i].TermRank > TeffArray[i].PopRank) {
					TeffArray[i].allrank = TeffArray[i].PopRank * TeffArray[i].PopRank;
				}
				else {
					TeffArray[i].allrank = TeffArray[i].TermRank * TeffArray[i].PopRank;
				}
				

			}
                gettimeofday(&end_time, NULL);
                (*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);

	}
	#endif


	gettimeofday(&start_time, NULL);

		printf("vil sortere %i\n",*TeffArrayElementer);
 		qsort(TeffArray, *TeffArrayElementer , sizeof(struct iindexFormat), compare_elements);
		printf("sort ferdig\n");
	gettimeofday(&end_time, NULL);
	(*queryTime).indexSort = getTimeDifference(&start_time,&end_time);


		//temp:
		/********************************************************************************/
		//if ((*TotaltTreff) > 30) {
		//	for (i=0;i<30;i++) {
		//		TeffArray[i].allrank = TeffArray[i].TermRank;
		//	}
		//}
		//
		//qsort(TeffArray, 30 , sizeof(struct iindexFormat), compare_elements);
		/********************************************************************************/

}

//qsort(subnames[i].filtypes,subnames[i].nrOfFiletypes,sizeof(struct subnamesFiltypesFormat),compare_filetypes);

int compare_filetypes (const void *p1, const void *p2) {
        if (((struct subnamesFiltypesFormat*)p1)->nrof > ((struct subnamesFiltypesFormat*)p2)->nrof)
                return -1;
        else
                return ((struct subnamesFiltypesFormat*)p1)->nrof < ((struct subnamesFiltypesFormat*)p2)->nrof;

}

int compare_elements (const void *p1, const void *p2) {


        if (((struct iindexFormat*)p1)->allrank > ((struct iindexFormat*)p2)->allrank)
                return -1;
        else
                return ((struct iindexFormat*)p1)->allrank < ((struct iindexFormat*)p2)->allrank;

}

