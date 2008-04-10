#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "../common/define.h"
#include "../3pLibs/keyValueHash/hashtable.h"

static unsigned int fileshashfromkey(void *ky)
{

        return(*(unsigned int *)ky);
}

static int filesequalkeys(void *k1, void *k2)
{
	

    return (*(unsigned int *)k1 == *(unsigned int *)k2);
}

int main (int argc, char *argv[]) {

	FILE *LINKDBFILE;



	struct linkdb_block linkdbPost;
	struct DocumentIndexFormat FromDocumentIndexPost;
	struct DocumentIndexFormat ToDocumentIndexPost;
	struct hashtable *h;

	int FoundFromInfo;
	int FoundToInfo;
	off_t count;
	unsigned int last;	
        unsigned int *filesKey;
        off_t *filesValue;

        if (argc < 2) {
                printf("Dette programet tar inn en linkdb fil og lager Brank\n\n\tUsage: ./BrankCalculate linkdb\n");
                exit(0);
        }

	if ((LINKDBFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }

	h = create_hashtable(200, fileshashfromkey, filesequalkeys);

	count = 0;
	last = 0;
	while (!feof(LINKDBFILE)) {
		fread(&linkdbPost,sizeof(linkdbPost),1,LINKDBFILE);

		if (last != linkdbPost.DocID_to) {
			#ifdef DEBUG
			printf("last\n");
			#endif
			if (NULL == (filesValue = hashtable_search(h,&last) )) {
				#ifdef DEBUG
                                printf("filtyper: not found!. Vil insert first\n");
				#endif
                                filesValue = malloc(sizeof(int));
                                (*filesValue) = count;
                                filesKey = malloc(sizeof(int));
				(*filesKey) = last;
                                if (! hashtable_insert(h,filesKey,filesValue) ) {
                                        printf("cant insert\n");
                                        exit(-1);
                                }

                        }
			else {
				printf("have seen this DocID %u before at line %"PRId64". File is corupt!\n",last,(*filesValue));
			}
		}

//		printf("%"PRId64": %u -> %u\n",count,linkdbPost.DocID_from,linkdbPost.DocID_to);

		last = linkdbPost.DocID_to;

		++count;
	}	



	fclose(LINKDBFILE);
}

