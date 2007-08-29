#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bs.h"

int main() {
	char *d;
	struct bs s;

	bs_init(&s,10);

	bs_add(&s,"aa");
	bs_add(&s,"ab");
	bs_add(&s,"ac");
	bs_add(&s,"ad123456789");

	bs_get(&s,&d);
	printf("ret \"%s\"\n",d);
	bs_get(&s,&d);
	printf("ret \"%s\"\n",d);
	bs_get(&s,&d);
	printf("ret \"%s\"\n",d);
	bs_get(&s,&d);
	printf("ret \"%s\"\n",d);
	bs_get(&s,&d);
	printf("ret \"%s\"\n",d);
	bs_get(&s,&d);
	printf("ret \"%s\"\n",d);
	bs_get(&s,&d);
	printf("ret \"%s\"\n",d);
}

