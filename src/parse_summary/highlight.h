
#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_

#include "./../query/query_parser.h"

void generate_highlighting( query_array qa, char text[], int text_size, char **output_text );

#endif	// _HIGHLIGHT_H_
