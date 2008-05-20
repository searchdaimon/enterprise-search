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
	fprintf(FH,"Document:\n\n%s\n",crawldocumentAdd->document);

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
	printf("\tcrawlManager2perltest lib.pm prefix\n\n");
}

int main(int argc, char *argv[]) {


	struct crawlLibInfoFormat *crawlLibInfo;
	struct collectionFormat collection;
        FILE *FH;
        struct stat inode;      // lager en struktur for fstat å returnere.

	crawlLibInfo = perlCrawlStart();

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"p:"))!=-1) {
		switch (c) {
                        case 'p':
                                //optMax = atoi(optarg);
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


        if ((FH = fopen(perlfile,"r"))== NULL) {
		perror(perlfile);
		exit(-1);
	}

        fstat(fileno(FH),&inode);

        collection.perlcode = malloc(inode.st_size +1);
        fread(collection.perlcode,inode.st_size,1,FH);
        collection.perlcode[inode.st_size] = '\0';;

        fclose(FH);



	if (!(*(*crawlLibInfo).crawlfirst)(&collection,documentExist,documentAdd,documentError,documentContinue)) {
                printf("problems in crawlfirst_ld\n");
                //overfører error
                printf("Error: %s\n",(*crawlLibInfo).strcrawlError());

                return 0;
        }

}
