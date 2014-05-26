#include <stdlib.h>
#include <stdio.h>
#include "Linkdb.h"

#include "../common/lot.h"
#include "../common/define.h"

//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE


int main (int argc, char *argv[]) {

	FILE *LINKDBFILE;
	FILE *INDEXFILE;

	unsigned int ranged;
	struct linkdb_block linkdbPost;
	off64_t offset;
	int lastLotNr, lotNr = -1;


	unsigned int lastDocID;


        if (argc < 3) {
                printf("Dette programet tar inn en linkdb fil og gjør den søkbar\n\n\tUsage: ./BrankCalculate linkdb indexfile\n");
                exit(0);
        }

	if ((LINKDBFILE = (FILE *)fopen64(argv[1],"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }
	if ((INDEXFILE = (FILE *)fopen64(argv[2],"wb")) == NULL) {
                printf("Cant read index ");
                perror(argv[2]);
                exit(1);
        }

	ranged = 0;
	lastDocID = 0;
	while (!feof(LINKDBFILE)) {


			fread(&linkdbPost,sizeof(linkdbPost),1,LINKDBFILE);


				//
				lotNr = rLotForDOCid(linkdbPost.DocID_to);
				if (lastLotNr != lotNr) {
					printf("%i\n",lotNr);
				}
				lastLotNr = lotNr;



			if (linkdbPost.DocID_to != lastDocID) {
				//printf("\nnew\n");

				//tar vare på ofsett // -sizeof(offset) da vi skal ha starten. Vi har jo allerede lest en
				//offset = (ftello64(LINKDBFILE) - sizeof(linkdbPost));
				offset = ftello64(LINKDBFILE);
				//printf("offset %li\n",offset);
				//søker oss til riktig plass
				fseeko64(INDEXFILE,linkdbPost.DocID_to * sizeof(offset),SEEK_SET);
				//for så å skrive dette til fil
				fwrite(&offset,sizeof(offset),1,INDEXFILE);

			}

			//printf("%u -> %u\n",linkdbPost.DocID_from,linkdbPost.DocID_to);

			
			lastDocID = linkdbPost.DocID_to;

			//if (ranged > 500) {
			//	break;
			//}

		++ranged;

	}	



	fclose(LINKDBFILE);
	fclose(INDEXFILE);
	printf("Rangerte %lu linker\n",ranged);
}

