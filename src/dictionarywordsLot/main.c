#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "../common/lot.h"
#include "../common/define.h"
#include "../common/stdlib.h"
#include "../common/bstr.h"


#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

static unsigned int fileshashfromkey(void *ky)
{
    char *k = (char *)ky;
    return((int)k[0]);
}

static int filesequalkeys(void *k1, void *k2)
{
    return (0 == strcmp(k1,k2));
}

int dictionarywordLineSplit(char line[], char word[], unsigned int *nr) {

	char *cp;

	if ((cp = strstr(line," ")) == NULL) {
		return 0;
	}		

	strcpy(word,line);
	word[cp - line] = '\0';

	cp = cp +1;
	
	(*nr) = atou(cp);	
}


int main (int argc, char *argv[]) {

	FILE *FH, *resultFH;

	char line[512];
	char word[maxWordlLen +1];
	unsigned int nr;
	unsigned int *filesValue;
	char *filesKey;
	struct hashtable *h;
	struct hashtable_itr *itr;
	
	if (argc != 3) {
		printf("usage: dictionarywordsLot lotnr subname\n");
		exit(1);
	}

	unsigned int lotNr = atou(argv[1]);
	char *subname = argv[2];

	FH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords_raw","r",'r',subname);

	h = create_hashtable(200, fileshashfromkey, filesequalkeys);

	while(fgets(line,sizeof(line),FH) != NULL) {
		chomp(line);
		//printf("line \"%s\"\n",line);

		if (!dictionarywordLineSplit(line, word, &nr)) {
			continue;
		}

		//printf("word \"%s\", nr %u\n",word,nr);


		if (NULL == (filesValue = hashtable_search(h,word) )) {
                	//printf("not found!. Vil insert first \"%s\"\n",word);
                        filesValue = malloc(sizeof(int));
                        (*filesValue) = 1;
                        filesKey = strdup(word);

                        if (! hashtable_insert(h,filesKey,filesValue) ) {
                        	printf("cant insert\n");
                                exit(-1);
                        }

                }
                else {
			++(*filesValue);
                }


	}

	fclose(FH);

	resultFH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords","w",'r',subname);

	if (hashtable_count(h) > 0) {


                itr = hashtable_iterator(h);

               	do {
                	filesKey = hashtable_iterator_key(itr);
                        filesValue = (unsigned int *)hashtable_iterator_value(itr);

                        printf("\"%s\": %i\n",filesKey,*filesValue);
			fprintf(resultFH,"%s %u\n",filesKey,*filesValue);

                } while (hashtable_iterator_advance(itr));
                free(itr);

	}

	hashtable_destroy(h,1);

	fclose(resultFH);
}

