#ifdef WITH_CONFIG

#define CONFIG_CACHE_IS_OK 0
#define CONFIG_NO_CACHE 1

#define cache_time 30

struct _configdataFormat {
        char configkey[255];
        char configvalue[255];
};

int bconfig_flush(int mode);
const char *bconfig_getentrystr(char vantkey[]);
int bconfig_getentryint(char vantkey[], int *val);
int bconfig_getentryuint(char vantkey[], unsigned int *val);
#endif
