#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <iconv.h>
#include <stdlib.h>

#include "../common/strlcpy.h"


int Base64Index[128] = {
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
	52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
	-1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	-1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};
#define base64lookup(c) Base64Index[(int)c]

int HexIndex[128] = {
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
	-1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

#define hexlookup(c) HexIndex[(int)c]


void
fix_subject(char *subject, size_t inlen)
{
	char charset[1024];
	char newsubj[1024];
	char *p, *p2, *start;
	int i, len, curlen;
	size_t some, some2;
	int type; /* 0 == quoted printable, 1 == base64 */
	iconv_t converter;
	int c, b = 0, k = 0;
	int special;

	#ifdef DEBUG
	printf("Looking at: %s\n", subject);
	#endif

	some = strlen(subject) + 1;
	if ((start = strstr(subject, "=?")) == NULL) {
		return;
	}
	if (start - subject > 0)
		strlcpy(newsubj, subject, start-subject);
	else
		newsubj[0] = '\0';
	p = start + 2;

	if ((p2 = strchr(p, '?')) == NULL) {
		return;
	}
	strlcpy(charset, p, (p2 - p) +1);

	p2++;
	if (*p2 == '\0') {
		return;
	}
	if (toupper(*p2) == 'Q')
		type = 0;
	else if (toupper(*p2) == 'B')
		type = 1;
	else
		return;
	p2++;
	if (*p2 == '\0') {
		subject[0] = '\0';
		return;
	}

	p2++;
	i = strlen(newsubj);
	curlen = 0;
	len = strlen(p2);
	special = 0;
	for (p = p2; *p != '\0'; p++, curlen++) {
		if (type == 0) {
			if (*p == '?' && *(p + 1) == '=') {
				newsubj[i++] = '\0';
				break;
			} else if (*p == '_') {
				newsubj[i++] = ' ';
			} else if (*p == '=' && len - curlen > 3 && hexlookup(*(p+1)) != -1 && hexlookup(*(p+2)) != -1) {
				int digit;

				digit = (hexlookup(*(p+1)) << 4) | hexlookup(*(p+2));
				p += 2;
				newsubj[i++] = digit;
				if (digit > 127 && special == 0)
					special = 1;
				else
					special = 0;

			} else {
				newsubj[i++] = *p;
			}
		} else if (type == 1) {
			if (*p == '=')
				break;
			if ((c = base64lookup(*p)) == -1) {
				continue;	
			}
			if (k + 6 >= 8) {
				k -= 2;
				newsubj[i++] = b | (c >> k);
				b = c << (8 - k);
			} else {
				b |= c << (k + 2);
				k += 6;
			}
		}
	}
	newsubj[special == 1 ? i-1 : i] = '\0';

	#ifdef DEBUG
	printf("iconv converting to UTF-8 from \"%s\"\n", charset);
	printf("newsubj: \"%s\"\n",newsubj);
	printf("type: %i\n",type);
	#endif

	//if (charset[strlen(charset)-1] == '_' || charset[strlen(charset)-1] == 'n')
	//	charset[strlen(charset)-1] = '\0';
	if (strncmp(charset, "utf-8", 5) == 0)
		strcpy(charset, "utf-8");
	else if (strncmp(charset, "iso-8859-1", 10) == 0)
		strcpy(charset, "iso-8859-1");
	converter = iconv_open("UTF-8", charset);
	if (converter != (iconv_t)(-1)) {
		some2 = strlen(newsubj) +1;
		char *p3 = strdup(newsubj);
		p = p3;
		char *p4 = malloc(some2 * 4);
		p2 = p4;
		some = strlen(newsubj) * 4;
		iconv(converter, &p, &some2, &p2, &some);
		p4[some] = '\0';
		free(p3);
		strncpy(subject, p4, inlen);
		/*
		runarb: 17 jan 2008: 
		ser ikke ut til at isprint() fungerer med utf-8.
		Ser uansett ikke ut til at vi trenger den, da vi har fikset alle problemene med 
		titler, men lar den stå litt for sikkerhets skyld.

		for(i = 0; i < strlen(subject); i++) {
			if (!isprint(subject[i])) {
				subject[i] = '\0';
				break;
			}
		}
		*/
		free(p4);
		iconv_close(converter);
	} else {
		printf("!! cant convert by iconv\n");
		strncpy(subject, p, inlen);
	}
}

