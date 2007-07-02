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

#include "set.h"

typedef struct {
	int hits;
	set acl_allow;
	set acl_denied;
} dictcontent_t;

static unsigned int fileshashfromkey(void *ky)
{
    char *p = ky;

    return((int)p[0]);
}

static int filesequalkeys(void *k1, void *k2)
{
    char *c1, *c2;

    c1 = k1;
    c2 = k2;

    return (strcmp(c1, c2) == 0);
}

int dictionarywordLineSplit(char line[], char word[], unsigned int *nr, char *acl_allow, char *acl_denied) {
	int splits;
	char **data;

	if ((splits = split(line, " ", &data)) < 3)
		return 0;

	strcpy(word, data[0]);
	free(data[0]);
	*nr = atou(data[1]);
	free(data[1]);
	strcpy(acl_allow, data[2]);
	free(data[2]);
	if (splits == 4) {
		strcpy(acl_denied, data[3]);
		free(data[3]);
	} else {
		acl_denied[0] = '\0';
	}

	free(data);
	
	return 1;
}

int
add_acls(char *acl, set *s)
{
	char **acls;
	int i;

	if (split(acl, ",", &acls) == -1)
		return 0;
	for (i = 0; acls[i] != NULL; i++)
		set_add(s, acls[i]);
	free(acls);

	return 1;
}

int main (int argc, char *argv[]) {
	FILE *FH, *resultFH;
	char line[512];
	char word[maxWordlLen +1];
	unsigned int nr;
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

	while(fgets(line, sizeof(line), FH) != NULL) {
		char acl_allow[100], acl_denied[100];
		dictcontent_t *dc;

		chomp(line);
		//printf("line \"%s\"\n",line);

		if (!dictionarywordLineSplit(line, word, &nr, acl_allow, acl_denied)) {
			puts("Error");
			continue;
		}

		//printf("word \"%s\", nr %u\n",word,nr);

		if ((dc = hashtable_search(h, word)) == NULL) {
                        filesKey = strdup(word);
			dc = malloc(sizeof(*dc));
			dc->hits = 1;
			set_init(&dc->acl_allow);
			set_init(&dc->acl_denied);
			add_acls(acl_allow, &dc->acl_allow);
			add_acls(acl_denied, &dc->acl_denied);

                        if (!hashtable_insert(h, filesKey, dc)) {
                        	printf("cant insert\n");
                                exit(-1);
                        }

                }
                else {
			add_acls(acl_allow, &dc->acl_allow);
			add_acls(acl_denied, &dc->acl_denied);
			(dc->hits)++;
                }
	}

	fclose(FH);

	resultFH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords","w",'r',subname);

	if (hashtable_count(h) > 0) {
                itr = hashtable_iterator(h);
               	do {
			char *p;
			int i;

			dictcontent_t *dc;
                	filesKey = hashtable_iterator_key(itr);
                        dc = hashtable_iterator_value(itr);

                        //printf("\"%s\": %i\n",filesKey,dc->hits);
			fprintf(resultFH,"%s %u ",filesKey,dc->hits);
			//printf("acl allow:\n");
			SET_FOREACH(i, &dc->acl_allow, p) {
				if (i > 0)
					fprintf(resultFH, ",");
				fprintf(resultFH, "%s", p);
				//printf("\t%s\n", p);
			}
			fprintf(resultFH, " ");
			//printf("acl denied:\n");
			SET_FOREACH(i, &dc->acl_denied, p) {
				if (i > 0)
					fprintf(resultFH, ",");
				fprintf(resultFH, "%s", p);
				//printf("\t%s\n", p);
			}
			fprintf(resultFH, "\n");
                } while (hashtable_iterator_advance(itr));
                free(itr);
	}

	hashtable_destroy(h,1);

	fclose(resultFH);

	return 0;
}

