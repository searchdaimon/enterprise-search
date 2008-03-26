
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
