#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "../common/re.h"
#include "../common/define.h"
#include "../common/langToNr.h"
#include "../common/stdlib.h"
#include "../common/url.h"
#include "../common/lot.h"
#include "../common/ht.h"

#include "../3pLibs/keyValueHash/hashtable.h"



int main (int argc, char *argv[]) {
	struct reformat *redi;
	int i, y;
        int LotNr;
        unsigned int DocID;
	FILE *f_crc32_words;
	struct hashtable *h;
	char *atts; 
	struct stat inode;
	int colloms;

        if (argc < 3) {
                printf("Program to read attributeIndex\n\n\tUsage: ./readAttributeIndex lotnr subname\n");
               exit(0);
        }



        LotNr = atoi(argv[1]);
        char *subname = argv[2];
        DocID = 0;


	if (stat(returnFilPathForLotFile(LotNr, subname, "attributeIndex"), &inode) != 0) {
		printf("Cant stat attributeIndex\n");
		return -1;
	}

	colloms = (inode.st_size / NrofDocIDsInLot) / 4;
	printf("aaa colloms %i\n",colloms);

	if ((redi = reopen(LotNr, sizeof(unsigned int) * colloms, "attributeIndex", subname, RE_READ_ONLY)) == NULL) {
		perror("reopen");
		exit(-1);
	}

	h = create_hashtable(16, ht_integerhash, ht_integercmp);
	if (NULL == h) exit(-1); /*oom*/


	f_crc32_words = lotOpenFileNoCasheByLotNr(1, "crc32attr.map", "r", 's', subname);

	char            f_value[MAX_ATTRIB_LEN];
	unsigned int	f_crc32;

	while (fread(&f_crc32,sizeof(f_crc32),1,f_crc32_words) && fread(&f_value,sizeof(f_value),1,f_crc32_words)) {
		printf("%u, %s\n", f_crc32,f_value);
		if (hashtable_insert(h, uinttouintp(f_crc32), strdup(f_value)) == 0) {
			printf("Can't insert key\n");
			exit(-1);
		}
	}


	DocID = LotDocIDOfset(LotNr);
        unsigned int **att;
	unsigned int it;
	for (i=0;i<NrofDocIDsInLot;i++) {



		printf("DocID %u:\n", DocID);

		att = renget(redi, i);

		for (y=0;y<colloms;y++) {

			it = (unsigned int)att[y];
		   	printf("\t%u " ,it);

			atts = hashtable_search(h, &it);
			if (atts != NULL) {
				printf("atts (%i) %s", y,atts);
			}

			printf("\n");
		}

		printf("\n");

		++DocID;
	}


	reclose(redi);
}
