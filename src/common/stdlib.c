#include <string.h>
#include <stdio.h>

//er ikke tread sikker
char *bitoa(int val) {
        static char buf[13];
        sprintf(buf,"%i",val);

        return buf;
}

unsigned int atou(char buf[]) {
	return strtoul(buf, (char **)NULL, 10);
}
