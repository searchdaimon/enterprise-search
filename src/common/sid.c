#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../base64/base64.h"

#include "sid.h"

/*
 * Revision: 1 byte
 * Subcount: 1 byte
 * Auth: 6 byte
 * rest: 4 byte * subcount
*/

char *
sid_b64totext(char *buf, size_t len)
{
	char *p;
	size_t outlen;
	char sid[1024], out[1024];
	char data[1024];
	unsigned int rev;
	unsigned int subcount;
	unsigned long long int auth;
	int i;
	size_t wlen;

	len = base64_decode(out, buf, sizeof(out));
	p = out;

	/* Rev */
	rev = *p++;
	printf("rev: %d\n", rev);

	/* Subcount */
	subcount = *p++;
	printf("subcount: %d\n", subcount);

	/* Auth */
	auth = 0;
	for (i = 0; i < 6; i++)
		auth = ((auth << 8) & 0xffffffffff00) | (*p++ & 0xff);
	printf("auth: %lld\n", auth);

	unsigned int subauth[subcount];
	wlen = snprintf(sid, sizeof(sid), "S-%d-%lld", rev, auth);

	for (i = 0; i < subcount; i++) {
		int j;
		char *n;

		subauth[i] = 0;
		n = (char *)&subauth[i];
		/* Handle little endian */
		for (j = 0; j < 4; j++) {
			n[j] = *p++;
		}
		printf("subauth[%d]: %u\n", i, subauth[i]);
		wlen += snprintf(sid+wlen, sizeof(sid)-wlen, "-%u", subauth[i]);
	}

	if ((p = malloc(MAX_SID_LEN)) == NULL)
		return NULL;
	strcpy(p, sid);

	return p;
}

int
sid_replacelast(char *sidt, char *new)
{
	char *p;

	if ((p = strrchr(sidt, '-')) == NULL)
		return 0;

	p++;
	strcpy(p, new);

	return 1;
}

#ifdef SID_WITH_MAIN

int
main(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		char *p;
		p = sid_b64totext(argv[i], strlen(argv[i]));
		printf("Got: %s\n", p);
		sid_replacelast(p, "513");
		printf("Got: %s\n", p);
		free(p);
	}
}

#endif /* SID_WITH_MAIN */
