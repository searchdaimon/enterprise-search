#include <stdarg.h>

void berror(const char *fmt, ...);
void berror_safe(const char *str);
void bperror(const char *fmt, ...);
char *bstrerror();
void bverror(const char *fmt,va_list     ap);
