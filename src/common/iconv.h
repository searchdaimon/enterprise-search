#include <iconv.h>

size_t iconv_simple(iconv_t cd,
                char *inbuf, size_t inbytes,
                char **outbuf, size_t outbytes);

size_t iconv_convert(iconv_t cd,char **inbuf, size_t inbytes);
