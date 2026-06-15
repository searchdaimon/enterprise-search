#include <stdlib.h>
#include <string.h>

#include "ir.h"

int isShortWord(char *term) {
	if (strlen(term) < 2) {
		return 1;
	}
	else {
		return 0;
	}
}

//check whether a word is a stop word
int isStoppWord(char *term) {

	if (isShortWord(term)) {
		return 1;
	}

	return 0;
	
}

