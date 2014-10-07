#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../common/define.h"
#include "../common/lot.h"

int attr_crc32_words_block_compare(const void *a, const void *b)
{
    unsigned int         i=*((unsigned int*)a), j=*((unsigned int*)b);


    if (i>j) return +1;
    if (i<j) return -1;
    return 0;
}


int main (int argc, char *argv[]) {

        if (argc < 2) {
                printf("Program to sort a crc32attr.map\n\nUsage:\n\t./sortCrc32attrMap subname\n");
                exit(1);
        }

        char 				*subname = argv[1];
	FILE            		*f_crc32_words = NULL, *f_crc32_words_new = NULL;
	int             		crc32_words_size = 0;
	struct stat     		inode;
	int             		attr_crc32_words_blocksize = sizeof(unsigned int) + sizeof(char)*MAX_ATTRIB_LEN;
	struct Crc32attrMapFormat	*m_crc32_words = NULL;
	unsigned int			last;
	int 				i;

	if ((f_crc32_words = lotOpenFileNoCasheByLotNr(1, "crc32attr.map", "r+", 's', subname)) == NULL) {
		perror("Can't open the crc32attr.map file.");
		return -1;
	}


     	fstat(fileno(f_crc32_words), &inode);
     	crc32_words_size = inode.st_size;

	if (crc32_words_size==0) {
		printf("crc32attr.map is 0 bytes. Skipping\n");
		return 0;
	}

	if ((m_crc32_words=mmap(NULL, crc32_words_size, PROT_READ|PROT_WRITE, MAP_SHARED, fileno(f_crc32_words), 0)) == MAP_FAILED)
        {
		perror("Can't mmap");
		return -1;
	}


	printf("Will sort %d elements of sise %d\n",(crc32_words_size / attr_crc32_words_blocksize), attr_crc32_words_blocksize);

	qsort(m_crc32_words,(crc32_words_size / attr_crc32_words_blocksize),attr_crc32_words_blocksize, attr_crc32_words_block_compare);


	/************************************************************************************
	 Now when we have it sorted we will print out only uniq elements in a new file. 
	************************************************************************************/
	if ((f_crc32_words_new = lotOpenFileNoCasheByLotNr(1, "crc32attr.map.new", "wb", 'e', subname)) == NULL) {
		perror("Can't open thecrc32attr.map.new file for lot");
		return -1;
	}

	last = 0;
        for(i=0;i<(crc32_words_size / attr_crc32_words_blocksize);i++) {

		if (m_crc32_words[i].crc32 != last) {
			#ifdef DEBUG
                	printf("crc32 %u, text %s\n",m_crc32_words[i].crc32, m_crc32_words[i].text);
			#endif

			if (fwrite(&m_crc32_words[i], sizeof(struct Crc32attrMapFormat), 1, f_crc32_words_new) != 1) {
				perror("fwrite crc32attr.map.new");
				return -1;
			}
		}

		last = m_crc32_words[i].crc32;
        }


	munmap(m_crc32_words,crc32_words_size);

	// Swap the files
	if (lotRename(1, subname, "crc32attr.map.new", "crc32attr.map") != 0) {
		perror("rename");
		return -1;
	}

	fclose(f_crc32_words_new);
	fclose(f_crc32_words);		

	printf("Done\n");

	return 0;
}
