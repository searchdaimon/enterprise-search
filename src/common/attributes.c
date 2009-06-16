
#include <stdio.h>
#include <string.h>

#include "attributes.h"
#include "define.h"


int
next_attribute(char *attributes, char **offset, char *k, char *v, char *kv)
{
	char *key, *value;
	char *p;
	int last = 0;

	if (*offset == NULL)
		*offset = attributes;
	
	if (**offset == '\0') return 0;

	key = *offset;
	while (key[0] == ',') key++;
	value = strchr(key, '=');
	if (value == NULL)
		return 0;

	if (value-key < MAX_ATTRIB_LEN)
	    {
		strncpy(k, key, value-key);
		k[value-key] = '\0';
	    }
	else
	    {
		strncpy(k, key, MAX_ATTRIB_LEN-1);
		k[MAX_ATTRIB_LEN-1] = '\0';
	    }

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

	if (p-value < MAX_ATTRIB_LEN)
	    {
		int ending_slash = 0;	// Fjern slash på slutten for normalisering
		if (p-value > 0 && value[p-value-1]=='/') ending_slash = 1;
		strncpy(v, value, p-value-ending_slash);// + (last ? 0 : -1));
		v[p-value-ending_slash] = '\0';
	    }
	else
	    {
		strncpy(v, value, MAX_ATTRIB_LEN-1);// + (last ? 0 : -1));
		v[MAX_ATTRIB_LEN-1] = '\0';
	    }

	if (p-key < MAX_ATTRIB_LEN)
	    {
		int ending_slash = 0;
		if (p-value > 0 && value[p-value-1]=='/') ending_slash = 1;
		strncpy(kv, key, p-key-ending_slash);// + (last ? 0 : -1));
		kv[p-key-ending_slash] = '\0';
	    }
	else
	    {
		strncpy(kv, key, MAX_ATTRIB_LEN-1);// + (last ? 0 : -1));
		kv[MAX_ATTRIB_LEN-1] = '\0';
	    }

	while (*p == ',') p++;
	*offset = p;

	return 1;
}

int
next_attribute_key(char *attributes, char **offset, char *k, char *v, char *kv, char *want)
{
	while (next_attribute(attributes, offset, k, v, kv)) {
		if (strcmp(want, k) == 0)
			return 1;
	}

	return 0;
}

/*
int next_attribute_kv(char *attributes, char **offset, char *kv)
{
	char *key, *value;
	char *p;
	int last = 0;

	if (*offset == NULL)
		*offset = attributes;
	
	if (**offset == '\0') return 0;

	key = *offset;
	value = strchr(key, '=');
	if (value == NULL)
		return 0;
/+
	if (value-key < MAX_ATTRIB_LEN)
	    {
		strncpy(k, key, value-key);
		k[value-key] = '\0';
	    }
	else
	    {
		strncpy(k, key, MAX_ATTRIB_LEN-1);
		k[MAX_ATTRIB_LEN-1] = '\0';
	    }
+/
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

	if (p-key < MAX_ATTRIB_LEN)
	    {
		strncpy(kv, key, p-key);// + (last ? 0 : -1));
		kv[p-key] = '\0';
	    }
	else
	    {
		strncpy(kv, value, (MAX_ATTRIB_LEN)-1);
		kv[(MAX_ATTRIB_LEN)-1] = '\0';
	    }
	
	if (*p == ',') p++;	
	*offset = p;

	return 1;
}
*/
#ifdef _ATTRIB_TEST_

int
main(int argc, char **argv)
{
	char key[MAX_ATTRIB_LEN], value[MAX_ATTRIB_LEN];
	char keyval[MAX_ATTRIB_LEN];
	char attributes[] = ",,hei=ho,no=er,,det=,jul=igjen,vi=baker,nissemenn=,folder=mg@searchdaimon/Inbox/,,,";
	//char attributes[] = "hei=ho,ha=hopp,finn=sesam";
	char *o = NULL;

	while (next_attribute(attributes, &o, key, value, keyval)) {
		printf("Got pair: %s => %s (%s)\n", key, value, keyval);
	}
	
	return 0;
}

#endif
