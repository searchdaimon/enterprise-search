#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#include "../acls/acls.h"


int main (int argc, char *argv[]) {
        FILE *fp;
        char username[MAX_USER_NAME_LEN];
        DB *dbp = NULL;
        DBT key, data;
        int ret;
        int *dbpp;

        if (argc != 3) {
                printf("usgae ./mergeUserToSubname lotnr subname\n");
                exit(1);
        }

        if (!userToSubname_open(&dbpp)) {
                perror("userToSubname_open");
                exit(1);
        }

	


	userToSubname_close(&dbpp);
}
