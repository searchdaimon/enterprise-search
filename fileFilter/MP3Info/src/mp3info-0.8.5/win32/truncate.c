#ifdef __MINGW32__

#include <fcntl.h>
#include <io.h>

int errno;

int truncate(const char *name, off_t length)
{
    int fd, code, xerrno;

    fd = open(name, O_WRONLY);
    if(fd < 0)
        return -1;
    code = chsize(fd, length);
    xerrno = errno;
    close(fd);
    errno = xerrno;
    return code;
}

#endif
