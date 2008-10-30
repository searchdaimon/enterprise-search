#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "verbose.h"

//#include "../common/define.h"
#include "../boithoadClientLib/boithoad.h"
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
#include "../common/utf8-strings.h"
#include "../common/ht.h"
#include "../common/re.h"
#include "../common/bprint.h"
#include "../common/search_automaton.h"
#include "../ds/dcontainer.h"
#include "../ds/dvector.h"
#include "../ds/dpair.h"
#include "../crawlManager/client.h"


#ifdef BLACK_BOKS

	//#include "../common/commondate/dateview.h"
	#include "../getdate/dateview.h"
	#include "../getdate/getdate.h"
	#include "../acls/acls.c"
	#include "../ds/dcontainer.h"
	#include "../ds/dpair.h"
	#include "../ds/dtuple.h"
	#include "../ds/dstack.h"
	#include "../ds/dqueue.h"
	#include "../ds/dmap.h"
	#include "../ds/dmultimap.h"
	#include "../getFiletype/identify_extension.h"
	#include "../attributes/show_attributes.h"
	#include "../attributes/attr_makexml.h"
#endif

#include <string.h>
#include <limits.h>

#define popvekt 0.6
#define termvekt 0.4

#define responseShortToMin 20000

#include <math.h>
#include <stdlib.h>

#include "search.h"

#ifdef WITH_THREAD
        #include <pthread.h>

#endif

//rangeringsfilter er der vi har et hvist antall treff, definert som RFC (Rank Filter Cutoff).
//har vi så mnage treff filtrerer vi ut de under en hvis rankt
#define AthorRFC1 	10000
#define MainRFC1 	200000
#define MainRFC2 	10000

int compare_elements (const void *,const void *);
int compare_filetypes (const void *p1, const void *p2);

/*
struct filesKeyFormat {
	struct subnamesFormat *subname;
	char filename[4];
};
*/

void allrankcalk(struct iindexFormat *TeffArray ,int *TeffArrayElementer) {

	int i;
	int TermRankNormalized, PopRankNormalized;

			for (i = 0; i < *TeffArrayElementer; i++) {
				//if (TeffArray[i].TermRank > 18) {
				//	TeffArray[i].allrank = TeffArray[i].PopRank;
				//	
				//}
				//legger til en her da vi kan ha 0, og vi har * med 0. Bør fikkses en anne plass. da 255++ rt 0
				++TeffArray->iindex[i].PopRank;

				//temp: runarb 10.may 2007: er det rikig å ha denne her. Nå har vi vi bedre metoder for å ikke få mer en max term rank
				/*
				if (TeffArray->iindex[i].TermRank > TeffArray->iindex[i].PopRank) {
					TeffArray->iindex[i].allrank = TeffArray->iindex[i].PopRank * TeffArray->iindex[i].PopRank;
				}
				else {
					TeffArray->iindex[i].allrank = TeffArray->iindex[i].TermRank * TeffArray->iindex[i].PopRank;
				}
				*/
				//OLD
				//TeffArray[i].allrank = TeffArray[i].TermRank * TeffArray[i].PopRank;
				//New
				//hvis vi har en veldig lav termrank lar vi ikke popranken telle
				//dette for og hindre et sider som google.com kommer opp på ale mulige rare termer
				/*
				if (TeffArray[i].TermRank <= 2) {
					TeffArray[i].allrank = TeffArray[i].TermRank;
				}
				else {
					TeffArray[i].allrank = TeffArray[i].TermRank + TeffArray[i].PopRank;
				}
				*/
				/*
				if (TeffArray[i].PopRank > TeffArray[i].TermRank) {
					//TeffArray[i].allrank = (((TeffArray[i].PopRank+1)/(TeffArray[i].TermRank+1))*100);
					TeffArray[i].allrank = (TeffArray[i].PopRank+TeffArray[i].TermRank) * ((255/TeffArray[i].PopRank+1)*TeffArray[i].TermRank);
				}
				else {
					TeffArray[i].allrank = (((TeffArray[i].TermRank+1)/(TeffArray[i].PopRank+1))*100);
					TeffArray[i].allrank = (TeffArray[i].PopRank+TeffArray[i].TermRank) * ((255/TeffArray[i].TermRank+1)*TeffArray[i].PopRank);
				}
				*/
				/*
				if (TeffArray->iindex[i].TermRank > TeffArray->iindex[i].PopRank) {
					//TeffArray->iindex[i].allrank = TeffArray->iindex[i].PopRank * TeffArray->iindex[i].PopRank;
					
					if (TeffArray->iindex[i].TermRank > 30) {
						TeffArray->iindex[i].TermRank = 30;
					}
					
					//TeffArray->iindex[i].allrank = TeffArray->iindex[i].PopRank * TeffArray->iindex[i].TermRank;
					TeffArray->iindex[i].allrank = (TeffArray->iindex[i].PopRank * TeffArray->iindex[i].PopRank) + TeffArray->iindex[i].TermRank;

					
				}
				else {
					TeffArray->iindex[i].allrank = TeffArray->iindex[i].PopRank * TeffArray->iindex[i].TermRank;

				}
				*/

				TermRankNormalized = TeffArray->iindex[i].TermRank;

				if (TermRankNormalized > (TeffArray->iindex[i].PopRank +30)) {
					TermRankNormalized = (TeffArray->iindex[i].PopRank +30);
				}


				PopRankNormalized = TeffArray->iindex[i].PopRank;
 
				if (PopRankNormalized > (TeffArray->iindex[i].TermRank +30)) {
					PopRankNormalized = (TeffArray->iindex[i].TermRank +30);
				}

				TeffArray->iindex[i].allrank = (((100.0/255.0) * TermRankNormalized) * PopRankNormalized);
				//printf(" $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Got a score of: %d\n", TeffArray->iindex[i].allrank);

				//TeffArray->iindex[i].allrank = ((1000 / TeffArray->iindex[i].TermRank) * TeffArray->iindex[i].PopRank);

				//(3/120)*100
				//TeffArray[i].allrank = ((5 * TeffArray[i].TermRank) + (5 * TeffArray[i].PopRank));
			}

}

static unsigned int fileshashfromkey(void *ky)
{
    char *k = (char *)ky;
    //struct filesKeyFormat *k = (struct filesKeyFormat *)ky;
    //return ((unsigned int)k->subname + (unsigned int)k->filename[0] + (unsigned int)k->filename[1] + (unsigned int)k->filename[2] + (unsigned int)k->filename[3]);
	return((int)k[0]);
}

static int filesequalkeys(void *k1, void *k2)
{
    return (0 == strcmp(k1,k2));
}

int resultArrayInit(struct iindexFormat *array) {
	array->nrofHits = 0;
	array->phrasenr = 0;
}

static inline int rank_calc(int nr, char *rankArray,char rankArrayLen) {

	if (nr == 0) {
		return 0;
	}
	else if (nr >= rankArrayLen) {
		//printf("to large. rankArrayLen %i, nr %i, returning %i\n",rankArrayLen,nr,rankArray[rankArrayLen -1]);
		return rankArray[rankArrayLen -1];
	}
	else {
		//printf("in range. rankArrayLen  %i. nr %i, value %i\n",rankArrayLen,nr,rankArray[nr -1]);
		return rankArray[nr -1];
	}
}

static inline int rankUrl(const struct hitsFormat *hits, int nrofhit,const unsigned int DocID,struct subnamesFormat *subname,struct iindexMainElements *TeffArray, int complicacy) {
	int rankDomain, nrDomain , rankSub, nrSub, rank, i;

	nrDomain = 0;
	nrSub = 0;
	rankDomain = 0;
	rankSub = 0;

	for (i = 0;i < nrofhit; i++) {
	
		if (hits[i].pos == 2) {
        		nrDomain =+ 1;
        	}
        	else {
        		nrSub =+ 1;

        	}
	}

	if (nrDomain != 0) {
		rankDomain = (*subname).config.rankUrlMainWord;
	}
	rankSub = rank_calc(nrSub,(*subname).config.rankUrlArray,(*subname).config.rankUrlArrayLen);

	rank = rankDomain + rankSub;

	//printf("rankUrl: DocID %u, rank %i\n",DocID,rank);

	#ifdef EXPLAIN_RANK
		TeffArray->rank_explaind.rankUrlDomain = rankDomain;
		TeffArray->rank_explaind.rankUrlSub = rankSub;

		TeffArray->rank_explaind.nrUrlDomain = nrDomain;
		TeffArray->rank_explaind.nrUrlSub = nrSub;
	#endif

	return rank;
}

#define logrank(v,r,d) ((log((v * d) +1) * r)) 


static inline int rankAthor(const struct hitsFormat *hits, int nrofhit,const unsigned int DocID,struct subnamesFormat *subname,struct iindexMainElements *TeffArray, int complicacy) {
	int rank, i;


	rank = logrank(nrofhit,40,0.009);

	#ifdef EXPLAIN_RANK
		TeffArray->rank_explaind.nrAthorPhrase 	= 0;
		TeffArray->rank_explaind.nrAthor 	= nrofhit;
		TeffArray->rank_explaind.rankAthor 	= rank;
	#endif

	if (rank > maxPoengAthor) {
		rank = maxPoengAthor;
	}


	return rank;
}

static inline int rankAthor_complicacy(const struct hitsFormat *hits, int nrofhit,const unsigned int DocID,struct subnamesFormat *subname,struct iindexMainElements *TeffArray, int complicacy) {
	int rank, i, nr, phrasenr, phraserank, simplerank;

	nr = 0;
	phrasenr = 0;

	#ifdef DEBUG
	printf("rankAthor_complicacy: nrofhit %i\n",nrofhit);
	#endif

	for (i = 0;i < nrofhit; i++) {
		if (hits[i].phrase == 1) {
			phrasenr++;
		}
		else {
			nr++;
		}
	}


	simplerank = logrank((nr/complicacy),40,0.009);// deler på complicacy for ikke fraser da vi ser på en total for alle ord
	phraserank = logrank(phrasenr,40,0.009); 


	if (phraserank > complicacy_maxPoengAthorPhraserank) {
		phraserank = complicacy_maxPoengAthorPhraserank;
	}

	if (simplerank > complicacy_maxPoengAthorSimple) {
		simplerank = complicacy_maxPoengAthorSimple;
	}

	rank = phraserank + simplerank;

	#ifdef EXPLAIN_RANK
		TeffArray->rank_explaind.nrAthorPhrase 	= phrasenr;
		TeffArray->rank_explaind.nrAthor 	= nr;
		TeffArray->rank_explaind.rankAthor 	= rank;
	#endif

	return rank;
}


static inline int rankAcl(const struct hitsFormat *hits, int nrofhit,const unsigned int DocID,
		struct subnamesFormat *subname,struct iindexMainElements *TeffArray, int complicacy) {
	return 1;
}
static inline int rankMain(const struct hitsFormat *hits, int nrofhit,const unsigned int DocID,
	struct subnamesFormat *subname,struct iindexMainElements *TeffArray, int complicacy) {


	int i;
        int nrBody, nrHeadline, nrTittel, nrUrl, TittelFirstWord, nrTittelFirst;
	int havePhrase;
        //double poengDouble;

	#ifdef DEBUG_II
		//printer ut elementet
		printf("rankMain: DocID %u, subname \"%s\"\n",DocID,subname->subname);
		for (i = 0;i < nrofhit; i++) {
			printf("\thit %i, phrase %i\n",(int)hits[i].pos,(int)hits[i].phrase);
		}
	#endif


	nrBody		 = 0;
	nrHeadline	 = 0;
	nrTittel	 = 0;
	nrUrl		 = 0;
	TittelFirstWord  = 0;
	havePhrase	 = 0;
	nrTittelFirst	 = 0;

	//poengDouble 	 = 0;
	int rank;
        // kjører gjenom anttall hit
	for (i = 0;i < nrofhit; i++) {

		//sjekker om vi noengang har hatt søkeordet som en frase
		if (hits[i].phrase == 1) {
			havePhrase = 1;
		}

		//printf("\thit %i\n",(int)hits[i].pos);
                /*************************************************
                lagger til poeng
                *************************************************/
                if (hits[i].pos >= 1000) {      //Body
                	++nrBody;
                }
                else if (hits[i].pos >= 500) {  //Headline
                	++nrHeadline;
                }
                else if (hits[i].pos >= 100) {  //Tittel
			//spesialtilfelle. er først title ord 
			//Hvis vi er første ord i titleen vil det rangeres spesielt
			if (hits[i].pos == 100) {	
				//rank += (*subname).config.rankTittelFirstWord;
				TittelFirstWord = (*subname).config.rankTittelFirstWord;
				#ifdef DEBUG
				printf("rank title hit. add %i, TittelFirstWord %i, subname \"%s\", DocID %u\n",(*subname).config.rankTittelFirstWord,TittelFirstWord,(*subname).subname,DocID);
				#endif
				++nrTittelFirst;
			}
			//starter på 100, så det mø være under. For eks under 106 gir til 6
			else if (hits[i].pos < 106) {
                               	++nrTittel;
			}
			else {
				#ifdef DEBUG
					printf("Word to long into title. pos %i (starts at 100)\n",hits[i].pos);
				#endif
			}
               	}
                else if (hits[i].pos == 2) {

                	//poengUrl += poengForMainUrlWord;
               	}
                else if (hits[i].pos >= 1) {    //url
                	// ingen urler i body mere
                	//poengUrl += poengForUrl;
                }
                else {
                	//printf("Error, fikk inn 0 eller mindre\n");
                }
                /**************************************************/
        }

	rank = 0;

	#ifdef EXPLAIN_RANK
		TeffArray->rank_explaind.rankBody = rank_calc(nrBody,(*subname).config.rankBodyArray,(*subname).config.rankBodyArrayLen);
		TeffArray->rank_explaind.rankHeadline = rank_calc(nrHeadline,(*subname).config.rankHeadlineArray,(*subname).config.rankHeadlineArrayLen);
		TeffArray->rank_explaind.rankTittel = rank_calc(nrTittel,(*subname).config.rankTittelArray,(*subname).config.rankTittelArrayLen) + TittelFirstWord;
		TeffArray->rank_explaind.rankUrl_mainbody = rank_calc(nrUrl,(*subname).config.rankUrlArray,(*subname).config.rankUrlArrayLen);

		TeffArray->rank_explaind.nrBody = nrBody;
		TeffArray->rank_explaind.nrHeadline = nrHeadline;
		TeffArray->rank_explaind.nrTittel = nrTittel + nrTittelFirst;
		TeffArray->rank_explaind.nrUrl_mainbody = nrUrl;


		rank += TeffArray->rank_explaind.rankBody + TeffArray->rank_explaind.rankHeadline + TeffArray->rank_explaind.rankTittel + TeffArray->rank_explaind.rankUrl_mainbody;


	#else
		rank += rank_calc(nrBody,(*subname).config.rankBodyArray,(*subname).config.rankBodyArrayLen);
		rank += rank_calc(nrHeadline,(*subname).config.rankHeadlineArray,(*subname).config.rankHeadlineArrayLen);
		rank += rank_calc(nrTittel,(*subname).config.rankTittelArray,(*subname).config.rankTittelArrayLen) + TittelFirstWord;
		rank += rank_calc(nrUrl,(*subname).config.rankUrlArray,(*subname).config.rankUrlArrayLen);
	#endif

	if (havePhrase) {
		//Phrase boost
		rank = rank * 2;
	}

	#ifdef DEBUG_II
		printf("rankMain: DocID %u, nrofhit %i ,rank %i, havePhrase %i\n",DocID,nrofhit,rank,havePhrase);
	#endif

	return rank;

}                          
/*****************************************************************/

#ifdef EXPLAIN_RANK

static inline void rank_explaindSumm(struct rank_explaindFormat *t, struct rank_explaindFormat *a, struct rank_explaindFormat *b, int or) {
         t->rankBody		= a->rankBody + b->rankBody;
         t->rankHeadline	= a->rankHeadline + b->rankHeadline;
         t->rankTittel		= a->rankTittel + b->rankTittel;
         t->rankAthor		= a->rankAthor + b->rankAthor;
         t->rankUrl_mainbody	= a->rankUrl_mainbody + b->rankUrl_mainbody;
         t->rankUrlDomain		= a->rankUrlDomain + b->rankUrlDomain;
         t->rankUrlSub		= a->rankUrlSub + b->rankUrlSub;


         t->nrAthorPhrase	= a->nrAthorPhrase + b->nrAthorPhrase;
         t->nrAthor		= a->nrAthor + b->nrAthor;

         t->nrBody		= a->nrBody + b->nrBody;
         t->nrHeadline		= a->nrHeadline + b->nrHeadline;
         t->nrTittel		= a->nrTittel + b->nrTittel;
         t->nrUrl_mainbody	= a->nrUrl_mainbody + b->nrUrl_mainbody;
         t->nrUrlDomain		= a->nrUrlDomain + b->nrUrlDomain;
         t->nrUrlSub		= a->nrUrlSub + b->nrUrlSub;

	//dette er egentlig en hurti fiks, da vi kan kansje kan få mer en maks for querys med fler en et ord 
	if (or) {
         t->maxBody		= a->maxBody;
         t->maxHeadline		= a->maxHeadline;
         t->maxTittel		= a->maxTittel;
         t->maxUrl_mainbody	= a->maxUrl_mainbody;
         t->maxUrlDomain	= a->maxUrlDomain;
         t->maxUrlSub		= a->maxUrlSub;
         t->maxAthor		= a->maxAthor;
	}
	else {
         t->maxBody		= a->maxBody + b->maxBody;
         t->maxHeadline		= a->maxHeadline + b->maxHeadline;
         t->maxTittel		= a->maxTittel + b->maxTittel;
         t->maxUrl_mainbody	= a->maxUrl_mainbody + b->maxUrl_mainbody;
         t->maxUrlDomain	= a->maxUrlDomain + b->maxUrlDomain;
         t->maxUrlSub		= a->maxUrlSub + b->maxUrlSub;
         t->maxAthor		= a->maxAthor + b->maxAthor;

	}


}
#endif

void iindexArrayHitsCopy(struct iindexFormat *c, int k, struct iindexFormat *b, int j) {

	int x;

	for(x=0;x<b->iindex[j].TermAntall;x++) {
		#ifdef DEBUG_II
		printf("iindexArrayHitsCopy: b %hu\n",b->iindex[j].hits[x].pos);
		#endif
		c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
		c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
		++c->iindex[k].TermAntall;
		++c->nrofHits;
	}
}


void or_merge(struct iindexFormat *c, int *baselen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {

	int x;

	int i=0;
	int j=0;
	int k=0;
	(*baselen) = 0;

	printf("or_merge(alen %i, blen %i)\n",alen,blen);
	
	//debug: print ot verdiene før de merges
	#ifdef DEBUG_II	
	x=0;
	printf("a array (100 max):\n");
	while (x<alen && x<100){
                printf("\t%u\n",a->iindex[x].DocID);
		++x;
	}
	
	x=0;
	printf("b array (100 max):\n");
	while (x<blen && x<100) {
                printf("\t%u\n",b->iindex[x].DocID);
		++x;
	}
	#endif

	while ((i<alen) && (j<blen) && (k < maxIndexElements))
	{

	//printf("or_merge: merge a: %i (\"%s\"), b: %i (\"%s\")\n",a[i].DocID,(*a[i].subname).subname,b[j].DocID,(*b[i].subname).subname);

	//if (b[j].DocID == 2788928) {
	//	printf("b: %i , %i\n",a[i].DocID,b[j].DocID);
	//	printf("b TermAntall %i\n",b[j].TermAntall);
	//}
		if (a->iindex[i].DocID == b->iindex[j].DocID) {
			#ifdef DEBUG_II
				printf("or_merge: %i == %i\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
			//c[k] = a[i];

			//c[k].TermRank = a[i].TermRank + b[j].TermRank;		

			//TermRank = a[i].TermRank + b[j].TermRank;
                        //c[k] = a[i];
                        //c[k].TermRank = TermRank;

			c->iindex[k] = a->iindex[i];
			c->iindex[k].TermRank = a->iindex[i].TermRank + b->iindex[j].TermRank;
			c->iindex[k].phraseMatch = a->iindex[i].phraseMatch + b->iindex[j].phraseMatch;

			#ifdef EXPLAIN_RANK
				rank_explaindSumm(&c->iindex[k].rank_explaind,&a->iindex[i].rank_explaind,&b->iindex[j].rank_explaind,1);
			#endif

			//copying hits
			//#ifdef DEBUG_II
			//printf("or_merge: hits a nr %hu, b nr %hu\n",a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			//#endif

			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			iindexArrayHitsCopy(c,k,a,i);
			iindexArrayHitsCopy(c,k,b,j);

			/*
			for(x=0;x<a->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("or_merge: a %hu\n",a->iindex[i].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			for(x=0;x<b->iindex[j].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("or_merge: b %hu\n",b->iindex[j].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			*/

			++k; ++j; ++i;
			++(*baselen);
		}
 		else if( a->iindex[i].DocID < b->iindex[j].DocID ) {


			#ifdef DEBUG_II
				printf("or_merge: a: %i < b: %i = copy a\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif

			if (&c->iindex[k] != &a->iindex[i]) {
                		c->iindex[k] = a->iindex[i];
			}
			//copying hits
			//#ifdef DEBUG_II
			//printf("or_merge: hist a %hu, b %hu\n",a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			//#endif
			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			iindexArrayHitsCopy(c,k,a,i);

			/*
			for(x=0;x<a->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("or_merge: a %hu\n",a->iindex[i].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			*/
			
			++i; 
			++k;
			++(*baselen);
		}
 		else {
			#ifdef DEBUG_II
				printf("or_merge: a: %i > b: %i = copy b\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif

			if (&c->iindex[k] != &b->iindex[j]) {
	                	c->iindex[k] = b->iindex[j];
			}
			//copying hits
			//#ifdef DEBUG_II
			//printf("or_merge: hits a nr %hu, b nr %hu\n",a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			//#endif
			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			iindexArrayHitsCopy(c,k,b,j);
			/*
			for(x=0;x<b->iindex[j].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("or_merge: b %hu\n",b->iindex[j].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			*/
			++j; 
			++k;
			++(*baselen);
		}
	}

	vboprintf("i %i, alen %i, j %i, blen %i. k %i\n",i,alen,j,blen,k);

	//runarb: 14 mai
	while (i<alen && (k < maxIndexElements)){

		//#ifdef DEBUG_VALGRIND
			//her kan minne overlappe. For å ungå det sjekker vi først om pekere til struktene er like
			if (&c->iindex[k] != &a->iindex[i]) {
		                c->iindex[k] = a->iindex[i];
			}
		//#else
	        //        c->iindex[k] = a->iindex[i];
		//#endif

			//copying hits
			//#ifdef DEBUG_II
			//printf("or_merge: hits a nr %hu, b nr %hu\n",a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			//#endif
			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			//runarb: 14 mai
			//iindexArrayHitsCopy(c,k,b,j);
			iindexArrayHitsCopy(c,k,a,i);
			/*
			for(x=0;x<b->iindex[j].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("or_merge: b %hu\n",b->iindex[j].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			*/

		++k; ++i;
		++(*baselen);
	}
	
	//runarb: 14 mai
	while (j<blen && (k < maxIndexElements)) {

			if (&c->iindex[k] != &b->iindex[j]) {
	                	c->iindex[k] = b->iindex[j];
			}
			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			//runarb: 14 mai
			//iindexArrayHitsCopy(c,k,a,i);
			iindexArrayHitsCopy(c,k,b,j);
			/*
			for(x=0;x<a->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("or_merge: a %hu\n",a->iindex[i].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			*/

		++k; ++j;
		++(*baselen);
	}

	#ifdef DEBUG_II
	printf("or_merge result (100 max):\n");
	x=0;
	while ((x<(*baselen)) && (x<100)) {
                printf("\t%u\n",c->iindex[x].DocID);
		++x;
	}
	#endif

	vboprintf("~or_merge a and b of length %i %i. Into %i\n",alen,blen,(*baselen));
}

void andNot_merge(struct iindexFormat *c, int *baselen, int *added,struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
	int i=0;
	int j=0;
	int x;

	#ifdef DEBUG
	printf("andNot_merge: start\n");
	printf("andNot_merge: baselen %i\n",(*baselen),added);
	#endif

	//temp: (*baselen) = 0;
	int k=(*baselen);

	while ((i<alen) && (j<blen) && (k < maxIndexElements))
	{

	//printf("or_merge: merge a: %i (\"%s\"), b: %i (\"%s\")\n",a[i].DocID,(*a[i].subname).subname,b[j].DocID,(*b[i].subname).subname);

		if (a->iindex[i].DocID == b->iindex[j].DocID) {
			#ifdef DEBUG_II
			printf("andNot_merge: Not DocID %u\n",a->iindex[i].DocID);
			#endif
			++j; ++i;
		}
 		else if( a->iindex[i].DocID < b->iindex[j].DocID ) {
			#ifdef DEBUG_II
			printf("andNot_merge: DocID %u < DocID %u. Add\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
                	c->iindex[k] = a->iindex[i];
			
			c->iindex[k].hits = &c->hits[c->nrofHits];

			//kopierer hits
			c->iindex[k].TermAntall = 0;
			#ifdef DEBUG_II
			printf("a TermAntall %i, c->nrofHits %i\n",a->iindex[i].TermAntall,c->nrofHits);
			printf("size %i\n",sizeof(c->iindex[k]));
			#endif
			for(x=0;x<a->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("aaa %hu\n",a->iindex[i].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;

			}
			
			++i; 
			++k;
			++(*baselen);
		}
 		else {
			#ifdef DEBUG_II
			printf("andNot_merge: DocID %u > DocID %u. Wont add\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
	                //c[k] = b[j];

			++j; 
			//++k;
			//++(*baselen);
		}
		
	}
	#ifdef DEBUG
	printf("andNot_merge: end of one array i %i, alen %i, j %i, blen %i. k %i\n",i,alen,j,blen,k);
	printf("andNot_merge: i %i, alen %i, blen %i\n",i,alen,blen);
	#endif
	while (i<alen && (k < maxIndexElements)){

                c->iindex[k] = a->iindex[i];

		//kopierer hits
		c->iindex[k].TermAntall = 0;
		c->iindex[k].hits = &c->hits[c->nrofHits];

		for(x=0;x<a->iindex[i].TermAntall;x++) {
			#ifdef DEBUG_II
			printf("pos %hu\n",a->iindex[i].hits[x].pos);
			#endif
			c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
			c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
			++c->iindex[k].TermAntall;
			++c->nrofHits;

		}
		#ifdef DEBUG_II
		printf("andNot_merge: overflow DocID %u\n",a->iindex[i].DocID);
		#endif

		++k; ++i;
		++(*baselen);
	}

	#ifdef DEBUG_II
	printf("andNot_merge: have:\n");
	for(i=0;i<k;i++) {
		printf("\tDocID: %u\n",c->iindex[i].DocID);
	}
	#endif

	(*added) = k;

	#ifdef DEBUG
	printf("andNot_merge: a and b of length %i %i. Into %i\n",alen,blen,(*baselen));
	printf("andNot_merge: end\n");
	#endif

}

void and_merge(struct iindexFormat *c, int *baselen, int originalLen, int *added, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
	int i=0,j=0;
	int TermRank;

	int x;
	int k=originalLen;

	//runarb: 28, jul 2007
	//bytter slik at vi har totalt elementer i array
	//(*baselen) = 0;
	(*baselen) = originalLen;

	printf("and_merge(originalLen=%i, alen=%i, blen=%i)\n",originalLen,alen,blen);

	#ifdef DEBUG_II
	printf("a, first hits (of %i total):\n",alen);
	for(x=0;x<alen && x<100;x++) {
		printf("\t%u\n",a->iindex[x].DocID);
	}
	printf("b, first hits (of %i total):\n",blen);
	for(x=0;x<blen && x<100;x++) {
		printf("\t%u\n",b->iindex[x].DocID);
	}
	#endif

	#ifdef DEBUG_II
	printf("and_merge: start\n");
	printf("and_merge:  originalLen %i\n",originalLen);
	#endif

	while (i<alen && j<blen)
	{
		if (a->iindex[i].DocID == b->iindex[j].DocID) {
			#ifdef DEBUG_II
				printf("\t%i == %i\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
			//c[k] = a[i];

			//c[k].TermRank = a[i].TermRank + b[j].TermRank;		

			TermRank = a->iindex[i].TermRank + b->iindex[j].TermRank;
                        c->iindex[k] = a->iindex[i];
                        c->iindex[k].TermRank = TermRank;
			c->iindex[k].phraseMatch = 0;


			//copying hits
			#ifdef DEBUG_II
			printf("and_merge: hist a %hu, b %hu\n",a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			#endif
			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			for(x=0;x<a->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("aaa %hu\n",a->iindex[i].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			for(x=0;x<b->iindex[j].TermAntall;x++) {
				#ifdef DEBUG_II
				printf("bbb %hu\n",b->iindex[j].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			

			k++; j++; i++;
			(*baselen)++;
		}
 		else if( a->iindex[i].DocID < b->iindex[j].DocID ) {
			#ifdef DEBUG_II
				printf("\t%i < %i\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
   			//c[k++] = a[i++];
			i++;
		}
 		else {
			#ifdef DEBUG_II	
				printf("\t%i > %i\n",a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
   			//c[k++] = b[j++];
			j++;
		}
	}

	(*added) = k;

	vboprintf("and_merge a and b of length %i %i, into %i, starting to add on element %i\n",alen,blen,(*baselen),originalLen);

}





//and søk med progsymasjon () mere vekt  hvis ordene er nerme en fjernt.
void andprox_merge(struct iindexFormat *c, int *baselen, int originalLen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
        int i=0,j=0;
	int k=originalLen;
	int y,x;
	int distance;
	int ah,bh;
	int TermRank;
        (*baselen) = 0;
	int found;
	
	#ifdef DEBUG
	struct timeval start_time, end_time;

	gettimeofday(&start_time, NULL);
	#endif

	vboprintf("a nrofHits %i, b nrofHits %i. (c nrofHits %i)\n",a->nrofHits,b->nrofHits,c->nrofHits);

        while (i<alen && j<blen)
        {


                if (a->iindex[i].DocID == b->iindex[j].DocID) {

                        //printf("%i == %i\n",a[i].DocID,b[j].DocID);

			c->iindex[k] = a->iindex[i];

			
			//20 mai 2007. Usikker på om dette er så lurt. fjerner for nå			

			//TermRank = a[i].TermRank + b[j].TermRank;
			//termrank blir den verdien som er minst. Det gjør at det lønner seg å ha 
			//høye verdier for begge. Ikke slik at et dokument som er "ord1, ord1, ord1 ord2"
			//komer bra ut på søk for både ord1 og ord2. Da ord2 forekommer skjelden.
			if (a->iindex[i].TermRank < b->iindex[j].TermRank) {
				TermRank = a->iindex[i].TermRank;
			}
			else {
				TermRank = b->iindex[j].TermRank;
			}
				

			c->iindex[k].TermRank = TermRank;
			
			//c[k].TermRank = ((a[i].TermRank + a[i].TermRank) / 2);

			c->iindex[k].phraseMatch = 0;

			#ifdef EXPLAIN_RANK
				rank_explaindSumm(&c->iindex[k].rank_explaind,&a->iindex[i].rank_explaind,&b->iindex[j].rank_explaind,0);
			#endif


			ah = bh = 0;


			//c->iindex[k].TermAntall = 0;
			int TermAntall;

			TermAntall = 0;
			//c->iindex[k].hits = &c->hits[c->nrofHits];
			struct hitsFormat *hits;

			hits = &c->hits[c->nrofHits];

			//printf("ah %i TermAntall %i, bh %i TermAntall %i, MaxsHitsInIndex %i\n",ah,a->iindex[i].TermAntall,bh,b->iindex[j].TermAntall,MaxsHitsInIndex);

			found = 0;
			//lopper gjenom alle hitene og finner de som er rett etter hverandre
			while ((ah < a->iindex[i].TermAntall)  && (bh < b->iindex[j].TermAntall)) {

				if (c->nrofHits == maxTotalIindexHits) {
					#ifdef DEBUG
					printf("have now maxTotalIindexHits hits in hits array\n");
					#endif
					break;
				}


				//sjekker om dette er en frase. Altså at "ord2" kommer ret etter "ord1"
				if ((b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos) == 1) {
					#ifdef DEBUG
					//if (c->iindex[k].DocID == 9516391) {
						printf("frase hit DocID %u, hit %hu %hu\n",a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
					//}
					#endif
					//c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[bh].pos;
					hits[TermAntall].pos = b->iindex[j].hits[bh].pos;
					
					if ((a->iindex[i].hits[ah].phrase != -1) && (b->iindex[j].hits[bh].phrase != -1)) {
						//c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 1;
						hits[TermAntall].phrase = 1;
						++c->phrasenr;
					}
					else {
						//c->iindex[k].hits[c->iindex[k].TermAntall].phrase = -1;
						hits[TermAntall].phrase = -1;
						--c->phrasenr;
					}
					//++c->iindex[k].TermAntall;
					++TermAntall;

					c->nrofHits++;
					found = 1;
					ah++;
					bh++;
				}
				else if (b->iindex[j].hits[bh].pos > a->iindex[i].hits[ah].pos) {
				//else if (a[i].hits[ah].pos < b[j].hits[bh].pos) {
					#ifdef DEBUG
					//if (c->iindex[k].DocID == 9516391) {
						printf("NOT frase hit DocID %u, hit a: %hu b: %hu. ah++\n",a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
					//}
					#endif
			
					//c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[ah].pos;
					//c->iindex[k].hits[c->iindex[k].TermAntall].phrase = -1;
					hits[TermAntall].pos = a->iindex[i].hits[ah].pos;
					hits[TermAntall].phrase = -1;
					//++c->iindex[k].TermAntall;
					++TermAntall;

					c->nrofHits++;
					
					//går videre
					ah++;
				}
				else {
					#ifdef DEBUG
					//if (c->iindex[k].DocID == 9516391) {
						printf("NOT frase hit DocID %u, hit a: %hu b: %hu. bh++\n",a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
					//}
					#endif
			
					//c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[bh].pos;
					//c->iindex[k].hits[c->iindex[k].TermAntall].phrase = -1;
					hits[TermAntall].pos = b->iindex[j].hits[bh].pos;
					hits[TermAntall].phrase = -1;
					//++c->iindex[k].TermAntall;
					++TermAntall;

					c->nrofHits++;
			
					//går videre
					bh++;
				}

						

			}

			if (found) {
				c->iindex[k].phraseMatch = 1;
			}

			c->iindex[k].TermAntall = TermAntall;
			c->iindex[k].hits = hits;


			/*
			for (x=0;x<c->iindex[k].TermAntall;x++) {
				printf("%hu %i\n",c->iindex[k].hits[x].pos,c->iindex[k].hits[x].phrase);
			}
			*/

                        k++; j++; i++;
                        (*baselen)++;
                }
                else if( a->iindex[i].DocID < b->iindex[j].DocID ) {
                        //printf("%i < %i\n",a[i].DocID,b[j].DocID);

                        //c[k++] = a[i++];
			//(*baselen)++;
                        i++;
                }
                else {
                        //printf("%i > %i\n",a[i].DocID,b[j].DocID);
                        //c[k++] = b[j++];
			//(*baselen)++;
                        j++;
                }
        }

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: andprox_merge %u\n",getTimeDifference(&start_time,&end_time));
	#endif

	vboprintf("andprox_merge: a and b of length %i %i. Into %i\n",alen,blen,(*baselen));
	vboprintf("andprox_merge: nrofHits: %i\n",c->nrofHits);

}

//hopper over orde. Må da flytte alle ordene et hakk bortover
void frase_stopword(struct iindexFormat *c, int clen) {
	int i,y;

	for (i=0;i<clen;i++) {

		for(y=0;y<c->iindex[i].TermAntall;y++) {
			c->iindex[i].hits[y].pos++;
		}
	
	}

}




// iindexArrayCat(TeffArray,TeffArrayOriginal,tmpAnser,tmpAnserElementer);



//!!!!!!! utestet, fungerer bare kansje

void iindexArrayCopy2(struct iindexFormat *c, int *baselen,int Originallen, struct iindexFormat *a, int alen) {

	fprintf(stderr, "search: iindexArrayCopy2() Warning! This function has not been tested.\n");

	int x;
        int i=0,j=0;
	int k=Originallen;
	int y;
	int ah,bh;
	//int hitcount;
	int found;
	int TermRank;
	//unsigned short hits[MaxTermHit];
        (*baselen) = Originallen;

	while (i<alen && (k < maxIndexElements)){

                c->iindex[k] = a->iindex[i];

		c->iindex[k].TermAntall = 0;
		c->iindex[k].hits = &c->hits[c->nrofHits];

		for(x=0;x<a->iindex[i].TermAntall;x++) {
			#ifdef DEBUG_II
			printf("aaa %hu\n",a->iindex[i].hits[x].pos);
			#endif
			c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
			c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
			++c->iindex[k].TermAntall;
			++c->nrofHits;
		}

		++k; ++i;
		++(*baselen);
	}

}

//frasesk. Denne er dog ikke bra, egentlig en versjon av andprox_merge der bare de sidene med distanse 1 blir med
void frase_merge(struct iindexFormat *c, int *baselen,int Originallen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
        int i=0,j=0;
	int k=Originallen;
	int y;
	int ah,bh;
	//int hitcount;
	int found;
	int TermRank;
	//unsigned short hits[MaxTermHit];
	//runarb: 21 aug 2007: tror vi har byttet til å returnere totalt antal dokumeneter i array nå, ikke bare de som ble lagt til. Da kan vi ikke sette denet til 0
        //(*baselen) = 0;
        (*baselen) = Originallen;

	struct hitsFormat *hits;
	int TermAntall;

	debug("frase_merge: start");
	debug("have %i hits from before",Originallen);
	debug("frase_merge: merging array a of len %i to b of len %i",alen,blen);
	debug("frase_merge: arrays %u %u",(unsigned int)a,(unsigned int)b);

	#ifdef DEBUG
	printf("frase_merge: from before\n");
	for (i=0;i<*baselen;i++) {
		printf("\t DocID %u, subname \"%s\"\n",c->iindex[i].DocID,(*c->iindex[i].subname).subname);
	}

	#endif

        while (i<alen && j<blen)
        {
                if (a->iindex[i].DocID == b->iindex[j].DocID) {



			#ifdef DEBUG
                        	printf("Have DocID match %u == %u\n",a->iindex[i].DocID,b->iindex[j].DocID);
                        
				printf("a: (TermAntall %i): ",a->iindex[i].TermAntall);
				for (y=0; (y < a->iindex[i].TermAntall) && (y < MaxTermHit); y++) {
                        		printf("%hu ",a->iindex[i].hits[y].pos);
        	        	}
				printf("\n");
				
				printf("b: (TermAntall %i): ",b->iindex[j].TermAntall);
                        	for (y=0; (y < b->iindex[j].TermAntall) && (y < MaxTermHit); y++) {
                        	        printf("%hu ",b->iindex[j].hits[y].pos);
                        	}
				printf("\n");
			#endif

                   	//c->iindex[k] = b->iindex[j];
	               	c->iindex[k] = a->iindex[i];

			//c->iindex[k].TermAntall = 0;
                        //c->iindex[k].hits = &c->hits[c->nrofHits];



			TermAntall = 0;

                        hits = &c->hits[c->nrofHits];


			ah = bh = 0;
			//hitcount = 0;
			found = 0;
			//lopper gjenom alle hitene og finner de som er rett etter hverandre


			debug("a TermAntall %i, b TermAntall %i\n",a->iindex[i].TermAntall,b->iindex[j].TermAntall);

			while ((ah < a->iindex[i].TermAntall) && (bh < b->iindex[j].TermAntall)) {

				//sjekker om dette er en frase. Altså at "ord2" kommer ret etter "ord1"
				if ((b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos) == 1) {
					found = 1;
					//hits[hitcount].pos = b->iindex[j].hits[bh].pos;
					//c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[bh].pos;
					//c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 1;
					hits[TermAntall].pos = b->iindex[j].hits[bh].pos;
					hits[TermAntall].phrase = 1;
					++c->phrasenr;

					debug("frase_merge: frase hit DocID %u %hu %hu is now %hu",c->iindex[k].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos,c->iindex[k].hits[c->iindex[k].TermAntall].pos);

					//c->iindex[k].TermAntall++;
					TermAntall++;
					c->nrofHits++;

					//hitcount++;
					ah++;
					bh++;

				}
				else if (a->iindex[i].hits[ah].pos < b->iindex[j].hits[bh].pos) {
					debug("frase_merge: not a hit a: %hu b: %hu, dist %i ah++",b->iindex[j].hits[bh].pos,a->iindex[i].hits[ah].pos,(b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos));
					ah++;
				}
				else {
					debug("frase_merge: not a hit a: %hu b: %hu, dist%i. bh++",b->iindex[j].hits[bh].pos,a->iindex[i].hits[ah].pos,(b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos));
					bh++;
				}

						

			}



                       	//c->iindex[k] = b->iindex[j];

			c->iindex[k].TermAntall = TermAntall;
			c->iindex[k].hits = hits;

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
                        	if (a->iindex[i].TermRank < b->iindex[j].TermRank) {
                                	TermRank = a->iindex[i].TermRank;
                        	}
                        	else {
                                	TermRank = b->iindex[j].TermRank;
                        	}
                        	//c->iindex[k] = b->iindex[j];
                        	c->iindex[k].TermRank = TermRank;
				c->iindex[k].phraseMatch = 1;


				/*
				for(y=0;y<hitcount;y++) {
                                        c->iindex[k].hits[y] = hits[y];
                                }
				*/

                        	(*baselen)++;
				k++;
			}

			//c[k].TermAntall = hitcount;

                        j++; i++;
                }
                else if( a->iindex[i].DocID < b->iindex[j].DocID ) {
                        //printf("%i < %i\n",a->iindex[i].DocID,b->iindex[j].DocID);

                        //c[k++] = a[i++];
                        i++;
                }
                else {
                        //printf("%i > %i\n",a->iindex[i].DocID,b->iindex[j].DocID);
                        //c[k++] = b[j++];
                        j++;
                }




        }

	#ifdef DEBUG
	printf("frase_merge: results\n");
	for (i=0;i<*baselen;i++) {
		printf("\t DocID %u, subname \"%s\"\n",c->iindex[i].DocID,(*c->iindex[i].subname).subname);
	}

	#endif

	vboprintf("frase_merge: got %i new elements. k %i, baselen %i\n",*baselen,k,*baselen);
	debug("frase_merge: end");

}


void searchIndex_filters(query_array *queryParsed, struct filteronFormat *filteron) {
	int i,len,j;
	//dagur:
	// Ax: Disse frigjÃ¸res i searchkernel.c:
	(*filteron).attributes = vector_container( pair_container( int_container(), string_container() ) );
	(*filteron).collection = NULL;
	(*filteron).date = NULL;
	(*filteron).sort = NULL;

	for (i=0; i<(*queryParsed).n; i++)
        {
            //struct text_list *t_it = (*queryParsed).elem[i];

            vboprintf("Search type %c\n", (*queryParsed).query[i].operand );

	    len = (*queryParsed).query[i].n -1;
	    for (j=0; j<(*queryParsed).query[i].n; j++)
		len+= strlen((*queryParsed).query[i].s[j]);

	    char	*buf = malloc(sizeof(char) * (len+1));

	    len = sprintf(buf, "%s", (*queryParsed).query[i].s[0]);
	    for (j=1; j<(*queryParsed).query[i].n; j++)
		len+= sprintf(&(buf[len]), " %s", (*queryParsed).query[i].s[j]);
	    buf[len] = '\0';

		switch ((*queryParsed).query[i].operand) {

			case QUERY_COLLECTION:
			    {
//				strscpy((*filteron).collection, buf, sizeof((*filteron).collection));
				(*filteron).collection = strdup(buf);
				vboprintf("Filtering on collection: \"%s\"\n",(*filteron).collection);
				break;
			    }
			case QUERY_GROUP:
			    {
//				(*filteron).filetype = (*queryParsed).query[i].s[0];
//				strscpy((*filteron).filetype, buf, sizeof((*filteron).collection));
				vector_pushback((*filteron).attributes, QUERY_GROUP, buf);
				vboprintf("Filtering on attributes: \"group/%s\"\n", buf);
//				free(buf);
				break;
			    }
			case QUERY_FILETYPE:
			    {
				vector_pushback((*filteron).attributes, QUERY_FILETYPE, buf);
				vboprintf("Filtering on attributes: \"filetype/%s\"\n", buf);
//				free(buf);
				break;
			    }
			case QUERY_ATTRIBUTE:
			    {
				vector_pushback((*filteron).attributes, QUERY_ATTRIBUTE, buf);
				vboprintf("Filtering on attributes: \"%s\"\n", buf);
//				free(buf);
				break;
			    }
			case QUERY_DATE:
			    {
//				(*filteron).date = (*queryParsed).query[i].s[0];
//				strscpy((*filteron).date, buf, sizeof((*filteron).collection));
				(*filteron).date = strdup(buf);
				vboprintf("Filtering on date: \"%s\"\n",(*filteron).date);
				break;
			    }
			case QUERY_SORT:
			    {
//				(*filteron).sort = (*queryParsed).query[i].s[0];
//				strscpy((*filteron).sort, buf, sizeof((*filteron).sort));
				(*filteron).sort = strdup(buf);
				vboprintf("wil filter on sort: \"%s\"\n",(*filteron).sort);
				break;
			    }
		}

	    free(buf);
	}
}
/*******************************************/


void rankUrlArray(int TeffArrayElementer, struct iindexFormat *TeffArray, int complicacy) {
	int y;

	for (y=0; y<TeffArrayElementer; y++) {
		TeffArray->iindex[y].TermRank = rankUrl(TeffArray->iindex[y].hits,TeffArray->iindex[y].TermAntall,
			TeffArray->iindex[y].DocID,TeffArray->iindex[y].subname,&TeffArray->iindex[y],complicacy);
	}
}
void rankAthorArray(int TeffArrayElementer, struct iindexFormat *TeffArray, int complicacy) {
	int y;

	if (complicacy == 1) {
		for (y=0; y<TeffArrayElementer; y++) {
			TeffArray->iindex[y].TermRank = rankAthor(TeffArray->iindex[y].hits,TeffArray->iindex[y].TermAntall,
				TeffArray->iindex[y].DocID,TeffArray->iindex[y].subname,&TeffArray->iindex[y],complicacy);
		}
	}
	else {
		for (y=0; y<TeffArrayElementer; y++) {
			TeffArray->iindex[y].TermRank = rankAthor_complicacy(TeffArray->iindex[y].hits,TeffArray->iindex[y].TermAntall,
				TeffArray->iindex[y].DocID,TeffArray->iindex[y].subname,&TeffArray->iindex[y],complicacy);
		}

	}
}
void rankAclArray(int TeffArrayElementer, struct iindexFormat *TeffArray, int complicacy) {
	int y;

	for (y=0; y<TeffArrayElementer; y++) {
		TeffArray->iindex[y].TermRank = rankAcl(TeffArray->iindex[y].hits,TeffArray->iindex[y].TermAntall,
			TeffArray->iindex[y].DocID,TeffArray->iindex[y].subname,&TeffArray->iindex[y],complicacy);
	}
}
void rankMainArray(int TeffArrayElementer, struct iindexFormat *TeffArray, int complicacy) {
	int y;

	for (y=0; y<TeffArrayElementer; y++) {
		TeffArray->iindex[y].TermRank = rankMain(TeffArray->iindex[y].hits,TeffArray->iindex[y].TermAntall,
			TeffArray->iindex[y].DocID,TeffArray->iindex[y].subname,&TeffArray->iindex[y],complicacy);
	}
}

void explain_max_rankArray(int TeffArrayElementer, struct iindexFormat *TeffArray, int complicacy) {
	int y;

	for (y=0; y<TeffArrayElementer; y++) {
		if (complicacy == 1) {
			TeffArray->iindex[y].rank_explaind.maxAthor 	= complicacy_maxPoengAthorPhraserank + complicacy_maxPoengAthorSimple;
		} else {
			TeffArray->iindex[y].rank_explaind.maxAthor 	= maxPoengAthor;
		}

		TeffArray->iindex[y].rank_explaind.maxBody = TeffArray->iindex[y].subname->config.rankBodyArray[TeffArray->iindex[y].subname->config.rankBodyArrayLen -1];
		TeffArray->iindex[y].rank_explaind.maxHeadline = TeffArray->iindex[y].subname->config.rankHeadlineArray[TeffArray->iindex[y].subname->config.rankHeadlineArrayLen -1];
		TeffArray->iindex[y].rank_explaind.maxTittel = TeffArray->iindex[y].subname->config.rankTittelArray[TeffArray->iindex[y].subname->config.rankTittelArrayLen -1] + TeffArray->iindex[y].subname->config.rankTittelFirstWord;
		TeffArray->iindex[y].rank_explaind.maxUrl_mainbody = TeffArray->iindex[y].subname->config.rankUrlArray[TeffArray->iindex[y].subname->config.rankUrlArrayLen -1];
		TeffArray->iindex[y].rank_explaind.maxUrlDomain = TeffArray->iindex[y].subname->config.rankUrlMainWord;
		TeffArray->iindex[y].rank_explaind.maxUrlSub = TeffArray->iindex[y].subname->config.rankUrlArray[TeffArray->iindex[y].subname->config.rankUrlArrayLen -1];




		//TeffArray->iindex[y].TermRank = rankMain(TeffArray->iindex[y].hits,TeffArray->iindex[y].TermAntall,
		//	TeffArray->iindex[y].DocID,TeffArray->iindex[y].subname,&TeffArray->iindex[y],complicacy);
	}
}
int searchIndex_getnrs(char *indexType,query_array *queryParsed,struct subnamesFormat *subname,int languageFilterNr,
                int languageFilterAsNr[]) {
	int nr;

	//dagur: 
	nr = 0;


	int nterm;
	int i, j;
	char queryelement[128];
	unsigned int WordIDcrc32;
	for (i=0; i<(*queryParsed).n; i++)
        {
            //struct text_list *t_it = (*queryParsed).elem[i];

            vboprintf("Search type %c\n", (*queryParsed).query[i].operand );



		switch ((*queryParsed).query[i].operand) {

			case '+':


					//(*TeffArrayElementer) = 0;

					queryelement[0] = '\0';
		                	//while ( t_it!=NULL )
                			//{
					for (j=0; j<(*queryParsed).query[i].n; j++) {
                    				vboprintf("aa_ søker på \"%s\"\n", (*queryParsed).query[i].s[j]);
                    				strncat(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));
			
                				vboprintf("queryelement:  %s\n", queryelement);

						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						vboprintf("searchIndex: WordIDcrc32 %u\n",WordIDcrc32);
						if (i == 0) {
					
							//GetIndexAsArray(TeffArrayElementer,TeffArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);
							//rank((*TeffArrayElementer),TeffArray,subname,(*complicacy));
							//void GetNForTerm(unsigned long WordIDcrc32, char *IndexType, char *IndexSprok, int  *TotaltTreff, char subname[]);
							GetNForTerm(WordIDcrc32,indexType,"aa",&nterm,subname);
							nr = nterm;
						}
						else {
							//TmpArrayLen = 0;
							//TmpArray->nrofHits = 0;
							//GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);

							GetNForTerm(WordIDcrc32,indexType,"aa",&nterm,subname);
							if (nterm == 0) {
								nr = 0;
							}
							else if (nterm > nr) {
								nr = nr / 2;
							}
							else {
								nr = nterm / 2;
							}
						}


					}

					
				break;


		}


 
        }


	vboprintf("searchIndex_getnrs: \"%s\": nrs %i\n",subname->subname,nr);

	vboprintf("searchIndex_getnrs: end\n");

	return nr;
}
void GetIndexAsArray_thesaurus (int *AntallTeff, struct iindexFormat *TeffArray,
                unsigned int WordIDcrc32, char * IndexType, char *IndexSprok,
                struct subnamesFormat *subname,
                int languageFilterNr, int languageFilterAsNr[],  
    		string_alternative  *alt,
		int alt_n
		 ) {

	int j,k;
	int TmpArrayLen;

#ifndef BLACK_BOKS
	GetIndexAsArray(AntallTeff,TeffArray,WordIDcrc32,IndexType,IndexSprok,subname,languageFilterNr,languageFilterAsNr);
#else

	printf("alt_n %i, alt %p\n",alt_n, alt);
	struct iindexFormat *TmpArray = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	struct iindexFormat *tmpAnser = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	
	if (alt == NULL) {
		GetIndexAsArray(AntallTeff,TeffArray,WordIDcrc32,IndexType,IndexSprok,subname,languageFilterNr,languageFilterAsNr);
	}
	else {

		printf("\n##########################################\n");
		printf("thesaurus search:\n\n");
	
		(*AntallTeff) = 0;

		for (j=0; j<alt_n; j++) {

                        for (k=0; k<alt[j].n; k++)
                        {
				WordIDcrc32 = crc32boitho(alt[j].s[k]);

				TmpArrayLen = 0;
				resultArrayInit(TmpArray);

				GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,IndexType,IndexSprok,subname,languageFilterNr, languageFilterAsNr);
				//rank(TmpArrayLen,TmpArray,subname,(*complicacy));

                                printf("%s (%i)\n", alt[j].s[k],TmpArrayLen);
				if (TmpArrayLen != 0) {									
					or_merge(tmpAnser,AntallTeff,TeffArray,(*AntallTeff),TmpArray,TmpArrayLen);
					iindexArrayCopy2(TeffArray,AntallTeff,0,tmpAnser,(*AntallTeff));							

				}
                        }
                        
		}	
		printf("\n##########################################\n\n");


	}

	free(TmpArray);
	free(tmpAnser);

#endif

}

void searchIndex (char *indexType, int *TeffArrayElementer, struct iindexFormat *TeffArray,
		query_array *queryParsed,struct iindexFormat *TmpArray,struct subnamesFormat *subname, 
		int languageFilterNr, 
		int languageFilterAsNr[], int *complicacy){

	vboprintf("search: searchIndex()\n");

	int i, y, j,k,h;
	char queryelement[128];
	unsigned int WordIDcrc32;
	int baseArrayLen;
        int TmpArrayLen;
	int TeffArrayOriginal;
	int newadded;

	//TeffArray->nrofHits = 0;
	//TeffArray->phrasenr = 0;

	(*complicacy) = 0;

	#ifdef DEBUG
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);
	#endif

	struct iindexFormat *tmpResult = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	int tmpResultElementer;

	vboprintf("######################################################################\n");
	vboprintf("searchIndex: vil search index \"%s\"\n",indexType);
	vboprintf("######################################################################\n");

	vboprintf("\nsearchIndex \"%s\", subname \"%s\"\n",indexType,(*subname).subname);
	TeffArrayOriginal = (*TeffArrayElementer);
	vboprintf("searchIndex: got that we have %i elements in array from before\n",TeffArrayOriginal);

//for (i=0; i<(*queryParsed).size; i++)
for (i=0; i<(*queryParsed).n; i++)
        {
            //struct text_list *t_it = (*queryParsed).elem[i];

            vboprintf("Search type %c\n", (*queryParsed).query[i].operand );



		switch ((*queryParsed).query[i].operand) {

			case '+':

					++(*complicacy);

					//(*TeffArrayElementer) = 0;

					queryelement[0] = '\0';

					for (j=0; j<(*queryParsed).query[i].n; j++) {
                    				vboprintf("aa_ søker på \"%s\"\n", (*queryParsed).query[i].s[j]);
                    				strncat(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));
                    			                			
                				vboprintf("queryelement:  %s\n", queryelement);

						// hvis vi er et kort ord så har vi ikke fått noen ord nummer palssering, så vi skal ikke søke etter det
						if ( isShortWord(queryelement) ) {
							printf("is short word\n");
							continue;
						}


						//gjør om il liten case
						//for(h=0;h<strlen(queryelement);h++) {
        					//	queryelement[h] = btolower(queryelement[h]);
        					//}
						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						vboprintf("searchIndex: WordIDcrc32 %u\n",WordIDcrc32);
						//hvis vi ikke har noen elementer i base arrayen, legger vi inn direkte
						//ToDo: kan ikke gjøre det da dette kansje ikke er første element
						//må skille her
						//if (*TeffArrayElementer == 0) {
						if (i == 0) {
							
							TmpArrayLen = (*TeffArrayElementer);
							GetIndexAsArray_thesaurus(TeffArrayElementer,TeffArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, (*queryParsed).query[i].alt, (*queryParsed).query[i].alt_n);
							//rank((*TeffArrayElementer),TeffArray,subname,(*complicacy));
							vboprintf("oooooo: (*TeffArrayElementer) %i,TmpArrayLen %i\n",(*TeffArrayElementer),TmpArrayLen);

							//rar b-2 bug her. Skal det være + ikke -?
							//Runarb: 19 aug 2007: skaper igjen problemer. ser ut til å skal være '-'
							(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;
							//(*TeffArrayElementer) = (*TeffArrayElementer) + TmpArrayLen;
							
							
						}
						else {
							TmpArrayLen = 0;
							TmpArray->nrofHits = 0;
							GetIndexAsArray_thesaurus(&TmpArrayLen,TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, (*queryParsed).query[i].alt, (*queryParsed).query[i].alt_n);

							vboprintf("did find %i pages\n",TmpArrayLen);
																									
							andprox_merge(TeffArray,&baseArrayLen,TeffArrayOriginal,TeffArray,(*TeffArrayElementer),TmpArray,TmpArrayLen);
							vboprintf("baseArrayLen %i\n",baseArrayLen);
							(*TeffArrayElementer) = baseArrayLen;

						}

					}

					
				break;


			case '|':
                                        //while ( t_it!=NULL )
                                        printf("will or search for:\n");
                                        for (j=0; j<(*queryParsed).query[i].n; j++) {
                                                printf("\t%s\n",(*queryParsed).query[i].s[j]);
                                        }
					for (j=0; j<(*queryParsed).query[i].n; j++) {

						++(*complicacy);

                    				printf("aa_ søker på \"%s\"\n", (*queryParsed).query[i].s[j]);
                    				strscpy(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));
                    			              
						#ifdef BLACK_BOKS  			
                    			        strsandr(queryelement,"_NBSP_"," ");
						#endif

                				printf("queryelement:  %s\n", queryelement);

						// hvis vi er et kort ord så har vi ikke fåt noen ord nummer palssering, så vi skal ikke øke hits
						if ( isShortWord(queryelement) ) {
							printf("is short word\n");
							continue;
						}


						//gjør om il liten case
						//for(h=0;h<strlen(queryelement);h++) {
        					//	queryelement[h] = btolower(queryelement[h]);
        					//}
						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						vboprintf("searchIndex: WordIDcrc32 %u\n",WordIDcrc32);
						//hvis vi ikke har noen elementer i base arrayen, legger vi inn direkte
						//ToDo: kan ikke gjøre det da dette kansje ikke er første element
						//må skille her
						//if (*TeffArrayElementer == 0) {
						if (i == 0) {
							TmpArrayLen = (*TeffArrayElementer);
							GetIndexAsArray(TeffArrayElementer,TeffArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);
							//rank((*TeffArrayElementer),TeffArray,subname,(*complicacy));

							(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;
						}
						else {
							TmpArrayLen = 0;
							GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);
							//rank(TmpArrayLen,TmpArray,subname,(*complicacy));

							printf("did find %i pages\n",TmpArrayLen);

							resultArrayInit(tmpResult);
							tmpResultElementer = 0;
												
							//merger først til en tempurar array, for så å kopiere inn denne.
							int ti;
							or_merge(tmpResult,&tmpResultElementer,TeffArray,(*TeffArrayElementer),TmpArray,TmpArrayLen);
							iindexArrayCopy2(TeffArray,&ti,TeffArrayOriginal,tmpResult,tmpResultElementer);

							vboprintf("tmpResultElementer %i\n",tmpResultElementer);
							baseArrayLen = ti;

							printf("baseArrayLen %i\n",baseArrayLen);
							(*TeffArrayElementer) = baseArrayLen;

						}

					//	t_it = t_it->next;
					}

					
				break;

			case '"':
					


                			//while ( t_it!=NULL )
					debug("will frases search for:");
					for (j=0; j<(*queryParsed).query[i].n; j++) {
						debug("\t%s",(*queryParsed).query[i].s[j]);
					}

					//Vi må først frasesøke de ordene vi skal, og lagre dete i en temp array. Så merge dette med resten
					resultArrayInit(tmpResult);
					tmpResultElementer = 0;

					//struct iindexFormat *tmpAnser = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
					//int tmpAnserElementer = 0;
					//resultArrayInit(tmpAnser);

					for (j=0; j<(*queryParsed).query[i].n; j++) {

						++(*complicacy);


			                	strscpy(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));

                    				vboprintf("\nelement %s\n", queryelement);

						//gjør om il liten case
						//for(h=0;h<strlen(queryelement);h++) {
        					//	queryelement[h] = btolower(queryelement[h]);
        					//}

						// hvis vi er et kort ord så har vi ikke fåt noen ord nummer palssering, så vi skal ikke øke hits
						if ( isShortWord(queryelement) ) {
							printf("is short word\n");
							continue;
						}
						//utestet
						//else if (isStoppWord(queryelement)) {
						//	printf("is stopword\n");
						//	 //frase_stopword(tmpResult,tmpResultElementer);
						//	(*queryParsed).query[i].stopword = 1;
						//	//exit(1);
						//	continue;
						//}
						

						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						vboprintf("searchIndex: WordIDcrc32 %u\n",WordIDcrc32);

                    				debug("crc32: %u",WordIDcrc32);
                    				
						//printf("word %s is st %i\n",t_it->text,t_it->stopword);

						//hvsi dette er første element leger vi de inn
						if (j == 0) {
			                                //TmpArrayLen = (*TeffArrayElementer);
							tmpResultElementer = 0;
							//tmpResult->nrofHits = 0;
							resultArrayInit(tmpResult);
							GetIndexAsArray(&tmpResultElementer,tmpResult,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);
							//rank(tmpResultElementer,tmpResult,subname,(*complicacy));

                                                        //(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;        	        
							//printf("TeffArrayElementer %i\n",(*TeffArrayElementer));
						}
						else {

							
							
							TmpArrayLen = 0;
							//TmpArray->nrofHits = 0;
							resultArrayInit(TmpArray);
							GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);
							//rank(TmpArrayLen,TmpArray,subname,(*complicacy));

							vboprintf("\t dddd: frase_merge %i %i\n",(*TeffArrayElementer),TmpArrayLen);

							//dette er ikke en ekts frasesøk, kan bare ha en frase
							frase_merge(tmpResult,&tmpResultElementer,0,tmpResult,tmpResultElementer,TmpArray,TmpArrayLen);

							// vi må ha en kopi av det gamle svaret slik at vi har noe og merge med, men vi kan ikke lagre svaret i oss selv. så vi må
							// ha en ny array til det.
							/*
							iindexArrayCopy(tmpResult,tmpAnser,tmpAnserElementer);							

							resultArrayInit(tmpAnser);
							tmpAnserElementer = 0;
							frase_merge(tmpAnser,&tmpAnserElementer,0,tmpResult,tmpResultElementer,TmpArray,TmpArrayLen);
							printf("after frase_merge(): tmpResultElementer: %i\n",tmpResultElementer);
							*/
							//ToDO: burde kansje bruke noe mem move eller slik her
							//for (y=0;y<baseArrayLen;y++) {
                                        	        //        TeffArray[y] = baseArray[y];
                                        	        //}
                                               					
							(*TeffArrayElementer) = baseArrayLen;
							
                				}
						

					}
						
					//så må vi and merge frasene inn i queryet
					//hvis dette er første forekomst så kopierer vi bare inn
					//hvis ikke må vi and merge
					if (i == 0) {
						vboprintf("er første fraseelement\n");
						
						k=TeffArrayOriginal;
						/*
						for (j=0;j<tmpResultElementer;j++) {
							//memcpy(TeffArray[j],tmpResult[j],sizeof(struct iindexFormat));
							//printf("k %i, j %i\n",k,j);
							
							TeffArray->iindex[k] = tmpResult->iindex[j];
							++k;
						}
						*/
						//iindexArrayCopy(TeffArray,tmpResult,tmpResultElementer);		
						//iindexArrayCopy(TeffArray,tmpAnser,tmpAnserElementer);
						int ti;
						iindexArrayCopy2(TeffArray,&ti,TeffArrayOriginal,tmpResult,tmpResultElementer);
						vboprintf("tmpResultElementer %i\n",tmpResultElementer);
						(*TeffArrayElementer) = tmpResultElementer;
					}
					else {
						and_merge(TeffArray,&baseArrayLen,TeffArrayOriginal,&newadded,TeffArray,(*TeffArrayElementer),tmpResult,tmpResultElementer);
						//and_merge(TeffArray,&baseArrayLen,TeffArrayOriginal,&newadded,TeffArray,(*TeffArrayElementer),tmpAnser,tmpAnserElementer);
						(*TeffArrayElementer) = baseArrayLen;
					}



                                break;

		}


 
        }

	vboprintf("searchIndex: (*TeffArrayElementer) %i, TeffArrayOriginal %i\n",(*TeffArrayElementer),TeffArrayOriginal);
//toDo: trenger vi denne nå???
//tror ikke vi trenger denne mere, da vi har merget queryet inn i den
	(*TeffArrayElementer) = (*TeffArrayElementer) + TeffArrayOriginal;
	TeffArrayOriginal = (*TeffArrayElementer);
	vboprintf("new len is %i\n",(*TeffArrayElementer));

	free(tmpResult);

	#ifdef DEBUG
	gettimeofday(&end_time, NULL);
	printf("Time debug: searchIndex %f\n",getTimeDifference(&start_time,&end_time));

	#endif

	vboprintf("searchIndex: end\n");
	vboprintf("######################################################################\n\n");

	vboprintf("search: ~searchIndex()\n");
}

struct searchIndex_thread_argFormat {
	char *indexType;
	struct subnamesFormat *subnames;
	int nrOfSubnames;
	query_array *queryParsed;
	query_array *search_user_as_query;
#ifdef ATTRIBUTES
	int attrib_count;
	query_array attribute_query[MAX_ATTRIBUTES_IN_QUERY];
//	struct iindexFormat *attribArray[MAX_ATTRIBUTES_IN_QUERY];
//	int attribArrayLen[MAX_ATTRIBUTES_IN_QUERY];
#endif
	int languageFilterNr;
	int *languageFilterAsNr;
	int resultArrayLen;
	struct iindexFormat *resultArray;
	double searchtime;
	char *search_user;
	int cmc_port;
};

void *searchIndex_thread(void *arg)
{
	vboprintf("search: searchIndex_thread()\n");

        struct searchIndex_thread_argFormat *searchIndex_thread_arg = (struct searchIndex_thread_argFormat *)arg;
	int i,j,y,x;

	int ArrayLen, TmpArrayLen, TmpArray2Len;

	struct iindexFormat *TmpArray, *TmpArray2;
	struct iindexFormat *Array;
	void (*rank)(int TeffArrayElementer, struct iindexFormat *TeffArray, int complicacy);
	int complicacy;

	struct timeval start_time, end_time;

	gettimeofday(&start_time, NULL);

	vboprintf("######################################################################\n");

	if ((TmpArray = malloc(sizeof(struct iindexFormat))) == NULL) {
                perror("malloc TmpArray");
                exit(1);
        }
	resultArrayInit(TmpArray);

	if ((TmpArray2 = malloc(sizeof(struct iindexFormat))) == NULL) {
                perror("malloc TmpArray2");
                exit(1);
        }
	resultArrayInit(TmpArray2);

	if ((Array = malloc(sizeof(struct iindexFormat))) == NULL) {
                perror("malloc main t Array");
                exit(1);
        }
	resultArrayInit(Array);

	#ifdef BLACK_BOKS
	struct iindexFormat *acl_allowArray;
	int acl_allowArrayLen;
	if ((acl_allowArray = malloc(sizeof(struct iindexFormat))) == NULL) {
                perror("malloc acl_allowArray");
                exit(1);
        }
	resultArrayInit(acl_allowArray);

	struct iindexFormat *acl_deniedArray;
	int acl_deniedArrayLen;
	if ((acl_deniedArray = malloc(sizeof(struct iindexFormat))) == NULL) {
                perror("malloc acl_deniedArray");
                exit(1);
        }
	resultArrayInit(acl_deniedArray);

	#ifdef ATTRIBUTES
	struct iindexFormat *tmpAttribArray[MAX_ATTRIBUTES_IN_QUERY];
	int tmpAttribArrayLen[MAX_ATTRIBUTES_IN_QUERY];

	for (i=0; i<(*searchIndex_thread_arg).attrib_count; i++)
	    {
		/*
		if (((*searchIndex_thread_arg).attribArray[i] = malloc(sizeof(struct iindexFormat))) == NULL) {
            		perror("malloc attribArray");
            		exit(1);
    		    }

		resultArrayInit((*searchIndex_thread_arg).attribArray[i]);
		*/

		if ((tmpAttribArray[i] = malloc(sizeof(struct iindexFormat))) == NULL) {
            		perror("malloc attribArray");
            		exit(1);
    		    }

		resultArrayInit(tmpAttribArray[i]);
	    }
	#endif

	struct iindexFormat *searcArray;
	int searcArrayLen;
	if ((searcArray	= malloc(sizeof(struct iindexFormat))) == NULL) {
		perror("malloc searcArray");
		exit(1);
	}
	resultArrayInit(searcArray);

	#endif

	int hits;

	if (strcmp((*searchIndex_thread_arg).indexType,"Athor") == 0) {
		rank = rankAthorArray;
	}
	else if (strcmp((*searchIndex_thread_arg).indexType,"Url") == 0) {
		rank = rankUrlArray;
	}
	else if (strcmp((*searchIndex_thread_arg).indexType,"Main") == 0) {
		rank = rankMainArray;
	}
	//else if (strcmp((*searchIndex_thread_arg).indexType,"attributes") == 0) {
	//	rank = rankMainArray;
	//}
	else {
		fprintf(stderr, "search: Error! Unknown index type \"%s\"\n",(*searchIndex_thread_arg).indexType);
		fprintf(stderr, "search: ~searchIndex_thread()\n");
		return;
	}
	#ifdef WITH_THREAD
	pthread_t tid;
        tid = pthread_self();
        vboprintf("is thread id %u. Wil search \"%s\"\n",(unsigned int)tid,(*searchIndex_thread_arg).indexType);
	#endif

	int cmc_sock;
	char cmc_statusbuf[1024];
	cmc_conect(&cmc_sock, cmc_statusbuf, sizeof(cmc_statusbuf), searchIndex_thread_arg->cmc_port);
	struct hashtable *groupqueries = create_hashtable(3, ht_integerhash, ht_integercmp);

	ArrayLen = 0;
	
	vboprintf("nrOfSubnames %i\n",(*searchIndex_thread_arg).nrOfSubnames);
	printf("Search user: %s\n", searchIndex_thread_arg->search_user);
	for(i=0;i<(*searchIndex_thread_arg).nrOfSubnames;i++) {
		query_array *groupquery;
		struct timeval starttime, endtime;
		printf("Checking subname: %s\n", searchIndex_thread_arg->subnames[i].subname);

		#if defined BLACK_BOKS && !defined _24SEVENOFFICE
	
		#ifdef IIACL

			int system = cmc_usersystemfromcollection(cmc_sock, searchIndex_thread_arg->subnames[i].subname);
			if (system == -1) {
				fprintf(stderr, "Unable to get usersystem for: %s\n", searchIndex_thread_arg->subnames[i].subname);
				continue;
			} else if ((groupquery = hashtable_search(groupqueries, &system)) == NULL) {
				char **groups;
				int n_groups, j;

				fprintf(stderr, "Getting mappings for system %d\n", system);
				gettimeofday(&starttime, NULL);
				n_groups = cmc_groupsforuserfromusersystem(cmc_sock, searchIndex_thread_arg->search_user,
				    system, &groups);
				gettimeofday(&endtime, NULL);
				fprintf(stderr, "Took this much time: %f\n", getTimeDifference(&starttime, &endtime));

				size_t grouplistsize = n_groups * (MAX_LDAP_ATTR_LEN+5);
				printf("n_groups: %d\n", n_groups);
				char *grouplist;
				
				if (n_groups > 0) {
					grouplist = malloc(grouplistsize);
					grouplist[0] = '\0';
				} else {
					grouplist = strdup("");
				}


				size_t grouplistlen = 0;
				for (j = 0; j < n_groups; j++) {
					strcpy(grouplist+grouplistlen," |\"");
					grouplistlen += 3;
					aclElementNormalize((char*)groups + j*MAX_LDAP_ATTR_LEN);
					strcpy(grouplist+grouplistlen, (char*)groups + j*MAX_LDAP_ATTR_LEN);
					grouplistlen += strlen((char*)groups + j*MAX_LDAP_ATTR_LEN);
					strcpy(grouplist+grouplistlen,"\"");
					grouplistlen += 1;
				}
				printf("grouplist: %s\n", grouplist);
				groupquery = malloc(sizeof(*groupquery));
				get_query(grouplist, strlen(grouplist), groupquery);
				free(groups);
				free(grouplist);
				hashtable_insert(groupqueries, uinttouintp(system), groupquery);
			} else {
				fprintf(stderr, "Reusing system mapping: %d\n", system);
			}



			searcArrayLen = 0;
			hits = ArrayLen;
	
			searchIndex((*searchIndex_thread_arg).indexType,
				&searcArrayLen,
				searcArray,
				(*searchIndex_thread_arg).queryParsed,
				TmpArray,
				&(*searchIndex_thread_arg).subnames[i],
				(*searchIndex_thread_arg).languageFilterNr, 
				(*searchIndex_thread_arg).languageFilterAsNr,
				&complicacy
			);

			//acl_allow sjekk
			acl_allowArrayLen = 0;
			acl_deniedArrayLen = 0;


			searchIndex("acl_allow",
				&acl_allowArrayLen,
				acl_allowArray,
				groupquery,
				TmpArray,
				&(*searchIndex_thread_arg).subnames[i],
				(*searchIndex_thread_arg).languageFilterNr, 
				(*searchIndex_thread_arg).languageFilterAsNr,
				&complicacy
			);

			searchIndex("acl_denied",
				&acl_deniedArrayLen,
				acl_deniedArray,
				groupquery,
				TmpArray,
				&(*searchIndex_thread_arg).subnames[i],
				(*searchIndex_thread_arg).languageFilterNr, 
				(*searchIndex_thread_arg).languageFilterAsNr,
				&complicacy
			);

			#ifdef ATTRIBUTES
			for (j=0; j<(*searchIndex_thread_arg).attrib_count; j++)
			    {
				tmpAttribArrayLen[j] = 0;

				// Skriv ut hvilke attributter det søkes på til skjermen:
				char	qbuf[1024];
				sprint_query(qbuf,1023,&(*searchIndex_thread_arg).attribute_query[j]);
				printf("Looking up attributes: %s\n", qbuf);

				    searchIndex("attributes",
					&tmpAttribArrayLen[j],
					tmpAttribArray[j],
					&(*searchIndex_thread_arg).attribute_query[j],
					TmpArray,
					&(*searchIndex_thread_arg).subnames[i],
					(*searchIndex_thread_arg).languageFilterNr, 
					(*searchIndex_thread_arg).languageFilterAsNr,
					&complicacy
				    );
				printf("%i hits.\n", tmpAttribArrayLen[j]);
			    }
			#endif

			#ifdef DEBUG_II
				printf("acl_allowArrayLen %i:\n",acl_allowArrayLen);
				for (y = 0; y < acl_allowArrayLen; y++) {
					printf("acl_allow TeffArray: DocID %u\n",acl_allowArray->iindex[y].DocID);			
				}

				printf("acl_deniedArrayLen %i:\n",acl_deniedArrayLen);
				for (y = 0; y < acl_deniedArrayLen; y++) {
					printf("acl_denied TeffArray: DocID %u\n",acl_deniedArray->iindex[y].DocID);			
				}

				printf("searcArrayLen %i:\n",searcArrayLen);
				for (y = 0; y < searcArrayLen; y++) {
					printf("Main TeffArray: DocID %u\nHits: %i\n",searcArray->iindex[y].DocID,searcArray->iindex[y].TermAntall);			
					for (x=0;x<searcArray->iindex[y].TermAntall;x++) {
						printf("\t%hu\n",searcArray->iindex[y].hits[x]);
					}		

				}
			#endif
			//hits = ArrayLen;
	
			//merger får å bare ta med de vi har en acl_allow til
			//and_merge(Array,&ArrayLen,ArrayLen,&hits,acl_allowArray,acl_allowArrayLen,searcArray,searcArrayLen);

			#ifdef ATTRIBUTES
			char	empty_search_query = 0;

			// Merge resultater med attributter og acl_allowed:
			if ((*searchIndex_thread_arg).attrib_count > 0)
			    {
				// Test for query-words i standard søkequery:
				char	null_query = 1;

				for (j=0; j<(*searchIndex_thread_arg).queryParsed->n && null_query; j++)
				    switch ((*searchIndex_thread_arg).queryParsed->query[j].operand)
					{
					    case QUERY_WORD:
					    case QUERY_PHRASE:
					    case QUERY_OR:
						null_query = 0;
						break;
					    default:
						break;
					}

				if (null_query) empty_search_query = 1;
			    }

			int	start = ArrayLen;

			if (!empty_search_query)
			    {
				and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,searcArray,searcArrayLen);
				// Merge acl_denied:			
	    			andNot_merge(Array,&ArrayLen,&hits,TmpArray,TmpArrayLen,acl_deniedArray,acl_deniedArrayLen);
			    }
			else
			    {
				// Merge med attributter istedet:
				and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,tmpAttribArray[0],tmpAttribArrayLen[0]);
				// Merge acl_denied:			
	    			andNot_merge(Array,&ArrayLen,&hits,TmpArray,TmpArrayLen,acl_deniedArray,acl_deniedArrayLen);
				//(*searchIndex_thread_arg).attribArrayLen[0] = 0;
			    }

			// Ettersom merging mÃ¥ gjÃ¸res per collection, gjÃ¸r vi attributt-filtreringa allerede her:
			for (x=start; x<ArrayLen; x++)
			    {
				Array->iindex[x].indexFiltered.is_filtered = 0;
				Array->iindex[x].indexFiltered.attribute = 0;
			    }

				//printf("\nSearchArray:");
				//for (y=0; y<ArrayLen; y++)
				//    printf(" %i", Array->iindex[y].DocID);
				//printf("\n\n");

			//for (j=empty_search_query; j<(*searchIndex_thread_arg).attrib_count; j++)
			for (j=0; j<(*searchIndex_thread_arg).attrib_count; j++)
			    {
				//printf("Attribute nr %i:", j);
				//for (y=0; y<tmpAttribArrayLen[j]; y++)
				//    printf(" %i", tmpAttribArray[j]->iindex[y].DocID);
				//printf("\n\n");
				for (x=start,y=0; x<ArrayLen; x++)
				    {
					while (y<tmpAttribArrayLen[j]
					    && Array->iindex[x].DocID > tmpAttribArray[j]->iindex[y].DocID) y++;

					if (y<tmpAttribArrayLen[j]
					    && Array->iindex[x].DocID == tmpAttribArray[j]->iindex[y].DocID)
					    {
						Array->iindex[x].indexFiltered.attrib[j] = 0;
						//vinn++;
					    }
					else
					    {
						//if (Array->iindex[x].indexFiltered.is_filtered == 0) --(*TotaltTreff);
					        Array->iindex[x].indexFiltered.is_filtered = 1;
					        Array->iindex[x].indexFiltered.attrib[j] = 1;
					        Array->iindex[x].indexFiltered.attribute = 1;
						//forsvinn++;
					    }
				    }
				//int	nhits;
				//and_merge((*searchIndex_thread_arg).attribArray[j], &(*searchIndex_thread_arg).attribArrayLen[j],
				//    0, &nhits, tmpAttribArry[j], tmpAttribArrayLen[j], Array, ArrayLen);
			    }
			/*
				    {
					and_merge(TmpArray2,&TmpArray2Len,0,&hits,attribArray,attribArrayLen,searcArray,searcArrayLen);
					and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,TmpArray2,TmpArray2Len);

			// Merge acl_denied:			
    			andNot_merge(Array,&ArrayLen,&hits,TmpArray,TmpArrayLen,acl_deniedArray,acl_deniedArrayLen);
				    }
				else
				    {
					// Vanlig søkequery er tomt, søk kun på attributter:
					and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,attribArray,attribArrayLen);

			// Merge acl_denied:			
    			andNot_merge(Array,&ArrayLen,&hits,TmpArray,TmpArrayLen,acl_deniedArray,acl_deniedArrayLen);
				    }
			    }
			else	// Attributtquery er tomt
			    {
			    }
			*/
			#else
			and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,searcArray,searcArrayLen);
			// Merge acl_denied:			
    			andNot_merge(Array,&ArrayLen,&hits,TmpArray,TmpArrayLen,acl_deniedArray,acl_deniedArrayLen);
			#endif


			#ifdef DEBUG_II
			printf("after first merge:\n");
			for (y = 0; y < ArrayLen; y++) {
				printf("TeffArray: DocID %u\nHits (%i): \n",Array->iindex[y].DocID,Array->iindex[y].TermAntall);	
				for (x=0;x<Array->iindex[y].TermAntall;x++) {
					printf("\t%hu\n",Array->iindex[y].hits[x]);
				}		
			}
			#endif

			#ifdef DEBUG_II
			printf("etter andNot_merge:\n");
			for (y = 0; y < ArrayLen; y++) {
				printf("TeffArray: DocID %u\nHits (%i): \n",Array->iindex[y].DocID,Array->iindex[y].TermAntall);	
				for (x=0;x<Array->iindex[y].TermAntall;x++) {
					printf("\t%hu\n",Array->iindex[y].hits[x]);
				}
			}
			#endif

			//ArrayLen = ArrayLen + hits;

			//hits = ArrayLen - hits;


		#else
			//iindexArrayCopy(Array, searcArray, searcArrayLen);
			//ArrayLen = searcArrayLen;
			hits = ArrayLen;
	
			searchIndex((*searchIndex_thread_arg).indexType,
				&ArrayLen,
				Array,
				(*searchIndex_thread_arg).queryParsed,
				TmpArray,
				&(*searchIndex_thread_arg).subnames[i],
				(*searchIndex_thread_arg).languageFilterNr, 
				(*searchIndex_thread_arg).languageFilterAsNr,
				&complicacy
			);

			hits = ArrayLen - hits;


		#endif


		(*searchIndex_thread_arg).subnames[i].hits += hits;
		vboprintf("searchIndex_thread: index %s, subname \"%s\",hits %i\n",(*searchIndex_thread_arg).indexType,(*searchIndex_thread_arg).subnames[i].subname,hits);


		#else

			hits = ArrayLen;
			searchIndex(
				(*searchIndex_thread_arg).indexType,
				&ArrayLen,
				Array,
				(*searchIndex_thread_arg).queryParsed,
				TmpArray,
				&(*searchIndex_thread_arg).subnames[i],
				(*searchIndex_thread_arg).languageFilterNr, 
				(*searchIndex_thread_arg).languageFilterAsNr,
				&complicacy
			);


			#ifdef DEBUG
				for (y = 0; y < ArrayLen; y++) {
					printf("searchIndex_thread: TeffArray: subname \"%s\", DocID %u\nHits (%i): \n",
						Array->iindex[y].subname->subname,Array->iindex[y].DocID,Array->iindex[y].TermAntall);
				}
			#endif

			hits = ArrayLen - hits;
			(*searchIndex_thread_arg).subnames[i].hits += hits;

			vboprintf("searchIndex_thread: ArrayLen %i\n",ArrayLen);
			vboprintf("searchIndex_thread: index %s, subname \"%s\",hits %i\n",(*searchIndex_thread_arg).indexType,(*searchIndex_thread_arg).subnames[i].subname,hits);


		#endif


		
	}
	cmc_close(cmc_sock);
	{
		struct hashtable_itr *itr;

		if (hashtable_count(groupqueries) > 0) {
			itr = hashtable_iterator(groupqueries);
			do {
				destroy_query(hashtable_iterator_value(itr));
			} while (hashtable_iterator_advance(itr) != 0);
		}
	}
	hashtable_destroy(groupqueries, 1);

	free(TmpArray);
	free(TmpArray2);

	#ifdef BLACK_BOKS
		free(searcArray);
		free(acl_deniedArray);
		free(acl_allowArray);
		//free(attribArray);
		#ifdef ATTRIBUTES
		for (i=0; i<(*searchIndex_thread_arg).attrib_count; i++)
		    {
			free(tmpAttribArray[i]);
		    }
		#endif
	#endif

	//rankering må være lengere oppe
	//rank(ArrayLen,Array,&(*searchIndex_thread_arg).subnames[i],complicacy);

	rank(ArrayLen,Array,complicacy);
	explain_max_rankArray(ArrayLen,Array,complicacy);


	(*searchIndex_thread_arg).resultArrayLen = ArrayLen;
	(*searchIndex_thread_arg).resultArray = Array;

	gettimeofday(&end_time, NULL);
	(*searchIndex_thread_arg).searchtime = getTimeDifference(&start_time,&end_time);
	vboprintf("searchtime %f\n",(*searchIndex_thread_arg).searchtime);

	vboprintf("######################################################################\n");

	vboprintf("search: ~searchIndex_thread()\n");
}

void searchSimple (int *TeffArrayElementer, struct iindexFormat *TeffArray,int *TotaltTreff,
//		int *unfilteredTeffArrayElementer, struct iindexFormat *unfilteredTeffArray,
		query_array *queryParsed, struct queryTimeFormat *queryTime, 
		struct subnamesFormat subnames[], int nrOfSubnames,int languageFilterNr, 
		int languageFilterAsNr[], char orderby[],
		struct filtersFormat *filters,
		struct filteronFormat *filteron,
		query_array *search_user_as_query,
		int ranking, struct hashtable **crc32maphash,
		char *search_user, int cmc_port
		) {

	int i,j,y,n;
	//int x=0,j=0,k=0;
	unsigned char PopRank;
	int responseShortTo;
	struct reformat *crc32map;

	int rankcount[256]; // rank går fra 0-252 (unsigned char)
	//int M,N;
	int tmpint;

	struct timeval start_time, end_time;
		
	//double TermRankTemp;
       	//double PopRankTemp;

	
	struct iindexFormat *TmpArray; 

	/*				
	struct iindexFormat *AthorArray;
	struct iindexFormat *MainArray;
	struct iindexFormat *UrlArray;

	int MainArrayLen;
	int UrlArrayLen;
	int AthorArrayLen;
	*/

	//char *WordID;
	struct searchIndex_thread_argFormat searchIndex_thread_arg_Athor;
	struct searchIndex_thread_argFormat searchIndex_thread_arg_Url;
	struct searchIndex_thread_argFormat searchIndex_thread_arg_Main;
	//struct searchIndex_thread_argFormat searchIndex_thread_arg_Acl;
	
     
        int baseArrayLen,MainArrayHits;
	int TmpArrayLen;	
	//unsigned long WordIDcrc32;
	unsigned int PredictNrAthor;
	unsigned int PredictNrUrl;
	unsigned int PredictNrMain;
//	unsigned int PredictNrattributes;
	//unsigned int PredictNrAcl;

	PredictNrAthor	= 0;
	PredictNrUrl	= 0;
	PredictNrMain	= 0;
//	PredictNrattributes = 0;

	#ifdef WITH_THREAD
		pthread_t threadid_Athor = 0;
		pthread_t threadid_Url = 0;
		pthread_t threadid_Main = 0;
		//pthread_t threadid_Acl = 0;
	#endif

	//resetter subnmes hits
	for(i=0;i<nrOfSubnames;i++) {		
		subnames[i].hits = 0;
	}

	#ifdef BLACK_BOKS

		//for(i=0;i<nrOfSubnames;i++) {		
		//	PredictNrAcl += searchIndex_getnrs("Acl",queryParsed,&subnames[i],languageFilterNr, languageFilterAsNr);
		//}

	#else
	//finner først ca hvor mange treff vi fil få. Dette brukes for å avgjøre om vi kan 
	//klare oss med å søke i bare url og athor, eller om vi må søke i alt
	//Athor
	for(i=0;i<nrOfSubnames;i++) {		
		PredictNrAthor += searchIndex_getnrs("Athor",queryParsed,&subnames[i],languageFilterNr, languageFilterAsNr);
	}

	//Url
	for(i=0;i<nrOfSubnames;i++) {		
		PredictNrUrl += searchIndex_getnrs("Url",queryParsed,&subnames[i],languageFilterNr, languageFilterAsNr);
	}
	#endif

	//Main
	for(i=0;i<nrOfSubnames;i++) {		
		PredictNrMain += searchIndex_getnrs("Main",queryParsed,&subnames[i],languageFilterNr, languageFilterAsNr);
		vboprintf("PredictNrMain total with \"%s\" %u\n",&subnames[i],PredictNrMain);
	}

	#ifdef BLACK_BOKS
		//filter
		searchIndex_filters(queryParsed, filteron);	//@ax
	#endif


	vboprintf("PredictNrAthor %u, PredictNrUrl %u, PredictNrMain %u\n",PredictNrAthor,PredictNrUrl,PredictNrMain);

	//nullstiller alle resultat tellere
	searchIndex_thread_arg_Athor.resultArrayLen = 0;
	searchIndex_thread_arg_Url.resultArrayLen = 0;
	searchIndex_thread_arg_Main.resultArrayLen = 0;
	//searchIndex_thread_arg_Acl.resultArrayLen = 0;


	searchIndex_thread_arg_Athor.searchtime = 0;
	searchIndex_thread_arg_Url.searchtime = 0;
	searchIndex_thread_arg_Main.searchtime = 0;
	//searchIndex_thread_arg_Acl.searchtime = 0;

	searchIndex_thread_arg_Main.resultArray		= NULL;
	searchIndex_thread_arg_Athor.resultArray 	= NULL;
	searchIndex_thread_arg_Url.resultArray		= NULL;


	
	#ifdef BLACK_BOKS


	#else
	//Athor	
	//if (PredictNrAthor > 0) {
		searchIndex_thread_arg_Athor.indexType = "Athor";
		searchIndex_thread_arg_Athor.nrOfSubnames = nrOfSubnames;
		searchIndex_thread_arg_Athor.subnames = subnames;
		searchIndex_thread_arg_Athor.queryParsed = queryParsed;
		searchIndex_thread_arg_Athor.search_user_as_query = search_user_as_query;
		searchIndex_thread_arg_Athor.languageFilterNr = languageFilterNr;
		searchIndex_thread_arg_Athor.languageFilterAsNr = languageFilterAsNr;
		#ifdef WITH_THREAD
			n = pthread_create(&threadid_Athor, NULL, searchIndex_thread, &searchIndex_thread_arg_Athor);
		#else
			searchIndex_thread(&searchIndex_thread_arg_Athor);
		#endif
	//}

	//Url
	//if (PredictNrUrl > 0) {
		searchIndex_thread_arg_Url.indexType = "Url";
		searchIndex_thread_arg_Url.nrOfSubnames = nrOfSubnames;
		searchIndex_thread_arg_Url.subnames = subnames;
		searchIndex_thread_arg_Url.queryParsed = queryParsed;
		searchIndex_thread_arg_Url.search_user_as_query = search_user_as_query;
		searchIndex_thread_arg_Url.languageFilterNr = languageFilterNr;
		searchIndex_thread_arg_Url.languageFilterAsNr = languageFilterAsNr;
		#ifdef WITH_THREAD
			n = pthread_create(&threadid_Url, NULL, searchIndex_thread, &searchIndex_thread_arg_Url);
		#else
			searchIndex_thread(&searchIndex_thread_arg_Url);	
		#endif
	//}
	#endif

	//Main
	//vi søker ikke main hvis vi antar at vi har flere en xxx elementer i Athor
	#ifndef BLACK_BOKS
	if (PredictNrAthor < 20000) {
	#else
	if(1) {
	#endif


		searchIndex_thread_arg_Main.indexType = "Main";
		searchIndex_thread_arg_Main.nrOfSubnames = nrOfSubnames;
		searchIndex_thread_arg_Main.subnames = subnames;
		searchIndex_thread_arg_Main.queryParsed = queryParsed;
		searchIndex_thread_arg_Main.search_user_as_query = search_user_as_query;
		searchIndex_thread_arg_Main.languageFilterNr = languageFilterNr;
		searchIndex_thread_arg_Main.languageFilterAsNr = languageFilterAsNr;
		searchIndex_thread_arg_Main.search_user = search_user;
		searchIndex_thread_arg_Main.cmc_port = cmc_port;

	#ifdef ATTRIBUTES
		int	attributes_count = 0;
		struct searchIndex_thread_argFormat searchIndex_thread_arg_attributes[MAX_ATTRIBUTES_IN_QUERY];
		pthread_t threadid_attributes[MAX_ATTRIBUTES_IN_QUERY];
		query_array	query_attributes[MAX_ATTRIBUTES_IN_QUERY];

		for (i=0; i<vector_size((*filteron).attributes) && attributes_count<MAX_ATTRIBUTES_IN_QUERY; i++)
		    if (pair(vector_get((*filteron).attributes,i)).first.i == QUERY_ATTRIBUTE)
			{
			    container	*attr_query = vector_container( pair_container( int_container(), vector_container( string_container() ) ) );

			    vector_pushback(attr_query, QUERY_WORD);
			    vector_pushback( pair(vector_get(attr_query,vector_size(attr_query)-1)).second.C,
				    pair(vector_get((*filteron).attributes,i)).second.ptr);

			    make_query_array(attr_query, &searchIndex_thread_arg_Main.attribute_query[attributes_count]);
			    destroy(attr_query);

			    attributes_count++;
			}

		searchIndex_thread_arg_Main.attrib_count = attributes_count;
	#endif

/*
		#if defined(BLACK_BOKS) && defined(ATTRIBUTES)
		container	*attr_query = vector_container( pair_container( int_container(), vector_container( string_container() ) ) );
		for (i=0; i<vector_size((*filteron).attributes); i++)
		    if (pair(vector_get((*filteron).attributes,i)).first.i == QUERY_ATTRIBUTE)
			{
			    vector_pushback(attr_query, QUERY_WORD);
			    vector_pushback( pair(vector_get(attr_query,vector_size(attr_query)-1)).second.C,
				    pair(vector_get((*filteron).attributes,i)).second.ptr);
			}

		make_query_array(attr_query, &searchIndex_thread_arg_Main.attribute_query);
		destroy(attr_query);
		#endif
*/
		#ifdef WITH_THREAD
			n = pthread_create(&threadid_Main, NULL, searchIndex_thread, &searchIndex_thread_arg_Main);
		#else
			searchIndex_thread(&searchIndex_thread_arg_Main);
		#endif
/*
		#if defined(BLACK_BOKS) && defined(ATTRIBUTES)
		destroy_query( &searchIndex_thread_arg_Main.attribute_query );
		#endif
*/
	}

	/*
	#ifdef ATTRIBUTES
		int	attributes_count = 0;
		struct searchIndex_thread_argFormat searchIndex_thread_arg_attributes[MAX_ATTRIBUTES_IN_QUERY];
		pthread_t threadid_attributes[MAX_ATTRIBUTES_IN_QUERY];
		query_array	query_attributes[MAX_ATTRIBUTES_IN_QUERY];

		for (i=0; i<vector_size((*filteron).attributes) && attributes_count<MAX_ATTRIBUTES_IN_QUERY; i++)
		    if (pair(vector_get((*filteron).attributes,i)).first.i == QUERY_ATTRIBUTE)
			{
			    container	*attr_query = vector_container( pair_container( int_container(), vector_container( string_container() ) ) );

			    vector_pushback(attr_query, QUERY_WORD);
			    vector_pushback( pair(vector_get(attr_query,vector_size(attr_query)-1)).second.C,
				    pair(vector_get((*filteron).attributes,i)).second.ptr);

			    make_query_array(attr_query, &query_attributes[attributes_count]);
			    destroy(attr_query);

			    attributes_count++;
			}


		for (i=0; i<attributes_count; i++)
		    {
			threadid_attributes[i] = 0;

			searchIndex_thread_arg_attributes[i].resultArrayLen = 0;
			searchIndex_thread_arg_attributes[i].searchtime = 0;
			searchIndex_thread_arg_attributes[i].resultArray = NULL;
			searchIndex_thread_arg_attributes[i].indexType = "attributes";
			searchIndex_thread_arg_attributes[i].nrOfSubnames = nrOfSubnames;
			searchIndex_thread_arg_attributes[i].subnames = subnames;
			searchIndex_thread_arg_attributes[i].queryParsed = &query_attributes[i];
			searchIndex_thread_arg_attributes[i].search_user_as_query = search_user_as_query;
			searchIndex_thread_arg_attributes[i].languageFilterNr = languageFilterNr;
			searchIndex_thread_arg_attributes[i].languageFilterAsNr = languageFilterAsNr;

			#ifdef WITH_THREAD
				n = pthread_create(&threadid_attributes[i], NULL, searchIndex_thread, &searchIndex_thread_arg_attributes[i]);
			#else
				searchIndex_thread(&searchIndex_thread_arg_attributes[i]);
			#endif

			printf("Attribute query(%i): %s\n", i, asprint_query(&query_attributes[i]));
			destroy_query( &query_attributes[i] );
		    }
	#endif
	*/
	
	//joiner trådene
	#ifdef BLACK_BOKS
		#ifdef WITH_THREAD
			//pthread_join(threadid_Acl, NULL);
		#endif
	#else
		#ifdef WITH_THREAD
			//joiner trådene
			if (threadid_Athor != 0) {
				pthread_join(threadid_Athor, NULL);
			}
			if (threadid_Url != 0) {
				pthread_join(threadid_Url, NULL);
			}
		#endif
	#endif

	#ifdef WITH_THREAD
		if (threadid_Main != 0) {
			pthread_join(threadid_Main, NULL);
		}
		/*
		#ifdef ATTRIBUTES
		for (i=0; i<attributes_count; i++)
		    if (threadid_attributes[i] != 0)
			{
			    pthread_join(threadid_attributes[i], NULL);
			}
		#endif
		*/
	#endif

	/*
	for (i = 0; i < searchIndex_thread_arg_Main.resultArrayLen; i++) {
		printf("abb TeffArray: \"%s\" (i %i)\n",(*searchIndex_thread_arg_Main.resultArray[i].subname).subname,i);			
	}
	*/

	vboprintf("Athor ArrayLen %i, Url ArrayLen %i, Main ArrayLen %i\n",searchIndex_thread_arg_Athor.resultArrayLen,
			searchIndex_thread_arg_Url.resultArrayLen,searchIndex_thread_arg_Main.resultArrayLen);

	//sanker inn tiden
	(*queryTime).AthorSearch = searchIndex_thread_arg_Athor.searchtime;
	(*queryTime).UrlSearch = searchIndex_thread_arg_Url.searchtime;
	(*queryTime).MainSearch = searchIndex_thread_arg_Main.searchtime;

	gettimeofday(&start_time, NULL);

	TmpArray 	= (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	TmpArrayLen = 0;
	resultArrayInit(TmpArray);

	#ifdef BLACK_BOKS

	#else
	//ormerger Athor og Url inn i en temper array
	or_merge(TmpArray,&TmpArrayLen,searchIndex_thread_arg_Athor.resultArray,searchIndex_thread_arg_Athor.resultArrayLen,
		searchIndex_thread_arg_Url.resultArray,searchIndex_thread_arg_Url.resultArrayLen);



	if (searchIndex_thread_arg_Athor.resultArray != NULL) {
		free(searchIndex_thread_arg_Athor.resultArray);
	}

	if (searchIndex_thread_arg_Url.resultArray != NULL) {
		free(searchIndex_thread_arg_Url.resultArray);
	}
	#endif

	/*
	//Main
	if (searchIndex_thread_arg_Main.resultArrayLen > 0) {
		or_merge(TmpArray,&TmpArrayLen,TeffArray,(*TeffArrayElementer),
			searchIndex_thread_arg_Main.resultArray,searchIndex_thread_arg_Main.resultArrayLen);
		free(searchIndex_thread_arg_Main.resultArray);
		memcpy(TeffArray,TmpArray,sizeof(struct iindexFormat) * TmpArrayLen);
		(*TeffArrayElementer) = TmpArrayLen;
	}
	*/

//	*unfilteredTeffArrayElementer = *TeffArrayElementer;
//	unfilteredTeffArray = TeffArray;
//	iindexArrayCopy2(unfilteredTeffArray, unfilteredTeffArrayElementer, 0, TeffArray, *TeffArrayElementer);

	//#ifdef ATTRIBUTES
/*
		struct iindexFormat *filteredTeffArray[MAX_ATTRIBUTES_IN_QUERY];
		int	filteredTeffArrayLen[MAX_ATTRIBUTES_IN_QUERY];

		for (i=0; i<attributes_count; i++)
		    {
			filteredTeffArray[i] = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
			filteredTeffArrayLen[i] = 0;
			resultArrayInit(filteredTeffArray[i]);
*/
/*
		// Merge resultater med attributter:
		if (attributes_count > 0)
		    {
			// Test for query-words i standard søkequery:
			char	null_query = 1;
			int	j, hits;

			for (j=0; j<queryParsed->n && null_query; j++)
			    switch (queryParsed->query[j].operand)
				{
				    case QUERY_WORD:
				    case QUERY_PHRASE:
				    case QUERY_OR:
					null_query = 0;
					break;
				    default:
					break;
				}

			if (!null_query)
			    {
				fprintf(stderr, "search: Merging query.Main and query.attributes\n");
				or_merge(TeffArray,TeffArrayElementer,TmpArray,TmpArrayLen,
					searchIndex_thread_arg_Main.resultArray,searchIndex_thread_arg_Main.resultArrayLen);
				and_merge(filteredTeffArray,&filteredTeffArrayLen,0,&hits,
					searchIndex_thread_arg_attributes.resultArray,searchIndex_thread_arg_attributes.resultArrayLen,
					TeffArray,*TeffArrayElementer);
			    }
			else
			    {
				// Vanlig søkequery er tomt, søk kun på attributter:
				// TODO!!!!!!!!!!!
				fprintf(stderr, "search: query.attributes only\n");
				or_merge(TeffArray,TeffArrayElementer,TmpArray,TmpArrayLen,
					searchIndex_thread_arg_attributes.resultArray,searchIndex_thread_arg_attributes.resultArrayLen);
			    }
		    }
		else
		    {
			// Attributtquery er tomt:
			fprintf(stderr, "search: query.Main only\n");
			or_merge(TeffArray,TeffArrayElementer,TmpArray,TmpArrayLen,
				searchIndex_thread_arg_Main.resultArray,searchIndex_thread_arg_Main.resultArrayLen);
		    }
*/
	/*
	or_merge(TeffArray,TeffArrayElementer,TmpArray,TmpArrayLen,
		searchIndex_thread_arg_Main.resultArray,searchIndex_thread_arg_Main.resultArrayLen);

	if (searchIndex_thread_arg_Main.resultArray != NULL) {
		free(searchIndex_thread_arg_Main.resultArray);
	}
	*/
	//if (searchIndex_thread_arg_attributes.resultArray != NULL) {
	//	free(searchIndex_thread_arg_attributes.resultArray);
	//}
//	#endif
	//#else
	//merger inn temperer og main 

	#ifdef BLACK_BOKS

	    #ifdef ATTRIBUTES
		for (i=0; i<searchIndex_thread_arg_Main.attrib_count; i++)
		    destroy_query( &searchIndex_thread_arg_Main.attribute_query[i] );
	    #endif

	or_merge(TeffArray,TeffArrayElementer,TmpArray,TmpArrayLen,
		searchIndex_thread_arg_Main.resultArray,searchIndex_thread_arg_Main.resultArrayLen);

	// For Ã¥ bevare attributtfilter!!
	for (i=0; i<*TeffArrayElementer; i++)
	    {
		TeffArray->iindex[i].indexFiltered.is_filtered = searchIndex_thread_arg_Main.resultArray->iindex[i].indexFiltered.is_filtered;
		TeffArray->iindex[i].indexFiltered.attribute = searchIndex_thread_arg_Main.resultArray->iindex[i].indexFiltered.attribute;
		for (j=0; j<searchIndex_thread_arg_Main.attrib_count; j++)
		    TeffArray->iindex[i].indexFiltered.attrib[j] =
			searchIndex_thread_arg_Main.resultArray->iindex[i].indexFiltered.attrib[j];
	    }

	if (searchIndex_thread_arg_Main.resultArray != NULL) {
		free(searchIndex_thread_arg_Main.resultArray);
	}
	#else
	or_merge(TeffArray,TeffArrayElementer,TmpArray,TmpArrayLen,
		searchIndex_thread_arg_Main.resultArray,searchIndex_thread_arg_Main.resultArrayLen);

	if (searchIndex_thread_arg_Main.resultArray != NULL) {
		free(searchIndex_thread_arg_Main.resultArray);
	}
	#endif

	//sjekker at dokumenter er i Aclen

	free(TmpArray);

        gettimeofday(&end_time, NULL);
        (*queryTime).MainAthorMerge = getTimeDifference(&start_time,&end_time);


	gettimeofday(&start_time, NULL);

        gettimeofday(&end_time, NULL);
        (*queryTime).popRank = getTimeDifference(&start_time,&end_time);

	//kutter ned på treff errayen, basert på rank. Slik at vå får ferre elemeneter å sortere

	//totalt treff. Vi vil så korte ned TeffArray
	*TotaltTreff = *TeffArrayElementer;

	gettimeofday(&start_time, NULL);

	#ifdef WITH_RANK_FILTER
	
	if ((*TeffArrayElementer) > 20000 && !ranking) {

		for(i=0; i<= 255;i++) {
			rankcount[i] = 0;
		}

		//først gå vi gjenom alle hittene og finner hvilkene rank som fårekommer
		for (i=0; i < (*TeffArrayElementer); i++) {


			++rankcount[TeffArray->iindex[i].PopRank];
		}

		//vi går så gjenom alle rankene og finner den største ranken som vil gi 
		//oss responseShortToMin sider
		y=0;
		responseShortTo=256;
		//for(i=255;(i>=0) && (y < responseShortToMin);i--) {
		//while ((responseShortTo>=0) && (y < responseShortToMin)) {
		do {
			--responseShortTo;

			y += rankcount[responseShortTo];
			#ifdef DEBUG
			if (rankcount[responseShortTo] != 0) {
				printf("rank %i: %i\n",responseShortTo,(int)rankcount[responseShortTo]);
			}
			#endif

		} while((responseShortTo>0) && (y <= responseShortToMin));
		
		vboprintf("responseShortTo: %i, y: %i\n",responseShortTo,y);
		
		if (responseShortTo != 0) {
			//fjerner sider som er lavere en responseShortTo 
			y=0;
       			for (i = 0; i < (*TeffArrayElementer); i++) {
			
				if (TeffArray->iindex[i].PopRank >= responseShortTo) {
        		       		TeffArray->iindex[y] = TeffArray->iindex[i];
        		        	++y;
				}
			}
		}
		vboprintf("shortet respons array from %i to %i.\n",(*TeffArrayElementer),y);
		(*TeffArrayElementer) = y;

		
	}
	
	#endif



        gettimeofday(&end_time, NULL);
        (*queryTime).responseShortning = getTimeDifference(&start_time,&end_time);





	
	#ifdef BLACK_BOKS

		for (i=0; i<*TeffArrayElementer; i++)
		    {
			TeffArray->iindex[i].indexFiltered.duplicate = 0;
			TeffArray->iindex[i].indexFiltered.duplicate_in_collection = -1;

			// For filtrert i search_thread_ting:
			if (TeffArray->iindex[i].indexFiltered.is_filtered)
			    --(*TotaltTreff);
		    }

		#ifdef ATTRIBUTES
		TeffArray->attrib_count = searchIndex_thread_arg_Main.attrib_count;

/*
		printf("\nTeffArray:");
		for (j=0; j<*TeffArrayElementer; j++)
		    printf(" %i(%s)", TeffArray->iindex[j].DocID, TeffArray->iindex[j].subname);
		printf("\n\n");
*/
/*
		if (*TeffArrayElementer > 0)
		    for (i=0; i<attributes_count; i++)
			{
			    int		j, k;
			    int		vinn=0, forsvinn=0;
			    int		hits;

			    printf("AttribArray %i:", i);
			    for (j=0; j<searchIndex_thread_arg_attributes[i].resultArrayLen; j++)
				printf(" %i(%s)", searchIndex_thread_arg_attributes[i].resultArray->iindex[j].DocID
				    , searchIndex_thread_arg_attributes[i].resultArray->iindex[j].subname);
			    printf("\n\n");

			    struct iindexFormat *filteredTeffArray;
			    int	filteredTeffArrayLen;

			    filteredTeffArray = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
			    filteredTeffArrayLen = 0;
			    resultArrayInit(filteredTeffArray);
			    and_merge(filteredTeffArray,&filteredTeffArrayLen,0,&hits,
					searchIndex_thread_arg_attributes[i].resultArray,searchIndex_thread_arg_attributes[i].resultArrayLen,
					TeffArray,*TeffArrayElementer);
			    printf("MergeArray %i:", i);
			    for (j=0; j<filteredTeffArrayLen; j++)
				printf(" %i", filteredTeffArray->iindex[j].DocID);
			    printf("\n\n");
			    free(filteredTeffArray);
*/
/*
			    for (j=0,k=0; j<*TeffArrayElementer; j++)
				{
				    while (TeffArray->iindex[j].DocID > searchIndex_thread_arg_attributes[i].resultArray->iindex[k].DocID
					&& k<searchIndex_thread_arg_attributes[i].resultArrayLen) k++;

				    if (k<searchIndex_thread_arg_attributes[i].resultArrayLen
					&& TeffArray->iindex[j].DocID == searchIndex_thread_arg_attributes[i].resultArray->iindex[k].DocID)
					{
					    TeffArray->iindex[j].indexFiltered.attrib[i] = 0;
					    vinn++;
					}
				    else
					{
					    if (TeffArray->iindex[j].indexFiltered.is_filtered == 0) --(*TotaltTreff);
					    TeffArray->iindex[j].indexFiltered.is_filtered = 1;
					    TeffArray->iindex[j].indexFiltered.attrib[i] = 1;
					    TeffArray->iindex[j].indexFiltered.attribute = 1;
					    forsvinn++;
					}
				}
			}
*/
/*
		if (*TeffArrayElementer > 0 && filteredTeffArrayLen > 0)
		    {
			for (i=0,j=0; i<*TeffArrayElementer; i++)
			    {
				while (TeffArray->iindex[i].DocID > filteredTeffArray->iindex[j].DocID && j<filteredTeffArrayLen) j++;

				if (j<filteredTeffArrayLen && TeffArray->iindex[i].DocID == filteredTeffArray->iindex[j].DocID)
				    {
					TeffArray->iindex[i].indexFiltered.attrib[0] = 0;	// Trengs denne?
					TeffArray->iindex[i].indexFiltered.is_filtered = 0;
				    }
				else
				    {
					TeffArray->iindex[i].indexFiltered.is_filtered = 1;
					TeffArray->iindex[i].indexFiltered.attrib[0] = 1;
					--(*TotaltTreff);
				    }
			    }
		    }

		free(filteredTeffArray);
*/
		#endif


		/*
		*********************************************************************************************************************
		collection
		*********************************************************************************************************************
		*/

		if ((*filteron).collection != NULL) {
		

			printf("will filter on collection \"%s\"\n",(*filteron).collection);
			/*
			y=0;
       			for (i = 0; i < (*TeffArrayElementer); i++) {
				printf("TeffArray \"%s\" ? filteron \"%s\"\n",(*TeffArray->iindex[i].subname).subname,(*filteron).collection);
				if (strcmp((*TeffArray->iindex[i].subname).subname,(*filteron).collection) == 0) {
        	       			TeffArray->iindex[y] = TeffArray->iindex[i];
        		        	++y;
				}
			}
			printf("filteron.collection: filter dovn array to from %i, to %i\n",(*TeffArrayElementer),y);

			(*TeffArrayElementer) = y;
			*/

			for (i = 0; i < (*TeffArrayElementer); i++) {
				if (strcmp((*TeffArray->iindex[i].subname).subname,(*filteron).collection) != 0) {
					if (!TeffArray->iindex[i].indexFiltered.is_filtered) --(*TotaltTreff);
					TeffArray->iindex[i].indexFiltered.is_filtered = 1;
					TeffArray->iindex[i].indexFiltered.subname = 1;
				}
			}

		}

		


		/*
		*********************************************************************************************************************
		filformater 

		lager en oversikt over, og filtrerer på, filformater

			Dette her blit en ganske komplisert sak. Vi har hash nøkler på fårmatet "subname-filtype", i en strukt.
			
			Vi har også med en peker til subname strukten, slik at vi kan manipulere denne direkte. Slik har vi
			bare en gjenomgang av dataene får å legge inn filtype infoen. Så en gjenomgang av alle subname-filetype
			som finnes for å lagge de inn i subname struktuen.

		*********************************************************************************************************************
		*/

		gettimeofday(&start_time, NULL);

		//char filetype[5];
		//struct filesKeyFormat *filesKey;

		for (i = 0; i < (*TeffArrayElementer); i++) {
			//printf("$%%$$%%$$$$$$$$$$$$$$ Eirik: %d\n", TeffArray->iindex[i].DocID);
			#ifdef DEBUG
			printf("i = %i, subname \"%s\"\n",i,(*TeffArray->iindex[i].subname).subname);
			#endif
			if (iintegerGetValue(&TeffArray->iindex[i].filetype,4,TeffArray->iindex[i].DocID,"filtypes",(*TeffArray->iindex[i].subname).subname) == 0) {
				printf("woldent get integerindex\n");
				TeffArray->iindex[i].filetype[0] = '\0';
			}
			else {
				#ifdef DEBUG
				printf("file typs is: \"%c%c%c%c\"\n",TeffArray->iindex[i].filetype[0],TeffArray->iindex[i].filetype[1],TeffArray->iindex[i].filetype[2],TeffArray->iindex[i].filetype[3]);
				#endif				

				// filetype kan være på opptil 4 bokstaver. Hvis det er ferre en 4 så vil 
				// det være \0 er paddet på slutten, men hvsi det er 4 så er det ikke det.
				// legger derfor til \0 som 5 char, slik at vi har en gyldig string
				TeffArray->iindex[i].filetype[4] = '\0';

				
				if (strstr(TeffArray->iindex[i].filetype,"10") != NULL) {
					printf("werd. DocID %u, subname %s\n",TeffArray->iindex[i].DocID,(*TeffArray->iindex[i].subname).subname);
					//exit(1);
				}
				

			}
		}


		/*
		// sort by nrof
		for(i=0;i<nrOfSubnames;i++) {
			printf("qsort filtypes %i\n",subnames[i].nrOfFiletypes);
			qsort(subnames[i].filtypes,subnames[i].nrOfFiletypes,sizeof(struct subnamesFiltypesFormat),compare_filetypes);
		}
		*/

		gettimeofday(&end_time, NULL);
		(*queryTime).filetypes = getTimeDifference(&start_time,&end_time);


		#ifdef DEBUG
		/*
		//show subnames and hits in then
		for(i=0;i<nrOfSubnames;i++) {
                        printf("nrOfFiletypes %s: %i\n",subnames[i].subname,subnames[i].nrOfFiletypes);
			for(y=0;y<subnames[i].nrOfFiletypes;y++) {
				printf("files %s %i\n",subnames[i].filtypes[y].name,subnames[i].filtypes[y].nrof);
			}
                }
		*/
		#endif

		#ifdef ATTRIBUTES

		//
		// filtrerer
		if (vector_size((*filteron).attributes) > 0)
		    {
//			fprintf(stderr, "search: Filtering on attributes: ");
//			println((*filteron).attributes, container_value((*filteron).attributes));
//			fprintf(stderr, "search: *** Warning! Not implemented yet! ***\n");

		        struct fte_data	*fdata = NULL;
			int		j;
			char		*D = calloc(*TeffArrayElementer, sizeof(char));
			char		filtered = 0;

			for (j=0; j<vector_size((*filteron).attributes); j++)
			    {
				int	atype = pair(vector_get((*filteron).attributes,j)).first.i;
				char	*attrib = pair(vector_get((*filteron).attributes,j)).second.ptr;

				if (atype == QUERY_GROUP || atype == QUERY_FILETYPE)
				    {
					//char	other_group = 0;

					filtered++;
					if (fdata==NULL) fdata = fte_init(bfile("config/file_extensions.conf"));
					//if (atype==QUERY_GROUP && !strcasecmp(fte_getdefaultgroup(fdata, "nbo"), attrib)) other_group = 1;

					char		**ptr1, **ptr2;
					int		ok = 0;

					if (atype==QUERY_FILETYPE)
					    {
						ok = fte_getext_from_ext(fdata, attrib, &ptr1, &ptr2);
					    }

					for (i = 0; i < (*TeffArrayElementer); i++)
					    {
						char		**ptr_i;

						if (atype==QUERY_GROUP)
						    {
							if (fte_belongs_to_group(fdata, "nbo", TeffArray->iindex[i].filetype, attrib))
							    {
								D[i]++;
							    }
						    }
						else if (ok)
						/*
				    		else if ((atype==QUERY_FILETYPE && fte_getext_from_ext(fdata, attrib, &ptr1, &ptr2))
						    || (atype==QUERY_GROUP && fte_getextension(fdata, "nbo", attrib, &ptr1, &ptr2)))
						*/
						    {
						        for (ptr_i=ptr1; ptr_i<ptr2; ptr_i++)
							    {
								if (!strcmp(TeffArray->iindex[i].filetype, *ptr_i))
								    {
									D[i]++;
									break;
								    }
							    }
						    }
						else
						    {
							//if (!fte_getext_from_ext(fdata, TeffArray->iindex[i].filetype, &ptr1, &ptr2))
							if (!strcasecmp(attrib, TeffArray->iindex[i].filetype))
							    {
								D[i]++;
							    }
						    }
					    }
				    }
			    }

			// Marker dokumenter som filtrert ut:
			if (filtered)
			    {
				for (i = 0; i < (*TeffArrayElementer); i++)
				    {
					if (D[i] < filtered)
					    {
					        if (TeffArray->iindex[i].indexFiltered.is_filtered) {
						    #ifdef DEBUG
							printf("is already filtered out\n");
						    #endif
					        }
					        else {
						    --(*TotaltTreff);
						}

						TeffArray->iindex[i].indexFiltered.is_filtered = 1;
			    		        TeffArray->iindex[i].indexFiltered.filename = 1;
					    }
				    }
			    }

			free(D);
			if (fdata!=NULL) fte_destroy(fdata);
		    }


		#endif	// ATTRIBUTES

		/*
		*********************************************************************************************************************
		datoer

		lager en oversikt over, og filtrerer på, dato
		*********************************************************************************************************************
		*/
		printf("<################################# date filter ######################################>\n");
		gettimeofday(&start_time, NULL);


		printf("looking opp dates\n");

		//slår opp alle datoene
		for (i = 0; i < *TeffArrayElementer; i++) {
			iintegerGetValue(&TeffArray->iindex[i].date,sizeof(int),TeffArray->iindex[i].DocID,"dates",(*TeffArray->iindex[i].subname).subname);
			#ifdef DEBUG
			printf("got %u\n",TeffArray->iindex[i].date);
			#endif
		}
		printf("looking opp dates end\n");

		gettimeofday(&end_time, NULL);
		(*queryTime).iintegerGetValueDate = getTimeDifference(&start_time,&end_time);




		gettimeofday(&start_time, NULL);

		//
		//filter på dato

		if ((*filteron).date != NULL) {
			printf("wil filter on date \"%s\"\n",(*filteron).date);

			struct datelib dl;

			dl.start = dl.end = 0;
			getdate((*filteron).date, &dl);

			printf("start %u, end %u\n",dl.start,dl.end);
			printf("start: %s",ctime(&dl.start));
			printf("end:   %s",ctime(&dl.end));

			int notFiltered, filtered;

			notFiltered = 0;
			filtered = 0;

			for (i = 0; i < *TeffArrayElementer; i++) {

				/*
				if (TeffArray->iindex[i].indexFiltered.subname || TeffArray->iindex[i].indexFiltered.filename) {
					#ifdef DEBUG
						printf("is al redu filtered out ");
					#endif				
				}
				else 
				*/
				if ((TeffArray->iindex[i].date >= dl.start) && (TeffArray->iindex[i].date <= dl.end)) {
					#ifdef DEBUG
					printf("time hit %s",ctime(&TeffArray->iindex[i].date));
					#endif
					if (TeffArray->iindex[i].indexFiltered.is_filtered) {
						#ifdef DEBUG
							printf("is already filtered out ");
						#endif				
					}
					else {
						++notFiltered;
					}
				}
				else {
					#ifdef DEBUG
					printf("not time hit %s",ctime(&TeffArray->iindex[i].date));
					#endif

					if (TeffArray->iindex[i].indexFiltered.is_filtered) {
						#ifdef DEBUG
							printf("is already filtered out ");
						#endif	
					}
					else {
						--(*TotaltTreff);
						++filtered;
					}

					TeffArray->iindex[i].indexFiltered.is_filtered = 1;
					TeffArray->iindex[i].indexFiltered.date = 1;
				}

			}		

			vboprintf("date filter: filtered %i, notFiltered %i\n",filtered,notFiltered);
			
			

		}

		printf("</################################# date filter ######################################>\n");


		/*
		*********************************************************************************************************************

		duplicate checking
		*********************************************************************************************************************
		*/

		printf("<################################# duplicate checking ######################################>\n");

		// Loop over all results and do duplicate checking...
		if (crc32maphash != NULL)
			*crc32maphash = create_hashtable(41, ht_integerhash, ht_integercmp);

		TeffArray->subnames = malloc(sizeof(char*) * nrOfSubnames);
		for (i=0; i<nrOfSubnames; i++)
		    {
			TeffArray->subnames[i] = subnames[i].subname;
		    }
		automaton	*subnames_automaton = build_automaton(nrOfSubnames, (unsigned char**)TeffArray->subnames);

		if (crc32maphash != NULL)
	       	for (i = 0; i < (*TeffArrayElementer); i++) {
			//runarb 22 sept 2008: hvorfor er denne her? Brukes bare på websøk
			//TeffArray->iindex[i].PopRank = popRankForDocIDMemArray(TeffArray->iindex[i].DocID);
#if 1
//			if (crc32maphash == NULL)
//				continue;

			// This document is filtered out, don't even think about it.
			if (TeffArray->iindex[i].indexFiltered.filename == 1
			    || TeffArray->iindex[i].indexFiltered.date == 1
			    || TeffArray->iindex[i].indexFiltered.attribute == 1)
				continue;

			//TeffArray->iindex[i].indexFiltered.duplicate = 0; // Allerede initiert (mÃ¥ initieres for *alle*).


			int	subname_nr = search_automaton(subnames_automaton, TeffArray->iindex[i].subname->subname);
			printf("%s has nr %i\n", TeffArray->iindex[i].subname->subname, subname_nr);
			assert(subname_nr>=0);

			/* XXX: Don't reopen all the time */
			if ((crc32map = reopen(rLotForDOCid(TeffArray->iindex[i].DocID), sizeof(unsigned int), "crc32map", TeffArray->iindex[i].subname->subname, RE_READ_ONLY)) == NULL) {
//			if ((crc32map = reopen(rLotForDOCid(TeffArray->iindex[i].DocID), sizeof(unsigned int), "crc32map", TeffArray->subnames[subname_nr], RE_READ_ONLY)) == NULL) {
				debug("reopen(crc32map)\n");
				continue;
			}

			unsigned int crc32;
			crc32 = *RE_Uint(crc32map, TeffArray->iindex[i].DocID);
			//printf("Got hash value: %x\n", crc32);
			reclose(crc32map);

			if (crc32 == 0) {
				debug("don't have crc32 value for DocID");
				continue;
			}

			// Byttet fra list til vector da vector er raskere.
			struct duplicate_docids *dup = hashtable_search(*crc32maphash, &crc32);

			if (dup == NULL) {
				dup = malloc(sizeof(struct duplicate_docids));
				dup->coll = calloc(nrOfSubnames, sizeof(char));
				dup->V = vector_container( pair_container( int_container(), int_container() ) );
				hashtable_insert(*crc32maphash, uinttouintp(crc32), dup);

				if (subname_nr >= 0) dup->coll[subname_nr]++;
				vector_pushback(dup->V, TeffArray->iindex[i].DocID, subname_nr);

			} else {
				/* Remove duplicated */
				if (TeffArray->iindex[i].indexFiltered.is_filtered) {
					#ifdef DEBUG
						printf("is already filtered out ");
					#endif				
				}
				else {
					--(*TotaltTreff);
				}

				//printf("DUPLICATE: %s / %s\n", TeffArray->iindex[i].subname->subname, TeffArray->subnames[subname_nr]);
				vector_pushback(dup->V, TeffArray->iindex[i].DocID, subname_nr);

				//if (subname_nr >= 0)
				    {
					if (dup->coll[subname_nr] > 0)
					    TeffArray->iindex[i].indexFiltered.duplicate_in_collection = subname_nr;

					dup->coll[subname_nr]++;
				    }

				TeffArray->iindex[i].indexFiltered.is_filtered = 1;
				TeffArray->iindex[i].indexFiltered.duplicate = 1;
			}
#endif
		}
		//reclose_cache();

		free_automaton(subnames_automaton);



		//debug: printer ut alle treff, og litt om de.
		//#ifdef DEBUG_II
			printf("hits after duplicate checking:\n\n");
			printf("\t| %-5s | %-20s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", "DocId", "Subname", "Date", "Subname", "Filename",
			    "dup", "dup_c", "attr", "fltr");
			for (i = 0; (i < (*TeffArrayElementer)) && (i < 100); i++) {
				printf("\t| %-5u | %-20s | %-8d | %-8d | %-8d | %-8d | %-8d | %-8d | %-8d |\n",
					TeffArray->iindex[i].DocID,
					(*TeffArray->iindex[i].subname).subname,
					TeffArray->iindex[i].indexFiltered.date,
					TeffArray->iindex[i].indexFiltered.subname,
					TeffArray->iindex[i].indexFiltered.filename,
					TeffArray->iindex[i].indexFiltered.duplicate,
					TeffArray->iindex[i].indexFiltered.duplicate_in_collection,
					TeffArray->iindex[i].indexFiltered.attribute,
					TeffArray->iindex[i].indexFiltered.is_filtered
				);
			}
			printf("\n");
		//#endif

		printf("</################################# duplicate checking ######################################>\n");

		printf("order by \"%s\"\n",orderby);

		if (((*filteron).sort != NULL) && (strcmp((*filteron).sort,"newest") == 0)) {
			printf("will do newest sort\n");
			for (i = 0; i < *TeffArrayElementer; i++) {
				TeffArray->iindex[i].allrank = TeffArray->iindex[i].date;
			}
		}
		else if ( ((*filteron).sort != NULL) && (strcmp((*filteron).sort,"oldest") == 0) ) {
			printf("will do oldest sort\n");
			for (i = 0; i < *TeffArrayElementer; i++) {
				//4294967295 unsigned int (long) max
				TeffArray->iindex[i].allrank = ULONG_MAX - TeffArray->iindex[i].date;
			}			
		}
		else {
			printf("do normal sort\n");
			for (i = 0; i < *TeffArrayElementer; i++) {
				TeffArray->iindex[i].allrank = TeffArray->iindex[i].TermRank;

				if (TeffArray->iindex[i].phraseMatch) {
                                        TeffArray->iindex[i].allrank = TeffArray->iindex[i].allrank * 2;
                                }
				//printf(" $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Got a score of: %d\n", TeffArray->iindex[i].allrank);
			}
		}

		gettimeofday(&end_time, NULL);
                (*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);

	#else
	//hvis vi har få traff bruker vi en annen rangering
	//if ((*TeffArrayElementer) < 2000) {
	//if ((*TotaltTreff) < 2000) {
	if (0) {

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
				++TeffArray->iindex[i].PopRank;

				//hvis vi har en treff som er i frase øker vi betydligheten av den siden
				//ToDo: bør ha noe annt en .PopRank * 2 her. Kansje en verdi som er 0, eller 
				//xx hvis vi har en frasetreff?
				if (TeffArray->iindex[i].phraseMatch) {
					TeffArray->iindex[i].PopRank = TeffArray->iindex[i].PopRank * 2;
				}

				TeffArray->iindex[i].allrank = floor((TeffArray->iindex[i].TermRank * TeffArray->iindex[i].TermRank) * TeffArray->iindex[i].PopRank);
				//TeffArray->iindex[i].allrank = TeffArray->iindex[i].TermRank + TeffArray->iindex[i].PopRank;
				//}
			}
			gettimeofday(&end_time, NULL);
			(*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);

	}
	else {

		gettimeofday(&start_time, NULL);

		allrankcalk(TeffArray,TeffArrayElementer);

                gettimeofday(&end_time, NULL);
                (*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);

	}
	#endif


	gettimeofday(&start_time, NULL);

		vboprintf("vil sortere %i\n",*TeffArrayElementer);
 		qsort(TeffArray->iindex, *TeffArrayElementer , sizeof(struct iindexMainElements), compare_elements);
		vboprintf("sort ferdig\n");
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

	vboprintf("search: ~searchSimple()\n");
}

#ifdef BLACK_BOKS

void searchFilterInit(struct filtersFormat *filters, int dates[]) {

	int i;

	fprintf(stderr, "search: searchFilterInit()\n");

	(*filters).filtypes.nrof 	= 0;
	(*filters).collections.nrof 	= 0;

	for (i=0;i<10;i++) {
		dates[i] = 0;
	}

}

static attr_crc32_words_block_compare(const void *a, const void *b)
{
    int		i=*((int*)a), j=*((int*)b);

    if (i>j) return +1;
    if (i<j) return -1;
    return 0;
}


char* searchFilterCount(int *TeffArrayElementer, 
			struct iindexFormat *TeffArray, 
			struct filtersFormat *filters,
			struct subnamesFormat subnames[], 
			int nrOfSubnames,
			struct filteronFormat *filteron,
			int dates[],
			struct queryTimeFormat *queryTime,
			struct fte_data *getfiletypep,
			attr_conf *showattrp,
			query_array *qa
		) {

		char *filesKey;
		int *filesValue;
		struct hashtable *h;
		int i, j;
		struct timeval start_time, end_time;
		/***********************************************************************************************
		teller filtyper
		***********************************************************************************************/

		fprintf(stderr, "search: searchFilterCount()\n");

		#ifdef ATTRIBUTES
		// subname -> (re, attr_columns, crc32->word)
		container	*attr_subname_re = map_container( string_container(),
						tuple_container( 5, map_container( int_container(), ptr_container() ),
						    set_container( string_container() ),
						    ptr_container(), ptr_container(), int_container() ) );
		int		attr_crc32_words_blocksize = sizeof(unsigned int) + sizeof(char)*MAX_ATTRIB_LEN;
		// key -> (val->#), #
		container	*attributes = map_container( string_container(), pair_container( ptr_container(), int_container() ) );
		container	*file_groups = map_container( string_container(), string_container() );
		#endif


//ax		h = create_hashtable(200, fileshashfromkey, filesequalkeys);

		for (i = 0; i < (*TeffArrayElementer); i++) {

			//his dette er en slettet index element så teller vi den ikke.
			//dette så vi ikke skal telle ting som folk ikke her tilgang til
			//runarb: 1 now 2007: Hvorfor er denne halet ut ?? Da gjør at ting som filtreres ut i filtere i søkekjernen ikke vises riktig
			//legger den inn
			//runarb: 2 nov 2007: Skaper problmer med at tallene forandrer seg, gjør slik at de fortsat blir med i tallene, men ikke
			//vises i resultatene. Gør så at brukeren kan slå av filteret ved å vise filterbeskjeden
			/*
			if (TeffArray->iindex[i].deleted) {
				continue;
			}
			else 
			*/
			if (TeffArray->iindex[i].indexFiltered.date == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.subname == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.duplicate == 1) {
				continue;
			}

			#ifdef ATTRIBUTES
			// Ax: Slå opp attributter for dokumentet.

			int	DocID = TeffArray->iindex[i].DocID;
			char	*subname = TeffArray->iindex[i].subname->subname;
//			printf("DocID=%i (subname=%s)\n", DocID, subname);
			iterator	it_re = map_find(attr_subname_re, subname);
			container	*lot_re;
			struct reformat	*re = NULL;
			container	*attr_keys = NULL;
			FILE		*f_crc32_words = NULL;
			void		*m_crc32_words = NULL;
			int		crc32_words_size = 0;
			char		no_attributes = 0;

			// Les inn og åpne nødvendige filer:
			if (!it_re.valid)
			    {
			        f_crc32_words = lotOpenFileNoCasheByLotNr(1, "crc32attr.map", "r", 's', subname);

				if (f_crc32_words==NULL)
				    {
					no_attributes = 1;
				    }
				else
				    {
					struct stat	inode;
					fstat(fileno(f_crc32_words), &inode);
					crc32_words_size = inode.st_size;
					m_crc32_words = mmap(NULL, crc32_words_size, PROT_READ, MAP_PRIVATE, fileno(f_crc32_words), 0);

					crc32_words_size/= attr_crc32_words_blocksize;

					iterator it_pre = map_insert(attr_subname_re, subname, f_crc32_words, m_crc32_words, crc32_words_size);
					attr_keys = tuple(map_val(it_pre)).element[1].C;
					// Read attribute columns for this subname.
					FILE	*f_attrcols = lotOpenFileNoCasheByLotNr(1, "reposetory.attribute_columns", "r", 's', subname);

					if (f_attrcols!=NULL)
					    {
						char		key[MAX_ATTRIB_LEN];

						while (fgets(key, MAX_ATTRIB_LEN, f_attrcols) != NULL)
						    {
							key[strlen(key)-1] = '\0';
							set_insert(attr_keys, key);
						    }

						fclose(f_attrcols);
					    }

//					printf("reopen(lotNr=1, structsize=%i, file=%s, subname=%s, flags=%i)\n",
//					sizeof(unsigned int)*set_size(attr_keys), "attributeIndex", subname, 0);
//					re = reopen(1, sizeof(unsigned int)*set_size(attr_keys), "attributeIndex", subname, 0);
//					tuple(map_val(it_pre)).element[0].ptr = re;
					lot_re = tuple(map_val(it_pre)).element[0].C;
				    }
			    }
			else
			    {
				// Values already in cache:
				lot_re = tuple(map_val(it_re)).element[0].C;
				attr_keys = tuple(map_val(it_re)).element[1].C;
				f_crc32_words = tuple(map_val(it_re)).element[2].ptr;
				m_crc32_words = tuple(map_val(it_re)).element[3].ptr;
				crc32_words_size = tuple(map_val(it_re)).element[4].i;
			    }

			if (!no_attributes)
			    {
				int	lotNr = rLotForDOCid(DocID);
				it_re = map_find(lot_re, lotNr);
				if (!it_re.valid)
				    {
					re = reopen(lotNr, sizeof(unsigned int)*set_size(attr_keys), "attributeIndex", subname, 0);
					if (re!=NULL) map_insert(lot_re, lotNr, re);
				    }
				else
				    {
					re = map_val(it_re).ptr;
				    }
			    }

			// Beregning av attributt-bits:
			int	len = TeffArray->attrib_count + 1;
			int	count = 0;
			for (j=0; j<len-1; j++)
			    count+= (1 - TeffArray->iindex[i].indexFiltered.attrib[j])<<j;
			count+= (1 - TeffArray->iindex[i].indexFiltered.filename)<<(len-1);

			if (!no_attributes)
			    {
				// Faktisk oppslag:
				if (re!=NULL && attr_keys!=NULL && f_crc32_words!=NULL && m_crc32_words!=NULL)
				    {
					unsigned int	*crc32val = reget(re, DocID);
				        iterator	it_s1 = set_begin(attr_keys);

					for (; it_s1.valid; it_s1=set_next(it_s1))
					    {
						if (*crc32val != 0)
						    {
							char	*key = (char*)set_key(it_s1).ptr;
							char	*value = (char*)bsearch((const void*)(crc32val), (const void*)m_crc32_words, crc32_words_size,
							    attr_crc32_words_blocksize, attr_crc32_words_block_compare );

							if (value != NULL)
							    {
								value+= sizeof(unsigned int);

								// Legg til:
								attribute_count_add(count, attributes, 2, key, value);
							    }
						    }

						crc32val++;
					    }
				    }
			    }
			#endif	// ATTRIBUTES
/*ax
			// Slå opp filendelse:
			if (NULL == (filesValue = hashtable_search(h,TeffArray->iindex[i].filetype) )) {
//				fprintf(stderr, "search: Hash does not contain filetype '%s'. Adding filetype.\n", TeffArray->iindex[i].filetype);
				filesValue = malloc(sizeof(int));
				(*filesValue) = 1;
				filesKey = strdup(TeffArray->iindex[i].filetype);
				if (! hashtable_insert(h,filesKey,filesValue) ) {
					printf("cant insert\n");     
					exit(-1);
				}

		        }
			else {
				++(*filesValue);
			}
*/
			#ifdef ATTRIBUTES
			char	**ptr1, **ptr2;
			char	*file_ext;

			if (fte_getext_from_ext(getfiletypep, TeffArray->iindex[i].filetype, &ptr1, &ptr2))
			    {
				ptr2--;
				file_ext = *ptr2;
			    }
			else
			    {
				file_ext = TeffArray->iindex[i].filetype;
			    }

			iterator	it_gr = map_find(file_groups, file_ext);

			if (it_gr.valid)
			    {
				attribute_count_add(count, attributes, 3, "group", map_val(it_gr).ptr, file_ext);
			        attribute_count_add(count, attributes, 2, "filetype", file_ext);
			    }
			else
			    {
			        char	*group, *descr, *icon;

				fte_getdescription(getfiletypep, "nbo", file_ext, &group, &descr, &icon);
				map_insert(file_groups, file_ext, group);

				attribute_count_add(count, attributes, 3, "group", group, file_ext);
			        attribute_count_add(count, attributes, 2, "filetype", file_ext);
			    }
			#endif	// ATTRIBUTES
		}

		#ifdef ATTRIBUTES
		// Lukk åpne filer og frigjør ledig minne:
		{
		    iterator	it_re1 = map_begin(attr_subname_re);
		    for (; it_re1.valid; it_re1=map_next(it_re1))
			{
			    iterator	it_re2 = map_begin(tuple(map_val(it_re1)).element[0].C);
			    for (; it_re2.valid; it_re2=map_next(it_re2))
				{
				    reclose(map_val(it_re2).ptr);
				}

			    munmap(tuple(map_val(it_re1)).element[3].ptr, tuple(map_val(it_re1)).element[4].i);
			    fclose(tuple(map_val(it_re1)).element[2].ptr);
			}
		    destroy(attr_subname_re);
		}
		destroy(file_groups);
		#endif

/*
		(*filters).filtypes.nrof = 0;

		if (hashtable_count(h) > 0)
		{

			//legger inn All feltet
			strscpy((*filters).filtypes.elements[ (*filters).filtypes.nrof ].name,
                                "All",
	                        sizeof((*filters).filtypes.elements[ (*filters).filtypes.nrof ].name));
			strscpy((*filters).filtypes.elements[ (*filters).filtypes.nrof ].longname,
                                "All",
	                        sizeof((*filters).filtypes.elements[ (*filters).filtypes.nrof ].longname));
			(*filters).filtypes.elements[(*filters).filtypes.nrof].nrof = 0;

			++(*filters).filtypes.nrof;

			//itererer over hash
			struct hashtable_itr *itr;

       			itr = hashtable_iterator(h);
			
       			do {
       				filesKey = hashtable_iterator_key(itr);
       				filesValue = (int *)hashtable_iterator_value(itr);

				//ignorerer filnavn som er blanke, eller har % i seg
				if ((strchr(filesKey,'%') != NULL) || (filesKey[0] == '\0')) {
					printf("ignoring file name \"%s\" that has %i files\n",filesKey,(*filesValue));
					continue;
				}

				printf("files \"%s\": %i\n",filesKey,*filesValue);

				strscpy(
					(*filters).filtypes.elements[ (*filters).filtypes.nrof ].name,
					filesKey,
					sizeof((*filters).filtypes.elements[ (*filters).filtypes.nrof ].name));

				(*filters).filtypes.elements[(*filters).filtypes.nrof].nrof = (*filesValue);
				++(*filters).filtypes.nrof;
				
				
       			} while ((hashtable_iterator_advance(itr)) && ((*filters).filtypes.nrof<MAXFILTERELEMENTS));
    			free(itr);

			//sorterer på forekomst
//@ax-			qsort((*filters).filtypes.elements,(*filters).filtypes.nrof,sizeof(struct filterinfoElementsFormat),compare_filetypes);

		}

		hashtable_destroy(h,1); 


				// key==group_id, value=={group, size}:
		container	*G = map_container( int_container(), pair_container( string_container(), int_container() ) );	// @ax+
				// key==descr_id, value=={descr, size, group_id, postfix}:
		container	*D = map_container( int_container(),
		    tuple_container( 4, string_container(), int_container(), int_container(), string_container() ) );	// @ax+

		char *cpnt;

		for (i=1; i<(*filters).filtypes.nrof; i++)
		    {
		        char		*group, *descr;
			int		ret;
			ret = fte_getdescription(getfiletypep, "nbo", (*filters).filtypes.elements[i].name, &group, &descr);

//			printf("  %s:%s\t%i = [%i|%i]\n", group, descr, ret, ret/256, ret%256);

			iterator	mit = map_find(G, ret%256);
			if (mit.valid)
			    pair(map_val(mit)).second.i+= (*filters).filtypes.elements[i].nrof;
			else
			    map_insert(G, ret%256, group, (*filters).filtypes.elements[i].nrof);

			mit = map_find(D, ret/256);
			if (mit.valid)
			    tuple(map_val(mit)).element[1].i+= (*filters).filtypes.elements[i].nrof;
			else
			    {
				char	**ptr1, **ptr2;
				if (fte_getext_from_ext(getfiletypep, (*filters).filtypes.elements[i].name, &ptr1, &ptr2))
				    {
					ptr2--;
					map_insert(D, ret/256, descr, (*filters).filtypes.elements[i].nrof, ret%256, *ptr2);
				    }
				else
				    {
					map_insert(D, ret/256, descr, (*filters).filtypes.elements[i].nrof, ret%256,
					    (*filters).filtypes.elements[i].name);
				    }
			    }
		    }

				// key==size, value=={group, group_id}:
		container	*G2 = multimap_container( int_container(), pair_container( string_container(), int_container() ) );
				// key=={group_id, size}, value=={descr, postfix}:
		container	*D2 = multimap_container( pair_container(int_container(), int_container()), pair_container( string_container(), string_container() ) );

		iterator	git = map_begin(G);
		for (; git.valid; git=map_next(git))
		    multimap_insert(G2, pair(map_val(git)).second.i, pair(map_val(git)).first.ptr, map_key(git).i);

		iterator	dit = map_begin(D);
		for (; dit.valid; dit=map_next(dit))
		    multimap_insert(D2, tuple(map_val(dit)).element[2].i, tuple(map_val(dit)).element[1].i, tuple(map_val(dit)).element[0].ptr, tuple(map_val(dit)).element[3].ptr);

		(*filters).filtypes.nrof = 1;
		git = multimap_end(G2);
		for (; git.valid; git=multimap_previous(git))
		    {
			printf("  %s (%i)\n", pair(multimap_val(git)).first.ptr, multimap_key(git).i);

			i = (*filters).filtypes.nrof;

			dit = multimap_end(D2);
			for (; dit.valid; dit=multimap_previous(dit))
			    if (pair(multimap_key(dit)).first.i == pair(multimap_val(git)).second.i)
				{
				    strscpy( (*filters).filtypes.elements[i].name, pair(multimap_val(dit)).second.ptr, sizeof((*filters).filtypes.elements[i].name) );
				    printf("    [%s] %s (%i)\n", pair(multimap_val(dit)).second.ptr, pair(multimap_val(dit)).first.ptr, pair(multimap_key(dit)).second.i);
				    #ifdef ATTRIBUTES
				    int	len = TeffArray->attrib_count + 1;
				    int count = 0;
				    //int	*count = malloc(sizeof(int)*len);
				    //for (j=0; j<len-1; j++) count[j] = pair(multimap_key(dit)).second.i;	// TODO: Fiks senere!!!
				    //count[len-1] = pair(multimap_key(dit)).second.i;

				    for (j=0; j<len-1; j++)
					count+= 1<<j;
				    count+= 1<<(len-1);

				    for (j=0; j<pair(multimap_key(dit)).second.i; j++)
					{
					    attribute_count_add(count, attributes, 3, "group", pair(multimap_val(git)).first.ptr, pair(multimap_val(dit)).second.ptr);
					    attribute_count_add(count, attributes, 2, "filetype", pair(multimap_val(dit)).second.ptr);
					}
				    //free(count);
				    #endif
				}

			strscpy( (*filters).filtypes.elements[i].longname, pair(multimap_val(git)).first.ptr, sizeof((*filters).filtypes.elements[i].longname) );
			(*filters).filtypes.elements[i].nrof = multimap_key(git).i;
			(*filters).filtypes.nrof++;
		    }

		destroy(G);	// @ax+
		destroy(D);	// @ax+
		destroy(G2);	// @ax+
		destroy(D2);	// @ax+
*/
		#if defined(ATTRIBUTES) && defined(DEBUG)
		printf("attributes:\n");
		attribute_count_print(attributes, TeffArray->attrib_count+1, 2);
		printf("------\n");
		#endif

		/***********************************************************************************************
		 collections
		***********************************************************************************************/

		//collections
		//finner hvilken vi har trykket på, og markerer denne slik at det kan markeres i designed i klienten
		//kopierer også inn antall treff i hver subname

		h = create_hashtable(200, fileshashfromkey, filesequalkeys);

		//legger først inn alle subnames med verdien 0, slik at de blir med i tellingen.
		for(i=0;i<nrOfSubnames;i++) {


			//ser ut til at subnames kan komme flere ganger. Hvis man for eks er med i to grupper som begge har tilgant til "felles"
			if (NULL == (filesValue = hashtable_search(h,subnames[i].subname) )) {    

				filesValue = malloc(sizeof(int));
				(*filesValue) = 0;

               			filesKey = strdup(subnames[i].subname);

				if (! hashtable_insert(h,filesKey,filesValue) ) {
					printf("cant insert\n");     
					exit(-1);
				}
			}

        	}


		for (i = 0; i < (*TeffArrayElementer); i++) {

			//his dette er en slettet index element så teller vi den ikke.
			//dette så vi ikke skal telle ting som folk ikke her tilgang til
			//runarb: 1 nov 2007: Hvorfor er denne halet ut ?? Da gjør at ting som filtreres ut i filtere i søkekjernen ikke vises riktig
			//legger den inn
			//runarb: 2 nov 2007: Skaper problmer med at tallene forandrer seg, gjør slik at de fortsat blir med i tallene, men ikke
			//vises i resultatene. Gør så at brukeren kan slå av filteret ved å vise filterbeskjeden.
			/*
			if (TeffArray->iindex[i].deleted == 1) {
				continue;
			}
			else 
			*/

			
			if (TeffArray->iindex[i].indexFiltered.filename == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.date == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.attribute == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.duplicate_in_collection >= 0) {
				continue;
			}
				
			if (NULL == (filesValue = hashtable_search(h,(*TeffArray->iindex[i].subname).subname) )) {    
				printf("not found!. Vil insert first \"%s\"\n",(*TeffArray->iindex[i].subname).subname);
				filesValue = malloc(sizeof(int));
				(*filesValue) = 1;
				filesKey = strdup((*TeffArray->iindex[i].subname).subname);
				if (! hashtable_insert(h,filesKey,filesValue) ) {
					printf("cant insert\n");     
					exit(-1);
				}

		        }
			else {
				++(*filesValue);
			}
		
		}

		/* Iterator constructor only returns a valid iterator if
		 the hashtable is not empty 
		*/

		(*filters).collections.nrof = 0;

		if (hashtable_count(h) > 0)
		{

			//legger inn All feltet
			strscpy((*filters).collections.elements[ (*filters).collections.nrof ].name,
                                "All",
	                        sizeof((*filters).collections.elements[ (*filters).collections.nrof ].name));
			(*filters).collections.elements[(*filters).collections.nrof].nrof = 0; //må vi ha dene her. Blir All brukt ?
			++(*filters).collections.nrof;

			struct hashtable_itr *itr;

       			itr = hashtable_iterator(h);
			
			do {
       				filesKey = hashtable_iterator_key(itr);
       				filesValue = (int *)hashtable_iterator_value(itr);

				//ToDo: siden vi mangler sletting av subname to username kan vi fort ende opp med masse col med 0
				// vi tar de derfor ikke med
				// vil dette føre til forviring blant brukere
				//runarb: 04 mars 2008
				//if (*filesValue != 0) {
					printf("collection \"%s\": %i\n",filesKey,*filesValue);

					strscpy(
						(*filters).collections.elements[ (*filters).collections.nrof ].name,
						filesKey,
						sizeof((*filters).collections.elements[ (*filters).collections.nrof ].name));

					(*filters).collections.elements[(*filters).collections.nrof].nrof = (*filesValue);

					++(*filters).collections.nrof;
				//}
				
       			} while ((hashtable_iterator_advance(itr)) && ((*filters).collections.nrof<MAXFILTERELEMENTS));
    			free(itr);

		}
		hashtable_destroy(h,1); 

		//legger en hvilken som er trykket på, om noen
		//inaliserer alle først
		for(i=0;((i<nrOfSubnames +1) && (i<MAXFILTERELEMENTS));i++) { //+1 for og få med All
				(*filters).collections.elements[i].checked = 0;			
		}
		//hvis vi ikke har trykket på noen så markerer vi All, som er nr 0.
		//hvis ikke skal vi søke oss gjenom og finne den som er trykket på

		if ((*filteron).collection == NULL) {
			(*filters).collections.elements[0].checked = 1;
		}
		else {
			for(i=0;((i<nrOfSubnames +1) && (i<MAXFILTERELEMENTS));i++) { //+1 for og få med All

				if (strcasecmp((*filters).collections.elements[i].name,(*filteron).collection) == 0) {
					(*filters).collections.elements[i].checked = 1;
				}
			}
		}
		
		printf("filtering on coll \"%s\"\n",filteron->collection);
		for (i=0;i<(*filters).collections.nrof;i++) {
			printf("coll \"%s\", checked %i\n",(*filters).collections.elements[i].name,(*filters).collections.elements[i].checked);
		}

		/***********************************************************************************************
		//dates
		***********************************************************************************************/
		gettimeofday(&start_time, NULL);
		//kjører dateview
		dateview dv;
		dateview *dvo;

		printf("dateview start\n");
		date_info_start(&dv, 0, -1);
		
		printf("for all dates\n");
		for (i = 0; i < *TeffArrayElementer; i++) {

			if (TeffArray->iindex[i].indexFiltered.filename == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.subname == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.attribute == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.duplicate == 1) {
				continue;
			}

			date_info_add(&dv, (time_t)TeffArray->iindex[i].date);
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
		date_info_free(dvo); // ax: Fjerner minnelekasje.
		printf("dateview end\n");

		gettimeofday(&end_time, NULL);
		(*queryTime).dateview = getTimeDifference(&start_time,&end_time);

#ifdef ATTRIBUTES
		// Attributter:
		fprintf(stderr, "search: generating xml for attributes\n");

		char	*nav_xml = attribute_generate_xml(attributes, TeffArray->attrib_count+1, showattrp, getfiletypep, qa);
		fprintf(stderr, "%s", nav_xml);

		println(attributes);
		attribute_destroy_recursive(attributes);

		fprintf(stderr, "search: done attributes\n");
#endif
		fprintf(stderr, "search: ~searchFilterCount()\n");
#ifdef ATTRIBUTES
    return nav_xml;
#else
    return NULL;
#endif
}
#endif
int compare_filetypes (const void *p1, const void *p2) {
        if ((((struct filterinfoElementsFormat *)p1)->nrof > ((struct filterinfoElementsFormat *)p2)->nrof) 
		|| (strncmp(((struct filterinfoElementsFormat *)p1)->name,"All",3) == 0)
	) {
                return -1;
	}
        else {
                return ((struct filterinfoElementsFormat *)p1)->nrof < ((struct filterinfoElementsFormat *)p2)->nrof;
	}
}

int compare_elements (const void *p1, const void *p2) {


        if (((struct iindexMainElements *)p1)->allrank > ((struct iindexMainElements *)p2)->allrank)
                return -1;
        else
                return ((struct iindexMainElements *)p1)->allrank < ((struct iindexMainElements *)p2)->allrank;

}


