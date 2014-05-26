#include <stdlib.h>
#include <stdio.h>
#include "Linkdb.h"
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>

#include "../common/lot.h"
#include "../common/poprank.h"
#include "../common/ipdb.h"

//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

int main (int argc, char *argv[]) {

        FILE *LINKDBFILE;
        FILE *INDEXFILE;

	struct ipdbl ipdbha;

	int count;
        int ranged;
        struct linkdb_block linkdbPost;
	unsigned int DocID;
	off64_t offset;
        unsigned int lastDocID;
	unsigned int ipadress;

	struct popl popha;
	int rank;

        if (argc < 4) {
                printf("Dette programet tar inn en MainLinkDB, linkdbindex og DocID query får søk\n\n\tUsage: ./searchIndexLinkdb MainLinkDB linkdbindex DocID\n");
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

	popopen (&popha, "/home/boitho/config/popindex");
	ipdbOpen(&ipdbha);

	DocID = atol(argv[3]);

	printf("links ti %u:\n",DocID);

	rank =  popRankForDocID(&popha,DocID);
        ipadress = ipdbForDocID(&ipdbha,DocID);

	printf("rank %i, ipadress %u\n",rank,ipadress);

	//finner offset
	fseek(INDEXFILE,DocID * sizeof(offset),SEEK_SET);
	fread(&offset,sizeof(offset),1,INDEXFILE);
	printf("offset: %llu\n",offset);

	printf("offset: %" PRId64 "\n",offset);

	//søker til offsetten
	if(fseeko64(LINKDBFILE,offset,SEEK_SET) == -1) {
		perror("seek");
		exit(1);
	}

	printf("søkte til %" PRId64 "\n",ftello64(LINKDBFILE));

	count = 0;
        while (!feof(LINKDBFILE)) {


                if (fread(&linkdbPost,sizeof(linkdbPost),1,LINKDBFILE) == 0) {
			perror("read");
			exit(1);
		}

		rank =  popRankForDocID(&popha,linkdbPost.DocID_from);		
		ipadress = ipdbForDocID(&ipdbha,linkdbPost.DocID_from);

		if (linkdbPost.DocID_to == DocID) {
			printf("%u (r %i, ip %u)-> %u\n",linkdbPost.DocID_from,rank,ipadress,linkdbPost.DocID_to);
		}
		else {
			printf("end %u\n",linkdbPost.DocID_to);
			printf("last: %u -> %u\n",linkdbPost.DocID_from,linkdbPost.DocID_to);
			break;
		}

		++count;
	}

	popclose(&popha);

	ipdbClose(&ipdbha);

	fclose(LINKDBFILE);
	fclose(INDEXFILE);
	
	printf("%i links\n",count);
}
