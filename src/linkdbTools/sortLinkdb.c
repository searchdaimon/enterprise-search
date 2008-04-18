#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../common/mgsort.h"

	// 8 bytes
	struct linkdb_block
        {
                unsigned int        DocID_from;
                unsigned int        DocID_to;
        };

int compare_elements (const void *p1, const void *p2);

int main (int argc, char *argv[]) {

	FILE *OLDLINKDBFILE;
	FILE *NEWLINKDBFILE;


	struct stat inode;      // lager en struktur for fstat å returnere.

	struct linkdb_block linkdbPost;
	struct linkdb_block *linkdbArray;
	int i,y;

	//printf("block size %i\n",sizeof(struct linkdb_block));

        if (argc < 3) {
                printf("Dette programet tar inn en linkdb fil og sorterer den.\n\n\tsortLinkdb old new\n\n");
                exit(0);
        }

	printf("sort %s -> %s\n",argv[1],argv[2]);

	if ((OLDLINKDBFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }


	//kontrolerer at vi ikke overskriver en fil
	if ((NEWLINKDBFILE = fopen(argv[2],"rb")) != NULL) {
		printf("New file exsist. It shud not!\n");
		exit(1);
	}
	if ((NEWLINKDBFILE = fopen(argv[2],"wb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[2]);
                exit(1);
        }

	fstat(fileno(OLDLINKDBFILE),&inode);	
	if ((linkdbArray = malloc(inode.st_size)) == NULL) {
		perror("malloc");
		exit(1);
	}

	i =0;
	while (!feof(OLDLINKDBFILE)) {
		fread(&linkdbArray[i],sizeof(linkdbPost),1,OLDLINKDBFILE);
		//printf("%lu -> %lu\n",linkdbPost.DocID_from,linkdbPost.DocID_to);

		++i;	
	}	

	//qsort(linkdbArray, i , sizeof(struct linkdb_block), compare_elements);
	mgsort(linkdbArray, i , sizeof(struct linkdb_block), compare_elements);

	for(y=0;y<i;y++) {
		fwrite(&linkdbArray[y],sizeof(linkdbPost),1,NEWLINKDBFILE);
	}

	fclose(OLDLINKDBFILE);
	fclose(NEWLINKDBFILE);
}

int compare_elements (const void *p1, const void *p2) {


        //return i1 - i2;
        struct linkdb_block *t1 = (struct linkdb_block*)p1;
       	struct linkdb_block *t2 = (struct linkdb_block*)p2;

	if (t2->DocID_to == t1->DocID_to) {
        	if (t2->DocID_from > t1->DocID_from) {
        	        return -1;
		}
        	else {
        	        return t2->DocID_from < t1->DocID_from;
		}

	} else {
        	if (t2->DocID_to > t1->DocID_to) {
        	        return -1;
		}
        	else {
        	        return t2->DocID_to < t1->DocID_to;
		}
	}
}


