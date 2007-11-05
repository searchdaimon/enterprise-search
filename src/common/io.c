#define _XOPEN_SOURCE 600

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


ssize_t
io_read_align(int fd, void *buf, size_t count)
{
	off64_t offset, curoff;
	int pagesize;
	char *abuf;
	ssize_t acount;

	if (buf == NULL) {
		errno = EINVAL;
		return (ssize_t)-1;
	}

	pagesize = getpagesize();
	if ((offset = lseek64(fd, 0, SEEK_CUR)) == (off64_t)-1) {
		return (ssize_t)-1;
	}

	curoff = offset;

	acount = (((size_t)count+pagesize-1)/pagesize)*pagesize;
	offset = ((off64_t)offset/pagesize)*pagesize;
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

