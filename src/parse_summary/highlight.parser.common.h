
#ifndef _HIGHLIGHT_COMMON_H_
#define _HIGHLIGHT_COMMON_H_

typedef struct
{
    char*	str;
    char	space;
} Token;

#define h_maxNewString 60

struct _hp_yy_extra
{
    Token	token;
    int		stringtop;
    char	stringcircle[128][h_maxNewString +1];
};

#define YY_EXTRA_TYPE	struct _hp_yy_extra*

#endif	// _HIGHLIGHT_COMMON_H_
