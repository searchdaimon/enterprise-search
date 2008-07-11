
#include <stdio.h>
#include <string.h>

#include "attributes.h"

int
next_attribute(char *attributes, char **offset, char *k, char *v)
{
	char *key, *value;
	char *p;
	int last = 0;

	if (*offset == NULL)
		*offset = attributes;
	
	if (**offset == '\0')
		return 0;

	key = *offset;
	value = strchr(key, '=');
	if (value == NULL)
		return 0;
	strncpy(k, key, value-key);
	k[value-key] = '\0';
	value++;
	p = strchr(value, ',');
	if (p == NULL) {
		p = strchr(value, '\0');
		last = 1;
		if (p == NULL)
			return 0;
	} else {
		//p++;
	}

	strncpy(v, value, p-value);// + (last ? 0 : -1));
	v[p-value] = '\0';
	
	*offset = p;

	return 1;
}

#ifdef _ATTRIB_TEST_

int
main(int argc, char **argv)
{
	char key[1024], value[1024];
	char attributes[] = "hei=ho";
	//char attributes[] = "hei=ho,ha=hopp,finn=sesam";
	char *o = NULL;

	while (next_attribute(attributes, &o, key, value)) {
		printf("Got pair: %s => %s\n", key, value);
	}
	
	return 0;
}

#endif
