#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 40960000

int main (int argc, char *argv[]) {


        if (argc < 2) {
                printf("Dette programet kopierer en lot til en annen server\n\n\t./lotcp lotnr subname\n\n");
               exit(0);
        }

        char *filename = argv[1];


	int fd,n;
	int tot = 0;

	int pagesize;
	void *realbuff;
	void *alignedbuff;
	int readsize;

	pagesize=getpagesize();
	readsize = BUFFER_SIZE+pagesize;	
	realbuff=malloc(readsize);
	alignedbuff=((((int unsigned)realbuff+pagesize-1)/pagesize)*pagesize);

	printf("pagesize %i\n",pagesize);

        if ((fd = open(filename, O_RDONLY|O_DIRECT)) == -1) {
		perror("open");
                return 0;
        }


	//ssize_t read(int fd, void *buf, size_t count);

	while((n=read(fd,alignedbuff,readsize)) > 0) {
		//printf("b \"%c\"\n",buf[0]);
		//printf("reading\n");
		tot += n;
	}
	printf("n: %i\n",n);
	perror("read");

	close(fd);

	printf("tot %i\n",tot);

	return 1;
}

