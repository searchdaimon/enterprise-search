
#ifndef _SUMMARY_COMMON_H_
#define _SUMMARY_COMMON_H_

typedef struct
{
    char*	str;
    char	space;
} Token;

#define maxNewString 2048

struct _sp_yy_extra
{
    Token	token;
    int		top;
    int		stack[128];
    int		stringtop;
    char	stringcircle[128][maxNewString +1];
};

#define YY_EXTRA_TYPE	struct _sp_yy_extra*

#endif	// _SUMMARY_COMMON_H_
