#include <stdlib.h>
#include <string.h>

#include "ir.h"

//sjekker om et ord er stoppord
int isStoppWord(char *term) {

	if (isShortWord(term)) {
		return 1;
	}

	return 0;
	
}

int isShortWord(char *term) {
	if (strlen(term) < 2) {
		return 1;
	}
	else {
		return 0;
	}
}
