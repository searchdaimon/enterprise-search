/*
 * Bison defaults the semantic value type (YYSTYPE) to int, but the lexer hands
 * the parser char* token text through it. On 64-bit that truncates the pointer
 * and sign-extends it back, so the first parsed tag segfaults (searchd result
 * threads, IndexerLot). Use a pointer-sized type. This header is included by
 * both the .y and the .l before the generated *.tab.h, which keeps its own
 * typedef only when YYSTYPE is still undefined.
 */
#ifndef YYSTYPE
#define YYSTYPE char *
#endif


#ifndef _CSS_COMMON_H_
#define _CSS_COMMON_H_

#include "css_parser.h"


struct bcpm_yy_extra
{
    char	text[32][512];
    int		string;

    int		state;
    int		in_function;
    int		inside_selector;
};

#define YY_EXTRA_TYPE struct bcpm_yy_extra*


#endif	// _CSS_COMMON_H_
