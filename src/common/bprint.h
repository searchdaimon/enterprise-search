
#ifndef _BPRINT_H_
#define _BPRINT_H_

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_BLOCKSIZE	16384

typedef struct
{
    char	*data;
    int		pos, maxsize;
    char	overflow, growing;
} buffer;


static inline buffer* buffer_init( int _maxsize )
{
    buffer	*B = malloc(sizeof(buffer));

    B->overflow = 0;
    B->pos = 0;

    if (_maxsize > 0)
	{
	    B->maxsize = _maxsize -1;
	    B->growing = 0;
	}
    else
	{
	    B->maxsize = BUFFER_BLOCKSIZE -1;
	    B->growing = 1;
	}

    B->data = malloc(B->maxsize +1);

    return B;
}

static inline int buffer_length(buffer *B)
{
	return B->pos;
}

static inline char* buffer_exit( buffer *B )
{
    char	*output;

    output = malloc( B->pos +1 );
    memcpy( output, B->data, B->pos );
    output[B->pos] = '\0';

    free( B->data );
    free( B );

    return output;
}

static inline char* buffer_abort( buffer *B )
{
    char	*output;

    output = strdup("");

    free( B->data );
    free( B );

    return output;
}

static inline void bmemcpy(buffer *B, void *src, size_t len)
{
	size_t needsize = 0;

	assert(B->growing);

	/* XXX: Learn division */
	while (len > B->maxsize - B->pos - 1 + needsize)
		needsize += BUFFER_BLOCKSIZE;

	if (needsize > 0) {
		B->data = realloc(B->data, B->maxsize + needsize);
		B->maxsize += needsize;
	}

	memcpy(B->data+B->pos, src, len);
	B->pos += len;
	B->data[B->pos] = '\0';
}


static inline void bprintf( buffer *B, const char *fmt, ... )
{
    va_list	ap;
    int		len_printed, old_pos;

    if (B->overflow) return;

    va_start(ap, fmt);

//    len_printed = vprintf( fmt, ap );
    old_pos = B->pos;
    len_printed = vsnprintf( &(B->data[B->pos]), B->maxsize - B->pos -1, fmt, ap );
    B->pos+= len_printed;

    va_end(ap);

    if (B->pos >= B->maxsize-1)
	{
	    if (B->growing)
		{
		    if (len_printed < BUFFER_BLOCKSIZE) B->maxsize+= BUFFER_BLOCKSIZE;
		    else B->maxsize+= len_printed+1;

		    B->data = realloc(B->data, B->maxsize);

		    va_start(ap, fmt);
		    B->pos = old_pos;
		    len_printed = vsnprintf( &(B->data[B->pos]), B->maxsize - B->pos -1, fmt, ap );
		    B->pos+= len_printed;
		    va_end(ap);
		}
	    else
		{
		    fprintf(stderr, "bprint: Warning! Buffer overflow.\n");
		    B->overflow = 1;
		}
	}
}


#endif	// _BPRINT_H_
