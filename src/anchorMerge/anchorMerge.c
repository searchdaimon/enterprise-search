#include <stdio.h>
#include <err.h>
#include <string.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/reposetory.h"

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"



static unsigned int ip_hashfromkey(void *ky)
{
	unsigned int k = (*(unsigned int *)ky);

	return k;
}

static int ip_equalkeys(void *k1, void *k2)
{
	if ((*(unsigned int *)k1) == (*(unsigned int *)k2)) {
		return 1;
	} else {
		return 0;
	}
}

struct hashvalue {
	char *text;
};

struct values {
	char *text;
	char *p;
	size_t len;
	size_t curlen;
};


void
alloc_values(struct values *val, int textlen)
{
	size_t newlen;

	newlen = val->len;
	if (textlen + val->curlen + 1 >=  val->len) {
		char *val2 = val->text;

		newlen *= 2;
		while (textlen + val->curlen + 1 >=  newlen)
			newlen *= 2;
		if ((val->text = malloc(newlen)) == NULL)
			err(1, "malloc(foo)");
		strcpy(val->text, val2);
		val->p = val->p - val2 + val->text;
		free(val2);
		val->len = newlen;
	}
}

void
setup_values(struct values **val)
{
	struct values *values;
	int i;

	values = malloc(sizeof(struct values) * NrofDocIDsInLot);
	if (values == NULL)
		err(1, "malloc(values)");
	for (i = 0; i < NrofDocIDsInLot; i++) {
		int size = 1024;

		values[i].len = size;
		values[i].curlen = 0;
		if ((values[i].text = malloc(size)) == NULL)
			err(1, "malloc(text size)");

		values[i].p = values[i].text;
	}

	*val = values;
}


int main (int argc, char **argv) {

	int LotNr;
	char *subname;
	unsigned int DocID;
	char text[1024];
	unsigned int raddress;
	unsigned int rsize;
	struct hashtable *hash;
	struct values *values;
	unsigned int firstId = 0;
	unsigned int c, d;
	struct DocumentIndexFormat DocumentIndexPost;
	int i;

        //tester for at vi har fåt hvilken lot vi skal bruke
        if (argc < 3)
                errx(1, "Usage: ./anchorread lotnr subname\n\n");

	LotNr = atoi(argv[1]);
	subname = argv[2];

	//hash = create_hashtable(1000100, ip_hashfromkey, ip_equalkeys);

	DIGetNext (&DocumentIndexPost, LotNr, &DocID, subname);
	firstId = DocID;

	setup_values(&values);

	i = 0;
	while (anchorGetNext(LotNr, &DocID, text, sizeof(text), &raddress, &rsize,
	                     subname)) {
		int len;
		char *newvalue;

		if (DocID == 63741334 || DocID == 63740388)
			printf("%d: %s\n", DocID, text);

		c = DocID - firstId;
		if (c > NrofDocIDsInLot-1) {
			//printf("This thing should not be here... %u\n", DocID);
			i++;
			continue;
		}

		len = strlen(text);

		alloc_values(&values[c], len);

		if (values[c].curlen > 0) {
			strcpy(values[c].p, " ");
			values[c].p++;
		}

		strcpy(values[c].p, text);
		values[c].p++;
		values[c].curlen += 1 + len;

#if 0
		struct hashvalue *hv;

		if ((hv = hashtable_search(hash, &DocID)) == NULL) {
			unsigned int *did;

			if ((did = malloc(sizeof *did)) == NULL)
				err(1, "malloc(sizeof *did)");
			*did = DocID;

			if ((hv = malloc(sizeof *hv)) == NULL)
				err(1, "malloc(hv)");

			if ((hv->text = strdup(text)) == NULL)
				err(1, "strdup(text)");

			if (!hashtable_insert(hash, did, hv))
				err(1, "hashtable_insert()");
		} else {
			char *newvalue;
			char *value;

			value = hv->text;

			newvalue = malloc(strlen(value) + strlen(text) + 2);
			if (newvalue == NULL)
				err(1, "malloc(newvalue)");
			sprintf(newvalue, "%s %s", value, text);

			hv->text = newvalue;
			free(value);
		}
#endif
	}

	//hashtable_destroy(hash, 1);
#if 1

	printf("%d\n", i);
#if 1
	i = 0;
	for (d = 0; d < NrofDocIDsInLot; d++) {
		if (values[d].curlen > 0)
			i++;
			//printf("DocID: %d: %s\n", firstId + d, values[d].text);
	}
#endif
	printf("%d\n", i);
#endif



	return 0;
}
