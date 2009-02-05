#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define tfile "mmap.dat"

int main() {

	struct stat inode;
	void *mem;
	int fd, i;
	
	if ((fd = open(tfile,O_RDONLY)) == -1) {
		perror(tfile);
		return -1;
	}

	fstat(fd,&inode);

        if ((mem = mmap(0,inode.st_size,PROT_READ,MAP_SHARED,fd,0) ) == MAP_FAILED) {
	        perror("mmap");
		return -1;
        }

	for(i=0;i<inode.st_size;i++) {
		int pr = 0;
        	pr += ((char *)mem)[i];
        }

	while(1) {
		for (i=0;i<inode.st_size;i++) {
			printf("%c\n",((char *)mem)[i]);
		}
		printf("\n");
		sleep(10);
	}
}

