
#include "../common/define.h"
#include "../logger/logger.h"

#include "merge.h"
#include "verbose.h"

static inline void iindexArrayHitsCopy(struct iindexFormat *c, int k, struct iindexFormat *b, int j) {
	int x;

	for(x=0;x<b->iindex[j].TermAntall;x++) {
#ifdef DEBUG_II
		bblog(DEBUG, "iindexArrayHitsCopy: b %hu", b->iindex[j].hits[x].pos);
#endif
		c->iindex[k].hits[c->iindex[k].TermAntall].pos = b->iindex[j].hits[x].pos;
		c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
		++c->iindex[k].TermAntall;
		++c->nrofHits;
	}
}



void or_merge(struct iindexFormat **c, int *baselen, struct iindexFormat **a, int alen, struct iindexFormat **b, int blen) {

	int x;

	int i=0;
	int j=0;
	int k=0;
	(*baselen) = 0;
	//struct iindexFormat *tempiip;

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
		//tempiip = *c;
		//*c = *b;
		//*b = tempiip;
		swapiindex(c, b);
		(*baselen) = blen;
		return;
	}
	else if (blen==0) {
		bblog(INFO, "blen is 0. Can swap a array and c array");
		//tempiip = *c;
		//*c = *a;
		//*a = tempiip;
		swapiindex(c, a);
		(*baselen) = alen;
		return;
	}
	

	//debug: print ot verdiene før de merges
	#ifdef DEBUG_II	
		x=0;
		bblog(DEBUG, "a array (100 max):");
		while (x<alen && x<100){
	                bblog(DEBUG, "\t%u", (*a)->iindex[x].DocID);
			++x;
		}
	
		x=0;
		bblog(DEBUG, "b array (100 max):");
		while (x<blen && x<100) {
	                bblog(DEBUG, "\t%u", (*b)->iindex[x].DocID);
			++x;
		}
	#endif

	while ((i<alen) && (j<blen) && (k < maxIndexElements))
	{

		if ((*a)->iindex[i].DocID == (*b)->iindex[j].DocID) {
			#ifdef DEBUG_II
				bblog(DEBUG, "or_merge: %i == %i", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
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
			//++(*baselen);
		}
 		else if( (*a)->iindex[i].DocID < (*b)->iindex[j].DocID ) {


			#ifdef DEBUG_II
				bblog(DEBUG, "or_merge: a: %i < b: %i = copy a", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
			#ifdef DEBUG_TIME
				++count_smal;
			#endif


			if (&(*c)->iindex[k] != &(*a)->iindex[i]) {
                		(*c)->iindex[k] = (*a)->iindex[i];
			}

			if ( (*a)->iindex[i].TermAntall != 0) {

				//copying hits
				//#ifdef DEBUG_II
				//printf("or_merge: hist a %hu, b %hu\n",(*a)->iindex[i].TermAntall,(*b)->iindex[j].TermAntall);
				//#endif
				(*c)->iindex[k].TermAntall = 0;
				(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

				iindexArrayHitsCopy(*c,k,*a,i);

			}
			
			++i; 
			++k;
			//++(*baselen);
		}
 		else {
			#ifdef DEBUG_II
				bblog(DEBUG, "or_merge: a: %i > b: %i = copy b", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
			#ifdef DEBUG_TIME
				++count_large;
			#endif

			if (&(*c)->iindex[k] != &(*b)->iindex[j]) {
	                	(*c)->iindex[k] = (*b)->iindex[j];
			}

			if ( (*b)->iindex[j].TermAntall != 0 ) {

				//copying hits
				//#ifdef DEBUG_II
				//printf("or_merge: hits a nr %hu, b nr %hu\n",(*a)->iindex[i].TermAntall,(*b)->iindex[j].TermAntall);
				//#endif
				(*c)->iindex[k].TermAntall = 0;
				(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

				iindexArrayHitsCopy(*c,k,*b,j);
			}
			++j; 
			++k;
			//++(*baselen);
		}
	}

	bblog(INFO, "i %i, alen %i, j %i, blen %i. k %i", i,alen,j,blen,k);

	//runarb: 14 mai
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
		//++(*baselen);
	}
	
	//runarb: 14 mai
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
		//++(*baselen);
	}

	(*baselen) += k;

	#ifdef DEBUG_II
	bblog(DEBUG, "or_merge result (100 max):");
	x=0;
	while ((x<(*baselen)) && (x<100)) {
                bblog(DEBUG, "\t%u", (*c)->iindex[x].DocID);
		++x;
	}
	#endif

	#ifdef DEBUG_TIME
		bblog(DEBUG, "or_merge: same: %i, smal %i, large %i, overflow %i",  count_same, count_smal, count_large, count_overflow);
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
			bblog(DEBUG, "andNot_merge: Not DocID %u", (*a)->iindex[i].DocID);
			#endif
			++j; ++i;
		}
 		else if( (*a)->iindex[i].DocID < (*b)->iindex[j].DocID ) {
			#ifdef DEBUG_II
			bblog(DEBUG, "andNot_merge: DocID %u < DocID %u. Add", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
                	(*c)->iindex[k] = (*a)->iindex[i];
			
			(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

			//kopierer hits
			(*c)->iindex[k].TermAntall = 0;
			#ifdef DEBUG_II
			bblog(DEBUG, "a TermAntall %i, (*c)->nrofHits %i", (*a)->iindex[i].TermAntall,(*c)->nrofHits);
			bblog(DEBUG, "size %i", sizeof((*c)->iindex[k]));
			#endif
			for(x=0;x<(*a)->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				bblog(DEBUG, "aaa %hu", (*a)->iindex[i].hits[x].pos);
				#endif
				(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].pos = (*a)->iindex[i].hits[x].pos;
				(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].phrase = 0;
				++(*c)->iindex[k].TermAntall;
				++(*c)->nrofHits;

			}
			
			++i; 
			++k;
			++(*baselen);
		}
 		else {
			#ifdef DEBUG_II
			bblog(DEBUG, "andNot_merge: DocID %u > DocID %u. Wont add", (*a)->iindex[i].DocID,(*b)->iindex[j].DocID);
			#endif
	                //c[k] = b[j];

			++j; 
			//++k;
			//++(*baselen);
		}
		
	}
	#ifdef DEBUG
	bblog(DEBUG, "andNot_merge: end of one array i %i, alen %i, j %i, blen %i. k %i", i,alen,j,blen,k);
	bblog(DEBUG, "andNot_merge: i %i, alen %i, blen %i", i,alen,blen);
	#endif
	while (i<alen && (k < maxIndexElements)){

                (*c)->iindex[k] = (*a)->iindex[i];

		//kopierer hits
		(*c)->iindex[k].TermAntall = 0;
		(*c)->iindex[k].hits = &(*c)->hits[(*c)->nrofHits];

		for(x=0;x<(*a)->iindex[i].TermAntall;x++) {
			#ifdef DEBUG_II
			bblog(DEBUG, "pos %hu", (*a)->iindex[i].hits[x].pos);
			#endif
			(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].pos = (*a)->iindex[i].hits[x].pos;
			(*c)->iindex[k].hits[(*c)->iindex[k].TermAntall].phrase = 0;
			++(*c)->iindex[k].TermAntall;
			++(*c)->nrofHits;

		}
		#ifdef DEBUG_II
		bblog(DEBUG, "andNot_merge: overflow DocID %u", (*a)->iindex[i].DocID);
		#endif

		++k; ++i;
		++(*baselen);
	}

	#ifdef DEBUG_II
	bblog(DEBUG, "andNot_merge: have:");
	for(i=0;i<k;i++) {
		bblog(DEBUG, "\tDocID: %u", (*c)->iindex[i].DocID);
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

	//runarb: 28, jul 2007
	//bytter slik at vi har totalt elementer i array
	//(*baselen) = 0;
	(*baselen) = originalLen;

	bblog(INFO, "and_merge(originalLen=%i, alen=%i, blen=%i)", originalLen,alen,blen);

	#ifdef DEBUG_II
	bblog(DEBUG, "a, first hits (of %i total):", alen);
	for(x=0;x<alen && x<100;x++) {
		bblog(DEBUG, "\t%u", a->iindex[x].DocID);
	}
	bblog(DEBUG, "b, first hits (of %i total):", blen);
	for(x=0;x<blen && x<100;x++) {
		bblog(DEBUG, "\t%u", b->iindex[x].DocID);
	}
	#endif

	#ifdef DEBUG_II
	bblog(DEBUG, "and_merge: start");
	bblog(DEBUG, "and_merge:  originalLen %i", originalLen);
	#endif

	while (i<alen && j<blen)
	{
		if (a->iindex[i].DocID == b->iindex[j].DocID) {
			#ifdef DEBUG_II
				bblog(DEBUG, "\t%i == %i", a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
			//c[k] = a[i];

			//c[k].TermRank = a[i].TermRank + b[j].TermRank;		

			TermRank = a->iindex[i].TermRank + b->iindex[j].TermRank;
                        c->iindex[k] = a->iindex[i];
                        c->iindex[k].TermRank = TermRank;
			c->iindex[k].phraseMatch = 0;


			//copying hits
			#ifdef DEBUG_II
			bblog(DEBUG, "and_merge: hist a %hu, b %hu", a->iindex[i].TermAntall,b->iindex[j].TermAntall);
			#endif
			c->iindex[k].TermAntall = 0;
			c->iindex[k].hits = &c->hits[c->nrofHits];

			for(x=0;x<a->iindex[i].TermAntall;x++) {
				#ifdef DEBUG_II
				bblog(DEBUG, "aaa %hu", a->iindex[i].hits[x].pos);
				#endif
				c->iindex[k].hits[c->iindex[k].TermAntall].pos = a->iindex[i].hits[x].pos;
				c->iindex[k].hits[c->iindex[k].TermAntall].phrase = 0;
				++c->iindex[k].TermAntall;
				++c->nrofHits;
			}
			for(x=0;x<b->iindex[j].TermAntall;x++) {
				#ifdef DEBUG_II
				bblog(DEBUG, "bbb %hu", b->iindex[j].hits[x].pos);
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
				bblog(DEBUG, "\t%i < %i", a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
   			//c[k++] = a[i++];
			i++;
		}
 		else {
			#ifdef DEBUG_II	
				bblog(DEBUG, "\t%i > %i", a->iindex[i].DocID,b->iindex[j].DocID);
			#endif
   			//c[k++] = b[j++];
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

	bblog(INFO, "a nrofHits %i, b nrofHits %i. (c nrofHits %i)", a->nrofHits,b->nrofHits,c->nrofHits);

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
					bblog(DEBUG, "have now maxTotalIindexHits hits in hits array");
					#endif
					break;
				}


				//sjekker om dette er en frase. Altså at "ord2" kommer ret etter "ord1"
				if ((b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos) == 1) {
					#ifdef DEBUG
					//if (c->iindex[k].DocID == 9516391) {
						bblog(DEBUG, "frase hit DocID %u, hit %hu %hu", a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
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
						bblog(DEBUG, "NOT frase hit DocID %u, hit a: %hu b: %hu. ah++", a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
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
						bblog(DEBUG, "NOT frase hit DocID %u, hit a: %hu b: %hu. bh++", a->iindex[i].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos);
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
	bblog(DEBUG, "Time debug: andprox_merge %u", getTimeDifference(&start_time,&end_time));
	#endif

	bblog(INFO, "andprox_merge: a and b of length %i %i. Into %i", alen,blen,(*baselen));
	bblog(INFO, "andprox_merge: nrofHits: %i", c->nrofHits);

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

	bblog(DEBUG, "frase_merge: start");
	bblog(DEBUG, "have %i hits from before",Originallen);
	bblog(DEBUG, "frase_merge: merging array a of len %i to b of len %i",alen,blen);
	bblog(DEBUG, "frase_merge: arrays %u %u",(unsigned int)a,(unsigned int)b);

	#ifdef DEBUG
	bblog(DEBUG, "frase_merge: from before");
	for (i=0;i<*baselen;i++) {
		bblog(DEBUG, "\t DocID %u, subname \"%s\"", c->iindex[i].DocID,(*c->iindex[i].subname).subname);
	}

	#endif

        while (i<alen && j<blen)
        {
                if (a->iindex[i].DocID == b->iindex[j].DocID) {



			#ifdef DEBUG
                        	bblog(DEBUG, "Have DocID match %u == %u", a->iindex[i].DocID,b->iindex[j].DocID);
                        
				bblog(DEBUG, "a: (TermAntall %i): ",a->iindex[i].TermAntall);
				for (y=0; (y < a->iindex[i].TermAntall) && (y < MaxTermHit); y++) {
                        		bblog(DEBUG, "%hu ",a->iindex[i].hits[y].pos);
        	        	}
				bblog(DEBUG, "");
				
				bblog(DEBUG, "b: (TermAntall %i): ",b->iindex[j].TermAntall);
                        	for (y=0; (y < b->iindex[j].TermAntall) && (y < MaxTermHit); y++) {
                        	        bblog(DEBUG, "%hu ",b->iindex[j].hits[y].pos);
                        	}
				bblog(DEBUG, "");
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


			bblog(DEBUG, "a TermAntall %i, b TermAntall %i",a->iindex[i].TermAntall,b->iindex[j].TermAntall);

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

					bblog(DEBUG, "frase_merge: frase hit DocID %u %hu %hu is now %hu",c->iindex[k].DocID,a->iindex[i].hits[ah].pos,b->iindex[j].hits[bh].pos,c->iindex[k].hits[c->iindex[k].TermAntall].pos);

					//c->iindex[k].TermAntall++;
					TermAntall++;
					c->nrofHits++;

					//hitcount++;
					ah++;
					bh++;

				}
				else if (a->iindex[i].hits[ah].pos < b->iindex[j].hits[bh].pos) {
					bblog(DEBUG, "frase_merge: not a hit a: %hu b: %hu, dist %i ah++",b->iindex[j].hits[bh].pos,a->iindex[i].hits[ah].pos,(b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos));
					ah++;
				}
				else {
					bblog(DEBUG, "frase_merge: not a hit a: %hu b: %hu, dist%i. bh++",b->iindex[j].hits[bh].pos,a->iindex[i].hits[ah].pos,(b->iindex[j].hits[bh].pos - a->iindex[i].hits[ah].pos));
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
	bblog(DEBUG, "frase_merge: results");
	for (i=0;i<*baselen;i++) {
		bblog(DEBUG, "\t DocID %u, subname \"%s\"", c->iindex[i].DocID,(*c->iindex[i].subname).subname);
	}

	#endif

	bblog(INFO, "frase_merge: got %i new elements. k %i, baselen %i", *baselen,k,*baselen);
	bblog(DEBUG, "frase_merge: end");

}
