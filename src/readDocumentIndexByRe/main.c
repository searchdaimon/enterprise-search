
#include "../common/re.h"



int main() {
	struct reformat *re;

	if ((re = reopen(1, sizeof(struct DocumentIndexFormat), "bar", "www", RE_COPYONCLOSE)) == NULL) {
		perror("reopen");
		exit(-1);
	}

	RE_DocumentIndex(re, 3)->htmlSize = 3;

	strcpy( RE_DocumentIndex(re, 3)->Url, "aaa" );
	strcpy( RE_DocumentIndex(re,10)->Url, "bbb" );


	reclose(re);
}
