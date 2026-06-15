#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/file.h>
#include <stdlib.h>
#include <stdio.h>

struct b_fh_cookie {
	int fdOriginal;
	int fdAtomicallyTmp;

	char pathOriginal[PATH_MAX];
	char pathAtomicallyTmp[PATH_MAX];

	char mode[4];
	off_t   offset;     /* Current file offset in buf */

};

ssize_t breader (void *cookie, char *buffer, size_t size) {
	#ifdef DEBUG
		printf("reader(size=%d)\n",size);
		printf("breader offset %u\n",(unsigned)((struct b_fh_cookie *)cookie)->offset);
	#endif

	((struct b_fh_cookie *)cookie)->offset += size;


	return read(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp,buffer,size);
}
ssize_t bwriter (void *cookie, const char *buffer, size_t size) {
	#ifdef DEBUG
		printf("writer(size=%d)\n",size);
		printf("bwriter offset %u\n",(unsigned)((struct b_fh_cookie *)cookie)->offset);
	#endif

	((struct b_fh_cookie *)cookie)->offset += size;

	return write(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp,buffer,size);;
}
int bseeker (void *cookie, off64_t *position, int whence) {
	#ifdef DEBUG
		printf("seeker(position=%lld,whence=%i)\n",*position,whence);
		printf("seeker(SEEK_SET=%i, SEEK_CUR=%i)\n",SEEK_SET,SEEK_CUR);
	#endif

	// Bugfix: ftell calls bseeker(position=0, whence=SEEK_CUR) to find the position or to buffer data.
	// this does not work on the bbh kernel and we must use batomicallyopen(). This also resets the position of the stream and
	// thus works. For all of this we just return 1 (ok). 
	if ( (*position==0) && (whence==SEEK_CUR) ) {
		#ifdef DEBUG
			printf("bseeker(): got asked for offset\n");
		#endif
		return 1;
	}


	if (whence == SEEK_SET) {
		#ifdef DEBUG
			printf("bseeker(): SEEK_SET to %lu\n",*position);
		#endif

		((struct b_fh_cookie *)cookie)->offset = *position;
	}
	else if (whence == SEEK_CUR) {
		#ifdef DEBUG
			printf("bseeker(): SEEK_CUR to %lu\n",*position);
		#endif
		((struct b_fh_cookie *)cookie)->offset += *position;
	}
	else {
		printf("bseeker: Unsoportet seek mode\n");
		return -1;
	}

	*position = ((struct b_fh_cookie *)cookie)->offset;
	
	return lseek(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp, *position, whence);
}
int bcleaner (void *cookie) {
	#ifdef DEBUG
		printf("cleaner()\n");
	#endif

	// close down fdAtomicallyTmp
	if (close(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp) == -1) {
		return -1;
	}

	// rename fdAtomicallyTmp -> fdOriginal
	#ifdef DEBUG
		printf("rename %s -> %s\n",((struct b_fh_cookie *)cookie)->pathAtomicallyTmp,((struct b_fh_cookie *)cookie)->pathOriginal);
	#endif

	if (rename(((struct b_fh_cookie *)cookie)->pathAtomicallyTmp,((struct b_fh_cookie *)cookie)->pathOriginal) == -1) {
		return -1;
	}

	// close down fdOriginal
	if (((struct b_fh_cookie *)cookie)->fdOriginal != -1) {
		if (close(((struct b_fh_cookie *)cookie)->fdOriginal) == -1) {
			return -1;
		}
	}

	// free the cookie
	free(cookie);

	return 0;
}






FILE *batomicallyopen(char path[], char mode[]) {


	FILE *FH;
	struct b_fh_cookie *bfh;
	cookie_io_functions_t iofunctions;
	int flagsAtomicallyTmp;

	#ifdef DEBUG
		printf("batomicallyopen(path=\"%s\",mode=\"%s\")\n",path,mode);		
	#endif

	if ((bfh = malloc(sizeof(struct b_fh_cookie))) == NULL) {
		perror("malloc b_fh_cookie");
		return NULL;
	}


	strcpy(bfh->mode,mode);
	strcpy(bfh->pathOriginal,path);
	strcpy(bfh->pathAtomicallyTmp,path);
	strcat(bfh->pathAtomicallyTmp,".AtomicallyTmp");

	bfh->offset = 0;

	// figure out which flags we should use
	if ((strcmp(mode,"w") == 0) || (strcmp(mode,"wb") == 0)) {
		flagsAtomicallyTmp = O_CREAT|O_TRUNC|O_WRONLY;
	}
	else {
		fprintf(stderr,"batomicallyopen: unsoportet open mode \"%s\"!\n",mode);
		exit(-1);
	}

	// open the original file
	if((bfh->fdOriginal = open(bfh->pathOriginal,O_RDONLY))< 0) {
		// failed to open. Maybe it does not exist? That is ok.
	}
	else {
		// maybe only on write
                #ifdef SD_CLOEXEC
	                if (!closeAtExexo(bfh->fdOriginal)) {
				fprintf(stderr,"fcntl closeAtExexo fdOriginal.\n");
				perror(bfh->pathOriginal);
			}
                #endif
		flock(bfh->fdOriginal,LOCK_SH);
	}
	// open the atomically tmp file
	if((bfh->fdAtomicallyTmp = open(bfh->pathAtomicallyTmp,flagsAtomicallyTmp,0664))< 0) {
		perror(bfh->pathAtomicallyTmp);
		return NULL;
	}
	// maybe only on write
        #ifdef SD_CLOEXEC
                if (!closeAtExexo(bfh->pathAtomicallyTmp)) {
			fprintf(stderr,"fcntl closeAtExexo pathAtomicallyTmp.\n");
			perror(bfh->pathAtomicallyTmp);
		}
        #endif

	// ideally we would have locked the file here, so that others cannot start messing with our copy and we do not get any races
	// but since this is a drop-in replacement for fopen() it may be that others lock in other places.
	//flock(bfh->fdAtomicallyTmp,LOCK_EX);



	// set up which functions to use for reading and writing
  	iofunctions.read 	= breader;
  	iofunctions.write 	= bwriter;
  	iofunctions.seek 	= bseeker;
  	iofunctions.close 	= bcleaner;

	// create a stream 
	if((FH = fopencookie(bfh,mode,iofunctions)) == NULL) {
		perror("fopencookie");
	}


	return FH;
}

/*
 Egne ftell funkasjo da ftell ikke fungerer riktig i libc versjonen som ligger på bbh-001. 

 This is a known bug, and is documented at http://sources.redhat.com/ml/libc-alpha/2005-04/msg00035.html

 Strukten _IO_cookie_file er nevnt i /usr/include/libio.h men ikke "deklarert" har derfor tatt den fra 
 http://sources.redhat.com/ml/libc-hacker/2000-06/msg00230.html , so we just have to hope it is never updated :(
*/
off_t batomicallyftell(FILE *stream) {


	struct _B_IO_cookie_file
	{
	  struct _IO_FILE __file;
	  const void *__vtable;
	  void *__cookie;
	  cookie_io_functions_t __io_functions;
	};

	// the f* functions buffer the data. Call fseek to force it to send data to the writer function, and thus update cookie->offset
	fseek(stream, 0, SEEK_CUR);

	struct _B_IO_cookie_file *cfile = (struct _B_IO_cookie_file *) stream;

	#ifdef DEBUG
	printf("~batomicallyftell=%u\n",((struct b_fh_cookie *)cfile->__cookie)->offset);
	#endif

	return ((struct b_fh_cookie *)cfile->__cookie)->offset;
}

/*
int main() {

	char buf[30] = { "qwerty" };
	FILE *FH;

	FH = batomicallyopen("/tmp/aa","wb");


	printf("buf \"%s\"\n",buf);

	fwrite(&buf,10,1,FH);

	fseek(FH,28,SEEK_SET);

	fclose(FH);

	return 0;
}

*/
