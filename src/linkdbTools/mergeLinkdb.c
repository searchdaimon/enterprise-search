#include <stdio.h>
#include <stdlib.h>

        struct linkdb_block
        {
                unsigned int        DocID_from;
                unsigned int        DocID_to;
        };

        struct filesFormat {
                struct linkdb_block lastlink;
                int eof;
                FILE *FH;
		int filenr;
        };

int compare_elements_eof (const void *p1, const void *p2);
int compare_elements (const void *p1, const void *p2);

int main (int argc, char *argv[]) {

	FILE *MAINLINKDBFILE;
	int i;
	int nrOffiles, nrOffOpenfiles;
	unsigned int current;

	struct filesFormat files[256];

        if (argc < 3) {
                printf("Dette programet tar inn N linkdb filer og sorterer de.\n\n\tmergeLinkdb MainLinkDB linkdb .. N\n\n");
                exit(0);
        }

	//kontrolerer at vi ikke overskriver en fil
       	if ((MAINLINKDBFILE = fopen(argv[1],"rb")) != NULL) {
                printf("New file exsist. It shud not!\n");
                exit(1);
       	}
       	if ((MAINLINKDBFILE = fopen(argv[1],"wb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
       	}

	printf("argc %i\n",argc);

	nrOffiles =0;
	for (i=2;i<argc;i++) {

		files[nrOffiles].eof =0;
		files[nrOffiles].filenr = nrOffiles;

		if ((files[nrOffiles].FH = fopen(argv[i],"rb")) == NULL) {
                	perror(argv[i]);
                	exit(1);
        	}

		//leser første post
		fread(&files[nrOffiles].lastlink,sizeof(struct linkdb_block),1,files[nrOffiles].FH);

		printf("merge %s hav link to %u\n",argv[i],files[nrOffiles].lastlink.DocID_to);

		++nrOffiles;
	}

	nrOffOpenfiles = nrOffiles;
	
	
	while(nrOffOpenfiles != 0) {

		qsort(files, nrOffOpenfiles , sizeof(struct filesFormat), compare_elements);		
		
		current = files[0].lastlink.DocID_to;

		//fortsetter så lenge vi har samme DocID
		i=0;
		while ((current == files[i].lastlink.DocID_to) && (i<nrOffOpenfiles)) {
			//printf("i: %i write %u\n",i,files[i].lastlink.DocID_to);

			fwrite(&files[i].lastlink,sizeof(struct linkdb_block),1,MAINLINKDBFILE);

			fread(&files[i].lastlink,sizeof(struct linkdb_block),1,files[i].FH);

			if (feof(files[i].FH)) {
				files[i].eof = 1;
				printf("eof %i. File nr %i\n",i,files[i].filenr);

				qsort(files, nrOffOpenfiles , sizeof(struct filesFormat), compare_elements_eof);


				--nrOffOpenfiles;
			}

			i++;	
		}

	}
	
}


int compare_elements_eof (const void *p1, const void *p2) {


        //return i1 - i2;
        struct filesFormat *t1 = (struct filesFormat*)p1;
        struct filesFormat *t2 = (struct filesFormat*)p2;

        if (t2->eof > t1->eof)
                return -1;
        else
                return t2->eof < t1->eof;

}


int compare_elements (const void *p1, const void *p2) {


        //return i1 - i2;
        struct filesFormat *t1 = (struct filesFormat*)p1;
        struct filesFormat *t2 = (struct filesFormat*)p2;

        if (t2->lastlink.DocID_to > t1->lastlink.DocID_to)
                return -1;
        else
                return t2->lastlink.DocID_to < t1->lastlink.DocID_to;

}

