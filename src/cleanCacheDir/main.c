#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>


#define cashedir "/tmp"



int main () {

	DIR *TD;
	struct dirent *dp;
	struct stat inode;
	char path[512];

	if ((TD = opendir(cashedir)) == NULL) {
		perror(cashedir);
	}

	
	while ((dp = readdir(TD)) != NULL) {
		if (dp->d_type == DT_REG) {

			sprintf(path,"%s/%s",cashedir,dp->d_name);

			if (stat(path, &inode) != 0) {
				perror(dp->d_name);
			}

			printf("aa %s, %i, %li %li %li\n",dp->d_name,dp->d_fileno,inode.st_atime,inode.st_mtime,inode.st_ctime);

		}
	}
}
