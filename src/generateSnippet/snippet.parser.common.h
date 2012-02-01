
#ifndef _SNIPPET_PARSER_COMMON_H_
#define	_SNIPPET_PARSER_COMMON_H_

#define bsgp_maxNewString 1023

struct bsgp_yy_extra
{
    char	space;
    char	stringcircle[32][bsgp_maxNewString +1];
    int		stringtop;
};

#ifdef __LP64__
#define YYSTYPE long int
#endif

#define YY_EXTRA_TYPE	struct bsgp_yy_extra*

#endif	// _SNIPPET_PARSER_COMMON_H_
