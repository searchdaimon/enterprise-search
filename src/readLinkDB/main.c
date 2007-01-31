#include <stdio.h>
#include "../common/define.h"
#include "../common/DocumentIndex.h"

int main (int argc, char *argv[]) {

	FILE *LINKDBFILE;

	struct linkdb_block
	{
    		unsigned int        DocID_from;
    		unsigned int        DocID_to;
	};


	struct linkdb_block linkdbPost;
	struct DocumentIndexFormat FromDocumentIndexPost;
	struct DocumentIndexFormat ToDocumentIndexPost;

	int FoundFromInfo;
	int FoundToInfo;

        if (argc < 2) {
                printf("Dette programet tar inn en linkdb fil og lager Brank\n\n\tUsage: ./BrankCalculate linkdb\n");
                exit(0);
        }

	if ((LINKDBFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }

	
	while (!feof(LINKDBFILE)) {
		fread(&linkdbPost,sizeof(linkdbPost),1,LINKDBFILE);
		//printf("%lu -> %lu\n",linkdbPost.DocID_from,linkdbPost.DocID_to);

		
		//på grun av bugs må vi filtrere ut nest DocID som var nestførst i forige  run
		if (linkdbPost.DocID_to == 5619738) {		
			FoundFromInfo = DIRead(&FromDocumentIndexPost,linkdbPost.DocID_from);

			FoundToInfo = DIRead(&ToDocumentIndexPost,linkdbPost.DocID_to);
	
			if ((FoundFromInfo) && (FoundToInfo)) {
				printf("%s (fip %lu) d: %u -> %s(tip %lu) D: %u\n",FromDocumentIndexPost.Url,FromDocumentIndexPost.IPAddress,linkdbPost.DocID_from,ToDocumentIndexPost.Url,ToDocumentIndexPost.IPAddress,linkdbPost.DocID_to);
			}
			else if (FoundFromInfo) {
				printf("%s -> %u\n",FromDocumentIndexPost.Url,linkdbPost.DocID_to);
			}
			else if (FoundToInfo) {
				printf("%u -> %s\n",linkdbPost.DocID_from,ToDocumentIndexPost.Url);
                	}
			else {
				printf("a %u -> %u\n",linkdbPost.DocID_from,linkdbPost.DocID_to);
			}
		}
	}	



	fclose(LINKDBFILE);
}

