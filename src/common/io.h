
#ifndef _IO_H_
#define _IO_H_

ssize_t io_read_align(int fd, void *buf, size_t count);
size_t fread_all(const void *buf, size_t size, FILE *stream, int redlen);
#endif /* _IO_H_ */
