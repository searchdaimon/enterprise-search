
#include "../common/timediff.h"
#include "../common/define.h"
#include "../logger/logger.h"

#include "merge.h"
#include "verbose.h"

// swap two ponters
void swapiindex(struct iindexFormat **a, struct iindexFormat **b) {

        struct iindexFormat  *tmp;

        tmp = *a;
        *a = *b;
        *b = tmp;
}


static inline void iindexArrayHitsCopy(struct iindexFormat *c, int k, struct iindexFormat *b, int j) {
	int x;

	for(x=0;x<b->iindex[j].TermAntall;x++) {
		#ifdef DEBUG_II
			bblog(DEBUGINFO, "iindexArrayHitsCopy: b %hu", b->iindex[j].hits[x].pos);
		#endif
		c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
		c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
		++c->iindex[k].TermAntall;
		++c->nrofHits;
	}
}


#ifdef EXPLAIN_RANK

static inline void rank_explaindSumm(struct rank_explaindFormat *t, struct rank_explaindFormat *a, struct rank_explaindFormat *b, int or) {
         t->rankBody            = a->rankBody + b->rankBody;
         t->rankHeadline        = a->rankHeadline + b->rankHeadline;
         t->rankTittel          = a->rankTittel + b->rankTittel;
         t->rankAnchor           = a->rankAnchor + b->rankAnchor;
         t->rankUrl_mainbody    = a->rankUrl_mainbody + b->rankUrl_mainbody;
         t->rankUrlDomain               = a->rankUrlDomain + b->rankUrlDomain;
         t->rankUrlSub          = a->rankUrlSub + b->rankUrlSub;


         t->nrAnchorPhrase       = a->nrAnchorPhrase + b->nrAnchorPhrase;
         t->nrAnchor             = a->nrAnchor + b->nrAnchor;

         t->nrBody              = a->nrBody + b->nrBody;
         t->nrHeadline          = a->nrHeadline + b->nrHeadline;
         t->nrTittel            = a->nrTittel + b->nrTittel;
         t->nrUrl_mainbody      = a->nrUrl_mainbody + b->nrUrl_mainbody;
         t->nrUrlDomain         = a->nrUrlDomain + b->nrUrlDomain;
         t->nrUrlSub            = a->nrUrlSub + b->nrUrlSub;

        //dette er egentlig en hurti fiks, da vi kan kansje kan f� mer en maks for querys med fler en et ord
        if (or) {
		t->maxBody             = a->maxBody;
		t->maxHeadline         = a->maxHeadline;
		t->maxTittel           = a->maxTittel;
		t->maxUrl_mainbody     = a->maxUrl_mainbody;
		t->maxUrlDomain        = a->maxUrlDomain;
		t->maxUrlSub           = a->maxUrlSub;
		t->maxAnchor            = a->maxAnchor;
        }
        else {
		t->maxBody             = a->maxBody + b->maxBody;
		t->maxHeadline         = a->maxHeadline + b->maxHeadline;
		t->maxTittel           = a->maxTittel + b->maxTittel;
		t->maxUrl_mainbody     = a->maxUrl_mainbody + b->maxUrl_mainbody;
		t->maxUrlDomain        = a->maxUrlDomain + b->maxUrlDomain;
		t->maxUrlSub           = a->maxUrlSub + b->maxUrlSub;
		t->maxAnchor            = a->maxAnchor + b->maxAnchor;
        }


}
#endif


void or_merge(struct iindexFormat **c, int *baselen, struct iindexFormat **a, int alen, struct iindexFormat **b, int blen) {

	int i=0;
	int j=0;
	int k=0;
	(*baselen) = 0;

	bblog(INFO, "or_merge(alen %i, blen %i)", alen,blen);

	#ifdef DEBUG_TIME
		int count_same 		= 0;
		int count_smal 		= 0;
		int count_large 	= 0;
		int count_overflow 	= 0;
	#endif
	
	
	if ((alen== 0) && (blen==0)) {
		bblog(WARN, "Bug?: or_merge was pased teo enty arrays");
		return;
	}
	else if (alen== 0) {
		bblog(INFO, "alen is 0. Can swap b array and c array");
		
		swapiindex(c, b);
		(*baselen) = blen;
		return;
	}
	else if (blen==0) {
		bblog(INFO, "blen is 0. Can swap a array and c array");

		swapiindex(c, a);
		(*baselen) = alen;
		return;
	}
	

	//debug: print ot verdiene før de merges
	#ifdef DEBUG_II	
		int x;
		x=0;
		bblog(DEBUGINFO, "a array (100 max):");
		while (x<alen && x<100){
	                bblog(DEBUGINFO, "\t%u", (*a)->iindex[x].DocID);
			++x;
		}
	
		x=0;
		bblog(DEBUGINFO, "b array (100 max):");
		while (x<blen && x<100) {
	                bblog(DEBUGINFO, "\t%u", (*b)->iindex[x].DocID);
			++x;
		}
	#endif

	while ((i<alen) && (j<blen) && (k < maxIndexElements))
	{

		if ((*a)->iindex[i].DocID == (*b)->iindex[j].DocID) {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "or_merge: %i == %i", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
			#ifdef DEBUG_TIME
				++count_same;
			#endif

			(*c)->iindex[k] = (*a)->iindex[i];
		
			//hvis vi ikke har noen hits så a og b helt like. Dette skjer typisk for acl'er. Ved å ikke merge hits da
			//kan vi merge store acl arrayer kjapt.
			if ( ((*a)->iindex[i].TermAntall != 0) &&  ((*b)->iindex[j].TermAntall != 0) ) {

				(*c)->iindex[k].TermRank = (*a)->iindex[i].TermRank + (*b)->iindex[j].TermRank;
				(*c)->iindex[k].phraseMatch = (*a)->iindex[i].phraseMatch + (*b)->iindex[j].phraseMatch;

				#ifdef EXPLAIN_RANK
					rank_explaindSumm(&(*c)->iindex[k].rank_explaind,&(*a)->iindex[i].rank_explaind,&(*b)->iindex[j].rank_explaind,1);
				#endif


				(*c)->iindex[k].TermAntall = 0;
				(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

				iindexArrayHitsCopy(*c,k,*a,i);
				iindexArrayHitsCopy(*c,k,*b,j);

			}

			++k; ++j; ++i;
		}
 		else if( (*a)->iindex[i].DocID < (*b)->iindex[j].DocID ) {


			#ifdef DEBUG_II
				bblog(DEBUGINFO, "or_merge: a: %i < b: %i = copy a", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
			#ifdef DEBUG_TIME
				++count_smal;
			#endif


			if (&(*c)->iindex[k] != &(*a)->iindex[i]) {
                		(*c)->iindex[k] = (*a)->iindex[i];
			}

			if ( (*a)->iindex[i].TermAntall != 0) {

				//copying hits
				(*c)->iindex[k].TermAntall = 0;
				(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

				iindexArrayHitsCopy(*c,k,*a,i);

			}
			
			++i; 
			++k;
		}
 		else {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "or_merge: a: %i > b: %i = copy b", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
			#ifdef DEBUG_TIME
				++count_large;
			#endif

			if (&(*c)->iindex[k] != &(*b)->iindex[j]) {
	                	(*c)->iindex[k] = (*b)->iindex[j];
			}

			if ( (*b)->iindex[j].TermAntall != 0 ) {

				//copying hits
				(*c)->iindex[k].TermAntall = 0;
				(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

				iindexArrayHitsCopy(*c,k,*b,j);
			}
			++j; 
			++k;
		}
	}

	bblog(INFO, "i %i, alen %i, j %i, blen %i. k %i", i,alen,j,blen,k);


	while (i<alen && (k < maxIndexElements)){

		#ifdef DEBUG_TIME
			++count_overflow;
		#endif

		//her kan minne overlappe. For å ungå det sjekker vi først om pekere til struktene er like
		if (&(*c)->iindex[k] != &(*a)->iindex[i]) {
	                (*c)->iindex[k] = (*a)->iindex[i];
		}

		if ( (*a)->iindex[i].TermAntall != 0 ) {

			(*c)->iindex[k].TermAntall = 0;
			(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

			iindexArrayHitsCopy(*c,k,*a,i);
		}
		++k; ++i;
	}
	

	while (j<blen && (k < maxIndexElements)) {

		#ifdef DEBUG_TIME
			++count_overflow;
		#endif

		if (&(*c)->iindex[k] != &(*b)->iindex[j]) {
                	(*c)->iindex[k] = (*b)->iindex[j];
		}

		if ( (*b)->iindex[j].TermAntall != 0 ) {

			(*c)->iindex[k].TermAntall = 0;
			(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

			iindexArrayHitsCopy(*c,k,*b,j);
		}
		++k; ++j;
	}

	(*baselen) += k;

	#ifdef DEBUG_II
		bblog(DEBUGINFO, "or_merge result (100 max):");
		x=0;
		while ((x<(*baselen)) && (x<100)) {
			bblog(DEBUGINFO, "\t%u", (*c)->iindex[x].DocID);
			++x;
		}
	#endif

	#ifdef DEBUG_TIME
		bblog(DEBUGINFO, "or_merge: same: %i, smal %i, large %i, overflow %i",  count_same, count_smal, count_large, count_overflow);
	#endif
	bblog(INFO, "~or_merge a and b of length %i %i. Into %i", alen,blen,(*baselen));
}

void andNot_merge(struct iindexFormat **c, int *baselen, int *added,struct iindexFormat **a, int alen, struct iindexFormat **b, int blen) {
	int i=0;
	int j=0;
	int x;
	int k=(*baselen);

	bblog(INFO, "andNot_merge (baselen=%i, alen=%i, blen=%i)", (*baselen),alen, blen);

	if ((blen==0) && ((*baselen)==0)) {
		bblog(INFO, "andNot_merge: we got an emty not array. Can just swap c and a");
		swapiindex(c,a);
		(*baselen) = alen;
		(*added) = alen;
		return;
	}

	while ((i<alen) && (j<blen) && (k < maxIndexElements))
	{


		if ((*a)->iindex[i].DocID == (*b)->iindex[j].DocID) {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "andNot_merge: Not DocID %u", (*a)->iindex[i].DocID);
			#endif
			++j; ++i;
		}
 		else if( (*a)->iindex[i].DocID < (*b)->iindex[j].DocID ) {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "andNot_merge: DocID %u < DocID %u. Add", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
                	(*c)->iindex[k] = (*a)->iindex[i];
			
			(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

			//kopierer hits
			(*c)->iindex[k].TermAntall = 0;
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "a TermAntall %i, (*c)->nrofHits %i", (*a)->iindex[i].TermAntall,(*c)->nrofHits);
				bblog(DEBUGINFO, "size %i", sizeof((*c)->iindex[k]));
			#endif
			for(x=0;x<(*a)->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
					bblog(DEBUGINFO, "aaa %hu", (*a)->iindex[i].hits[x].pos);
				#endif
				(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].pos = (*a)->iindex[i].hits[x].pos;
				(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].phrase = (*a)->iindex[i].hits[x].phrase;
				++(*c)->iindex[k].TermAntall;
				++(*c)->nrofHits;
			}
			
			++i; 
			++k;
			++(*baselen);
		}
 		else {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "andNot_merge: DocID %u > DocID %u. Wont add", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif

			++j; 

		}
		
	}
	#ifdef DEBUG
		bblog(DEBUGINFO, "andNot_merge: end of one array i %i, alen %i, j %i, blen %i. k %i", i,alen,j,blen,k);
		bblog(DEBUGINFO, "andNot_merge: i %i, alen %i, blen %i", i,alen,blen);
	#endif
	while (i<alen && (k < maxIndexElements)){

                (*c)->iindex[k] = (*a)->iindex[i];

		//kopierer hits
		(*c)->iindex[k].TermAntall = 0;
		(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

		for(x=0;x<(*a)->iindex[i].TermAntall;x++) {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "pos %hu", (*a)->iindex[i].hits[x].pos);
			#endif
			(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].pos = (*a)->iindex[i].hits[x].pos;
			(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].phrase = (*a)->iindex[i].hits[x].phrase;
			++(*c)->iindex[k].TermAntall;
			++(*c)->nrofHits;
		}
		
		#ifdef DEBUG_II
			bblog(DEBUGINFO, "andNot_merge: overflow DocID %u", (*a)->iindex[i].DocID);
		#endif

		++k; ++i;
		++(*baselen);
	}

	#ifdef DEBUG_II
		bblog(DEBUGINFO, "andNot_merge: have:");
		for(i=0;i<k;i++) {
			bblog(DEBUGINFO, "\tDocID: %u", (*c)->iindex[i].DocID);
		}
	#endif

	(*added) = k;

	bblog(INFO, "~andNot_merge: a and b of length %i %i. Into %i", alen,blen,(*baselen));

}

void and_merge(struct iindexFormat *c, int *baselen, int originalLen, int *added, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
	int i=0,j=0;
	int TermRank;

	int x;
	int k=originalLen;

	(*baselen) = originalLen;

	bblog(INFO, "and_merge(originalLen=%i, alen=%i, blen=%i)", originalLen,alen,blen);

	#ifdef DEBUG_II
		bblog(DEBUGINFO, "a, first DocID hits (of %i total):", alen);
		for(x=0;x<alen && x<100;x++) {
			bblog(DEBUGINFO, "\t%u", a->iindex[x].DocID);
		}
		bblog(DEBUGINFO, "b, first DocID hits (of %i total):", blen);
		for(x=0;x<blen && x<100;x++) {
			bblog(DEBUGINFO, "\t%u", b->iindex[x].DocID);
		}
	#endif

	#ifdef DEBUG_II
		bblog(DEBUGINFO, "and_merge: start");
		bblog(DEBUGINFO, "and_merge:  originalLen %i", originalLen);
	#endif

	while (i<alen && j<blen)
	{
		if (a->iindex[i].DocID == b->iindex[j].DocID) {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "\t%i == %i", a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
	

			TermRank = a->iindex[i].TermRank + b->iindex[j].TermRank;
                        c->iindex[k] = a->iindex[i];
                        c->iindex[k].TermRank = TermRank;
			c->iindex[k].phraseMatch = 0;


			//copying hits
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "and_merge: hist a %hu, b %hu", a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			#endif
			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			for(x=0;x<a->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
					bblog(DEBUGINFO, "aaa %hu", a->iindex[i].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = a->iindex[i].hits[x].phrase;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			for(x=0;x<b->iindex[j].TermAntall;x++) {
				#ifdef DEBUG_II
					bblog(DEBUGINFO, "bbb %hu", b->iindex[j].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = b->iindex[j].hits[x].phrase;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			

			k++; j++; i++;
			(*baselen)++;
		}
 		else if( a->iindex[i].DocID < b->iindex[j].DocID ) {
			#ifdef DEBUG_II
				bblog(DEBUGINFO, "\t%i < %i", a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
			
			i++;
		}
 		else {
			#ifdef DEBUG_II	
				bblog(DEBUGINFO, "\t%i > %i", a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
			j++;
		}
	}

	(*added) = k;

	bblog(INFO, "and_merge a and b of length %i %i, into %i, starting to add on element %i", alen,blen,(*baselen),originalLen);

}





//and søk med progsymasjon () mere vekt  hvis ordene er nerme en fjernt.
void andprox_merge(struct iindexFormat *c, int *baselen, int originalLen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {
        int i=0,j=0;
	int k=originalLen;
	int ah,bh;
	int TermRank;
        (*baselen) = 0;
	int found;
	
	#ifdef DEBUG
		struct timeval start_time, end_time;
		gettimeofday(&start_time, NULL);
	#endif

	bblog(INFO, "a nrofHits %i, b nrofHits %i. (c nrofHits %i)", a->nrofHits,b->nrofHits,c->nrofHits);

        while (i<alen && j<blen)
        {


                if (a->iindex[i].DocID == b->iindex[j].DocID) {


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
			

			c->iindex[k].phraseMatch = 0;

			#ifdef EXPLAIN_RANK
				rank_explaindSumm(&c->iindex[k].rank_explaind,&a->iindex[i].rank_explaind,&b->iindex[j].rank_explaind,0);

				printf("a (DocID %u, TermAntall: %i): ",a->iindex[i].DocID, a->iindex[i].TermAntall);
				for (x=0;x<a->iindex[i].TermAntall;x++) {
					printf("%hu,",a->iindex[i].hits[x].pos);
				}
				printf("\n");
				printf("b (DocID %u, TermAntall: %i):",b->iindex[j].DocID, b->iindex[j].TermAntall);
				for (x=0;x<b->iindex[j].TermAntall;x++) {
					printf("%hu,",b->iindex[j].hits[x].pos);
				}
				printf("\n");
			#endif


			ah = bh = 0;

			int TermAntall;

			TermAntall = 0;
			struct hitsFormat *hits;

			hits = &c->hits[c->nrofHits];

			found = 0;
			//lopper gjenom alle hitene og finner de som er rett etter hverandre
			while ((ah < a->iindex[i].TermAntall) && (bh < b->iindex[j].TermAntall)) {

				if (c->nrofHits == maxTotalIindexHits) {
					#ifdef EXPLAIN_RANK
						bblog(DEBUGINFO, "have now maxTotalIindexHits hits in hits array");
					#endif
					break;
				}


				//sjekker om dette er en frase. Altså at "ord2" kommer ret etter "ord1"
				if ((b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos) == 1) {
					hits[TermAntall].pos = b->iindex[j].hits[bh].pos;
					
					if ((a->iindex[i].hits[ah].phrase != -1) && (b->iindex[j].hits[bh].phrase != -1)) {
						hits[TermAntall].phrase = 1;
						++c->phrasenr;
					}
					else {
						hits[TermAntall].phrase = -1;
						--c->phrasenr;
					}
					#ifdef EXPLAIN_RANK
						bblog(DEBUGINFO, "frase hit DocID: %u, hit a: %hu b: %hu. Marked as phrase: %d", 
							a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos, hits[TermAntall].phrase);
					#endif

					++TermAntall;

					c->nrofHits++;
					found = 1;
					ah++;
					bh++;
				}
				else if (b->iindex[j].hits[bh].pos > a->iindex[i].hits[ah].pos) {
					#ifdef EXPLAIN_RANK
						bblog(DEBUGINFO, "NOT frase hit DocID %u, hit a: %hu b: %hu. ah++", a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
					#endif
			
					hits[TermAntall].pos = a->iindex[i].hits[ah].pos;
					hits[TermAntall].phrase = -1;
					++TermAntall;

					c->nrofHits++;
					
					ah++;
				}
				else {
					#ifdef EXPLAIN_RANK
						bblog(DEBUGINFO, "NOT frase hit DocID %u, hit a: %hu b: %hu. bh++", a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
					#endif
			
					hits[TermAntall].pos = b->iindex[j].hits[bh].pos;
					hits[TermAntall].phrase = -1;
					++TermAntall;

					c->nrofHits++;
			
					bh++;
				}
				

						

			}

			// kopis inn the last one
			while ((ah < a->iindex[i].TermAntall) && (c->nrofHits < maxTotalIindexHits)) {
				#ifdef EXPLAIN_RANK
					bblog(DEBUGINFO, "Tail hit DocID %u, hit a: %hu b: %hu. ah++", a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
				#endif

				hits[TermAntall].pos = a->iindex[i].hits[ah].pos;
				hits[TermAntall].phrase = -1;
				++TermAntall;

				c->nrofHits++;

				ah++;
			}

			while ((bh < b->iindex[j].TermAntall) && (c->nrofHits < maxTotalIindexHits)) {
				#ifdef EXPLAIN_RANK
					bblog(DEBUGINFO, "Tail hit DocID %u, hit a: %hu b: %hu. bh++", a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
				#endif

				hits[TermAntall].pos = b->iindex[j].hits[bh].pos;
				hits[TermAntall].phrase = -1;
				++TermAntall;

				c->nrofHits++;
			
				bh++;
			}

			if (found) {
				c->iindex[k].phraseMatch = 1;
			}

			c->iindex[k].TermAntall = TermAntall;
			c->iindex[k].hits = hits;


			#ifdef EXPLAIN_RANK
				printf("c (final: )");
				for (x=0;x<c->iindex[k].TermAntall;x++) {
					printf("%hu,",c->iindex[k].hits[x].pos,c->iindex[k].hits[x].phrase);
				}
				printf("\n");
			#endif

                        k++; j++; i++;
                        (*baselen)++;
                }
                else if( a->iindex[i].DocID < b->iindex[j].DocID ) {
                        i++;
                }
                else {
                        j++;
                }
        }

	#ifdef DEBUG
		gettimeofday(&end_time, NULL);
		bblog(DEBUGINFO, "Time debug: andprox_merge %u", getTimeDifference(&start_time,&end_time));
	#endif


	#ifdef EXPLAIN_RANK
		bblog(INFO, "andprox_merge: Merged a and b of length %i %i. Into %i", alen,blen,(*baselen));
		bblog(INFO, "andprox_merge: nrofHits: %i", c->nrofHits);
	#endif
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






//frasesk. Denne er dog ikke bra, egentlig en versjon av andprox_merge der bare de sidene med distanse 1 blir med
void frase_merge(struct iindexFormat *c, int *baselen,int Originallen, struct iindexFormat *a, int alen, struct iindexFormat *b, int blen) {

        int i=0,j=0;
	int k=Originallen;
	int ah,bh;
	int found;
	int TermRank;
	
        (*baselen) = Originallen;

	struct hitsFormat *hits;
	int TermAntall;

	bblog(DEBUGINFO, "frase_merge: start");
	bblog(DEBUGINFO, "have %i hits from before",Originallen);
	bblog(DEBUGINFO, "frase_merge: merging array a of len %i to b of len %i",alen,blen);
	bblog(DEBUGINFO, "frase_merge: arrays %u %u",(unsigned int)a,(unsigned int)b);

	#ifdef DEBUG
		bblog(DEBUGINFO, "frase_merge: from before");
		for (i=0;i<*baselen;i++) {
			bblog(DEBUGINFO, "\t DocID %u, subname \"%s\"", c->iindex[i].DocID,(*c->iindex[i].subname).subname);
		}
	#endif

        while (i<alen && j<blen)
        {
                if (a->iindex[i].DocID == b->iindex[j].DocID) {



			#ifdef EXPLAIN_RANK
                        	bblog(DEBUGINFO, "Have DocID match %u == %u", a->iindex[i].DocID,b->iindex[j].DocID);
                        
				bblog(DEBUGINFO, "a: (TermAntall %i): ",a->iindex[i].TermAntall);
				for (y=0; (y < a->iindex[i].TermAntall) && (y < MaxTermHit); y++) {
                        		bblog(DEBUGINFO, "%hu ",a->iindex[i].hits[y].pos);
        	        	}
				bblog(DEBUGINFO, "");
				
				bblog(DEBUGINFO, "b: (TermAntall %i): ",b->iindex[j].TermAntall);
                        	for (y=0; (y < b->iindex[j].TermAntall) && (y < MaxTermHit); y++) {
                        	        bblog(DEBUGINFO, "%hu ",b->iindex[j].hits[y].pos);
                        	}
				bblog(DEBUGINFO, "");
			#endif

	               	c->iindex[k] = a->iindex[i];


			TermAntall = 0;

                        hits = &c->hits[c->nrofHits];


			ah = bh = 0;
			found = 0;
			//lopper gjenom alle hitene og finner de som er rett etter hverandre

			#ifdef EXPLAIN_RANK
				bblog(DEBUGINFO, "a TermAntall %i, b TermAntall %i",a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			#endif

			while ((ah < a->iindex[i].TermAntall) && (bh < b->iindex[j].TermAntall)) {

				//sjekker om dette er en frase. Altså at "ord2" kommer ret etter "ord1"
				if ((b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos) == 1) {
					found = 1;

					hits[TermAntall].pos = b->iindex[j].hits[bh].pos;
					hits[TermAntall].phrase = 1;
					++c->phrasenr;

					#ifdef EXPLAIN_RANK
						bblog(DEBUGINFO, "frase_merge: frase hit DocID %u %hu %hu is now %hu",c->iindex[k].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos,c->iindex[k].hits[c->iindex[k].TermAntall].pos);
					#endif

					TermAntall++;
					c->nrofHits++;

					ah++;
					bh++;

				}
				else if (a->iindex[i].hits[ah].pos < b->iindex[j].hits[bh].pos) {
					#ifdef EXPLAIN_RANK
						bblog(DEBUGINFO, "frase_merge: not a hit a: %hu b: %hu, dist %i ah++",b->iindex[j].hits[bh].pos,a->iindex[i].hits[ah].pos,(b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos));
					#endif
					ah++;
				}
				else {
					#ifdef EXPLAIN_RANK
						bblog(DEBUGINFO, "frase_merge: not a hit a: %hu b: %hu, dist%i. bh++",b->iindex[j].hits[bh].pos,a->iindex[i].hits[ah].pos,(b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos));
					#endif
					bh++;
				}

						

			}



			c->iindex[k].TermAntall = TermAntall;
			c->iindex[k].hits = hits;

			if (found) {

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
				c->iindex[k].phraseMatch = 1;


                        	(*baselen)++;
				k++;
			}


                        j++; i++;
                }
                else if( a->iindex[i].DocID < b->iindex[j].DocID ) {
                        i++;
                }
                else {
                        j++;
                }

        }

	#ifdef DEBUG
		bblog(DEBUGINFO, "frase_merge: results");
		for (i=0;i<*baselen;i++) {
			bblog(DEBUGINFO, "\t DocID %u, subname \"%s\"", c->iindex[i].DocID,(*c->iindex[i].subname).subname);
		}
	#endif

	bblog(INFO, "frase_merge: got %i new elements. k %i, baselen %i", *baselen,k,*baselen);
	bblog(DEBUGINFO, "frase_merge: end");

}
