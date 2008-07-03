#include <string.h>
#include <stdlib.h>

unsigned int
ht_stringhash(void *ky)
{
	char *p = ky;
	unsigned int hash = 5381;
	int c;

	while ((c = *p++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

int
ht_stringcmp(void *k1, void *k2)
{
	char *c1, *c2;

	c1 = k1;
	c2 = k2;

	return (strcmp(c1, c2) == 0);
}

unsigned int
ht_wstringhash(void *ky)
{
	wchar_t *p = ky;
	unsigned int hash = 5381;
	int c;

	while ((c = *p++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

int
ht_wstringcmp(void *k1, void *k2)
{
	wchar_t *c1, *c2;

	c1 = k1;
	c2 = k2;

	return (wcscmp(c1, c2) == 0);
}


unsigned int
ht_integerhash(void *ky)
{
	return *((int *)ky);
}

int
ht_integercmp(void *k1, void *k2)
{
	int *c1, *c2;

	c1 = k1;
	c2 = k2;

	return (*c1 == *c2);
}

