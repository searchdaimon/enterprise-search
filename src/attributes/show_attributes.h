
#ifndef READ_ATTR_CONF_H
#define READ_ATTR_CONF_H

#include "../ds/dcontainer.h"


enum attr_sort_enum { sort_none, sort_hits, sort_alpha };
enum attr_item_flags { show_empty=2, is_expanded=4, sort_reverse=8, build_groups=16 };
enum attr_item_type { item_group, item_select, item_import };

struct attr_group
{
    container	*child;
    char	*name;
    container	*alt_names;
    container	*hide;
    char 	flags;
    enum attr_sort_enum	sort;
    int		max_items;
    int		max_sub_items;
};

struct attr_select
{
    char	**select;//, **from;
    char	flags;
    int		size;
};

struct attr_import
{
    char	**import;
    int		size;
};


typedef struct attr_group attr_conf;

attr_conf* show_attributes_init( char *conf_file, char **warnings, int *failed, int verbose);

void show_attributes_destroy( attr_conf *ac );

#endif	// READ_ATTR_CONF_H
