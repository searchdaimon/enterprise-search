/**
 *	Record Engine
 */

#ifndef _RE__H_
#define _RE__H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "define.h"

/*
        RE_COPYONCLOSE: creates a copy of the file, and then copies this copy back in when we close. This lets us work on our own private, locked copy, and
        then copy it back in atomically when we are done with it and everything went well. Reading from the original is still allowed, but new copies cannot be made until we are done.
*/
#define RE_COPYONCLOSE 			0x1
#define RE_HAVE_4_BYTES_VERSION_PREFIX 	0x2
#define RE_READ_ONLY 			0x4
#define RE_STARTS_AT_0 			0x8
#define RE_POPULATE 			0x10
#define RE_CREATE_AND_STRETCH		0x20
#define RE_STRETCH			0x40

struct reformat {
        void *mem;
        int fd;
        size_t maxsize;
        size_t structsize;
        int flags;
        int lotNr;
        char subname[maxSubnameLength];
        char mainfile[62];
        char tmpfile[62];
        int fd_tmp;
};


#define reIsOpen(ire,ilotNr,isubname,ifile) (( ire != NULL ) && ( re->lotNr == ilotNr ) && ( strcmp(re->subname,isubname) == 0 ) && (strcmp(re->mainfile,ifile) == 0))


struct reformat *reopen(int lotNr, size_t structsize, char file[], char subname[], int flags);
struct reformat *reopen_cache(int lotNr, size_t structsize, char file[], char subname[], int flags);
void reclose(struct reformat *re);
void reclose_cache(void);
void *reget(struct reformat *re, unsigned int DocID);
void *renget(struct reformat *re, size_t nr);

#define RE_DocumentIndex(re, DocID) ((struct DocumentIndexFormat *)reget(re, DocID))
#define REN_DocumentIndex(re, nr) ((struct DocumentIndexFormat *)renget(re, nr))
#define RE_Uint(re, DocID) ((unsigned int *)reget(re, DocID))
#define REN_Uint(re, nr) ((unsigned int *)renget(re, nr))

#define RE_Int(re, DocID) ((int *)reget(re, DocID))
#define REN_Int(re, nr) ((int *)renget(re, nr))

#define RE_Char(re, DocID) ((char *)reget(re, DocID))
#define RE_Uchar(re, DocID) ((unsigned char *)reget(re, DocID))
#define REN_Uchar(re, nr) ((unsigned char *)renget(re, nr))

#define RE_Brank(re, DocID) ((struct brank *)reget(re, DocID))
#define REN_Brank(re, nr) ((struct brank *)renget(re, nr))


#endif // _RE__H_
