/*
 *	(C) Boitho 2004-2007, Written by Magnus Galåen
 *
 *  Støttede operatorer er som følger;
 *	+	(Pluss)
 *	-	(Minus)
 *	"	(Strofe)
 *	~	(Minus-strofe)
 *
 *  CHANGELOG:
 *
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


struct
{
    int			n;
    int			stopword;
    char		operand;
    char		**s;
    char		**spelled;
} typedef string_array;

struct
{
    int			n;
    string_array	*query;
} typedef query_array;


/*
 *	Tolker query i 'text', og strukturerer svaret i 'qa':
 */
void get_query( char text[], int text_size, query_array *qa );

/*
 *	Frigjør og deallokerer minne i datastrukturen 'qa':
 */
void destroy_query( query_array *qa );

/*
 *	Gjør om utvidede ascii-tegn til html-escapes i query-et:
 */
void copy_htmlescaped_query( query_array *qa_dest, query_array *qa_src );

#endif	// _QUERY_PARSER_H_
