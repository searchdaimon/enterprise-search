#ifndef HEADER_PERLEMBED
#define HEADER_PERLEMBED
#include <EXTERN.h>
#include <perl.h>
#include "perlxsi.h"
#include "../3pLibs/keyValueHash/hashtable.h"

#define PERSISTENT_PATH "perl/persistent.pl"
#define SD_CRAWL_LIB_PATH "perlxs/SD-Crawl"

/* needs to be called my_perl, because of perlxs macros. 
   Also needs to be global if we want to use perlxs functions
   in other places. */
PerlInterpreter *my_perl; 

void perl_embed_init(char **incl_path, int cache_perl_files);
void perl_embed_clean(void);
int perl_embed_run(char *file_path, char *func_name, HV *func_params, char *obj_name, HV *obj_attr, char *error, int errorlength);
int perl_embed_run_arr(char *file_path, char *func_name, HV *func_params, char *obj_name, HV *obj_attr, char *error, int errorlength, char ***retarray, int *retlength);

void ht_to_perl_ht(HV *perl_ht, struct hashtable *params);
void perl_ht_to_ht(HV *perl_ht, struct hashtable *ht);

int perl_ht_add_str(HV *ht, char *key, char *val);

#endif
