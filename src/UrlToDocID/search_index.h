
#ifndef _SEARCH_URLTODOCID_INDEX_
#define _SEARCH_URLTODOCID_INDEX_


#include <stdio.h>

typedef struct
{
    FILE	*indexf, *dbf;
    int		index_size;
    unsigned int one_unit;
} urldocid_data;


// init:
urldocid_data* urldocid_search_init(char *index_filename, char *db_filename);
// exit:
void urldocid_search_exit(urldocid_data *data);

// search:
unsigned int urldocid_search_index(urldocid_data *data, unsigned char *sha1);


#endif	// _SEARCH_URLTODOCID_INDEX_
