#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "../common/exeoc.h"

#define DO_SUID 1
#define UID_USER 0

#define exec_string "yum -c /etc/yumBoithoOnly.conf shell /etc/yumBoithoOnly.yum"

int main(int argc, char **argv) {
    char exeocbuf[4048];
    int exeocbuflen;
    int exec_return;

        if (DO_SUID) {

            if (setuid(UID_USER) != 0) {
                printf("Unable to setuid(%d)\n", UID_USER);
                exit(2);
            }

        }


    	char *shargs[] = {"/bin/sh", "-c",exec_string, '\0'};
    	exeocbuflen = sizeof(exeocbuf);

    	//printf("kjorer %s %s %s \n", shargs[0], shargs[1], shargs[2]);

    	if (!exeoc(shargs, exeocbuf, &exeocbuflen, &exec_return)) {
        	printf("Unable to execute service\n");
        	exit(EXIT_FAILURE);
    	}
	
	printf(exeocbuf);
	
}
