#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "libcache.h"

void
ourfreevalue(void *value)
{
	free(value);
}

int
main(int argc, char **argv)
{
	cache_t c;
	char *v;

	if (!cache_init(&c, ourfreevalue))
		err(1, "Unable to init cache");
	
	cache_add(&c, "some", "1", strdup("Quux!"));
	cache_add(&c, "some", "2", strdup("BAr!"));
	cache_add(&c, "some", "3", strdup("Baz!"));
	cache_add(&c, "some", "4", strdup("Foo!"));

	v = cache_fetch(&c, "some", "3");
	printf("%s\n", v);

	return 0;
}
