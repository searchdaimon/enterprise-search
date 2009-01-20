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
        RE_COPYONCLOSE: lager en kopi av filen, og kopierer så inn denne kopien når vi avslutter. Dette gjør at vi kan jobbe på vår egen private, låste kopi, og
        så kopiere den inn anatomisk, når vi er ferdige med den, og alt gikk bra. Lesing til orginalen er fortsatt tilat, men man kan ikke lge nye kopier før vi er ferdige.
*/
#define RE_COPYONCLOSE 0x1
#define RE_HAVE_4_BYTES_VERSION_PREFIX 0x2
#define RE_READ_ONLY 0x4
#define RE_STARTS_AT_0 0x8
#define RE_POPULATE 0x10

struct reformat {
        void *mem;
        //char *mem;
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

#define RE_Brank(re, DocID) ((struct brank *)reget(re, DocID))
#define REN_Brank(re, nr) ((struct brank *)renget(re, nr))


#endif // _RE__H_
