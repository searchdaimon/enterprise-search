#ifndef _BOITHOHOME__H_
#define _BOITHOHOME__H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *bfopen(char name[],char flags[]) {

        char *cptr;
        char fulname[512];

        if ((cptr = getenv("BOITHOHOME")) == NULL) {
                fprintf(stderr,"Error: Can't get environment value \"BOITHOHOME\"\n");
                exit(1);
        }
        else if (name[0] == '/') {
                fprintf(stderr,"Error: name starts with a /. Yoy must use virtual adressing\n");
        }
        else {
                sprintf(fulname,"%s/%s",cptr,name);
                return fopen(fulname,flags);
        }

/*
        if ((cptr = getenv("BOITHOHOME")) != NULL) {
                sprintf(fulname,"%s/%s",cptr,name);
                return fopen(fulname,flags);
        }
        else {
                return fopen(name,flags);
        }
*/

	return NULL;
}

static char *sbfile(char fulname[] ,char name[]) {
	char *cptr;

	if ((cptr = getenv("BOITHOHOME")) == NULL) {
                fprintf(stderr,"Error: Can't get environment value \"BOITHOHOME\"\n");
                return NULL;
        }
        else if (name[0] == '/') {
                fprintf(stderr,"Error: name starts with a /. Yoy must use virtual adressing\n");
		return NULL;
        }

        sprintf(fulname,"%s/%s",cptr,name);
        return fulname;
}

static char *bfile(char name[]) {

	//må legge til trå støtte her
	static char fulname[512];

	if (sbfile(fulname,name) == NULL) {
		return NULL;
	}

	return fulname;
}
static char *bfile2(char name[]) {

	//må legge til trå støtte her
	static char fulname[512];

	if (sbfile(fulname,name) == NULL) {
		return NULL;
	}

	return fulname;
}
static char *bfile3(char name[]) {

	//må legge til trå støtte her
	static char fulname[512];

	if (sbfile(fulname,name) == NULL) {
		return NULL;
	}

	return fulname;
}


#endif
