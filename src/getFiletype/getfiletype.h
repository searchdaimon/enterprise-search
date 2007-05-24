
#ifndef GET_FILETYPE_H
#define GET_FILETYPE_H


#include "../common/search_automaton.h"

typedef struct
{
    automaton	*A;
    char	**desc;
    int		size;
} filetypes_info;


filetypes_info* getfiletype_init(char *lang_file);

void getfiletype_destroy(filetypes_info *fti);

char* getfiletype(filetypes_info *fti, char *extension);


#endif	// GET_FILETYPE_H
