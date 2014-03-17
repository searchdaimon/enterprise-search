
#ifndef _SNIPPET_PARSER_H_
#define _SNIPPET_PARSER_H_

/*
 *	qa: The query to highlight
 *	text: Preparsed document (from parser)
 *	output_text: output is malloc-ed here
 *	b_start, b_end: Separators for highlighting, ex: "<strong>" and "</strong>"
 *	snippet_size: Maximum length of snippet
 *	is_table: render as table, not as text
 *      rows: number of rows to display in table-form
 *
 *	Return value: 1 if successful, 0 if failure.
 */

enum snippet_mode { plain_snippet, first_snippet, db_snippet, mplain_snippet/*intern*/ };


int generate_snippet( query_array qa, char text[], int text_size, char **output_text, char* b_start, char* b_end, int mode, int _snippet_size, int rows, int cols, int *has_hits, int format);


#endif	// _SNIPPET_PARSER_H_
