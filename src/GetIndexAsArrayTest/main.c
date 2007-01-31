#include "../common/iindex.h"
#include "../common/crc32.h"
#include <string.h>
#include <stdio.h>

#define subname "www"

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


int main () {

	struct iindexFormat *AthorArray;
	int AthorArrayLen;
	unsigned long WordIDcrc32;
	
	char query[] = "chat";

	char servername[] = "bbs-002.boitho.com";

        //starter opp
        //laster inn alle poprankene
        printf("loding pop MemArray\n");
        popopenMemArray(servername,"www"); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får et inn 
        printf("done\n");

        printf("loding adultWeight MemArray\n");
        adultWeightopenMemArray(servername,"www"); // ToDo: hardkoder subname her, da vi ikke vet siden vi ikke her får 
        printf("done\n");

        IIndexInaliser();

        
        
        

	
	AthorArray = (struct iindexFormat *)malloc(maxIndexElements * sizeof(struct iindexFormat));
        AthorArrayLen = 0;

	WordIDcrc32 = crc32boitho(query);


	GetIndexAsArray(&AthorArrayLen,AthorArray,&WordIDcrc32,"Athor","aa",subname,rankAthor);


	printf("AthorArrayLen %i\n",AthorArrayLen);
}
