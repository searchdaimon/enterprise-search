#include "../common/define.h"
#include "../common/reposetory.h"

int main (int argc, char *argv[]) {

	int LotNr;
	unsigned int DocID;
	char text[50];
	unsigned int radress;
	unsigned int rsize;
	char *subname;

        //tester for at vi har fåt hvilken lot vi skal bruke
        if (argc < 3) {
                printf("Usage: ./anchorreadnew lotnr subname\n\n");
		exit(1);
        }

	LotNr = atoi(argv[1]);
	subname = argv[2];

	//int anchorGetNext (int LotNr,unsigned int *DocID,char *text,unsigned int *radress,unsigned int *rsize)
	while (anchorGetNextnew(LotNr,&DocID,text,sizeof(text),&radress,&rsize, subname) ) {	
		printf("DocID %i, text: %s\n",DocID,text);
	}
}
