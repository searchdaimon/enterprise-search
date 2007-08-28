
#ifndef _HTML_COMMON_H_
#define _HTML_COMMON_H_


#include <stdarg.h>
#include <stdlib.h>

#include "../ds/dcontainer.h"
#include "html_parser.h"
#include "css_parser.h"

#define maxNewString 1023

typedef struct
{
    char	*data;
    int		pos, maxsize;
    char	overflow;
} buffer;

struct bhpm_yy_extra
{
    char	space;
    int		slen, tt;
    int		stringtop;
    char	stringcircle[32][maxNewString +1];
    char	flush;
    char	field_delimit;
    int		nested_obj;

    char	*stylesheet;
    int		ss_size, ss_block;
    css_selector_block	*css_selector_block;

    char	title, alink; //, script=0;	init:=0
    int		wordcount, linkcount;	// init:=0
    int		h;	// init:=0
    char	invisible_text;

    buffer	*Btitle, *Bbody;
    char	newhead, newdiv, newspan, inhead, indiv, inspan, newendhead, inlink;

    void	*wordlist;
    void	(*user_fn)(char*,int,enum parsed_unit,enum parsed_unit_flag,void* wordlist);
};

#define YY_EXTRA_TYPE	struct bhpm_yy_extra*


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

static inline void bprintf( buffer *B, const char *fmt, ... )
{
    va_list	ap;
    int		len_printed;

    if (B->overflow) return;

    va_start(ap, fmt);
    len_printed = vsnprintf( &(B->data[B->pos]), B->maxsize - B->pos -1, fmt, ap );

    B->pos+= len_printed;

    if (B->pos >= B->maxsize-1) B->overflow = 1;
}


#endif	// _HTML_COMMON_H_
