#include <string.h>
#include <stdlib.h>
#include <wchar.h>

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
	unsigned int a = *((int *)ky);
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);

	return a;
}

int
ht_integercmp(void *k1, void *k2)
{
	return (*((int *)k1) == *((int *)k2));
}

unsigned int *
uinttouintp(unsigned int a)
{
	unsigned int *p;

	p = malloc(sizeof(*p));
	*p = a;

	return p;
}
