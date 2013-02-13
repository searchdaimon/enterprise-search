#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//er ikke tread sikker
char *bitoa(int val) {
        static char buf[13];
        sprintf(buf,"%i",val);

        return buf;
}

//er ikke tread sikker
char *utoa(unsigned int val) {
        static char buf[13];
        sprintf(buf,"%u",val);

        return buf;
}

//er ikke tread sikker
char *ftoa(double val) {
        static char buf[13];
        sprintf(buf,"%f",val);

        return buf;
}

unsigned int atou(char buf[]) {
	return strtoul(buf, (char **)NULL, 10);
}

int *intdup(int i) {
        int *ret;

        if ((ret = malloc(sizeof(int))) == NULL) {
                return NULL;
        }
        (*ret) = i;

        return ret;
}
unsigned int *intudup(unsigned int i) {
        unsigned int *ret;

        if ((ret = malloc(sizeof(unsigned int))) == NULL) {
                return NULL;
        }
        (*ret) = i;

        return ret;
}

