

#include "getpath.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define _MAX_DIR 128

char *getpath(char *filename) {

	int name_len;
	int i;
	static char pathRet[PATH_MAX];
	strcpy(pathRet,filename);

	#ifdef DEBUG
	printf("getpath(filename=%s)\n",filename);
	#endif

	name_len = strlen(filename);

        for (i = name_len; i > 0; i--) {
                if( (filename[i] == '\057') || (filename[i] == '\134')) {
			break;
		}

	}
	++i;
	pathRet[i] = '\0';

	#ifdef DEBUG
	printf("~getpath(return=%s)\n",pathRet);
	#endif


	return pathRet;        
}




