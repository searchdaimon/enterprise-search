
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#if defined (_OPENMP)
	#include <omp.h>
#endif

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
#include "../logger/logger.h"

#include "merge.h"


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
	#include "../attributes/attribute_descriptions.h"
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

#include "../searchkernel/preopen.h"

//rangeringsfilter er der vi har et hvist antall treff, definert som RFC (Rank Filter Cutoff).
//har vi så mnage treff filtrerer vi ut de under en hvis rankt
#define AthorRFC1 	10000
#define MainRFC1 	200000
#define MainRFC2 	10000

#ifndef BLACK_BOKS
void *
cache_index_get(char *path, size_t *size)
{
	return NULL;
}
#endif

int compare_elements (const void *,const void *);
int compare_filetypes (const void *p1, const void *p2);

/*
struct filesKeyFormat {
	struct subnamesFormat *subname;
	char filename[4];
};
*/

// swap two ponters
void swapiindex(struct iindexFormat **a, struct iindexFormat **b) {

	struct iindexFormat  *tmp;
	
	tmp = *a;
	*a = *b;
	*b = tmp;

}

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

void resultArrayInit(struct iindexFormat *array) {
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
	bblog(DEBUG, "rankAthor_complicacy: nrofhit %i", nrofhit);
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
		bblog(DEBUG, "rankMain: DocID %u, subname \"%s\"", DocID,subname->subname);
		for (i = 0;i < nrofhit; i++) {
			bblog(DEBUG, "\thit %i, phrase %i", (int)hits[i].pos,(int)hits[i].phrase);
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
				bblog(DEBUG, "rank title hit. add %i, TittelFirstWord %i, subname \"%s\", DocID %u", (*subname).config.rankTittelFirstWord,TittelFirstWord,(*subname).subname,DocID);
				#endif
				++nrTittelFirst;
			}
			//starter på 100, så det mø være under. For eks under 106 gir til 6
			else if (hits[i].pos < 106) {
                               	++nrTittel;
			}
			else {
				#ifdef DEBUG
					bblog(DEBUG, "Word to long into title. pos %i (starts at 100)", hits[i].pos);
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
		bblog(DEBUG, "rankMain: DocID %u, nrofhit %i ,rank %i, havePhrase %i", DocID,nrofhit,rank,havePhrase);
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


//!!!!!!! utestet, fungerer bare kansje

void iindexArrayCopy2(struct iindexFormat **c, int *baselen,int Originallen, struct iindexFormat **a, int alen) {

	//printf("iindexArrayCopy2(baselen=%i, Originallen=%i, alen=%i)\n",*baselen,Originallen,alen);
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

	if (Originallen==0) {
		//hvis vi ikke har hits, og ikke har noe fra før kan vi bare bruke memcpy
		bblog(INFO, "can use swap in iindexArrayCopy2()");
		//flytter bare pekerene
		//struct iindexFormat *tmparr;
		//tmparr = *c;
		//*c = *a;
		//*a = tmparr;
		swapiindex(c, a);

		(*baselen) = alen;
	}
	else {
		while (i<alen && (k < maxIndexElements)){

                	(*c)->iindex[k] = (*a)->iindex[i];

			(*c)->iindex[k].TermAntall = 0;
			(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

			for(x=0;x<(*a)->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				bblog(DEBUG, "aaa %hu", (*a)->iindex[i].hits[x].pos);
				#endif
				(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].pos = (*a)->iindex[i].hits[x].pos;
				(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].phrase = 0;
				++(*c)->iindex[k].TermAntall;
				++(*c)->nrofHits;
			}

			++k; ++i;
			++(*baselen);
		}
	}
}




enum iff_filter_flag {
FILTER_DATE=1,
FILTER_COLLECTION=2,
FILTER_DUPLICATE=4,
FILTER_ATTRIBUTE=8,
FILTER_FILETYPE=16,
FILTER_ALL=31 };

/*
char iff_is_filtered(struct indexFilteredFormat *filter, unsigned int mask)
{
    if ((mask & FILTER_DATE) && filter->date) return 1;
    if ((mask & FILTER_COLLECTION) && filter->subname) return 1;
    if ((mask & FILTER_DUPLICATE) && filter->duplicate) return 1;
    if ((mask & FILTER_FILETYPE) && filter->filename) return 1;
    if ((mask & FILTER_ATTRIBUTE) && filter->attribute) return 1;

    return 0;
}
*/
void iff_clear_all(struct indexFilteredFormat *filter)
{
    int		i;

    filter->date = 0;
    filter->subname = 0;
    filter->duplicate = 0;
    filter->filename = 0;
    filter->attribute = 0;
    filter->is_filtered = 0;
    filter->duplicate_in_collection = 0;
    filter->duplicate_to_show = 0;

    for (i=0; i<MAX_ATTRIBUTES_IN_QUERY; i++)
	filter->attrib[i] = 0;
}

// Dette er filter som filtrerer dokumenter.
// Filter kun for telling (som duplicate_in_collection) håndteres separat.
char iff_set_filter(struct indexFilteredFormat *filter, unsigned int mask)
{
    char	is_filtered = filter->is_filtered;

    if (mask & FILTER_DATE) filter->date = 1;
    if (mask & FILTER_COLLECTION) filter->subname = 1;
    if (mask & FILTER_DUPLICATE) filter->duplicate = 1;
    if (mask & FILTER_FILETYPE) filter->filename = 1;
    if (mask & FILTER_ATTRIBUTE) filter->attribute = 1;

    filter->is_filtered = 1;

    return !is_filtered;
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

            bblog(INFO, "Search type %c",  (*queryParsed).query[i].operand );

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
				bblog(INFO, "Filtering on collection: \"%s\"", (*filteron).collection);
				break;
			    }
			case QUERY_GROUP:
			    {
//				(*filteron).filetype = (*queryParsed).query[i].s[0];
//				strscpy((*filteron).filetype, buf, sizeof((*filteron).collection));
				vector_pushback((*filteron).attributes, QUERY_GROUP, buf);
				bblog(INFO, "Filtering on attributes: \"group:%s\"",  buf);
//				free(buf);
				break;
			    }
			case QUERY_FILETYPE:
			    {
				vector_pushback((*filteron).attributes, QUERY_FILETYPE, buf);
				bblog(INFO, "Filtering on attributes: \"filetype:%s\"",  buf);
//				free(buf);
				break;
			    }
			case QUERY_ATTRIBUTE:
			    {
				vector_pushback((*filteron).attributes, QUERY_ATTRIBUTE, buf);
				bblog(INFO, "Filtering on attributes: \"%s\"",  buf);
//				free(buf);
				break;
			    }
			case QUERY_DATE:
			    {
//				(*filteron).date = (*queryParsed).query[i].s[0];
//				strscpy((*filteron).date, buf, sizeof((*filteron).collection));
				(*filteron).date = strdup(buf);
				bblog(INFO, "Filtering on date: \"%s\"", (*filteron).date);
				break;
			    }
			case QUERY_SORT:
			    {
//				(*filteron).sort = (*queryParsed).query[i].s[0];
//				strscpy((*filteron).sort, buf, sizeof((*filteron).sort));
				(*filteron).sort = strdup(buf);
				bblog(INFO, "wil filter on sort: \"%s\"", (*filteron).sort);
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

        #ifdef DEBUG_TIME
                struct timeval start_time, end_time;
                gettimeofday(&start_time, NULL);
        #endif

	#pragma omp parallel for
	for (y=0; y<TeffArrayElementer; y++) {
		TeffArray->iindex[y].TermRank = rankMain(TeffArray->iindex[y].hits,TeffArray->iindex[y].TermAntall,
			TeffArray->iindex[y].DocID,TeffArray->iindex[y].subname,&TeffArray->iindex[y],complicacy);
	}

        #ifdef DEBUG_TIME
                gettimeofday(&end_time, NULL);
                bblog(DEBUG, "Time debug: rankMainArray() time: %f", getTimeDifference(&start_time, &end_time));
        #endif
}

#ifdef EXPLAIN_RANK
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
#endif

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

            bblog(INFO, "Search type %c",  (*queryParsed).query[i].operand );



		switch ((*queryParsed).query[i].operand) {

			case '+':


					//(*TeffArrayElementer) = 0;

					queryelement[0] = '\0';
		                	//while ( t_it!=NULL )
                			//{
					for (j=0; j<(*queryParsed).query[i].n; j++) {
                    				bblog(INFO, "aa_ søker på \"%s\"",  (*queryParsed).query[i].s[j]);
                    				strncat(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));
			
                				bblog(INFO, "queryelement:  %s",  queryelement);

						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						bblog(INFO, "searchIndex: WordIDcrc32 %u", WordIDcrc32);
						if (i == 0) {
					
							//GetIndexAsArray(TeffArrayElementer,TeffArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);
							//rank((*TeffArrayElementer),TeffArray,subname,(*complicacy));
							//void GetNForTerm(unsigned long WordIDcrc32, char *IndexType, char *IndexSprok, int  *TotaltTreff, char subname[]);
							_GetNForTerm(WordIDcrc32,indexType,"aa",&nterm,subname, cache_index_get);
							nr = nterm;
						}
						else {
							//TmpArrayLen = 0;
							//TmpArray->nrofHits = 0;
							//GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr);

							_GetNForTerm(WordIDcrc32,indexType,"aa",&nterm,subname, cache_index_get);
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


	bblog(INFO, "searchIndex_getnrs: \"%s\": nrs %i", subname->subname,nr);

	bblog(INFO, "searchIndex_getnrs: end");

	return nr;
}
void GetIndexAsArray_thesaurus (int *AntallTeff, struct iindexFormat **TeffArray,
                unsigned int WordIDcrc32, char * IndexType, char *IndexSprok,
                struct subnamesFormat *subname,
                int languageFilterNr, int languageFilterAsNr[],  
    		string_alternative  *alt,
		int alt_n
		 ) {

	int j,k;
	int TmpArrayLen;

#ifndef BLACK_BOKS
	_GetIndexAsArray(AntallTeff,*TeffArray,WordIDcrc32,IndexType,IndexSprok,subname,languageFilterNr,languageFilterAsNr, cache_index_get);
#else

	bblog(INFO, "alt_n %i, alt %p", alt_n, alt);
	struct iindexFormat *TmpArray = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	resultArrayInit(TmpArray);

	struct iindexFormat *tmpAnser = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	resultArrayInit(tmpAnser);

	if (alt == NULL) {
		_GetIndexAsArray(AntallTeff,*TeffArray,WordIDcrc32,IndexType,IndexSprok,subname,languageFilterNr,languageFilterAsNr, cache_index_get);
	}
	else {

		bblog(INFO, "##########################################");
		bblog(INFO, "thesaurus search:");
	
		(*AntallTeff) = 0;

		for (j=0; j<alt_n; j++) {

                        for (k=0; k<alt[j].n; k++)
                        {
				WordIDcrc32 = crc32boitho(alt[j].s[k]);

				TmpArrayLen = 0;
				resultArrayInit(TmpArray);

				_GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,IndexType,IndexSprok,subname,languageFilterNr, languageFilterAsNr, cache_index_get);
				//rank(TmpArrayLen,TmpArray,subname,(*complicacy));

                                bblog(INFO, "%s (%i)",  alt[j].s[k],TmpArrayLen);
				if (TmpArrayLen != 0) {									
					or_merge(&tmpAnser,AntallTeff,TeffArray,(*AntallTeff),&TmpArray,TmpArrayLen);
					iindexArrayCopy2(TeffArray,AntallTeff,0,&tmpAnser,(*AntallTeff));							

				}
                        }
                        
		}	
		bblog(INFO, "##########################################");


	}


	free(TmpArray);
	free(tmpAnser);

#endif

}

void searchIndex (char *indexType, int *TeffArrayElementer, struct iindexFormat **TeffArray,
		query_array *queryParsed,struct subnamesFormat *subname, 
		int languageFilterNr, 
		int languageFilterAsNr[], int *complicacy){

	bblog(INFO, "search: searchIndex()");

	int i, y, j,k,h;
	char queryelement[128];
	unsigned int WordIDcrc32;
	int baseArrayLen;
        int TmpArrayLen;
	int TeffArrayOriginal;
	int newadded;
	struct iindexFormat *TmpArray;
	//TeffArray->nrofHits = 0;
	//TeffArray->phrasenr = 0;

	(*complicacy) = 0;

	#ifdef DEBUG_TIME
	struct timeval start_time, end_time;
	struct timeval start_foo_time, end_foo_time;
	struct timeval start_time_element, end_time_element;
	gettimeofday(&start_time, NULL);
	#endif

	struct iindexFormat *tmpResult = (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	resultArrayInit(tmpResult);
	int tmpResultElementer;

        if ((TmpArray = malloc(sizeof(struct iindexFormat))) == NULL) {
                perror("malloc TmpArray");
                exit(1);
        }
	//Runarb 29 jan 2008: usikker om vi trenger denne
        //resultArrayInit(TmpArray);

	bblog(INFO, "######################################################################");
	bblog(INFO, "searchIndex: vil search index \"%s\"", indexType);
	bblog(INFO, "######################################################################");

	bblog(INFO, "searchIndex \"%s\", subname \"%s\"", indexType,(*subname).subname);
	TeffArrayOriginal = (*TeffArrayElementer);
	bblog(INFO, "searchIndex: got that we have %i elements in array from before", TeffArrayOriginal);

	char	first = 1;

//for (i=0; i<(*queryParsed).size; i++)
for (i=0; i<(*queryParsed).n; i++)
        {
            //struct text_list *t_it = (*queryParsed).elem[i];

        	bblog(INFO, "Search type %c",  (*queryParsed).query[i].operand );
		#ifdef DEBUG_TIME
		gettimeofday(&start_time_element, NULL);
		#endif


		switch ((*queryParsed).query[i].operand) {

			case '+':

					++(*complicacy);

					//(*TeffArrayElementer) = 0;

					queryelement[0] = '\0';

					for (j=0; j<(*queryParsed).query[i].n; j++) {
                    				bblog(INFO, "aa_ søker på \"%s\"",  (*queryParsed).query[i].s[j]);
                    				strncat(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));
                    			                			
                				bblog(INFO, "queryelement:  %s",  queryelement);

						// hvis vi er et kort ord så har vi ikke fått noen ord nummer palssering, så vi skal ikke søke etter det
						if ( isShortWord(queryelement) ) {
							bblog(INFO, "is short word");
							continue;
						}


						//gjør om il liten case
						//for(h=0;h<strlen(queryelement);h++) {
        					//	queryelement[h] = btolower(queryelement[h]);
        					//}
						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						bblog(INFO, "searchIndex: WordIDcrc32 %u", WordIDcrc32);
						//hvis vi ikke har noen elementer i base arrayen, legger vi inn direkte
						//ToDo: kan ikke gjøre det da dette kansje ikke er første element
						//må skille her
						//if (*TeffArrayElementer == 0) {
						if (first) {
							
							TmpArrayLen = (*TeffArrayElementer);
							GetIndexAsArray_thesaurus(TeffArrayElementer,TeffArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, (*queryParsed).query[i].alt, (*queryParsed).query[i].alt_n);
							//rank((*TeffArrayElementer),TeffArray,subname,(*complicacy));
							bblog(INFO, "oooooo: (*TeffArrayElementer) %i,TmpArrayLen %i", (*TeffArrayElementer),TmpArrayLen);

							//rar b-2 bug her. Skal det være + ikke -?
							//Runarb: 19 aug 2007: skaper igjen problemer. ser ut til å skal være '-'
							(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;
							//(*TeffArrayElementer) = (*TeffArrayElementer) + TmpArrayLen;
							
							
						}
						else {
							TmpArrayLen = 0;
							TmpArray->nrofHits = 0;
							GetIndexAsArray_thesaurus(&TmpArrayLen,&TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, (*queryParsed).query[i].alt, (*queryParsed).query[i].alt_n);

							bblog(INFO, "did find %i pages", TmpArrayLen);
																									
							andprox_merge(*TeffArray,&baseArrayLen,TeffArrayOriginal,*TeffArray,(*TeffArrayElementer),TmpArray,TmpArrayLen);
							bblog(INFO, "baseArrayLen %i", baseArrayLen);
							(*TeffArrayElementer) = baseArrayLen;

						}

					}

				first = 0;
				break;


			case '|':
					#ifdef DEBUG
                                        	bblog(DEBUG, "will or search for:");
                                        	for (j=0; j<(*queryParsed).query[i].n; j++) {
                                        	        bblog(DEBUG, "\t%s", (*queryParsed).query[i].s[j]);
                                        	}
					#endif
					for (j=0; j<(*queryParsed).query[i].n; j++) {

						++(*complicacy);

                    				bblog(INFO, "aa_ søker på \"%s\"",  (*queryParsed).query[i].s[j]);
                    				strscpy(queryelement,(*queryParsed).query[i].s[j],sizeof(queryelement));
                    			              
						#ifdef BLACK_BOKS  			
                    			        strsandr(queryelement,"_NBSP_"," ");
						#endif

                				bblog(INFO, "queryelement:  %s",  queryelement);

						// hvis vi er et kort ord så har vi ikke fåt noen ord nummer palssering, så vi skal ikke øke hits
						if ( isShortWord(queryelement) ) {
							bblog(INFO, "is short word");
							continue;
						}


						//gjør om il liten case
						//for(h=0;h<strlen(queryelement);h++) {
        					//	queryelement[h] = btolower(queryelement[h]);
        					//}
						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						bblog(INFO, "searchIndex: WordIDcrc32 %u", WordIDcrc32);
						//hvis vi ikke har noen elementer i base arrayen, legger vi inn direkte
						//ToDo: kan ikke gjøre det da dette kansje ikke er første element
						//må skille her
						//if (*TeffArrayElementer == 0) {
						if (first) {
							TmpArrayLen = (*TeffArrayElementer);
							_GetIndexAsArray(TeffArrayElementer,*TeffArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, cache_index_get);
							//rank((*TeffArrayElementer),TeffArray,subname,(*complicacy));

							(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;
						}
						else {

							#ifdef DEBUG_TIME				
								gettimeofday(&start_foo_time, NULL);
							#endif

							TmpArrayLen = 0;

							_GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, cache_index_get);

							#ifdef DEBUG_TIME
								gettimeofday(&end_foo_time, NULL);
								bblog(DEBUG, "Time debug: searchIndex GetIndexAsArray(subname=%s, word=%s) time: %f", subname, (*queryParsed).query[i].s[j], getTimeDifference(&start_foo_time, &end_foo_time));
							#endif
							

							bblog(INFO, "did find %i pages", TmpArrayLen);
							//hvis vi ikke hadde noe resulateter kan vi droppe å merge denne inn.
							if (TmpArrayLen == 0) {
								continue;
							}

							resultArrayInit(tmpResult);
							tmpResultElementer = 0;
												
							//merger først til en tempurar array, for så å kopiere inn denne.
							int ti;
							#ifdef DEBUG_TIME				
								gettimeofday(&start_foo_time, NULL);
							#endif
							or_merge(&tmpResult,&tmpResultElementer,
									TeffArray,(*TeffArrayElementer),&TmpArray,TmpArrayLen);

							#ifdef DEBUG_TIME
								gettimeofday(&end_foo_time, NULL);
								bblog(DEBUG, "Time debug: searchIndex or_merge() time: %f", getTimeDifference(&start_foo_time, &end_foo_time));
							#endif

							#ifdef DEBUG_TIME				
								gettimeofday(&start_foo_time, NULL);
							#endif

							iindexArrayCopy2(TeffArray,&ti,TeffArrayOriginal,&tmpResult,tmpResultElementer);

							#ifdef DEBUG_TIME
								gettimeofday(&end_foo_time, NULL);
								bblog(DEBUG, "Time debug: searchIndex iindexArrayCopy2() time: %f", getTimeDifference(&start_foo_time, &end_foo_time));
							#endif

							bblog(INFO, "tmpResultElementer %i", tmpResultElementer);
							baseArrayLen = ti;

							bblog(INFO, "baseArrayLen %i", baseArrayLen);
							(*TeffArrayElementer) = baseArrayLen;

						}

					//	t_it = t_it->next;
					}

				first = 0;
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

                    				bblog(INFO, "element %s",  queryelement);

						//gjør om il liten case
						//for(h=0;h<strlen(queryelement);h++) {
        					//	queryelement[h] = btolower(queryelement[h]);
        					//}

						// hvis vi er et kort ord så har vi ikke fåt noen ord nummer palssering, så vi skal ikke øke hits
						if ( isShortWord(queryelement) ) {
							bblog(INFO, "is short word");
							continue;
						}
						//utestet
						//else if (isStoppWord(queryelement)) {
						//	printf("is stopword");
						//	 //frase_stopword(tmpResult,tmpResultElementer);
						//	(*queryParsed).query[i].stopword = 1;
						//	//exit(1);
						//	continue;
						//}
						

						convert_to_lowercase((unsigned char *)queryelement);

						WordIDcrc32 = crc32boitho(queryelement);
						bblog(INFO, "searchIndex: WordIDcrc32 %u", WordIDcrc32);

                    				debug("crc32: %u",WordIDcrc32);
                    				
						//printf("word %s is st %i\n",t_it->text,t_it->stopword);

						//hvsi dette er første element leger vi de inn
						if (j == 0) {
			                                //TmpArrayLen = (*TeffArrayElementer);
							tmpResultElementer = 0;
							//tmpResult->nrofHits = 0;
							resultArrayInit(tmpResult);
							_GetIndexAsArray(&tmpResultElementer,tmpResult,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, cache_index_get);
							//rank(tmpResultElementer,tmpResult,subname,(*complicacy));

                                                        //(*TeffArrayElementer) = (*TeffArrayElementer) - TmpArrayLen;        	        
							//printf("TeffArrayElementer %i", (*TeffArrayElementer));
						}
						else {

							
							
							TmpArrayLen = 0;
							//TmpArray->nrofHits = 0;

							resultArrayInit(TmpArray);
							_GetIndexAsArray(&TmpArrayLen,TmpArray,WordIDcrc32,indexType,"aa",subname,languageFilterNr, languageFilterAsNr, cache_index_get);

							bblog(INFO, "\t dddd: frase_merge %i %i", (*TeffArrayElementer),TmpArrayLen);

							//dette er ikke en ekts frasesøk, kan bare ha en frase
							frase_merge(tmpResult,&tmpResultElementer,0,tmpResult,tmpResultElementer,TmpArray,TmpArrayLen);

							// vi må ha en kopi av det gamle svaret slik at vi har noe og merge med, men vi kan ikke lagre svaret i oss selv. så vi må
							// ha en ny array til det.
							/*
							iindexArrayCopy(tmpResult,tmpAnser,tmpAnserElementer);							

							resultArrayInit(tmpAnser);
							tmpAnserElementer = 0;
							frase_merge(tmpAnser,&tmpAnserElementer,0,tmpResult,tmpResultElementer,TmpArray,TmpArrayLen);
							printf("after frase_merge(): tmpResultElementer: %i", tmpResultElementer);
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
					if (first) {
						bblog(INFO, "er første fraseelement");
						
						k=TeffArrayOriginal;
						//iindexArrayCopy(TeffArray,tmpResult,tmpResultElementer);		
						//iindexArrayCopy(TeffArray,tmpAnser,tmpAnserElementer);
						int ti;
						iindexArrayCopy2(TeffArray,&ti,TeffArrayOriginal,&tmpResult,tmpResultElementer);
						bblog(INFO, "tmpResultElementer %i", tmpResultElementer);
						(*TeffArrayElementer) = tmpResultElementer;
					}
					else {
						and_merge(*TeffArray,&baseArrayLen,TeffArrayOriginal,&newadded,*TeffArray,(*TeffArrayElementer),tmpResult,tmpResultElementer);
						//and_merge(TeffArray,&baseArrayLen,TeffArrayOriginal,&newadded,TeffArray,(*TeffArrayElementer),tmpAnser,tmpAnserElementer);
						(*TeffArrayElementer) = baseArrayLen;
					}


				first = 0;
                                break;

		}

		#ifdef DEBUG_TIME
		gettimeofday(&end_time_element, NULL);
		bblog(DEBUG, "Time debug: searchIndex element time: %f", getTimeDifference(&start_time_element,&end_time_element));
		#endif

 
        }

	bblog(INFO, "~searchIndex: (*TeffArrayElementer) %i, TeffArrayOriginal %i for subname \"%s\"", (*TeffArrayElementer),TeffArrayOriginal,subname->subname);
//toDo: trenger vi denne nå???
//tror ikke vi trenger denne mere, da vi har merget queryet inn i den
	(*TeffArrayElementer) = (*TeffArrayElementer) + TeffArrayOriginal;
	TeffArrayOriginal = (*TeffArrayElementer);
	bblog(INFO, "new len is %i", (*TeffArrayElementer));

	free(tmpResult);
	free(TmpArray);

	#ifdef DEBUG_TIME
		gettimeofday(&end_time, NULL);
		bblog(DEBUG, "Time debug: ~searchIndex: Type: \"%s\", time: %f", indexType,getTimeDifference(&start_time,&end_time));
	#endif

	bblog(INFO, "searchIndex: end");
	bblog(INFO, "######################################################################");

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
	int anonymous;
};

void *searchIndex_thread(void *arg)
{
	bblog(INFO, "search: searchIndex_thread()");

        struct searchIndex_thread_argFormat *searchIndex_thread_arg = (struct searchIndex_thread_argFormat *)arg;
	int i,j,y,x;

	int ArrayLen, TmpArrayLen, TmpArray2Len, totAttribLength;

	struct iindexFormat *TmpArray;
	struct iindexFormat *Array;
	void (*rank)(int TeffArrayElementer, struct iindexFormat *TeffArray, int complicacy);
	int complicacy;

	struct timeval start_time, end_time;

	gettimeofday(&start_time, NULL);

	bblog(INFO, "######################################################################");

	if ((TmpArray = malloc(sizeof(struct iindexFormat))) == NULL) {
                perror("malloc TmpArray");
                exit(1);
        }
	resultArrayInit(TmpArray);


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
		bblog(ERROR, "search: Error! Unknown index type \"%s\"",(*searchIndex_thread_arg).indexType);
		bblog(ERROR, "search: ~searchIndex_thread()");
		return;
	}
	#ifdef WITH_THREAD
	pthread_t tid;
        tid = pthread_self();
        bblog(INFO, "is thread id %u. Wil search \"%s\"", (unsigned int)tid,(*searchIndex_thread_arg).indexType);
	#endif

#ifdef BLACK_BOKS
	int cmc_sock;
	char cmc_statusbuf[1024];
	cmc_conect(&cmc_sock, cmc_statusbuf, sizeof(cmc_statusbuf), searchIndex_thread_arg->cmc_port);
	struct hashtable *groupqueries = create_hashtable(3, ht_integerhash, ht_integercmp);
#endif

	ArrayLen = 0;
	
	bblog(INFO, "nrOfSubnames %i Search user: %s", (*searchIndex_thread_arg).nrOfSubnames, searchIndex_thread_arg->search_user);

	for(i=0;i<(*searchIndex_thread_arg).nrOfSubnames;i++) {
		query_array *groupquery;
		struct timeval starttime, endtime;

		#ifdef DEBUG_TIME
		struct timeval subname_starttime, subname_endtime;
		gettimeofday(&subname_starttime, NULL);
		#endif

		bblog(INFO, "Checking subname: %s",  searchIndex_thread_arg->subnames[i].subname);

		#if defined BLACK_BOKS && !defined _24SEVENOFFICE
	
		#ifdef IIACL

		if (!searchIndex_thread_arg->anonymous) {
			int system = cmc_usersystemfromcollection(cmc_sock, searchIndex_thread_arg->subnames[i].subname);
			if (system == -1) {
				bblog(ERROR, "Unable to get usersystem for: %s", searchIndex_thread_arg->subnames[i].subname);
				continue;
			} else if ((groupquery = hashtable_search(groupqueries, &system)) == NULL) {
				char **groups;
				int n_groups, j;

				bblog(INFO, "Getting mappings for system %d",  system);

#ifdef DEBUG_TIME
				gettimeofday(&starttime, NULL);
#endif

				n_groups = cmc_groupsforuserfromusersystem(cmc_sock, searchIndex_thread_arg->search_user,
						system, &groups, "");

#ifdef DEBUG_TIME
				gettimeofday(&endtime, NULL);
				bblog(DEBUG, "Time debug: cmc_groupsforuserfromusersystem(): %f",  getTimeDifference(&starttime, &endtime));
#endif

				size_t grouplistsize = n_groups * (MAX_LDAP_ATTR_LEN+5);
				bblog(INFO, "n_groups: %d",  n_groups);
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
				bblog(INFO, "grouplist: %s",  grouplist);
				groupquery = malloc(sizeof(*groupquery));
				get_query(grouplist, strlen(grouplist), groupquery);
				if (n_groups > 0)
					free(groups);
				free(grouplist);
				hashtable_insert(groupqueries, uinttouintp(system), groupquery);
			} else {
				bblog(INFO, "Reusing system mapping: %d",  system);
			}
		}



			searcArrayLen = 0;

			//acl_allow sjekk
			acl_allowArrayLen = 0;
			acl_deniedArrayLen = 0;

			totAttribLength = 0;

			hits = ArrayLen;

//   			omp_set_dynamic(0);
//   			omp_set_num_threads(2);

			#pragma omp parallel for
			for(y=0;y<2;y++) {

				#if defined (_OPENMP)
      				bblog(INFO, "Hello World from thread %d, runing %d",  omp_get_thread_num(),y);
				#endif
				if (y==0) {

					#ifdef ATTRIBUTES
					for (j=0; j<(*searchIndex_thread_arg).attrib_count; j++)
					    {
						tmpAttribArrayLen[j] = 0;

						// Skriv ut hvilke attributter det søkes på til skjermen:
						char	qbuf[1024];
						sprint_query(qbuf,1023,&(*searchIndex_thread_arg).attribute_query[j]);
						bblog(INFO, "Looking up attributes: %s",  qbuf);

						    searchIndex("attributes",
							&tmpAttribArrayLen[j],
							&tmpAttribArray[j],
							&(*searchIndex_thread_arg).attribute_query[j],
							&(*searchIndex_thread_arg).subnames[i],
							(*searchIndex_thread_arg).languageFilterNr, 
							(*searchIndex_thread_arg).languageFilterAsNr,
							&complicacy
						    );
							totAttribLength += tmpAttribArrayLen[j];
				

						bblog(INFO, "%i hits.",  tmpAttribArrayLen[j]);
					    }
					#endif
				}
				else if (y==1) {
					searchIndex((*searchIndex_thread_arg).indexType,
						&searcArrayLen,
						&searcArray,
						(*searchIndex_thread_arg).queryParsed,
						&(*searchIndex_thread_arg).subnames[i],
						(*searchIndex_thread_arg).languageFilterNr, 
						(*searchIndex_thread_arg).languageFilterAsNr,
						&complicacy
					);

				}
			} //omp for

			if ((searcArrayLen == 0) && (totAttribLength == 0)) {
				bblog(INFO, "diden't find any hits for this subname, skipping it.");
				continue;
			}

			if (!searchIndex_thread_arg->anonymous) {
				#pragma omp parallel for
				for(y=0;y<2;y++) {
		
					if (y==0) {
						searchIndex("acl_allow",
							&acl_allowArrayLen,
							&acl_allowArray,
							groupquery,
							&(*searchIndex_thread_arg).subnames[i],
							(*searchIndex_thread_arg).languageFilterNr, 
							(*searchIndex_thread_arg).languageFilterAsNr,
							&complicacy
						);
					} else if (y==1) {
						searchIndex("acl_denied",
							&acl_deniedArrayLen,
							&acl_deniedArray,
							groupquery,
							&(*searchIndex_thread_arg).subnames[i],
							(*searchIndex_thread_arg).languageFilterNr, 
							(*searchIndex_thread_arg).languageFilterAsNr,
							&complicacy
						);
					}
				} // omp for
			}
				

			#ifdef DEBUG_II
			if (!searchIndex_thread_arg->anonymous) {
				bblog(DEBUG, "acl_allowArrayLen %i:", acl_allowArrayLen);
				for (y = 0; y < acl_allowArrayLen; y++) {
					bblog(DEBUG, "acl_allow TeffArray: DocID %u", acl_allowArray->iindex[y].DocID);			
				}

				bblog(DEBUG, "acl_deniedArrayLen %i:", acl_deniedArrayLen);
				for (y = 0; y < acl_deniedArrayLen; y++) {
					bblog(DEBUG, "acl_denied TeffArray: DocID %u", acl_deniedArray->iindex[y].DocID);			
				}
			}

				bblog(DEBUG, "searcArrayLen %i:", searcArrayLen);
				for (y = 0; y < searcArrayLen; y++) {
					bblog(DEBUG, "Main TeffArray: DocID %u Hits: %i", searcArray->iindex[y].DocID,searcArray->iindex[y].TermAntall);			
					for (x=0;x<searcArray->iindex[y].TermAntall;x++) {
						bblog(DEBUG, "\t%hu", searcArray->iindex[y].hits[x]);
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
					}

				if (null_query) empty_search_query = 1;
			    }

			int	start = ArrayLen;

			if (!empty_search_query)
			    {
				if (!searchIndex_thread_arg->anonymous) {
					and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,searcArray,searcArrayLen);
					// Merge acl_denied:			
					andNot_merge(&Array,&ArrayLen,&hits,&TmpArray,TmpArrayLen,&acl_deniedArray,acl_deniedArrayLen);
				} else {
					swapiindex(&Array, &searcArray);
					ArrayLen = searcArrayLen;
				}
			    }
			else
			    {
				if (!searchIndex_thread_arg->anonymous) {
					// Merge med attributter istedet:
					and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,tmpAttribArray[0],tmpAttribArrayLen[0]);
					// Merge acl_denied:			
					andNot_merge(&Array,&ArrayLen,&hits,&TmpArray,TmpArrayLen,&acl_deniedArray,acl_deniedArrayLen);
				} else {
					swapiindex(&Array, &searcArray);
					ArrayLen = searcArrayLen;
				}
			//(*searchIndex_thread_arg).attribArrayLen[0] = 0;
			    }

			// Ettersom merging mÃ¥ gjÃ¸res per collection, gjÃ¸r vi attributt-filtreringa allerede her:
			for (x=start; x<ArrayLen; x++)
			    {
				iff_clear_all(&Array->iindex[x].indexFiltered);
				//Array->iindex[x].indexFiltered.is_filtered = 0;
				//Array->iindex[x].indexFiltered.attribute = 0;
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
						//Array->iindex[x].indexFiltered.attrib[j] = 0;
						//vinn++;
					    }
					else
					    {
						iff_set_filter(&Array->iindex[x].indexFiltered, FILTER_ATTRIBUTE);
					        Array->iindex[x].indexFiltered.attrib[j] = 1;
						//forsvinn++;
					    }
				}
				//int	nhits;
				//and_merge((*searchIndex_thread_arg).attribArray[j], &(*searchIndex_thread_arg).attribArrayLen[j],
				//    0, &nhits, tmpAttribArry[j], tmpAttribArrayLen[j], Array, ArrayLen);
			    }

			    #ifdef DEBUG_II
			        bblog(DEBUG, "Array:");
			        for (x=start; x<ArrayLen; x++)
			            {
			 	        bblog(DEBUG, "\tDocID: %u,\t Filtered %d", Array->iindex[x].DocID,Array->iindex[x].indexFiltered.is_filtered);
			            }
			    #endif

			#else // ATTRIBUTES
				if (!searchIndex_thread_arg->anonymous) {
					and_merge(TmpArray,&TmpArrayLen,0,&hits,acl_allowArray,acl_allowArrayLen,searcArray,searcArrayLen);
					// Merge acl_denied:			
					andNot_merge(&Array,&ArrayLen,&hits,&TmpArray,TmpArrayLen,&acl_deniedArray,acl_deniedArrayLen);
				} else {
					swapiindex(&Array, &searcArray);
					ArrayLen = searcArrayLen;
				}
			#endif


			#ifdef DEBUG_II
			bblog(DEBUG, "after first merge:");
			for (y = 0; y < ArrayLen; y++) {
				bblog(DEBUG, "TeffArray: DocID %u Hits (%i): ", Array->iindex[y].DocID,Array->iindex[y].TermAntall);	
				for (x=0;x<Array->iindex[y].TermAntall;x++) {
					bblog(DEBUG, "\t%hu", Array->iindex[y].hits[x]);
				}		
			}
			#endif

			#ifdef DEBUG_II
			bblog(DEBUG, "etter andNot_merge:");
			for (y = 0; y < ArrayLen; y++) {
				bblog(DEBUG, "TeffArray: DocID %u, filtered %d Hits (%i): ", Array->iindex[y].DocID,Array->iindex[y].indexFiltered.is_filtered,Array->iindex[y].TermAntall);
				for (x=0;x<Array->iindex[y].TermAntall;x++) {
					bblog(DEBUG, "\t%hu", Array->iindex[y].hits[x]);
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
				&(*searchIndex_thread_arg).subnames[i],
				(*searchIndex_thread_arg).languageFilterNr, 
				(*searchIndex_thread_arg).languageFilterAsNr,
				&complicacy
			);

			hits = ArrayLen - hits;


		#endif


		(*searchIndex_thread_arg).subnames[i].hits += hits;
		bblog(INFO, "searchIndex_thread: index %s, subname \"%s\",hits %i", (*searchIndex_thread_arg).indexType,(*searchIndex_thread_arg).subnames[i].subname,hits);


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
					bblog(DEBUG, "searchIndex_thread: TeffArray: subname \"%s\", DocID %u Hits (%i): ", 
						Array->iindex[y].subname->subname,Array->iindex[y].DocID,Array->iindex[y].TermAntall);
				}
			#endif

			hits = ArrayLen - hits;
			(*searchIndex_thread_arg).subnames[i].hits += hits;

			bblog(INFO, "searchIndex_thread: ArrayLen %i", ArrayLen);
			bblog(INFO, "searchIndex_thread: index %s, subname \"%s\",hits %i", (*searchIndex_thread_arg).indexType,(*searchIndex_thread_arg).subnames[i].subname,hits);


		#endif


		#ifdef DEBUG_TIME
			gettimeofday(&subname_endtime, NULL);
			bblog(DEBUG, "Time debug: all of subname: %f",  getTimeDifference(&subname_starttime, &subname_endtime));
		#endif
		
	}
#ifdef BLACK_BOKS
	cmc_close(cmc_sock);
	{
		struct hashtable_itr *itr;

		if (hashtable_count(groupqueries) > 0) {
			itr = hashtable_iterator(groupqueries);
			do {
				destroy_query(hashtable_iterator_value(itr));
			} while (hashtable_iterator_advance(itr) != 0);
			free(itr);
		}
	}
	hashtable_destroy(groupqueries, 1);
#endif

	free(TmpArray);

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
	#ifdef EXPLAIN_RANK
		explain_max_rankArray(ArrayLen,Array,complicacy);
	#endif

	(*searchIndex_thread_arg).resultArrayLen = ArrayLen;
	(*searchIndex_thread_arg).resultArray = Array;

	gettimeofday(&end_time, NULL);
	(*searchIndex_thread_arg).searchtime = getTimeDifference(&start_time,&end_time);
	bblog(INFO, "searchtime %f", (*searchIndex_thread_arg).searchtime);

	bblog(INFO, "######################################################################");

	bblog(INFO, "search: ~searchIndex_thread()");
}


void searchSimple (int *TeffArrayElementer, struct iindexFormat **TeffArray,int *TotaltTreff, 
		query_array *queryParsed, struct queryTimeFormat *queryTime, 
		struct subnamesFormat subnames[], int nrOfSubnames,int languageFilterNr, 
		int languageFilterAsNr[], char orderby[],
		struct filtersFormat *filters,
		struct filteronFormat *filteron,
		query_array *search_user_as_query,
		int ranking, struct hashtable **crc32maphash, struct duplicate_docids **dups,
		char *search_user, int cmc_port, int anonymous
		) {

	bblog(INFO, "search: searchSimple()");

	int i,j,y,n;
	//int x=0,j=0,k=0;

	unsigned char PopRank;
	int responseShortTo;
	struct reformat *re;

	int rankcount[256]; // rank går fra 0-252 (unsigned char)
	int tmpint;

	struct timeval start_time, end_time;
		
	
	struct iindexFormat *TmpArray; 

	//char *WordID;
	struct searchIndex_thread_argFormat searchIndex_thread_arg_Athor;
	struct searchIndex_thread_argFormat searchIndex_thread_arg_Url;
	struct searchIndex_thread_argFormat searchIndex_thread_arg_Main;
	
     
        int baseArrayLen,MainArrayHits;
	int TmpArrayLen;	

	unsigned int PredictNrAthor;
	unsigned int PredictNrUrl;
	unsigned int PredictNrMain;

	PredictNrAthor	= 0;
	PredictNrUrl	= 0;
	PredictNrMain	= 0;
//	PredictNrattributes = 0;

	#ifdef WITH_THREAD
		pthread_t threadid_Athor = 0;
		pthread_t threadid_Url = 0;
		pthread_t threadid_Main = 0;
	#endif

	//resetter subnmes hits
	for(i=0;i<nrOfSubnames;i++) {		
		subnames[i].hits = 0;
	}

	#ifdef BLACK_BOKS


	#else
		//finner først ca hvor mange treff vi fil få. Dette brukes for å avgjøre om vi kan 
		//klare oss med å søke i bare url og athor, eller om vi må søke i alt

		//Athor:
		for(i=0;i<nrOfSubnames;i++) {		
			PredictNrAthor += searchIndex_getnrs("Athor",queryParsed,&subnames[i],languageFilterNr, languageFilterAsNr);
		}

		//Url:
		for(i=0;i<nrOfSubnames;i++) {		
			PredictNrUrl += searchIndex_getnrs("Url",queryParsed,&subnames[i],languageFilterNr, languageFilterAsNr);
		}

		//Main:
		for(i=0;i<nrOfSubnames;i++) {		
			PredictNrMain += searchIndex_getnrs("Main",queryParsed,&subnames[i],languageFilterNr, languageFilterAsNr);
			bblog(INFO, "PredictNrMain total with \"%s\" %u", &subnames[i],PredictNrMain);
		}

		bblog(INFO, "PredictNrAthor %u, PredictNrUrl %u, PredictNrMain %u", PredictNrAthor,PredictNrUrl,PredictNrMain);
	#endif

	#ifdef BLACK_BOKS
		//filter
		searchIndex_filters(queryParsed, filteron);	//@ax
	#endif



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

	searchIndex_thread_arg_Athor.anonymous = anonymous;
	searchIndex_thread_arg_Url.anonymous = anonymous;
	searchIndex_thread_arg_Main.anonymous = anonymous;

	
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

		#ifdef WITH_THREAD
			n = pthread_create(&threadid_Main, NULL, searchIndex_thread, &searchIndex_thread_arg_Main);
		#else
			searchIndex_thread(&searchIndex_thread_arg_Main);
		#endif
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
	#ifndef BLACK_BOKS
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

	bblog(INFO, "Athor ArrayLen %i, Url ArrayLen %i, Main ArrayLen %i", searchIndex_thread_arg_Athor.resultArrayLen,
			searchIndex_thread_arg_Url.resultArrayLen,searchIndex_thread_arg_Main.resultArrayLen);

	//sanker inn tiden
	(*queryTime).AthorSearch = searchIndex_thread_arg_Athor.searchtime;
	(*queryTime).UrlSearch = searchIndex_thread_arg_Url.searchtime;
	(*queryTime).MainSearch = searchIndex_thread_arg_Main.searchtime;

	gettimeofday(&start_time, NULL);

	TmpArray 	= (struct iindexFormat *)malloc(sizeof(struct iindexFormat));
	TmpArrayLen = 0;
	resultArrayInit(TmpArray);

	#ifndef BLACK_BOKS
		//or_merger Athor og Url inn i en temper array
		or_merge(&TmpArray,&TmpArrayLen,&searchIndex_thread_arg_Athor.resultArray,searchIndex_thread_arg_Athor.resultArrayLen,
			&searchIndex_thread_arg_Url.resultArray,searchIndex_thread_arg_Url.resultArrayLen);



		if (searchIndex_thread_arg_Athor.resultArray != NULL) {
			free(searchIndex_thread_arg_Athor.resultArray);
		}

		if (searchIndex_thread_arg_Url.resultArray != NULL) {
			free(searchIndex_thread_arg_Url.resultArray);
		}
	#endif

	//merger inn temperer og main 

	#ifdef BLACK_BOKS

	    #ifdef ATTRIBUTES
		for (i=0; i<searchIndex_thread_arg_Main.attrib_count; i++)
		    destroy_query( &searchIndex_thread_arg_Main.attribute_query[i] );
	    #endif

	// Bytter om på disse arrayene da kunden nedenfor TeffArray.
	*TeffArrayElementer = searchIndex_thread_arg_Main.resultArrayLen;
	swapiindex(TeffArray,&searchIndex_thread_arg_Main.resultArray);


	if (searchIndex_thread_arg_Main.resultArray != NULL) {
		free(searchIndex_thread_arg_Main.resultArray);
	}
	#else
	or_merge((*TeffArray),TeffArrayElementer,TmpArray,TmpArrayLen,
		searchIndex_thread_arg_Main.resultArray,searchIndex_thread_arg_Main.resultArrayLen);

	if (searchIndex_thread_arg_Main.resultArray != NULL) {
		free(searchIndex_thread_arg_Main.resultArray);
	}
	#endif

	free(TmpArray);

        gettimeofday(&end_time, NULL);
        (*queryTime).MainAthorMerge = getTimeDifference(&start_time,&end_time);

	#ifdef DEBUG_II
		//debug: printer ut alle treff, og litt om de.
		bblog(DEBUG, "hits befoe filters:");
		bblog(DEBUG, "\t| %-5s | %-20s |", "DocID", "Subname");
		for (i = 0; (i < (*TeffArrayElementer)) && (i < 100); i++) {
			bblog(DEBUG, "\t| %-5u | %-20s |", 
				(*TeffArray)->iindex[i].DocID,
				(*(*TeffArray)->iindex[i].subname).subname
			);
		}
	#endif


	gettimeofday(&start_time, NULL);

	#if 0
	//runarb: bug: denne manglet i en cvs oppdatering. Skal nokk også bare være for websøk (ikke bb)
	//y=0;
       	for (i = 0; i < (*TeffArrayElementer); i++) {
       	        //PopRank = popRankForDocIDMemArray((*TeffArray)[i].DocID);
		//neste linje må fjeres hvis vi skal ha forkorting
		//(*TeffArray)[i].PopRank = PopRank;
		(*TeffArray)->iindex[i].PopRank = popRankForDocIDMemArray((*TeffArray)->iindex[i].DocID);

		//her kan vi ha forkortning av array
		//if (PopRank > 0) {
               	//	(*TeffArray)[y] = (*TeffArray)[i];
                //        (*TeffArray)[y].PopRank = PopRank;
                //	++y;
		//}
	}
	// 
	#endif
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


			++rankcount[(*TeffArray)->iindex[i].PopRank];
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
				bblog(DEBUG, "rank %i: %i", responseShortTo,(int)rankcount[responseShortTo]);
			}
			#endif

		} while((responseShortTo>0) && (y <= responseShortToMin));
		
		bblog(INFO, "responseShortTo: %i, y: %i", responseShortTo,y);
		
		if (responseShortTo != 0) {
			//fjerner sider som er lavere en responseShortTo 
			y=0;
       			for (i = 0; i < (*TeffArrayElementer); i++) {
			
				if ((*TeffArray)->iindex[i].PopRank >= responseShortTo) {
        		       		(*TeffArray)->iindex[y] = (*TeffArray)->iindex[i];
        		        	++y;
				}
			}
		}
		bblog(INFO, "shortet respons array from %i to %i.", (*TeffArrayElementer),y);
		(*TeffArrayElementer) = y;

		
	}
	
	#endif



        gettimeofday(&end_time, NULL);
        (*queryTime).responseShortning = getTimeDifference(&start_time,&end_time);





	
	#ifdef BLACK_BOKS

		// gjør ting med TeffArray før vi roter for mye med den.
		for (i=0; i<*TeffArrayElementer; i++)
		    {
			// lagrer den orgianla posisjoene. Trenger denne for å gjøre en stabil sortering siden.
			(*TeffArray)->iindex[i].originalPosition = i;


			// For filtrert i search_thread_ting:
			if ((*TeffArray)->iindex[i].indexFiltered.is_filtered)
			    --(*TotaltTreff);
		    }

		#ifdef ATTRIBUTES
		(*TeffArray)->attrib_count = searchIndex_thread_arg_Main.attrib_count;
		#endif


		/*
		*********************************************************************************************************************
		collection
		*********************************************************************************************************************
		*/

		if ((*filteron).collection != NULL) {
		

			bblog(INFO, "will filter on collection \"%s\"", (*filteron).collection);

			for (i = 0; i < (*TeffArrayElementer); i++) {

				if (strcmp((*(*TeffArray)->iindex[i].subname).subname,(*filteron).collection) != 0) {
					if (iff_set_filter(&(*TeffArray)->iindex[i].indexFiltered, FILTER_COLLECTION))
					    --(*TotaltTreff);
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

		bblog(INFO, "<################################# file filter ######################################>");

		gettimeofday(&start_time, NULL);


		//char filetype[5];
		//struct filesKeyFormat *filesKey;

		re = NULL;
		for (i = 0; i < (*TeffArrayElementer); i++) {
			//printf("$%%$$%%$$$$$$$$$$$$$$ Eirik: %d\n", (*TeffArray)->iindex[i].DocID);
			#ifdef DEBUG
			bblog(DEBUG, "i = %i, subname \"%s\"", i,(*(*TeffArray)->iindex[i].subname).subname);
			#endif
			#if 1
 
				if (reIsOpen(re,rLotForDOCid((*TeffArray)->iindex[i].DocID), (*TeffArray)->iindex[i].subname->subname, "filtypes") ) {

				}
				else if ((re = reopen_cache(rLotForDOCid((*TeffArray)->iindex[i].DocID), 4, "filtypes", (*TeffArray)->iindex[i].subname->subname, RE_READ_ONLY|RE_STARTS_AT_0)) == NULL) {
					bblog(DEBUG, "reopen(filtypes)");
					(*TeffArray)->iindex[i].filetype[0] = '\0';

					continue;
				}

				strncpy((*TeffArray)->iindex[i].filetype, RE_Char(re, (*TeffArray)->iindex[i].DocID),4);
			#else 
				if (iintegerGetValue(&(*TeffArray)->iindex[i].filetype,4,(*TeffArray)->iindex[i].DocID,"filtypes",(*(*TeffArray)->iindex[i].subname).subname) == 0) {
					bblog(WARN, "woldent get integerindex");
					(*TeffArray)->iindex[i].filetype[0] = '\0';
				}
				else {
					#ifdef DEBUG
					bblog(DEBUG, "file typs is: \"%c%c%c%c\"", (*TeffArray)->iindex[i].filetype[0],(*TeffArray)->iindex[i].filetype[1],(*TeffArray)->iindex[i].filetype[2],(*TeffArray)->iindex[i].filetype[3]);
					#endif								

				}
			#endif

			// filetype kan være på opptil 4 bokstaver. Hvis det er ferre en 4 så vil 
			// det være \0 er paddet på slutten, men hvis det er 4 så er det ikke det.
			// legger derfor til \0 som 5 char, slik at vi har en gyldig string
			(*TeffArray)->iindex[i].filetype[4] = '\0';
			

		}



		gettimeofday(&end_time, NULL);
		(*queryTime).filetypes = getTimeDifference(&start_time,&end_time);
		bblog(INFO, "<################################# /file filter ######################################>");


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
							if (fte_belongs_to_group(fdata, "eng", (*TeffArray)->iindex[i].filetype, attrib))
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
								if (!strcmp((*TeffArray)->iindex[i].filetype, *ptr_i))
								    {
									D[i]++;
									break;
								    }
							    }
						    }
						else
						    {
							//if (!fte_getext_from_ext(fdata, (*TeffArray)->iindex[i].filetype, &ptr1, &ptr2))
							if (!strcasecmp(attrib, (*TeffArray)->iindex[i].filetype))
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
						if (iff_set_filter(&(*TeffArray)->iindex[i].indexFiltered, FILTER_FILETYPE))
						    --(*TotaltTreff);
                                            }
                                    }
                            }

                        free(D);
                        if (fdata!=NULL) fte_destroy(fdata);
                    }


                #endif  // ATTRIBUTES


		/*
		*********************************************************************************************************************
		datoer

		lager en oversikt over, og filtrerer på, dato
		*********************************************************************************************************************
		*/
		bblog(INFO, "<################################# date filter ######################################>");
		gettimeofday(&start_time, NULL);


		bblog(INFO, "looking opp dates");

		//slår opp alle datoene
		re = NULL;
		int nreopen = 0;
		#pragma omp parallel for firstprivate(re)
		for (i = 0; i < *TeffArrayElementer; i++) {

				if (!reIsOpen(re,rLotForDOCid((*TeffArray)->iindex[i].DocID), (*TeffArray)->iindex[i].subname->subname, "dates") ) {

					#pragma omp critical
					{
						++nreopen;
						re = reopen_cache( rLotForDOCid((*TeffArray)->iindex[i].DocID), sizeof(int), "dates", (*TeffArray)->iindex[i].subname->subname, RE_READ_ONLY|RE_STARTS_AT_0);
					}

					if (re == NULL) {
						debug("reopen(dates)\n");
						continue;
					}

				}
				(*TeffArray)->iindex[i].date = *RE_Int(re, (*TeffArray)->iindex[i].DocID);


			#ifdef DEBUG
			bblog(DEBUG, "got %u", (*TeffArray)->iindex[i].date);
			#endif
		}

		bblog(INFO, "We did reopen %i times", nreopen);
		bblog(INFO, "looking opp dates end");

		gettimeofday(&end_time, NULL);
		(*queryTime).iintegerGetValueDate = getTimeDifference(&start_time,&end_time);





		//
		//filter på dato

		if ((*filteron).date != NULL) {
			bblog(DEBUG, "wil filter on date \"%s\"", (*filteron).date);

			struct datelib dl;

			dl.start = dl.end = 0;
			sd_getdate((*filteron).date, &dl);

			bblog(INFO, "start %u, end %u", dl.start,dl.end);
			bblog(INFO, "start: %s",ctime(&dl.start));
			bblog(INFO, "end:   %s",ctime(&dl.end));

			int notFiltered, filtered;

			notFiltered = 0;
			filtered = 0;

			for (i = 0; i < *TeffArrayElementer; i++) {

				
				if (((*TeffArray)->iindex[i].date >= dl.start) && ((*TeffArray)->iindex[i].date <= dl.end)) {
					#ifdef DEBUG
					bblog(DEBUG, "time hit %s",ctime(&(*TeffArray)->iindex[i].date));
					#endif
					if ((*TeffArray)->iindex[i].indexFiltered.is_filtered) {
						#ifdef DEBUG
							bblog(DEBUG, "is already filtered out ");
						#endif				
					}
					else {
						++notFiltered;
					}
				}
				else {
					#ifdef DEBUG
					bblog(DEBUG, "not time hit %s",ctime(&(*TeffArray)->iindex[i].date));
					#endif

					if (iff_set_filter(&(*TeffArray)->iindex[i].indexFiltered, FILTER_DATE))
					    {
						--(*TotaltTreff);
						++filtered;
					    }
				}

			}		

			bblog(INFO, "date filter: filtered %i, notFiltered %i", filtered,notFiltered);
			
			

		}

		bblog(INFO, "</################################# date filter ######################################>");


		/*
		*********************************************************************************************************************

		duplicate checking
		*********************************************************************************************************************
		*/

		bblog(INFO, "<################################# duplicate checking ######################################>");
		gettimeofday(&start_time, NULL);
		int k;
		// Loop over all results and do duplicate checking...

		if (( crc32maphash != NULL) && (dups != NULL) ) {
			*crc32maphash = create_hashtable((*TeffArrayElementer), ht_integerhash, ht_integercmp);		
			*dups = malloc( (sizeof(struct duplicate_docids) * (*TeffArrayElementer)) );

			unsigned int crc32;
			struct duplicate_docids *dup;

	       		for (i = 0; i < (*TeffArrayElementer); i++) {
				//runarb 22 sept 2008: hvorfor er denne her? Brukes bare på websøk
				//TeffArray->iindex[i].PopRank = popRankForDocIDMemArray(TeffArray->iindex[i].DocID);

				if (reIsOpen(re,rLotForDOCid((*TeffArray)->iindex[i].DocID), (*TeffArray)->iindex[i].subname->subname, "crc32map") ) {

				}
				else if ((re = reopen_cache(rLotForDOCid((*TeffArray)->iindex[i].DocID), sizeof(unsigned int), "crc32map", (*TeffArray)->iindex[i].subname->subname, RE_READ_ONLY)) == NULL) {
					debug("reopen(crc32map)\n");
					continue;
				}

				crc32 = *RE_Uint(re, (*TeffArray)->iindex[i].DocID);

				

				#ifdef DEBUG
				bblog(DEBUG, "Got hash value: %x",  crc32);
				#endif

				if (crc32 == 0) {
					bblog(WARN, "don't have crc32 value for DocID");
					continue;
				}

				dup = hashtable_search(*crc32maphash, &crc32);
				(*TeffArray)->iindex[i].crc32 = crc32;

				if (dup == NULL) {
					//dup = malloc(sizeof(struct duplicate_docids));
					dup = &(*dups)[i];
					dup->V = NULL;
					//setter dublicat rekorden til å peke på den første 
					dup->fistiindex = &(*TeffArray)->iindex[i];

					hashtable_insert(*crc32maphash, uinttouintp(crc32), dup);


				} else {

					/* Remove duplicated */
					if (dup->V == NULL) {
						dup->V = vector_container( pair_container( int_container(), ptr_container() ) );
						//legger til den første
						#ifdef DEBUG
						bblog(DEBUG, "DUPLICATE first: DocID=%u, subname=%s",  dup->fistiindex->DocID, dup->fistiindex->subname->subname);
						#endif
						vector_pushback(dup->V, dup->fistiindex->DocID, dup->fistiindex->subname->subname);
						dup->fistiindex->indexFiltered.duplicate_to_show = 1;
					}

					//går gjenom de vi har fra før, og sjekker om dette er den første duplikaten i en kollection
					//hvis det ikke er det skal vi markere det.
					for (k = 0; k<vector_size(dup->V); k++) { 

						if (pair(vector_get(dup->V,k)).second.ptr == (*TeffArray)->iindex[i].subname->subname) {

							(*TeffArray)->iindex[i].indexFiltered.duplicate_in_collection = 1;
							break;
						}
					}

					// Hvis den første allerede er filtrert ut:
					if (dup->fistiindex->indexFiltered.is_filtered && !(*TeffArray)->iindex[i].indexFiltered.is_filtered)
					    {
						if (iff_set_filter(&dup->fistiindex->indexFiltered, FILTER_DUPLICATE)) {
						    --(*TotaltTreff);	// Vil ikke denne alltid bli false?
						}
						dup->fistiindex->indexFiltered.duplicate_to_show = 0;
						dup->fistiindex = &(*TeffArray)->iindex[i];
						dup->fistiindex->indexFiltered.duplicate_to_show = 1;
					    }
					else
					    {
						if (iff_set_filter(&(*TeffArray)->iindex[i].indexFiltered, FILTER_DUPLICATE)) {
						    --(*TotaltTreff);
						}
					    }

					vector_pushback(dup->V, (*TeffArray)->iindex[i].DocID, (*TeffArray)->iindex[i].subname->subname);
				}
			}
		}


		reclose_cache();




		//debug: printer ut alle treff, og litt om de.
		#ifdef DEBUG_II
			bblog(DEBUG, "hits after duplicate checking:");
			bblog(DEBUG, "\t| %-5s | %-20s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s |",  "DocId", "Subname", "Date", "Subname", "Filename",
			    "dup", "dup_c", "attr", "fltr");
			for (i = 0; (i < (*TeffArrayElementer)) && (i < 100); i++) {
				bblog(DEBUG, "\t| %-5u | %-20s | %-8d | %-8d | %-8d | %-8d | %-8d | %-8d | %-8d |", 
					(*TeffArray)->iindex[i].DocID,
					(*(*TeffArray)->iindex[i].subname).subname,
					(*TeffArray)->iindex[i].indexFiltered.date,
					(*TeffArray)->iindex[i].indexFiltered.subname,
					(*TeffArray)->iindex[i].indexFiltered.filename,
					(*TeffArray)->iindex[i].indexFiltered.duplicate,
					(*TeffArray)->iindex[i].indexFiltered.duplicate_in_collection,
					(*TeffArray)->iindex[i].indexFiltered.attribute,
					(*TeffArray)->iindex[i].indexFiltered.is_filtered
				);
			}
		#endif

		gettimeofday(&end_time, NULL);
		(*queryTime).duplicat_echecking = getTimeDifference(&start_time,&end_time);

		bblog(INFO, "</################################# duplicate checking ######################################>");

		gettimeofday(&start_time, NULL);

		bblog(INFO, "order by \"%s\"", orderby);

		if (((*filteron).sort != NULL) && (strcmp((*filteron).sort,"newest") == 0)) {
			bblog(INFO, "will do newest sort");
			for (i = 0; i < *TeffArrayElementer; i++) {
				(*TeffArray)->iindex[i].allrank = (*TeffArray)->iindex[i].date;
			}
		}
		else if ( ((*filteron).sort != NULL) && (strcmp((*filteron).sort,"oldest") == 0) ) {
			bblog(INFO, "will do oldest sort");
			for (i = 0; i < *TeffArrayElementer; i++) {
				//4294967295 unsigned int (long) max
				(*TeffArray)->iindex[i].allrank = ULONG_MAX - (*TeffArray)->iindex[i].date;
			}			
		}
		else {
			bblog(INFO, "do normal sort");
			for (i = 0; i < *TeffArrayElementer; i++) {
				(*TeffArray)->iindex[i].allrank = (*TeffArray)->iindex[i].TermRank;

				if ((*TeffArray)->iindex[i].phraseMatch) {
                                        (*TeffArray)->iindex[i].allrank = (*TeffArray)->iindex[i].allrank * 2;
                                }
			}
		}

		gettimeofday(&end_time, NULL);
                (*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);

	#else

		gettimeofday(&start_time, NULL);

		allrankcalk((*TeffArray),TeffArrayElementer);

       		gettimeofday(&end_time, NULL);
        	(*queryTime).allrankCalc = getTimeDifference(&start_time,&end_time);

	#endif


	gettimeofday(&start_time, NULL);

		bblog(INFO, "vil sortere %i", *TeffArrayElementer);
 		qsort((*TeffArray)->iindex, *TeffArrayElementer , sizeof(struct iindexMainElements), compare_elements);
		bblog(INFO, "sort ferdig");
	gettimeofday(&end_time, NULL);
	(*queryTime).indexSort = getTimeDifference(&start_time,&end_time);

	
	// Debug: viser alle treffene 
	/********************************************************************************/
	//for (i=0;i<*TeffArrayElementer;i++) {
	//	printf("DocID=%u, rank=%hu, Subname=%s\n", (*TeffArray)->iindex[i].DocID, (*TeffArray)->iindex[i].allrank, (*TeffArray)->iindex[i].subname->subname);
	//}
	/********************************************************************************/

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

	bblog(INFO, "search: ~searchSimple()");
}

#ifdef BLACK_BOKS

void searchFilterInit(struct filtersFormat *filters, int dates[]) {

	int i;

	bblog(INFO, "search: searchFilterInit()");

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

struct _attribute_temp_1_key
{
    int		count;
    unsigned int crc32val;
    char	*key;
    char	*arg1, *arg2;
};

struct _attribute_temp_1_val
{
    int		count;
    char	*key, *value, *value2;
    int		size;
};

int _attribute_temp_1_cmp( void *key1, void *key2 )
{
    struct _attribute_temp_1_key	*a = (struct _attribute_temp_1_key*)key1,
					*b = (struct _attribute_temp_1_key*)key2;
//    printf("cmp()\n");
//    printf(" count:%i;crc32val:%i;key:%s", a->count, a->crc32val, a->key);
//    printf(" count:%i;crc32val:%i;key:%s", b->count, b->crc32val, b->key);
    if (a->count == b->count)
	{
	    if (a->arg1!=NULL && b->arg1!=NULL)
		{
		    if (strcmp(a->key, b->key)==0
			&& strcmp(a->arg1, b->arg1)==0)
			{
			    if (a->arg2==NULL && b->arg2==NULL) return 1;
			    if (a->arg2!=NULL && b->arg2!=NULL
				&& strcmp(a->arg2, b->arg2)==0) return 1;
			}
		}
	    else if (a->arg1==NULL && b->arg1==NULL && a->crc32val == b->crc32val)
		{
		    if (strcmp(a->key, b->key)==0) { /*printf(" (EQUAL)\n");*/ return 1; }
//		    else printf(" (_)\n");
//		    return 0;
		}
	}
//    printf("\n");
    /*
    if (a->count > b->count) return 1;
    if (a->count < b->count) return -1;
	    if (a->crc32val > b->crc32val) return 1;
    if (a->crc32val < b->crc32val) return -1;
    return strcmp(a->key, b->key);
    */
    return 0;
}

unsigned int _attribute_temp_1_hash( void *key )
{
    unsigned int	v;
    struct _attribute_temp_1_key	*a = (struct _attribute_temp_1_key*)key;

//    printf("hash() "); fflush(stdout);
//    printf("count:%i;crc32val:%i;key:%s", a->count, a->crc32val, a->key);
    if (a->arg1==NULL) v = a->crc32val & 0xffff0000;
    else
	{
	    int		i;
	    char	*last_arg = a->arg1;
	    if (a->arg2!=NULL) last_arg = a->arg2;
	    v = 1;
	    for (i=0; last_arg[i]!='\0'; i++) v*= last_arg[i];
	    v = (v<<16) & 0xffff0000;
	}
    v+= a->count & 0xffff;
//    printf("id:%u\n", v);
    return v;
}

struct _attribute_temp_dup_data_
{
    char	*key, *value;
    int		count;
    unsigned int crc32val;
};

struct _attribute_dups_
{
    container	*main, *dups;
};



char* searchFilterCount(int *TeffArrayElementer, 
			struct iindexFormat *TeffArray, 
			struct filtersFormat *filters,
			struct subnamesFormat subnames[], 
			int nrOfSubnames,
			struct filteronFormat *filteron,
			int dates[],
			struct queryTimeFormat *queryTime,
			struct fte_data *getfiletypep,
			struct adf_data	*attrdescrp,
			attr_conf *showattrp,
			query_array *qa
		) {

		char *filesKey;
		int *filesValue;
		struct hashtable *h;
		int i, j;
		struct timeval start_time, end_time;
		struct timeval tot_start_time, tot_end_time;
		struct timeval att_start_time, att_end_time;

	        #ifdef DEBUG_TIME		
		double att1=0, att2=0, att3=0, att4=0, att5=0;
		#endif

		gettimeofday(&tot_start_time, NULL);


		/***********************************************************************************************
		teller filtyper
		***********************************************************************************************/
	        #ifdef DEBUG_TIME
        	        gettimeofday(&start_time, NULL);
	        #endif

		bblog(INFO, "search: searchFilterCount()");

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
		container	*dup_attributes = map_container( int_container(), ptr_container() );

		attribute_init_count();
		struct hashtable *attrib_count_temp = create_hashtable(16, _attribute_temp_1_hash, _attribute_temp_1_cmp);



	        #ifdef DEBUG_TIME
	                gettimeofday(&end_time, NULL);
	                bblog(DEBUG, "Time debug: searchFilterCount init time: %f", getTimeDifference(&start_time, &end_time));
	        #endif

		// Teller filtyper og attributter til navigasjonsmenyen:
	        #ifdef DEBUG_TIME
        	        gettimeofday(&start_time, NULL);
	        #endif

		for (i = 0; i < (*TeffArrayElementer); i++) {


			if (TeffArray->iindex[i].indexFiltered.date == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.subname == 1) {
				continue;
			}
			else if (TeffArray->iindex[i].indexFiltered.duplicate == 1) {
				bblog(INFO, "docid(%i) (duplicate)",  TeffArray->iindex[i].DocID);
				//continue;
				// Ax: Vi merger attributter for alle duplikater.
				// Dette vil fikse tellefeil når en av duplikatene for et dokument f.eks
				// er lagret i SuperOffice (med tilhørende attributt).
				// Det vil feile miserabelt dersom duplikatene er i flere forskjellige systemer
				// (f.eks både .doc og .pdf e.l.).
			}

			// Ax: Slå opp attributter for dokumentet.
		        #ifdef DEBUG_TIME
			gettimeofday(&att_start_time, NULL);
			#endif

			int	DocID = TeffArray->iindex[i].DocID;
			int	checksum = TeffArray->iindex[i].crc32; // Døpt checksum for å ikke blande sammen med crc32attr.
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

					if (crc32_words_size > 0
					    && (m_crc32_words=mmap(NULL, crc32_words_size, PROT_READ, MAP_PRIVATE, fileno(f_crc32_words), 0))
						!= MAP_FAILED)
					    {
						crc32_words_size/= attr_crc32_words_blocksize;

						// Putt i cache:
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

						lot_re = tuple(map_val(it_pre)).element[0].C;
					    }
					else
					    {
						fclose(f_crc32_words);
						no_attributes = 1;
					    }
				    }

				if (no_attributes)
				    {
					map_insert(attr_subname_re, subname, NULL, NULL, 0);
				    }
			    }
			else
			    {
				if (tuple(map_val(it_re)).element[2].ptr == NULL)
				    {
					no_attributes = 1;
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
			    }

		        #ifdef DEBUG_TIME
			gettimeofday(&att_end_time, NULL);
			att1 += getTimeDifference(&att_start_time, &att_end_time);
			gettimeofday(&att_start_time, NULL);
			#endif

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

		        #ifdef DEBUG_TIME
	                gettimeofday(&att_end_time, NULL);
	                att2 += getTimeDifference(&att_start_time, &att_end_time);
                        gettimeofday(&att_start_time, NULL);
			#endif

			// Beregning av attributt-bits:
			int	len = TeffArray->attrib_count + 1;
			int	count = 0;
			//count = (1<<len) -1;
			for (j=0; j<len-1; j++)
			    count+= (1 - TeffArray->iindex[i].indexFiltered.attrib[j])<<j;
			count+= (1 - TeffArray->iindex[i].indexFiltered.filename)<<(len-1);

			// Ax: Attribute dup-handling:
			container *alist = NULL;

			if (TeffArray->iindex[i].indexFiltered.duplicate_to_show == 1
			    || TeffArray->iindex[i].indexFiltered.duplicate == 1)
			    {
				alist = vector_container( ptr_container() );
			    }

			if (TeffArray->iindex[i].indexFiltered.duplicate == 1 && TeffArray->iindex[i].indexFiltered.duplicate_to_show == 1)
			    bblog(ERROR, "DocID %i is both assigned as duplicate to be shown, and to be filtered out.", DocID);


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
							char	*value = NULL;
							// Sjekk cache
							struct _attribute_temp_1_key	this_key, *hash_key;
							struct _attribute_temp_1_val	*hash_val;
							this_key.count = count;
							this_key.key = key;
							this_key.crc32val = *crc32val;
							this_key.arg1 = NULL;
							this_key.arg2 = NULL;

							// Attr-dup:
							if (alist != NULL)
							    {
								value = (char*)bsearch((const void*)(crc32val), (const void*)m_crc32_words, crc32_words_size,
							    	    attr_crc32_words_blocksize, attr_crc32_words_block_compare );

								if (value != NULL)
								    {
								        value+= sizeof(unsigned int);

								    	struct _attribute_temp_dup_data_ *atdd =
									    malloc(sizeof(struct _attribute_temp_dup_data_));
									atdd->key = strdup(key);
									atdd->value = strdup(value);
									atdd->crc32val = *crc32val;
									atdd->count = count;

									vector_pushback(alist, atdd);
								    }
							    }

							if (TeffArray->iindex[i].indexFiltered.duplicate == 0)
							    {
								hash_val = (struct _attribute_temp_1_val*)hashtable_search(attrib_count_temp, (void*)&this_key);

								if (hash_val == NULL)
								    {
									if (value == NULL)
									    {
										value = (char*)bsearch((const void*)(crc32val), (const void*)m_crc32_words, crc32_words_size,
								    	    	    attr_crc32_words_blocksize, attr_crc32_words_block_compare );

									        if (value != NULL) value+= sizeof(unsigned int);
									    }

								        if (value != NULL)
									    {
										// Legg til:
										//attribute_count_add(count, attributes, 2, key, value);
										hash_key = malloc(sizeof(struct _attribute_temp_1_key));
										hash_key->count = count;
										hash_key->key = strdup(key);
										hash_key->crc32val = *crc32val;
										hash_key->arg1 = NULL;
										hash_key->arg2 = NULL;

										hash_val = malloc(sizeof(struct _attribute_temp_1_val));
										hash_val->count = count;
										hash_val->key = strdup(key);
										hash_val->value = strdup(value);
										hash_val->value2 = NULL;
										hash_val->size = 1;
										hashtable_insert(attrib_count_temp, hash_key, hash_val);
									    }
								    }
								else
								    {
									hash_val->size++;
								    }
							    }
						    }

						crc32val++;
					    }
				    }
			    }

			// Attr-dup:
			if (alist!=NULL && TeffArray->iindex[i].indexFiltered.duplicate_to_show == 1)
			    {
				// Hovedattr
				struct _attribute_dups_ *ads;
				iterator		it_ads = map_find(dup_attributes, checksum);

				if (!it_ads.valid)
				    {
					ads = malloc(sizeof(struct _attribute_dups_));
					ads->main = alist;
					ads->dups = NULL;
					map_insert(dup_attributes, checksum, ads);
				    }
				else
				    {
					ads = map_val(it_ads).ptr;
					ads->main = alist;
				    }
			    }
			else if (alist!=NULL && TeffArray->iindex[i].indexFiltered.duplicate == 1)
			    {
				// Merge attr
				struct _attribute_dups_ *ads;
				iterator		it_ads = map_find(dup_attributes, checksum);

				if (!it_ads.valid)
				    {
					ads = malloc(sizeof(struct _attribute_dups_));
					ads->main = NULL;
					ads->dups = alist;
					map_insert(dup_attributes, checksum, ads);
				    }
				else
				    {
					int	ai, aj;
					struct _attribute_temp_dup_data_ *aid, *ajd;

					ads = map_val(it_ads).ptr;
					if (ads->dups == NULL)
					    {
						ads->dups = alist;
					    }
					else
					    {
						// Ax: O(n^2), men antar n vil være så liten at dette allikevel er raskeste løsning.
						for (ai=0; ai<vector_size(alist); ai++)
						    {
							char	has_dup = 0;

							aid = vector_get(alist, ai).ptr;

							for (aj=0; aj<vector_size(ads->dups); aj++)
							    {
								ajd = vector_get(ads->dups, aj).ptr;
								// Ax: Om to duplikater har forskjellige verdier for samme nøkkel,
								// så vil bare den ene bli med, ettersom vi ikke støtter multinøkler.
								if (!strcmp(aid->key, ajd->key))
								    has_dup = 1;
							    }

							if (!has_dup)
							    {
								vector_pushback(ads->dups, aid);
							    }
							else
							    {
								free(aid->key);
								free(aid->value);
								free(aid);
							    }
						    }

						destroy(alist);
					    }
				    }

				// Vi merger bare duplikat-attributter, ikke filetype og group:
				continue;
			    }


		        #ifdef DEBUG_TIME		
	                gettimeofday(&att_end_time, NULL);
	                att3 += getTimeDifference(&att_start_time, &att_end_time);
			gettimeofday(&att_start_time, NULL);
			#endif

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

		        #ifdef DEBUG_TIME
                        gettimeofday(&att_end_time, NULL);
                        att4 += getTimeDifference(&att_start_time, &att_end_time);
                        gettimeofday(&att_start_time, NULL);
			#endif

			iterator	it_gr = map_find(file_groups, file_ext);
		        char	*group, *descr, *icon, *version;

			if (it_gr.valid)
			    {
				//attribute_count_add(1, count, attributes, 3, "group", map_val(it_gr).ptr, file_ext);
			        //attribute_count_add(1, count, attributes, 2, "filetype", file_ext);
				group = map_val(it_gr).ptr;
			    }
			else
			    {
				fte_getdescription(getfiletypep, "eng", file_ext, &group, &descr, &icon, &version);
				map_insert(file_groups, file_ext, group);

				//attribute_count_add(1, count, attributes, 3, "group", group, file_ext);
			        //attribute_count_add(1, count, attributes, 2, "filetype", file_ext);
			    }


			struct _attribute_temp_1_key	this_key, *hash_key;
			struct _attribute_temp_1_val	*hash_val;
			this_key.count = count;
			// Add group:
			    {
				this_key.key = "group";
				this_key.arg1 = group;
				this_key.arg2 = file_ext;

				hash_val = (struct _attribute_temp_1_val*)hashtable_search(attrib_count_temp, (void*)&this_key);

				if (hash_val == NULL)
				    {
					hash_key = malloc(sizeof(struct _attribute_temp_1_key));
					hash_key->count = count;
					hash_key->key = strdup("group");
					hash_key->arg1 = group;
					hash_key->arg2 = file_ext;

					hash_val = malloc(sizeof(struct _attribute_temp_1_val));
					hash_val->count = count;
					hash_val->key = strdup("group");
					hash_val->value = strdup(group);
					hash_val->value2 = strdup(file_ext);
					hash_val->size = 1;
					hashtable_insert(attrib_count_temp, hash_key, hash_val);
				    }
				else hash_val->size++;
			    }

			// Add filetype:
			    {
				this_key.key = "filetype";
				this_key.arg1 = file_ext;
				this_key.arg2 = NULL;

				hash_val = (struct _attribute_temp_1_val*)hashtable_search(attrib_count_temp, (void*)&this_key);

				if (hash_val == NULL)
				    {
					hash_key = malloc(sizeof(struct _attribute_temp_1_key));
					hash_key->count = count;
					hash_key->key = strdup("filetype");
					hash_key->arg1 = file_ext;
					hash_key->arg2 = NULL;

					hash_val = malloc(sizeof(struct _attribute_temp_1_val));
					hash_val->count = count;
					hash_val->key = strdup("filetype");
					hash_val->value = strdup(file_ext);
					hash_val->value2 = NULL;
					hash_val->size = 1;
					hashtable_insert(attrib_count_temp, hash_key, hash_val);
				    }
				else hash_val->size++;
			    }


		        #ifdef DEBUG_TIME
	                gettimeofday(&att_end_time, NULL);
	               	att5 += getTimeDifference(&att_start_time, &att_end_time);
			#endif
		}

		// Attr-dup:
		iterator	da_it = map_begin(dup_attributes);
		for (; da_it.valid; da_it=map_next(da_it))
		    {
			struct _attribute_dups_ *ads = map_val(da_it).ptr;

			int	ai, aj;
			struct _attribute_temp_dup_data_ *aid, *ajd;

			if (ads->dups!=NULL && ads->main!=NULL) {
			    // Ax: O(n^2), men antar n vil være så liten at dette allikevel er raskeste løsning.
			    for (ai=0; ai<vector_size(ads->dups); ai++)
				{
				    char	has_dup = 0;

    				    aid = vector_get(ads->dups, ai).ptr;

				    for (aj=0; aj<vector_size(ads->main); aj++)
					{
					    ajd = vector_get(ads->main, aj).ptr;
					    // Ax: Om to duplikater har forskjellige verdier for samme nøkkel,
					    // så vil bare den ene bli med, ettersom vi ikke støtter multinøkler.
					    if (!strcmp(aid->key, ajd->key))
						has_dup = 1;
				        }

				    if (!has_dup)
					{
					    // Sjekk cache
				    	    struct _attribute_temp_1_key	this_key, *hash_key;
					    struct _attribute_temp_1_val	*hash_val;
					    this_key.count = aid->count;
					    this_key.key = aid->key;
					    this_key.crc32val = aid->crc32val;
					    this_key.arg1 = NULL;
					    this_key.arg2 = NULL;

					    hash_val = (struct _attribute_temp_1_val*)hashtable_search(attrib_count_temp, (void*)&this_key);

					    if (hash_val == NULL)
						{
						    // Legg til:
						    hash_key = malloc(sizeof(struct _attribute_temp_1_key));
						    hash_key->count = aid->count;
						    hash_key->key = strdup(aid->key);
						    hash_key->crc32val = aid->crc32val;
						    hash_key->arg1 = NULL;
						    hash_key->arg2 = NULL;

						    hash_val = malloc(sizeof(struct _attribute_temp_1_val));
						    hash_val->count = aid->count;
						    hash_val->key = strdup(aid->key);
						    hash_val->value = strdup(aid->value);
						    hash_val->value2 = NULL;
						    hash_val->size = 1;
						    hashtable_insert(attrib_count_temp, hash_key, hash_val);
					        }
					    else
						{
						    hash_val->size++;
					        }
				        }
				}
			}

			// Free memory:
			if (ads->main != NULL) {
			    for (ai=0; ai<vector_size(ads->main); ai++)
				{
				    aid = vector_get(ads->main, ai).ptr;
				    free(aid->key);
				    free(aid->value);
				    free(aid);
				}

			    destroy(ads->main);
			}

			if (ads->dups != NULL) {
			    for (ai=0; ai<vector_size(ads->dups); ai++)
				{
				    aid = vector_get(ads->dups, ai).ptr;
				    free(aid->key);
				    free(aid->value);
				    free(aid);
				}

			    destroy(ads->dups);
			}

			free(ads);
		    }

		destroy(dup_attributes);

		#ifdef DEBUG_TIME
		bblog(DEBUG, "Time debug: searchFilterCount att1 time: %f", att1);
		bblog(DEBUG, "Time debug: searchFilterCount att2 time: %f", att2);
		bblog(DEBUG, "Time debug: searchFilterCount att3 time: %f", att3);
		bblog(DEBUG, "Time debug: searchFilterCount att4 time: %f", att4);
		bblog(DEBUG, "Time debug: searchFilterCount att5 time: %f", att5);

                gettimeofday(&att_start_time, NULL);
		#endif

		struct hashtable_itr	*h_it = hashtable_iterator(attrib_count_temp);
		if (hashtable_count(attrib_count_temp))
		    do
			{
			    struct _attribute_temp_1_val *val = hashtable_iterator_value(h_it);
			    if (val->value2 == NULL)
				attribute_count_add(val->size, val->count, attributes, 2, val->key, val->value);
			    else
				attribute_count_add(val->size, val->count, attributes, 3, val->key, val->value, val->value2);

			    free(val->key);
			    free(val->value);
			    free(val->value2);
			} while (hashtable_iterator_advance(h_it));

		free(h_it);
		hashtable_destroy(attrib_count_temp, 1);

		#ifdef DEBUG_TIME
                gettimeofday(&att_end_time, NULL);
               	bblog(DEBUG, "Time debug: attribute_count_add time: %f",  getTimeDifference(&att_start_time, &att_end_time));
		#endif

		attribute_finish_count();

	        #ifdef DEBUG_TIME
	                gettimeofday(&end_time, NULL);
	                bblog(DEBUG, "Time debug: searchFilterCount loop time: %f", getTimeDifference(&start_time, &end_time));
	        #endif

        	#ifdef DEBUG_TIME
        	        gettimeofday(&start_time, NULL);
        	#endif

		// Lukk åpne filer og frigjør ledig minne:
		{
		    iterator	it_re1 = map_begin(attr_subname_re);
		    for (; it_re1.valid; it_re1=map_next(it_re1))
			{
			    if (tuple(map_val(it_re1)).element[2].ptr == NULL) continue;

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


		//#ifdef DEBUG
		bblog(DEBUG, "attributes:");
		attribute_count_print(attributes, TeffArray->attrib_count+1, 2);
		bblog(DEBUG, "------");
		//#endif

		#endif // ATTRIBUTES

        	#ifdef DEBUG_TIME
                	gettimeofday(&end_time, NULL);
                	bblog(DEBUG, "Time debug: searchFilterCount attr end time: %f", getTimeDifference(&start_time, &end_time));
        	#endif

		/***********************************************************************************************
		 collections
		***********************************************************************************************/
        	#ifdef DEBUG_TIME
        	        gettimeofday(&start_time, NULL);
        	#endif
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
					bblog(DEBUG, "cant insert");     
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
			else if (TeffArray->iindex[i].indexFiltered.duplicate_in_collection == 1) {
				continue;
			}

			if (NULL == (filesValue = hashtable_search(h,(*TeffArray->iindex[i].subname).subname) )) {    
				bblog(DEBUG, "not found!. Vil insert first \"%s\"", (*TeffArray->iindex[i].subname).subname);
				filesValue = malloc(sizeof(int));
				(*filesValue) = 1;
				filesKey = strdup((*TeffArray->iindex[i].subname).subname);
				if (! hashtable_insert(h,filesKey,filesValue) ) {
					bblog(DEBUG, "cant insert");     
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

					bblog(INFO, "collection \"%s\": %i", filesKey,*filesValue);

					strscpy(
						(*filters).collections.elements[ (*filters).collections.nrof ].name,
						filesKey,
						sizeof((*filters).collections.elements[ (*filters).collections.nrof ].name));

					(*filters).collections.elements[(*filters).collections.nrof].nrof = (*filesValue);

					++(*filters).collections.nrof;
				
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

		#ifdef DEBUG		
		bblog(DEBUG, "filtering on coll \"%s\"", filteron->collection);
		for (i=0;i<(*filters).collections.nrof;i++) {
			bblog(DEBUG, "coll \"%s\", checked %i", (*filters).collections.elements[i].name,(*filters).collections.elements[i].checked);
		}
		#endif

	        #ifdef DEBUG_TIME
        	        gettimeofday(&end_time, NULL);
        	        bblog(DEBUG, "Time debug: searchFilterCount collsum time: %f", getTimeDifference(&start_time, &end_time));
        	#endif

		/***********************************************************************************************
		//dates
		***********************************************************************************************/
		gettimeofday(&start_time, NULL);
		//kjører dateview
		dateview dv;
		dateview *dvo;

		date_info_start(&dv, 0, -1);
		
		bblog(INFO, "for all dates");
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

		dvo = date_info_end(&dv);


		//inaliserer
		for (i=0;i<10;i++) {
			dates[i] = 0;
		}

		enum dateview_output_type type = TWO_YEARS_PLUS;
		bblog(INFO, "dateview result array:");
		for (i = 0; i < type; i++) {
		                bblog(INFO, "Type: %d Count: %d",  i, dvo->output[i]);
				dates[i] = dvo->output[i];
	        }
		
		//for (; dvo != NULL; dvo = dvo->next) {
		//
		//	dates[(int)dvo->type] = dvo->length;
		//}
		date_info_free(dvo); // ax: Fjerner minnelekasje.

		gettimeofday(&end_time, NULL);
		(*queryTime).dateview = getTimeDifference(&start_time,&end_time);

		#ifdef DEBUG_TIME
			bblog(DEBUG, "Time debug: searchFilterCount data sum time: %f", (*queryTime).dateview);
		#endif

#ifdef ATTRIBUTES
	        #ifdef DEBUG_TIME
        	        gettimeofday(&start_time, NULL);
	        #endif

		// Attributter:
		bblog(INFO, "search: generating xml for attributes");

		char	*nav_xml = attribute_generate_xml(attributes, TeffArray->attrib_count+1, showattrp, getfiletypep, attrdescrp, qa);

		#ifdef DEBUG
			//skriver ut atribut xml'en.
			bblog(DEBUG, "Navigation xml: %s", nav_xml);
			//Runarb 2 feb 2009: hva gjør denne funksjonen ????
			//println(attributes);
		#endif

		attribute_destroy_recursive(attributes);

		bblog(INFO, "search: done attributes");
		
        	#ifdef DEBUG_TIME
                	gettimeofday(&end_time, NULL);
                	bblog(DEBUG, "Time debug: searchFilterCount attr xml time: %f", getTimeDifference(&start_time, &end_time));
        	#endif
#endif

		gettimeofday(&tot_end_time, NULL);
                (*queryTime).FilterCount = getTimeDifference(&tot_start_time,&tot_end_time);

		bblog(INFO, "search: ~searchFilterCount()");
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

        if (((struct iindexMainElements *)p1)->allrank > ((struct iindexMainElements *)p2)->allrank) {
                return -1;
	}
        else {
                return ((struct iindexMainElements *)p1)->allrank < ((struct iindexMainElements *)p2)->allrank;
	}
}
