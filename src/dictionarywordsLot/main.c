#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>

#include "../common/lot.h"
#include "../common/define.h"
#include "../common/stdlib.h"
#include "../common/bstr.h"
#include "../common/boithohome.h"


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
	int a, i;

	if (split(acl, ",", &acls) == -1)
		return 0;
	for (i = 0; acls[i] != NULL; i++) {
		if (strcmp(acls[i], "") == 0) {
			free(acls[i]);
			continue;
		}
		if (set_add(s, acls[i]) == 2)
			free(acls[i]);
	}
	free(acls);

	return 1;
}

void
dolot(unsigned int lotNr, char *subname, struct hashtable *h)
{
	FILE *FH;
	char line[512];
	char word[maxWordlLen +1];
	char *filesKey;
	unsigned int nr;

	FH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords_raw","r",'r',subname);
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
			dc->hits = nr;
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
			dc->hits += nr;
                }
	}

	fclose(FH);


}

int main (int argc, char *argv[]) {
	FILE *resultFH;
	char *filesKey;
	struct hashtable *h;
	struct hashtable_itr *itr;
	int all = 0;

	if (argc >= 2 && strcmp(argv[1], "all") == 0) {
		all = 1;
	} else if (argc != 3) {
		printf("usage: dictionarywordsLot lotnr subname\n");
		printf("usage: dictionarywordsLot all\n");
		exit(1);
	}

	h = create_hashtable(200, fileshashfromkey, filesequalkeys);
	if (all == 0) {
		unsigned int lotNr = atou(argv[1]);
		char *subname = argv[2];

		dolot(lotNr, subname, h);
	} else {
		DIR *d;
		struct dirent *de;
		char pathname[PATH_MAX];

		if ((d = opendir(bfile("lot/"))) == NULL)
			err(1, "opendir(lot/)");

		while ((de = readdir(d))) {
			DIR *d2;
			struct dirent *de2;

			if (de->d_name[0] == '.' || !isdigit(de->d_name[0]))
				continue;

			sprintf(pathname, "%s/%s", bfile("lot"), de->d_name);
			if ((d2 = opendir(pathname)) == NULL)
				err(1, "opendir(lot/x/)");

			while ((de2 = readdir(d2))) {
				DIR *d3;
				struct dirent *de3;

				if (de2->d_name[0] == '.' || !isdigit(de2->d_name[0]))
					continue;

				sprintf(pathname, "%s/%s/%s", bfile("lot"), de->d_name, de2->d_name);

				if ((d3 = opendir(pathname)) == NULL)
					err(1, "opendir(lot/x/y/)");

				while ((de3 = readdir(d3))) {
					FILE *tmpfh;

					if (de3->d_name[0] == '.')
						continue;

					sprintf(pathname, "%s/%s/%s/%s/dictionarywords_raw",
					    bfile("lot"), de->d_name, de2->d_name, de3->d_name);
					printf("found dictionary: %s\n", pathname);
					/* XXX: Use stat(2) instead? */
					if ((tmpfh = fopen(pathname, "r")) != NULL) {
						fclose(tmpfh);
						dolot(atoi(de2->d_name), de3->d_name, h);
					}
				}
				closedir(d3);
			}
			closedir(d2);
		}
		closedir(d);
	}

	//resultFH = lotOpenFileNoCasheByLotNr(lotNr,"dictionarywords","w",'r',subname);
	resultFH = fopen(bfile("var/dictionarywords"), "w");

	if (hashtable_count(h) > 0) {
		printf("Writing %d words.\n", hashtable_count(h));
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
			set_free_all(&dc->acl_allow);
			set_free_all(&dc->acl_denied);
                } while (hashtable_iterator_advance(itr));
                free(itr);
	}

	hashtable_destroy(h,1);

	fclose(resultFH);

	return 0;
}

