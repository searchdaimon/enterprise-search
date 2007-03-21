#include <iconv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

size_t iconv_simple(iconv_t cd,
                char *inbuf, size_t inbytes,
                char **outbuf, size_t outbytes) {

        size_t outbytesleft = outbytes;
        size_t inbytesleft = inbytes +1;

        return iconv(cd,&inbuf,&inbytesleft,outbuf,&outbytesleft);
}

//inbytes er ikke strlen, men total størelse
size_t iconv_convert(iconv_t cd,char **inbuf, size_t inbytes) {

        size_t outbytes = (inbytes *2);
        char *out = malloc(outbytes);
        size_t n;
        char *outchar;

        outchar = out;

        printf("aa %s\n",(*inbuf));

        if ((n = iconv_simple(cd,(*inbuf),strlen((*inbuf)),&outchar,outbytes)) == -1) {
                perror("w_iconv");
        }

        strncpy((*inbuf),out,inbytes);
}

