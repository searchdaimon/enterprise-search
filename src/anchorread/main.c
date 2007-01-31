#include "../common/reposetory.h"

int main (int argc, char *argv[]) {

	int LotNr;
	unsigned int DocID;
	char text[50];
	unsigned int radress;
	unsigned int rsize;

        //tester for at vi har fåt hvilken lot vi skal bruke
        if (argc < 2) {
                printf("Usage: ./anchorread lotnr\n\n");
		exit(1);
        }

	LotNr = atoi(argv[1]);

	//int anchorGetNext (int LotNr,unsigned int *DocID,char *text,unsigned int *radress,unsigned int *rsize)
	while (anchorGetNext(LotNr,&DocID,text,sizeof(text),&radress,&rsize) ) {	
	
		printf("DocID %i, text: %s\n",DocID,text);

	}
}
