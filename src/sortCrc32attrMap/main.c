#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../common/define.h"
#include "../common/lot.h"

static attr_crc32_words_block_compare(const void *a, const void *b)
{
    unsigned int         i=*((unsigned int*)a), j=*((unsigned int*)b);


    if (i>j) return +1;
    if (i<j) return -1;
    return 0;
}


main (int argc, char *argv[]) {

        if (argc < 2) {
                printf("Error ingen subna,e spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread www\n");
                exit(1);
        }

        char *subname = argv[1];
	FILE            *f_crc32_words = NULL;
	int             crc32_words_size = 0;
	struct stat     inode;
	int             attr_crc32_words_blocksize = sizeof(unsigned int) + sizeof(char)*MAX_ATTRIB_LEN;
	void            *m_crc32_words = NULL;

	if ((f_crc32_words = lotOpenFileNoCasheByLotNr(1, "crc32attr.map", "r+", 's', subname)) == NULL) {
		perror("Can't open thecrc32attr.map file for lot");
		exit(-1);
	}


     	fstat(fileno(f_crc32_words), &inode);
     	crc32_words_size = inode.st_size;

	if (crc32_words_size==0) {
		printf("Map is 0 bytes. Skipping\n");
	}

	if ((m_crc32_words=mmap(NULL, crc32_words_size, PROT_READ|PROT_WRITE, MAP_SHARED, fileno(f_crc32_words), 0)) == MAP_FAILED)
        {
		perror("Can't mmap");
	}


	printf("Will sort %d elements of sise %d\n",(crc32_words_size / attr_crc32_words_blocksize), attr_crc32_words_blocksize);

	qsort(m_crc32_words,(crc32_words_size / attr_crc32_words_blocksize),attr_crc32_words_blocksize, attr_crc32_words_block_compare);

	munmap(m_crc32_words,crc32_words_size);
	fclose(f_crc32_words);		

	printf("Done\n");
}
