/*
 *	(C) Boitho 2004-2008, Written by Magnus Galåen
 *
 *  Supported operators are as follows;
 *	+	(Plus)
 *	-	(Minus)
 *	"	(Phrase)
 *	~	(Minus-phrase)
 *	|	(Or)
 *
 *  CHANGELOG:
 *
 *    11.07.2008	Added commands sort and group.
 *    23.01.2008	Added 'string_alternaive' and expand_query()
 *    10.01.2008	Added functions copy_query() and sprint_query().
 *    06.03.2007	Email addresses are now rewritten into phrases (with '.' and '@' as delimiters).
 *    22.02.2007	Added support for utf-8 unicode. Latin-1 supplement is automatically converted.
 *    23.11.2006	Added support for commands: filetype/language/collection/date/status
 *    30.10.2006	Added support for the minus operator. Fixed a potential memory leak on malformatted query.
 *    10.03.2006	Changed data structure from "struct query" (based on a linked
 * 					list) to "query_array" (direct lookup).
 */

#ifndef _QUERY_PARSER_H_
#define _QUERY_PARSER_H_

#define QUERY_WORD	'+'
#define QUERY_SUB	'-'
#define QUERY_PHRASE	'"'
#define QUERY_SUBPHRASE	'~'
#define QUERY_FILETYPE	'f'
#define QUERY_LANGUAGE	'l'
#define QUERY_COLLECTION 'c'
#define QUERY_DATE	'd'
#define QUERY_STATUS	's'
#define QUERY_OR	'|'
#define QUERY_SORT	'k'
#define QUERY_GROUP	'g'
#define QUERY_ATTRIBUTE	'a'

#include "../ds/dcontainer.h"
#include "../common/bprint.h"

typedef struct
{
    int			n;
    int			stopword;
    char		**s;
} string_alternative;

typedef struct
{
    int			n;
    int			stopword;
    char		operand;
    char		**s;
    char		**spelled;
    char		hide;
    int			alt_n;
    string_alternative	*alt;		// Close words and phrases (stems and/or synonyms)
} string_array;

typedef struct
{
    int			n;
    string_array	*query;
} query_array;


/*
 *	Parses the query in 'text', and structures the result in 'qa':
 */
void get_query( char text[], int text_size, query_array *qa );

/*
 *	Builds a query_array from vector< pair< int, vector<string> > >:
 */
void make_query_array( container *query, query_array *qa );

/*
 *	Frees and deallocates memory in the 'qa' data structure:
 *	NB: *qa itself is not deallocated! (see test.c)
 */
void destroy_query( query_array *qa );

/*
 *	Copies and allocates a new qa:
 */
void copy_query( query_array *dest, query_array *src );

/*
 *	Extract a "clean" query string:
 */
void sprint_query( char *s, int n, query_array *qa );
char* asprint_query( query_array *qa );
int bsprint_query_with_remove( buffer *B, container *remove, query_array *qa, int escape );

/*
 *	Extract a "clean" query string:
 */
void sprint_expanded_query( char *s, int n, query_array *qa );

/*
 *	Print the query_array:
 */
void sprint_query_array( char *s, int n, query_array *qa );

/*
 *	Deprecated (we have switched to utf-8): Converts extended ascii characters into html escapes in the query:
 */
void copy_htmlescaped_query( query_array *qa_dest, query_array *qa_src );

#endif	// _QUERY_PARSER_H_
