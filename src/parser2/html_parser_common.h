
#ifndef _HTML_COMMON_H_
#define _HTML_COMMON_H_


#include <stdarg.h>
#include <stdlib.h>

#include "../common/bprint.h"

#include "../ds/dcontainer.h"
#include "html_parser.h"
#include "css_parser.h"

#define maxNewString 1023


struct bhpm_yy_extra
{
#ifdef PARSE_ERR
    char	last[201];
#endif
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

    char	title, alink, nlink; //, script=0;	init:=0
    int		title_nr;
    int		wordcount, linkcount;	// init:=0
    int		h;	// init:=0
    char	invisible_text, illegal_charset;

    buffer	*Btitle, *Bbody;
    char	newhead, newdiv, newspan, inhead, indiv, inspan, newendhead, inlink;

    void	*wordlist;
    void	(*user_fn)(char*,int,enum parsed_unit,enum parsed_unit_flag,void* wordlist);
};

#define YY_EXTRA_TYPE	struct bhpm_yy_extra*


#ifdef PARSE_ERR
static inline void PRINT( struct bhpm_yy_extra *he, const char *fmt, ... )
{
    va_list	ap;
    int		len_printed = 0;
    char	buf[512];

    va_start(ap, fmt);

    len_printed = vsnprintf( buf, 511, fmt, ap );

    if (len_printed>=200)
	{
	    memcpy(he->last, &(buf[len_printed-200]), 200);
	}
    else
	{
	    memmove(he->last, &(he->last[len_printed]), 200 - len_printed);
	    memcpy(&(he->last[200-len_printed]), buf, len_printed);
	}
}
#endif



#endif	// _HTML_COMMON_H_
