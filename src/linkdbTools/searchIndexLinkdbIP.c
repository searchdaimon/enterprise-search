#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "Linkdb.h"
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include "../common/define.h"
#include "chtbl.h"

#define PRIME_TBLSIZ 100


int h(const void *key);
int match(const void *key1, const void *key2);


//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

int main (int argc, char *argv[]) {

        FILE *LINKDBFILE;
        FILE *INDEXFILE;

	int unixipes, unixclassa;
	struct in_addr addr;
	int count;
        int ranged;
        struct linkdb_block linkdbPost;
	unsigned int DocID;
	off64_t offset;
        unsigned int lastDocID;
	FILE *IPDBHA;
	unsigned int ipadress, classaadress;
	CHTbl htbl;
	CHTbl htbl_classa;

	unsigned int *prti;
        if (argc < 4) {
                printf("Dette programet tar inn en linkdb fil og lager Brank\n\n\tUsage: ./searchIndexLinkdb LINKDBFILE INDEXFILE\n");
                exit(0);
        }

        if ((LINKDBFILE = (FILE *)fopen64(argv[1],"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }


        if ((INDEXFILE = fopen(argv[2],"rb")) == NULL) {
                printf("Cant read index ");
                perror(argv[2]);
                exit(1);
        }

        if ((IPDBHA = fopen(IPDBPATH,"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(IPDBPATH);
                exit(1);
        }

	DocID = atol(argv[3]);

	printf("links ti %u:\n",DocID);

	//finner offset
	fseek(INDEXFILE,DocID * sizeof(offset),SEEK_SET);
	fread(&offset,sizeof(offset),1,INDEXFILE);

	//søker til offsetten
	if(fseeko64(LINKDBFILE,offset,SEEK_SET) == -1) {
		perror("seek");
		exit(1);
	}


	//inaliserer hashen
	chtbl_init(&htbl, PRIME_TBLSIZ, h, match, free);
	chtbl_init(&htbl_classa, PRIME_TBLSIZ, h, match, free);
	

	unixipes = 0;
	unixclassa = 0;
	count = 0;
        while (!feof(LINKDBFILE)) {


                if (fread(&linkdbPost,sizeof(linkdbPost),1,LINKDBFILE) == 0) {
			perror("read");
			exit(1);
		}

		

		if (linkdbPost.DocID_to == DocID) {
			fseek(IPDBHA,linkdbPost.DocID_from * sizeof(ipadress),SEEK_SET);
                        fread(&ipadress,sizeof(ipadress),1,IPDBHA);
			addr.s_addr = ipadress;
			classaadress = ipadress & 0xffffff;

			prti = &ipadress;

			if (chtbl_lookup(&htbl,(void *)&prti) == 0) {
				//printf("set ip før\n");
			}
			else {
				//siden denne dataen skal ligge i hashen må vi ha et egetminne om råde på hippen for $
                                prti = malloc(sizeof(unsigned int));
				*prti = ipadress;
                                chtbl_insert(&htbl,(void *)prti);
				++unixipes;

			}


			prti = &classaadress;

			if (chtbl_lookup(&htbl_classa,(void *)&prti) == 0) {
				//printf("set ip før\n");
			}
			else {
				//siden denne dataen skal ligge i hashen må vi ha et egetminne om råde på hippen for $
                                prti = malloc(sizeof(unsigned int));
				*prti = classaadress;
                                chtbl_insert(&htbl_classa,(void *)prti);
				++unixclassa;

			}

			//printf("%u (%s) -> %u %u\n",linkdbPost.DocID_from,inet_ntoa(addr),linkdbPost.DocID_to,ipadress);
		}
		else {
			//printf("end %u\n",linkdbPost.DocID_to);
			//printf("last: %u -> %u\n",linkdbPost.DocID_from,linkdbPost.DocID_to);
			break;
		}

		++count;
	}
	chtbl_destroy(&htbl);
	chtbl_destroy(&htbl_classa);

	fclose(LINKDBFILE);
	fclose(INDEXFILE);
	fclose(IPDBHA);
	
	printf("%i links\nUnike IPer %i\nunix class A %i\n",count,unixipes,unixclassa);
}


int h(const void *key) {

        int ir;

        //int i2 = * (int *) p2
        unsigned int  *i  = (unsigned int  *) key;
        ir = *i % PRIME_TBLSIZ;
        return ir;

}



int match(const void *key1, const void *key2) {

        unsigned int  *i1  = (unsigned int  *) key1;
        unsigned int  *i2  = (unsigned int  *) key2;

        if (*i1 == *i2) {
                return 1;
        }
        else {
                return 0;
        }

}

