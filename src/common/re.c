

#include "lot.h"
#include "define.h"
#include "re.h"


void _filecpy(int into, int from) {

	char buf[4096];
	size_t n = 0;
	
	printf("copying file...\n");
	while((n = read(from,buf,sizeof(buf))) > 1) {
		write(into,buf,n);
	}
	printf("..done.\n");
}

struct reformat *reopen(int lotNr, size_t structsize, char file[], char subname[], int flags) {

	struct stat inode;
	struct reformat *re;

	printf("reopen(lotNr=%i, structsize=%d, file=%s, subname=%s, flags=%i)\n",lotNr,structsize,file,subname,flags);

	if ((re = malloc(sizeof(struct reformat))) == NULL) {
		return NULL;
	}

	re->flags = flags;

	if ((re->flags & RE_HAVE_4_BYTES_VERSION_PREFIX) == RE_HAVE_4_BYTES_VERSION_PREFIX) {
		#ifdef DEBUG
		printf("reopen: have RE_HAVE_4_BYTES_VERSION_PREFIX\n");
		#endif
		structsize += 4;
	}

	re->lotNr = lotNr;
	re->structsize = structsize;
	re->maxsize = re->structsize * NrofDocIDsInLot;
	
	strcpy(re->mainfile,file);
	strcpy(re->subname,subname);

	//lager en privat kopi, som vil erstatte orginalen når vi lokker filen.
	if ((re->flags & RE_COPYONCLOSE) == RE_COPYONCLOSE) {
		printf("Making private copy of file \"%s\"\n",file);
		//åpner en temperer fil
		sprintf(re->tmpfile,"%s_retmp",re->mainfile);
		if((re->fd = lotOpenFileNoCasheByLotNrl(lotNr, re->tmpfile, ">>", 'w', subname)) == -1) {
			return NULL;
		}
		if ((re->fd_tmp = lotOpenFileNoCasheByLotNrl(lotNr, re->mainfile, ">>", 'r', subname)) == -1) {
			return NULL;
		}

		_filecpy(re->fd, re->fd_tmp);

	}
	else {
		if((re->fd = lotOpenFileNoCasheByLotNrl(lotNr, re->mainfile, ">>", 'r', subname)) == -1) {
			return NULL;
		}
	}

	fstat(re->fd,&inode);	

	if (inode.st_size < re->maxsize) {
                                /*
                                Stretch the file size to the size of the (mmapped) array of ints
                                */
                                if (lseek(re->fd, re->maxsize -1, SEEK_SET) == -1) {
                                        perror("Error calling fseek() to 'stretch' the file");
					return NULL;
                                }

                                /* Something needs to be written at the end of the file to
                                * have the file actually have the new size.
                                * Just writing an empty string at the current file position will do.
                                *
                                * Note:
                                *  - The current position in the file is at the end of the stretched
                                *    file due to the call to fseek().
                                *  - An empty string is actually a single '\0' character, so a zero-byte
                                *    will be written at the last byte of the file.
                                */
                                if (write(re->fd, "", 1) != 1) {
                                        perror("Error writing last byte of the file");
					return NULL;
                                }

	}

        if ((re->mem = mmap(0,re->maxsize,PROT_READ|PROT_WRITE,MAP_SHARED,re->fd,0) ) == MAP_FAILED) {
	        perror("mmap");
		//nullsetter dene, slik at det er lettere og se at det er en feil under debugging.
		re->mem = NULL;
		return NULL;
        }

	/*
	printf("mmap dump:\n");
	printf("########################################\n");
	int i;
	for(i=0;i<1000;i++) {
		printf("%c",(re->mem+i)[0]);
		//printf("=%c\n",(char)(re->mem + i));
		
	}
	printf("########################################\n");
	*/

	return re;

}

void reclose(struct reformat *re) {
	munmap(re->mem,re->maxsize);


	if ((re->flags & RE_COPYONCLOSE) == RE_COPYONCLOSE) {

		char fullmain[PATH_MAX];
		char fulltmp[PATH_MAX];

		GetFilPathForLotFile(fullmain,re->mainfile,re->lotNr,re->subname);
		GetFilPathForLotFile(fulltmp ,re->tmpfile,re->lotNr,re->subname);

		printf("renaming %s -> %s\n",fulltmp,fullmain);

		rename(fulltmp,fullmain);

		close(re->fd_tmp);

	}

	close(re->fd);

	free(re);
}


void *reposread(struct reformat *re, size_t position) {

	void *p;


	if (position > re->maxsize) {
		fprintf(stderr,"DocID is not in the lot that is open!\n");
		exit(-1);
	}

	p = re->mem;
	p += position;

	if ((re->flags & RE_HAVE_4_BYTES_VERSION_PREFIX) == RE_HAVE_4_BYTES_VERSION_PREFIX) {
		p += 4;
	}
	
	/*
	//debug: dumper allle data i struct.
	int i;
	printf("pointer p: %u, mem %u\n",p,re->mem);
	printf("dumping record of %i bytes\n",re->structsize);
	printf("########################################\n");
	for (i=0;i<re->structsize;i++) {
		printf("%c",((char *)p + i)[0]);
	}
	printf("########################################\n");
	*/

	return p;
}







void *reget(struct reformat *re, unsigned int DocID) {

	size_t position = (re->structsize * (DocID - LotDocIDOfset(re->lotNr)));

	#ifdef DEBUG	
	printf("regetp: DocID %u, position %i\n",DocID, (int)position);
	#endif

	return reposread(re,position);;

}


void *renget(struct reformat *re, size_t nr) {

	size_t position = (re->structsize * nr);

	#ifdef DEBUG	
	printf("regetp: nr %u, position %u\n",nr, position);
	#endif
	
	return reposread(re,position);;
}

