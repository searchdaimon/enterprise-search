#include <string.h>
#include <stdio.h>

#include "../common/bprint.h"

int
main(void)
{
	char *a, *am;
	buffer *b;
	size_t al;
	size_t i;

	al = BUFFER_BLOCKSIZE * 10;
	a = malloc(al+1);
	for (i = 0; i < al; i++) {
		a[i] = 'a';
	}
	a[al] = '\0';
	
	b = buffer_init(0);
	//bprintf(b, "%s", a);
	bmemcpy(b, a, strlen(a));
	am = buffer_exit(b);
//	printf("A: %s\n", a);
//	printf("Am: %s\n", am);
	if (strcmp(a, am) != 0) {
		printf("Input and output a does not match\n");
	} else {
		printf("A okey\n");
	}
	
	return 0;
}
