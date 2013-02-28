#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/poprank.h"

//var 10 her
#define Rank_intern_max 0
#define Rank_noc_max 0

int main (int argc, char *argv[]) {
        struct popl popextern;
        struct popl popintern;
        struct popl popnoc;

	struct popl popindex;

	int Rank_extern,Rank_intern,Rank_noc;

	unsigned int DocID_intern,DocID_noc,DocID_extern;

	int pop;

        popopen (&popextern,"/home/boitho/config/popextern");
        popopen (&popintern,"/home/boitho/config/popintern");
        popopen (&popnoc,"/home/boitho/config/popnoc");
	popopen (&popindex,"/home/boitho/config/popindex");

	//int popGetNext (struct popl *popha, int *Rank,unsigned int *rDocID)

	while (popGetNext(&popextern,&Rank_extern,&DocID_extern) && popGetNext(&popintern,&Rank_intern,&DocID_intern) && popGetNext(&popnoc,&Rank_noc,&DocID_noc)) {
		#ifdef DEBUG
			printf("extern: %i %i\nintern: %i %i\nnoc: %i %i\n",DocID_extern,Rank_extern,DocID_intern,Rank_intern,DocID_noc,Rank_noc);
		#endif

		if ((DocID_extern != DocID_intern) || (DocID_extern != DocID_noc)) {
			printf("DocID error\n");
			exit(1);
		}

		if (Rank_intern > Rank_intern_max) {
			Rank_intern = Rank_intern_max;
		}
		if (Rank_noc > Rank_noc_max) {
			Rank_noc = Rank_noc_max;
		}

		pop = Rank_extern + Rank_intern + Rank_noc;

		if (pop == 0) {
			pop = 1;
		}
	
		#ifdef DEBUG
			printf("pop: %i\n\n",pop);
		#endif

		//void popset (struct popl *popha,int DocID,int pop);
		popset(&popindex,DocID_extern,pop);


	}


        popclose(&popextern);
        popclose(&popintern);
        popclose(&popnoc);
	popclose(&popindex);

}
