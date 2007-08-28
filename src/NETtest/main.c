#include "../common/define.h"
#include "../common/reposetoryNET.h"
#include "../common/lot.h"
#include "../common/DocumentIndex.h"

#include <stdlib.h>
#include <string.h>




int
main(int argc, char **argv) {
	int LotNr;
	char lotPath[255];
	struct DocumentIndexFormat docindex;
	unsigned int DocID;
	char text[40];
	unsigned long int radress;

	// 125502594
	anchoraddnewNET("localhost", 125502594, "fii faa foo", strlen("fii faa foo"), "www");
	anchorReadNET("localhost", "www", 125502594, text, sizeof(text));

	
	return 0;
}
