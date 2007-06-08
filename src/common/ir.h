#include "crc32.h"

int isStoppWord(char *term);
int isShortWord(char *term);

#define calcDomainID(x) (crc32boitho(x) % 65535)
