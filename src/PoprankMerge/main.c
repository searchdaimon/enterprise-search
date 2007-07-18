#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>

#define mmap_MaxDocIDToAdd 300000000 //3 000 millioner

struct popmemmapFormat {
        int *ranks;
        off_t size;
        unsigned int largesDocID;
        int fd;
};

int popopenMmap(struct popmemmapFormat *popmemmap,char *filname) {

 	//int fd;
        struct stat inode;      // lager en struktur for fstat å returnere.
	int i;
	popmemmap->largesDocID = 0;

        if ((popmemmap->fd = open(filname, O_RDWR)) == -1) {
                perror(filname);
		return 0;
        }

        fstat(popmemmap->fd,&inode);

	popmemmap->size = inode.st_size;
	printf("old file size %"PRId64"\n",popmemmap->size);

        popmemmap->size += (sizeof(unsigned int) * mmap_MaxDocIDToAdd);
	printf("new file size %"PRId64"\n",popmemmap->size);

	#ifdef DEBUG

	#else
        /*
        Stretch the file size to the size of the (mmapped) array of ints
        */

        if (lseek(popmemmap->fd, popmemmap->size +1, SEEK_SET) == -1) {
                perror("Error calling lseek() to 'stretch' the file");
                exit(EXIT_FAILURE);
        }

        /* Something needs to be written at the end of the file to
        * have the file actually have the new size.
        * Just writing an empty string at the current file position will do.
        *
        * Note:
        *  - The current position in the file is at the end of the stretched
        *    file due to the call to lseek().
        *  - An empty string is actually a single '\0' character, so a zero-byte
        *    will be written at the last byte of the file.
        */
        if (write(popmemmap->fd, "", 1) != 1) {
                perror("Error writing last byte of the file");
                exit(EXIT_FAILURE);
        }
	#endif



        popmemmap->ranks = mmap(0,popmemmap->size,PROT_READ|PROT_WRITE,MAP_SHARED,popmemmap->fd,0);
	printf("mmap ret %"PRId64"\n",popmemmap->ranks);


       	if ((int)popmemmap->ranks == -1) {
       	        perror("mmap");
		return 0;
        }


	return 1;

}


void popcloseMmap (struct popmemmapFormat *popmemmap) {
	ftruncate(popmemmap->fd,(popmemmap->largesDocID * sizeof(unsigned int)) );

	munmap(popmemmap->ranks,popmemmap->size);

        close(popmemmap->fd);
	
}
int popRankForDocIDMmap(struct popmemmapFormat *popmemmap,unsigned int DocID) {
	//printf("DocID %i\n",DocID);
	//fohindrer at vi ber om en docid som er størrre en minne område og segfeiler.
	if ((DocID * sizeof(unsigned int)) < popmemmap->size) {
		return popmemmap->ranks[DocID];
	}
	else {
		return -1;
	}
}
int popRankForDocIDMmapSet(struct popmemmapFormat *popmemmap,unsigned int DocID,int increasement) {
	//printf("DocID %i\n",DocID);
	//fohindrer at vi ber om en docid som er størrre en minne område og segfeiler.

	off_t size = (DocID * sizeof(unsigned int));

	if (size < popmemmap->size) {
		popmemmap->ranks[DocID] += increasement;

		if (DocID > popmemmap->largesDocID) {
			popmemmap->largesDocID = DocID;
		}

		return popmemmap->ranks[DocID];

	}
	else {
		printf("DocID %u out of mmap bounds\n",DocID);
		return -1;

	}


}


int main (int argc, char *argv[]) {
	FILE *POPFILE;
	unsigned int DocID;
	unsigned int LastDocID = -1;
	int increasement = 0; 
	int i;
	//struct popl popha;
	struct popmemmapFormat popmemmap;

	if (argc < 3) {
		printf("No popfile given\n\nUsage:\n\t./PoprankMerge mainPopfile updatePopfil\n\n");
		exit(1);
	}

	char *mainpopfile = argv[1];

	if ((POPFILE = fopen(argv[2],"rb")) == NULL) {
		perror(argv[2]);
	}


	//popopen(&popha,"/home/boitho/config/popindex");
	if (!popopenMmap(&popmemmap,mainpopfile)) {
		printf("can't mmap\n");
		exit(1);
	}

	while (! feof(POPFILE)) {
	//for (i=0;i<10;i++) {
		fread(&DocID,sizeof(DocID),1,POPFILE);
		//printf("%i\n",DocID);
	
		if (DocID == LastDocID) {
			increasement++;
		}
		else {

			#ifdef DEBUG
			printf("setting DocID %u , increasement %i\n",LastDocID,increasement);
			#endif
			//popadd(&popha,LastDocID,increasement);		
			popRankForDocIDMmapSet(&popmemmap,DocID,increasement);

			increasement = 1;
			LastDocID = DocID;
		}
		//printf("%i\n",DocID);

	}

	//popclose(&popha);
	popcloseMmap(&popmemmap);
}




