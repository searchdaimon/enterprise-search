/*
 *	(C) Boitho 2004-2008, Written by Magnus Galåen
 *
 *  Støttede operatorer er som følger;
 *	+	(Pluss)
 *	-	(Minus)
 *	"	(Strofe)
 *	~	(Minus-strofe)
 *	|	(Or)
 *
 *  CHANGELOG:
 *
 *    11.07.2008	Lagt til kommandoer sort og group.
 *    23.01.2008	Lagt til 'string_alternaive' og expand_query()
 *    10.01.2008	Lagt til funksjoner copy_query() og sprint_query().
 *    06.03.2007	Epostadresser blir nå skrevet om til fraser (med '.' og '@' som delimitere).
 *    22.02.2007	Har lagt til støtte for utf-8 unicode. Latin-1-supplement blir automatisk konvertert.
 *    23.11.2006	Har lagt til støtte for kommandoer: filetype/language/collection/date/status
 *    30.10.2006	La til støtte for minus-operator. Fikset potensiell memory-leak ved malformatert query.
 *    10.03.2006	Skiftet datastruktur fra "struct query" (basert på lenket
 * 					liste) til "query_array" (direkte-oppslag).
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
 *	Tolker query i 'text', og strukturerer svaret i 'qa':
 */
void get_query( char text[], int text_size, query_array *qa );

/*
 *	Lager query_array fra vector< pair< int, vector<string> > >:
 */
void make_query_array( container *query, query_array *qa );

/*
 *	Frigjør og deallokerer minne i datastrukturen 'qa':
 *	NB: selve *qa blir ikke deallokert! (se test.c)
 */
void destroy_query( query_array *qa );

/*
 *	Kopierer og allokerer en ny qa:
 */
void copy_query( query_array *dest, query_array *src );

/*
 *	Hent ut "ryddig" query-streng:
 */
void sprint_query( char *s, int n, query_array *qa );
char* asprint_query( query_array *qa );
int bsprint_query_with_remove( buffer *B, container *remove, query_array *qa, int escape );

/*
 *	Hent ut "ryddig" query-streng:
 */
void sprint_expanded_query( char *s, int n, query_array *qa );

/*
 *	Skriv ut query_array:
 */
void sprint_query_array( char *s, int n, query_array *qa );

/*
 *	Deprecated (vi har gått over til utf-8): Gjør om utvidede ascii-tegn til html-escapes i query-et:
 */
void copy_htmlescaped_query( query_array *qa_dest, query_array *qa_src );

#endif	// _QUERY_PARSER_H_
