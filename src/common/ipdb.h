#include <stdio.h>

struct ipdbl {
        FILE *FH;
};


int ipdbOpen(struct ipdbl *ipdbha);
unsigned int ipdbForDocID(struct ipdbl *ipdbha,unsigned int DocID);
void ipdbClose(struct ipdbl *ipdbha);
