#ifndef _HEADER_BBFILTERS_
#define _HEADER_BBFILTERS_

#include <EXTERN.h>
#include <perl.h>

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../common/define.h"

#include "bbdocdefine.h"

#define FILTER_EXEOC 1
#define FILTER_PERL_PLUGIN 2

#define FILTER_EXEOC_STR "exeoc"
#define FILTER_PERL_PLUGIN_STR "perlplugin"

void run_filter_perlplugin(char *dst, size_t dst_size, struct fileFilterFormat *filter, struct hashtable **metahash);

void run_filter_exeoc(char *dst, const size_t dst_size, struct fileFilterFormat *fileFilter, struct hashtable **metahash);

#ifdef USE_LIBEXTRACTOR
void add_libextractor_attr(struct hashtable **metadata, char *filepath, char **whitelist);
void parse_libextractor_output(struct hashtable *dst, const char *output, char **whitelist);
#endif

#endif
