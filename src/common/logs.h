void blog(FILE *LOG, int level, const char *fmt, ...);
int openlogs(FILE **LOGACCESS, FILE **LOGERROR, char name[]);
void bvlog(FILE *LOG, int level,const char *fmt,va_list ap);
