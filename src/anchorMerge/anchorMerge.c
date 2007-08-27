#include <stdio.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/reposetory.h"

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"


struct values {
	char *text;
	size_t len;
	size_t curlen;
};


void
alloc_values(struct values *val, int textlen)
{
	size_t newlen;

	newlen = val->len;
	if (textlen + val->curlen >=  val->len) {
		char *val2 = val->text;

		newlen *= 2;
		while (textlen + val->curlen >=  newlen)
			newlen *= 2;
		if ((val->text = malloc(newlen)) == NULL)
			err(1, "malloc(foo)");
		strcpy(val->text, val2);
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
	}

	*val = values;
}


int
main(int argc, char **argv)
{
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
	char anchorPath[512], anchorPath2[512];
	int i;

        //tester for at vi har fåt hvilken lot vi skal bruke
        if (argc < 3)
                errx(1, "Usage: ./anchorread lotnr subname\n\n");

	LotNr = atoi(argv[1]);
	subname = argv[2];

	firstId = LotDocIDOfset(LotNr);

	setup_values(&values);

	i = 0;
	while (anchorGetNext(LotNr, &DocID, text, sizeof(text), &raddress, &rsize,
	                     subname)) {
		int len;
		char *newvalue;

		//if (DocID == 63741334 || DocID == 63740388)
		//	printf("%d: %s\n", DocID, text);

		c = DocID - firstId;
		if (c > NrofDocIDsInLot-1) {
			//printf("This thing should not be here... %u\n", DocID);
			//i++;
			continue;
		}

		len = strlen(text);

		alloc_values(&values[c], len+1);

		if (values[c].curlen == 0) {
			strcpy(values[c].text, text);
			values[c].curlen = len + 1;
		} else {
			char *p;

			p = values[c].text;
			p += values[c].curlen - 1;

			strcpy(p, " ");
			p++;
			strcpy(p, text);
			values[c].curlen += len + 1;
		}
	}

#if 1

	//printf("%d\n", i);
#if 1
	i = 0;
	for (d = 0; d < NrofDocIDsInLot; d++) {
		if (values[d].curlen > 0) {
			unsigned int DocID;

			DocID = firstId + d;
			anchoraddnew(DocID, values[d].text, values[d].curlen-1, subname, "anchors.new");
			i++;
		}
	}
#endif
	printf("%d\n", i);
#endif

#if 0
	GetFilPathForLot(anchorPath, LotNr, subname);

	strcpy(anchorPath2, anchorPath);
	strcat(anchorPath, "anchor.new");
	strcat(anchorPath2, "anchors");

	rename(anchorPath, anchorPath2);
#endif

	lotCloseFiles();

	return 0;
}
