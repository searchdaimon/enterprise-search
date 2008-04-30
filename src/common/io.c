#define _XOPEN_SOURCE 600

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

ssize_t
io_read_align(int fd, void *buf, size_t count)
{
	off_t offset, curoff;
	int pagesize;
	char *abuf;
	ssize_t acount;

	if (buf == NULL) {
		errno = EINVAL;
		return (ssize_t)-1;
	}

	pagesize = getpagesize();
	if ((offset = lseek64(fd, 0, SEEK_CUR)) == (off_t)-1) {
		return (ssize_t)-1;
	}

	curoff = offset;

	acount = (((size_t)count+pagesize-1)/pagesize)*pagesize;
	offset = ((off_t)offset/pagesize)*pagesize;
	if (lseek64(fd, offset, SEEK_SET) == -1) {
		return (ssize_t)-1;
	}

	if (posix_memalign((void **)&abuf, pagesize, acount) != 0) {
		return (ssize_t)-1;
	}


	if ((acount = read(fd, abuf, acount)) == -1) {
		free(abuf);
		return (ssize_t)-1;
	}

	count = count < acount ? count : acount;
	memcpy(buf, abuf+(curoff-offset), count);
	free(abuf);

	return count;
}


size_t fread_all(const void *buf, size_t size, FILE *stream, int redlen) {

        off_t total = 0;        // how many bytes we've sent
        off_t bytesleft = size;  // how many we have left to send
        off_t n;
        int toread;


        while(total < size) {

            if (bytesleft > redlen) {
                toread = redlen;
            }
            else {
                toread = bytesleft;
            }



            if ((n = fread((void *)buf+total, 1, toread,  stream)) == -1) {
	                printf("dident manage to fread all the data as %s:%f.\n",__FILE__,__LINE__);
                        return 0;
            }

	 //	   printf("total %"PRId64" of %"PRId64": n %"PRId64", left %"PRId64"\n",total,size,n,bytesleft);


            total += n;
            bytesleft -= n;
        }

}


