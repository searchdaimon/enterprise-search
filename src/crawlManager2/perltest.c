#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "perlcrawl.h"
#include "../crawl/crawl.h"

int documentContinue(struct collectionFormat *collection) {


	return 1;
}

int documentExist(struct collectionFormat *collection, struct crawldocumentExistFormat *crawldocumentExist) {

	printf("documentExist: uri %s\n",crawldocumentExist->documenturi );

	return 0;
}

int documentAdd(struct collectionFormat *collection, struct crawldocumentAddFormat *crawldocumentAdd) {

	static unsigned int DocID = 0;

	char filename[512];
	FILE *FH;

	printf("documentAdd: uri %s, title %s\n",crawldocumentAdd->documenturi,crawldocumentAdd->title);

	sprintf(filename,"%s_%u",collection->test_file_prefix,DocID);
	
	if ((FH = fopen(filename,"w")) == NULL) {
		perror(filename);
		return 0;
	}

	fprintf(FH,"uri: %s\n",crawldocumentAdd->documenturi);
	fprintf(FH,"title: %s\n",crawldocumentAdd->title);
	fprintf(FH,"doctype: %s\n",crawldocumentAdd->doctype);
	fprintf(FH,"Type: %s\n",crawldocumentAdd->documenttype);
	fprintf(FH,"acl_allow: %s\n",crawldocumentAdd->acl_allow);
	fprintf(FH,"acl_denied: %s\n",crawldocumentAdd->acl_denied);
	fprintf(FH,"\n\n");
	fprintf(FH,"Document: %s\n",crawldocumentAdd->document);

	fclose(FH);
	
	++DocID;

}

int documentError(int level, const char *fmt, ...) {



        va_list     ap;

        va_start(ap, fmt);

        printf("documentError: ");
        vprintf(fmt,ap);


        va_end(ap);

}

void usage() {
	printf("usage:\n");
	printf("\tcrawlManager2perltest folder prefix\n\n");
	printf("\n");
	printf("Options:\n");
	printf("\t-u user\n");
	printf("\t-p password\n");
	printf("\t-r resource\n");
	printf("Folder must have the main perl file named main.pm\n");
}

int main(int argc, char *argv[]) {


	struct crawlLibInfoFormat *crawlLibInfo;
	struct collectionFormat collection;
        FILE *FH;
        struct stat inode;      // lager en struktur for fstat å returnere.

	collectionReset (&collection);

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"p:u:r:"))!=-1) {
		switch (c) {
                        case 'p':
                                collection.password = optarg;
                                break;
                        case 'u':
                                collection.user = optarg;
                                break;
                        case 'r':
                                collection.resource = optarg;
                                break;
                        default:
				usage();
                                exit(-1);
                }

        }
        --optind;

	if (argc < 3) {
		usage();
		exit(-1);
	}

	char *perlfile = argv[1 +optind];
	collection.test_file_prefix = argv[2 +optind];

	crawlLibInfo = perlCrawlStart(perlfile,"");


	collection.crawlLibInfo = crawlLibInfo;

	if (!(*(*crawlLibInfo).crawlfirst)(&collection,documentExist,documentAdd,documentError,documentContinue)) {
                printf("problems in crawlfirst_ld\n");
                //overfører error
                printf("Error: %s\n",(*crawlLibInfo).strcrawlError());

                return 0;
        }

}
