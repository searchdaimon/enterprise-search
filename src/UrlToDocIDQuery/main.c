
#include "../common/crc32.h"
#include "../common/lot.h"


int main (int argc, char *argv[]) {

    	
	unsigned int DocID;
        
	if (argc < 2) {
                printf("Dette programet indekserer en ud file\n\n\tBruke\n\tUrlToDocIDQuery url\n\n");
                exit(0);
        }

	if (getDocIDFromUrl(argv[1],&DocID) != 0) {

		printf("DocID: %u\n",DocID);
	}
	else {
		printf("ukjent DocID\n");
	}


}
