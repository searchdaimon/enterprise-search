#include "ipdb.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>

#include "define.h"


int ipdbOpen(struct ipdbl *ipdbha) {

        if (((*ipdbha).FH = fopen(IPDBPATH,"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(IPDBPATH);
                exit(1);
        }
	printf("file opend %s\n",IPDBPATH);
}

unsigned int ipdbForDocID(struct ipdbl *ipdbha,unsigned int DocID) {

	unsigned long int toIPAddress;


                                if (fseek((*ipdbha).FH,DocID * sizeof(toIPAddress),SEEK_SET) == -1) {
                                        perror("fseek");
                                        toIPAddress = 0;
                                }
                                else if (fread(&toIPAddress,sizeof(toIPAddress),1,(*ipdbha).FH) != 1) {
                                        perror("read");
                                        toIPAddress = 0;
			       }

	return toIPAddress;
}

void ipdbClose(struct ipdbl *ipdbha) {
	fclose((*ipdbha).FH);
}
