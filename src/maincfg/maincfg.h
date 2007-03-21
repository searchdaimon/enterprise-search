#ifndef _MAINFCG__H_
#define _MAINFCG__H_

#include <libconfig.h>

struct config_t maincfgopen();
int maincfg_get_int(const config_t *cfg, const char *val);

#endif
