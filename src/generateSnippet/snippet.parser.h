
#ifndef _SNIPPET_PARSER_H_
#define _SNIPPET_PARSER_H_

/*
 *	qa: The query to highlight
 *	text: Preparsed document (from parser)
 *	output_text: output is malloc-ed here
 *	b_start, b_end: Separators for highlighting, ex: "<strong>" and "</strong>"
 */
void generate_snippet( query_array qa, char text[], int text_size, char **output_text, char* b_start, char* b_end );

#endif	// _SNIPPET_PARSER_H_
