#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>

#include <libio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

struct b_fh_cookie {
	int fdOriginal;
	int fdAtomicallyTmp;

	char pathOriginal[PATH_MAX];
	char pathAtomicallyTmp[PATH_MAX];

	char mode[4];

};

ssize_t breader (void *cookie, char *buffer, size_t size) {
	#ifdef DEBUG
	printf("reader(size=%d)\n",size);
	#endif

	return read(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp,buffer,size);
}
ssize_t bwriter (void *cookie, const char *buffer, size_t size) {
	#ifdef DEBUG
	printf("writer(size=%d)\n",size);
	#endif

	return write(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp,buffer,size);;
}
int bseeker (void *cookie, _IO_off64_t *position, int whence) {
	#ifdef DEBUG
	printf("seeker(position=%lu,whence=%i)\n",position,whence);
	#endif
	
	return lseek(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp,(*position),whence);
}
int bcleaner (void *cookie) {
	#ifdef DEBUG
	printf("cleaner()\n");
	#endif

	//stenger ned fdAtomicallyTmp
	if (close(((struct b_fh_cookie *)cookie)->fdAtomicallyTmp) == -1) {
		return -1;
	}

	//renamer fdAtomicallyTmp -> fdOriginal
	printf("rename %s -> %s\n",((struct b_fh_cookie *)cookie)->pathAtomicallyTmp,((struct b_fh_cookie *)cookie)->pathOriginal);

	if (rename(((struct b_fh_cookie *)cookie)->pathAtomicallyTmp,((struct b_fh_cookie *)cookie)->pathOriginal) == -1) {
		return -1;
	}

	//stenger ned fdOriginal
	if (((struct b_fh_cookie *)cookie)->fdOriginal != -1) {
		if (close(((struct b_fh_cookie *)cookie)->fdOriginal) == -1) {
			return -1;
		}
	}


	return 0;
}






FILE *batomicallyopen(char path[], char mode[]) {


	FILE *FH;
	struct b_fh_cookie *bfh;
	cookie_io_functions_t iofunctions;
	int flagsAtomicallyTmp;

	if ((bfh = malloc(sizeof(struct b_fh_cookie))) == NULL) {
		perror("malloc b_fh_cookie");
		return NULL;
	}


	strcpy(bfh->mode,mode);
	strcpy(bfh->pathOriginal,path);
	strcpy(bfh->pathAtomicallyTmp,path);
	strcat(bfh->pathAtomicallyTmp,".AtomicallyTmp");

	//finner ut hvilkene flag vi skal bruke
	if ((strcmp(mode,"w") == 0) || (strcmp(mode,"wb") == 0)) {
		flagsAtomicallyTmp = O_CREAT|O_TRUNC|O_WRONLY;
	}
	else {
		fprintf(stderr,"batomicallyopen: unsoportet open mode \"%s\"!\n",mode);
		exit(-1);
	}

	//åpner orginal filen
	if((bfh->fdOriginal = open(bfh->pathOriginal,O_RDONLY))< 0) {
	}
	else {
		flock(bfh->fdOriginal,LOCK_SH);
	}
	//åpner atomically tmp filen
	if((bfh->fdAtomicallyTmp = open(bfh->pathAtomicallyTmp,flagsAtomicallyTmp,0664))< 0) {
		perror(bfh->pathAtomicallyTmp);
		return NULL;
	}
	//helst skulle vi ha lokket filen her, slik at ikke andre kan begynne og kjase med vår kopi, og vi ikke for noen rases
	//men siden dette er en drop inn replacement for fopen() kan det være at andre låser andre plasser.
	//flock(bfh->fdAtomicallyTmp,LOCK_EX);



	//setter opp hvilkene funksjoner i skal bruke for å lese og skrive
  	iofunctions.read 	= breader;
  	iofunctions.write 	= bwriter;
  	iofunctions.seek 	= bseeker;
  	iofunctions.close 	= bcleaner;

	//lager et stream 
	if((FH = fopencookie(bfh,mode,iofunctions)) == NULL) {
		perror("fopencookie");
	}


	return FH;
}


/*
int main() {

	char buf[30] = { "qwerty" };
	FILE *FH;

	FH = batomicallyopen("/tmp/aa","wb");

//	fread(&buf,10,1,FH);

	printf("buf \"%s\"\n",buf);

	fwrite(&buf,10,1,FH);

	fseek(FH,28,SEEK_SET);

	fclose(FH);

	return 0;
}

*/
