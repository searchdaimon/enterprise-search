#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>

#include "lot.h"
#include "define.h"
#include "re.h"
#include "ht.h"

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../common/timediff.h"


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
	struct reformat *re = NULL;
	char openmode[4];
	int mmapmode;
	size_t i;
	int stretch = 0;

	#ifdef DEBUG
	printf("reopen(lotNr=%i, structsize=%zu, file=%s, subname=%s, flags=%i)\n",lotNr,structsize,file,subname,flags);
	#endif

	if ((re = malloc(sizeof(struct reformat))) == NULL) {
		return NULL;
	}

	re->fd = -1;
	re->flags = flags;

	if ((re->flags & RE_HAVE_4_BYTES_VERSION_PREFIX) == RE_HAVE_4_BYTES_VERSION_PREFIX) {
		#ifdef DEBUG
		printf("reopen: have RE_HAVE_4_BYTES_VERSION_PREFIX\n");
		#endif
		structsize += 4;
	}

	if ( ( (re->flags & RE_READ_ONLY) == RE_READ_ONLY ) && ( (re->flags & RE_STRETCH) == RE_STRETCH ) ) {
		// skal strekke hvis filen er for liten, men ikke opprette hvis den ikke er der.
		strcpy(openmode,"r+b");
		mmapmode = PROT_READ;
		stretch = 1;
	}
	else if ( ( (re->flags & RE_READ_ONLY) == RE_READ_ONLY ) && ( (re->flags & RE_CREATE_AND_STRETCH) == RE_CREATE_AND_STRETCH ) ) {
		strcpy(openmode,">>");
		mmapmode = PROT_READ;
		stretch = 1;
	}
	else if ((re->flags & RE_READ_ONLY) == RE_READ_ONLY) {
		strcpy(openmode,"rb");
		mmapmode = PROT_READ;
		stretch = 0;
	}
	else {
		strcpy(openmode,">>");
		mmapmode = PROT_READ|PROT_WRITE;
		stretch = 1;
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
			goto reopen_error;
		}
		if ((re->fd_tmp = lotOpenFileNoCasheByLotNrl(lotNr, re->mainfile, ">>", 'r', subname)) == -1) {
			goto reopen_error;
		}

		//kopierer tmp til fd
		_filecpy(re->fd, re->fd_tmp);

	}
	else {
		if((re->fd = lotOpenFileNoCasheByLotNrl(lotNr, re->mainfile, openmode, 'r', subname)) == -1) {
			#ifdef DEBUG
			fprintf(stderr,"can't open file %s. for lot %i, subname %s\n",re->mainfile,lotNr,subname);
			perror("lot file");
			#endif
			goto reopen_error;
		}
	}

	fstat(re->fd,&inode);	

	if ((stretch == 1) && (inode.st_size < re->maxsize) ) {
				printf("Stretching re file..\n");
                                /*
                                Stretch the file size to the size of the (mmapped) array of ints
                                */
                                if (lseek(re->fd, re->maxsize -1, SEEK_SET) == -1) {
                                        perror("Error calling fseek() to 'stretch' the file");
					goto reopen_error;
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
					goto reopen_error;
                                }

	}

        if ((re->mem = mmap(0,re->maxsize,mmapmode,MAP_SHARED,re->fd,0) ) == MAP_FAILED) {
		printf("reopen(lotNr=%i, structsize=%zu, file=%s, subname=%s, flags=%i)\n",lotNr,structsize,file,subname,flags);
	        perror("reopen mmap");
		//nullsetter dene, slik at det er lettere og se at det er en feil under debugging.
		re->mem = NULL;
		goto reopen_error;
        }

	if ((re->flags & RE_POPULATE) == RE_POPULATE) {
		int pr = 0;
		#ifdef DEBUG
			printf("RE_POPULATE'ing\n");
		#endif
		for(i=0;i<re->maxsize;i++) {
			pr += ((char *)re->mem)[i]; 	
		}
	}

	/*
	printf("mmap dump:\n");
	printf("########################################\n");
	int i;
	for(i=0;i<1000;i++) {
		printf("%c",(re->mem+i)[0]);		
	}
	printf("########################################\n");
	*/

	return re;

	reopen_error:
		if (re->fd != -1) {
			close(re->fd);
		}
		free(re);
		
		return NULL;
}

static struct hashtable *lots_cache;

struct reformat *reopen_cache(int lotNr, size_t structsize, char file[], char subname[], int flags) {
	char lotfile[PATH_MAX];
	struct reformat *re;

	sprintf(lotfile, "%s\x10%s\x10%d", subname, file, lotNr);
	if (lots_cache == NULL) {
		lots_cache = create_hashtable(3, ht_stringhash, ht_stringcmp);
		if (lots_cache == NULL)
			err(1, "hashtable_create(reopen_cache)");
	} else {
		re = hashtable_search(lots_cache, lotfile);
		#ifdef DEBUG
			printf("has cahce for %s:%s lot %d. pointer %p\n", subname, file, lotNr, re);
		#endif
		if (re != NULL)
			return re;
	}
	#ifdef DEBUG
		printf("reopen_cache: Cache miss for %s:%s lot %d\n", subname, file, lotNr);
	#endif

	re = reopen(lotNr, structsize, file, subname, flags);
	//Runarb 11 feb 2009:
	//vi må nesten cache at filen ikke fantes, for bakoverkompetabilitet. Når vi siden gjør om dette på preopning til å ikke
	//reåpne etter hver crawl, må vi gjøre om
	if (re == NULL) {
		return NULL;
	}
	hashtable_insert(lots_cache, strdup(lotfile), re);


	return re;
}

void
reclose_cache(void)
{
	struct reformat *re;
        struct hashtable_itr *itr;
	char *filesname;

	#ifdef DEBUG_TIME
	        struct timeval start_time, end_time;
	        gettimeofday(&start_time, NULL);
	#endif

      	//itererer over hash, frigjør alle elementer
        if (lots_cache!= NULL && hashtable_count(lots_cache) > 0)
        {

                itr = hashtable_iterator(lots_cache);

                do {
                	filesname = hashtable_iterator_key(itr);
                        re = hashtable_iterator_value(itr);

			#ifdef DEBUG
				printf("reclose_cache: closing \"%s\"\n",filesname);
			#endif

			if (re != NULL) {
				reclose(re);
			}

             	} while (hashtable_iterator_remove(itr));

                free(itr);
	}
	#ifdef DEBUG_TIME
	        gettimeofday(&end_time, NULL);
	        printf("Time debug: reclose_cache %f\n",getTimeDifference(&start_time,&end_time));
	#endif


	return; 
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


static inline void *reposread(struct reformat *re, size_t position) {

	void *p;


	if (position > re->maxsize) {
		fprintf(stderr,"DocID is not in the lot that is open!. position %zu > maxsize %zu.\n",position,re->maxsize);
		exit(-1);
	}

	p = re->mem + position;

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
	if ((re->flags & RE_STARTS_AT_0) == RE_STARTS_AT_0) {
		#ifdef DEBUG
			printf("position %zu, structsize %zu\n", position,re->structsize);
		#endif
		if (position != 0) {
			position -= re->structsize;
		}
	}
	#ifdef DEBUG	
		printf("regetp: DocID %u, position %zu, lot %i, structsize %zu\n",DocID, position, re->lotNr, re->structsize);
	#endif

	return reposread(re,position);;

}


void *renget(struct reformat *re, size_t nr) {

	size_t position = (re->structsize * nr);

	#ifdef DEBUG	
		printf("rengetp: nr %u, position %u\n",nr, position);
	#endif
	
	return reposread(re,position);;
}

