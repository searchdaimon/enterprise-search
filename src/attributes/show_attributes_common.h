
#ifndef _READ_ATTRIB_CONF_COMMON_H_
#define _READ_ATTRIB_CONF_COMMON_H_

#include "../common/bprint.h"

struct rac_yy_extra
{
    int		space, next;
    int		line;
    char	buf[4096];
    int		ptr, last_ptr;
    buffer	*Bwarnings;
};

#ifdef __LP64__
#define YYSTYPE long int
#endif

#define YY_EXTRA_TYPE struct rac_yy_extra*

#endif	// _READ_ATTRIB_CONF_COMMON_H_
