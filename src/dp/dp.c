#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

#include "../common/boithohome.h"

#define DP_LOCK_FILE "var/dp.lock"

static int DP_LOCK = NULL;

int dp_priority_locl_start() {

        //open the lock file
        //if ((DP_LOCK =open(DP_LOCK_FILE,O_CREAT|O_RDWR,S_IRWXU)) == -1) {
	if ((DP_LOCK =open(bfile(DP_LOCK_FILE),O_CREAT|O_RDWR,S_IRWXU)) == -1) {

                perror(DP_LOCK_FILE);
	
		return 0;
        }

        flock(DP_LOCK,LOCK_SH);

	return 1;
}

int dp_priority_locl_end() {

	close(DP_LOCK);

	return 1;
}
