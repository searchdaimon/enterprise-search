
#ifndef _BPRINT_H_
#define _BPRINT_H_


#include <stdarg.h>
#include <stdlib.h>


typedef struct
{
    char	*data;
    int		pos, maxsize;
    char	overflow;
} buffer;


static inline buffer* buffer_init( int _maxsize )
{
    buffer	*B = malloc(sizeof(buffer));

    B->overflow = 0;
    B->pos = 0;
    B->maxsize = _maxsize -1;
    B->data = malloc(_maxsize);

    return B;
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


static inline void bprintf( buffer *B, const char *fmt, ... )
{
    va_list	ap;
    int		len_printed;

    if (B->overflow) return;

    va_start(ap, fmt);

#ifdef SUPERDEBUG
    len_printed = vprintf( fmt, ap );
#else
    len_printed = vsnprintf( &(B->data[B->pos]), B->maxsize - B->pos -1, fmt, ap );
#endif

    B->pos+= len_printed;

    if (B->pos >= B->maxsize-1)
	{
	    fprintf(stderr, "html_parser: Warning! Buffer overflow, skipping document.\n");
	    B->overflow = 1;
	}
}


#endif	// _BPRINT_H_
