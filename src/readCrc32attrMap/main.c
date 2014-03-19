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



int main (int argc, char *argv[]) {

        if (argc < 2) {
                printf("Error ingen subna,e spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread www\n");
                exit(1);
        }


        char *subname = argv[1];
	FILE            *f_crc32_words = NULL;
	int             crc32_words_size = 0;
	struct stat     inode;
	int             attr_crc32_words_blocksize = sizeof(unsigned int) + sizeof(char)*MAX_ATTRIB_LEN;
	//void            *m_crc32_words = NULL;
	struct Crc32attrMapFormat            *m_crc32_words = NULL;
	int i;

	if ((f_crc32_words = lotOpenFileNoCasheByLotNr(1, "crc32attr.map", "r", 's', subname)) == NULL) {
		perror("Can't open thecrc32attr.map file for lot");
		return -1;
	}


     	fstat(fileno(f_crc32_words), &inode);
     	crc32_words_size = inode.st_size;

	if (crc32_words_size==0) {
		printf("Map is 0 bytes. Skipping\n");
		return -1;
	}

	if ((m_crc32_words=mmap(NULL, crc32_words_size, PROT_READ, MAP_SHARED, fileno(f_crc32_words), 0)) == MAP_FAILED)
        {
		perror("Can't mmap");
		return -1;
	}


	printf("Hvae %d elements of sise %d\n",(crc32_words_size / attr_crc32_words_blocksize), attr_crc32_words_blocksize);

	for(i=0;i<(crc32_words_size / attr_crc32_words_blocksize);i++) {
		printf("crc32 %u, text %s\n",m_crc32_words[i].crc32, m_crc32_words[i].text);
	}

	munmap(m_crc32_words,crc32_words_size);
	fclose(f_crc32_words);		


	return 1;
}
