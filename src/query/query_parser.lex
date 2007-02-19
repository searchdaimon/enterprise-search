%{
/*
 *	(C) Boitho 2004-2006, Written by Magnus Galåen
 *
 */
#include <stdio.h>
#include <string.h>
#include "query_parser.h"


query_array _query_array_init( int n );
void _query_array_destroy( query_array qa );
string_array _string_array_init( int n );
void _string_array_destroy( string_array sa );

struct _qp_word_temp_query;
void _qp_word_init( char operand, yyscan_t scanner );
void _qp_word_add( char *text, yyscan_t scanner );
void _qp_word_exit( yyscan_t scanner );

struct _qp_text_list
{
    char			*text;
    struct _qp_text_list	*next;
};

struct _qp_word_temp_query
{
    char			operand;
    struct _qp_text_list	*elem;
    struct _qp_word_temp_query	*next;
};

struct _qp_yy_extra
{
    struct _qp_text_list		*_qp_word_tl_last;
    struct _qp_word_temp_query		*_qp_word_first, *_qp_word_last;
    int					_qp_word_size;

    char				*custom_input;
    int					custom_pos, custom_size;

    char				is_prefix, is_accepting;
};

/*
    Spesialtilfeller:

	Velger å tolke [ ole-"dole" ] som [ ole -"dole" ]
	Dersom avsluttende parantes mangler, tolker vi det som det skulle vært en etter siste ord i query-et.
*/

#define YY_EXTRA_TYPE	struct _qp_yy_extra*

%}

word		[0-9a-z'_ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ]
%option	noyywrap reentrant
%x PHRASE COMMAND CMD_PHRASE
%%
\+?[dD][aA][tT][eE]\ *:	{ _qp_word_init('d', yyscanner);		BEGIN COMMAND; }
\+?[sS][tT][aA][tT][uU][sS]\ *:	{ _qp_word_init('s', yyscanner);	BEGIN COMMAND; }
\+?[fF][iI][lL][eE][tT][yY][pP][eE]\ *:	{ _qp_word_init('f', yyscanner); BEGIN COMMAND; }
\+?[lL][aA][nN][gG][uU][aA][gG][eE]\ *:	{ _qp_word_init('l', yyscanner); BEGIN COMMAND; }
\+?[cC][oO][lL][lL][eE][cC][tT][iI][oO][nN]\ *:	{ _qp_word_init('c', yyscanner); BEGIN COMMAND; }
{word}+			{
			    _qp_word_init('+', yyscanner);
			    _qp_word_add( yytext, yyscanner );
			    _qp_word_exit( yyscanner );
			}
\+{word}+		{
			    _qp_word_init('+', yyscanner);
			    _qp_word_add( &(yytext[1]), yyscanner );
			    _qp_word_exit( yyscanner );
			}
\ 			{
			    yyget_extra(yyscanner)->is_prefix = 1;
			}
\-{word}+		{
			    if ( yyget_extra(yyscanner)->is_prefix )
				{
				    _qp_word_init('-', yyscanner);
				    _qp_word_add( &(yytext[1]), yyscanner );
				    _qp_word_exit( yyscanner );
				}
			    else
				{
				    _qp_word_init('+', yyscanner);
				    _qp_word_add( &(yytext[1]), yyscanner );
				    _qp_word_exit( yyscanner );
				}
			}
\"			{ _qp_word_init('"', yyscanner);		BEGIN PHRASE; }
\-\"			{ _qp_word_init('~', yyscanner);		BEGIN PHRASE; }
<COMMAND>\"		{ 						BEGIN CMD_PHRASE; }
<COMMAND>{word}+	{ _qp_word_add( yytext, yyscanner ); _qp_word_exit( yyscanner ); BEGIN INITIAL; }
<CMD_PHRASE>{word}+	{ _qp_word_add( yytext, yyscanner ); }
<CMD_PHRASE>\"		{ _qp_word_exit( yyscanner );			BEGIN INITIAL; }
<PHRASE>{word}+		{ _qp_word_add( yytext, yyscanner ); }
<PHRASE>\"		{ _qp_word_exit( yyscanner );			BEGIN INITIAL; }
<*>.|\n			{}
%%


// Initialiser og alloker et nytt query_array:
query_array _query_array_init( int n )
{
    query_array		qa;

    qa.n = n;
    qa.query = malloc(sizeof(string_array[qa.n]));

    return qa;
}

// Frigjør et gammelt query_array:
void _query_array_destroy( query_array qa )
{
    free( qa.query );
}

// Initialiser og alloker et nytt string_array:
string_array _string_array_init( int n )
{
    string_array	sa;

    sa.n = n;
    sa.s = malloc(sizeof(char*[sa.n]));

    return sa;
}

// Frigjør et gammelt string_array:
void _string_array_destroy( string_array sa )
{
    free( sa.s );
}

// Initialiser og klargjør for innlesing av nytt ord eller strofe:
void _qp_word_init( char operand, yyscan_t scanner )
{
    struct _qp_yy_extra		*qe = yyget_extra( scanner );

    qe->_qp_word_last->operand = operand;
    qe->_qp_word_last->elem = NULL;
    qe->_qp_word_tl_last = NULL;
    qe->is_accepting = 1;
}

// Legg til ord:
void _qp_word_add( char *text, yyscan_t scanner )
{
    struct _qp_yy_extra		*qe = yyget_extra( scanner );

    qe->is_prefix = 0;

    if (qe->_qp_word_tl_last==NULL)
	{
	    qe->_qp_word_last->elem = malloc(sizeof(struct _qp_text_list));
	    qe->_qp_word_tl_last = qe->_qp_word_last->elem;
	}
    else
	{
	    qe->_qp_word_tl_last->next = malloc(sizeof(struct _qp_text_list));
	    qe->_qp_word_tl_last = qe->_qp_word_tl_last->next;
	}

    qe->_qp_word_tl_last->text = strdup(text);
    qe->_qp_word_tl_last->next = NULL;
}

// Avslutt innlesing av ord eller strofe:
void _qp_word_exit( yyscan_t scanner )
{
    struct _qp_yy_extra		*qe = yyget_extra( scanner );

    if (qe->_qp_word_tl_last == NULL) // Ingen elementer har blitt lagt til! (f.eks tommer paranteser [ " " ])
	{
	    // Det har ikke blitt allokert noe minne, så det er ingenting å frigjøre.
	    qe->is_accepting = 0;
	    return;
	}

    qe->_qp_word_last->next = malloc(sizeof(struct _qp_word_temp_query));
    qe->_qp_word_last = qe->_qp_word_last->next;
    qe->_qp_word_last->next = NULL;
    qe->_qp_word_size++;
    qe->is_accepting = 0;
}


// Tolker query i 'text', resultatet legges i 'qa':
void get_query( char text[], int text_size, query_array *qa )
{
    struct _qp_yy_extra		*qe = malloc(sizeof(struct _qp_yy_extra));
    int				i;

    qe->is_prefix = 1;
    qe->is_accepting = 0;

    qe->custom_input = text;
    qe->custom_pos = 0;
    qe->custom_size = text_size;

    qe->_qp_word_first = malloc(sizeof(struct _qp_word_temp_query));
    qe->_qp_word_last = qe->_qp_word_first;
    qe->_qp_word_size = 0;

    yyscan_t	scanner;

    yylex_init( &scanner );
    yyset_extra( qe, scanner );
    YY_BUFFER_STATE	bs = yy_scan_bytes( text, text_size, scanner );

    yylex( scanner );

    // I tilfelle en parantes ikke er avsluttet:
    if (qe->is_accepting)
	_qp_word_exit( scanner );

    yy_delete_buffer( bs, scanner );
    yylex_destroy( scanner );

    // Gjør om querydata til en mer praktisk struktur (for direkteoppslag):

    (*qa) = _query_array_init(qe->_qp_word_size);

    struct _qp_word_temp_query	*it = qe->_qp_word_first, *old_it;

    for (i = 0; i<qe->_qp_word_size; i++)
	{
	    int		n=0;
	    struct _qp_text_list	*t_it = it->elem, *old_t_it;
	    while (t_it!=NULL)
	    	{
	    	    n++;
	    	    t_it = t_it->next;
	    	}

	    qa->query[i] = _string_array_init(n);

	    // Fiks for strofer med kun et element:
	    if (it->operand == '"' && n==1)
		qa->query[i].operand = '+';
	    else if (it->operand == '~' && n==1)
		qa->query[i].operand = '-';
	    else
		qa->query[i].operand = it->operand;

	    n = 0;
	    t_it = it->elem;
	    while (t_it!=NULL)
	        {
	            qa->query[i].s[n] = t_it->text;
	            n++;
	            old_t_it = t_it;
	            t_it = t_it->next;
	            free(old_t_it);
	        }


	    old_it = it;
	    it = it->next;
	    free(old_it);
	}

    free(it);	// Det vil alltid være et tomt siste element i lista som må deallokeres.
    free(qe);
}

/******************************************/

struct _qp_html_esc
{
    char	c, *esc;
};

struct _qp_html_esc _qp_he[65] = {
    {'²',"sup2"},{'³',"sup3"},{'¹',"sup1"},{'À',"Agrave"},{'Á',"Aacute"},{'Â',"Acirc"},{'Ã',"Atilde"},{'Ä',"Auml"},
    {'Å',"Aring"},{'Æ',"AElig"},{'Ç',"Ccedil"},{'È',"Egrave"},{'É',"Eacute"},{'Ê',"Ecirc"},{'Ë',"Euml"},{'Ì',"Igrave"},
    {'Í',"Iacute"},{'Î',"Icirc"},{'Ï',"Iuml"},{'Ð',"ETH"},{'Ñ',"Ntilde"},{'Ò',"Ograve"},{'Ó',"Oacute"},{'Ô',"Ocirc"},
    {'Õ',"Otilde"},{'Ö',"Ouml"},{'Ø',"Oslash"},{'Ù',"Ugrave"},{'Ú',"Uacute"},{'Û',"Ucirc"},{'Ü',"Uuml"},{'Ý',"Yacute"},
    {'Þ',"THORN"},{'ß',"szlig"},{'à',"agrave"},{'á',"aacute"},{'â',"acirc"},{'ã',"atilde"},{'ä',"auml"},{'å',"aring"},
    {'æ',"aelig"},{'ç',"ccedil"},{'è',"egrave"},{'é',"eacute"},{'ê',"ecirc"},{'ë',"euml"},{'ì',"igrave"},{'í',"iacute"},
    {'î',"icirc"},{'ï',"iuml"},{'ð',"eth"},{'ñ',"ntilde"},{'ò',"ograve"},{'ó',"oacute"},{'ô',"ocirc"},{'õ',"otilde"},
    {'ö',"ouml"},{'ø',"oslash"},{'ù',"ugrave"},{'ú',"uacute"},{'û',"ucirc"},{'ü',"uuml"},{'ý',"yacute"},{'þ',"thorn"},
    {'ÿ',"yuml"}};


int _qp_esc_compare(const void *a, const void *b)
{
    if (*((char*)a) < ((struct _qp_html_esc*)b)->c) return -1;
    if (*((char*)a) > ((struct _qp_html_esc*)b)->c) return +1;
    return 0;
}


char* _qp_convert_to_html_escapes( char *src )
{
    char	*dest;
    int		i, j, size=0;

    for (i=0; src[i]!='\0'; i++)
	{
	    if ((unsigned char)src[i]<128)
		{
		    size++;
		}
	    else
		{
		    struct _qp_html_esc	*p = (struct _qp_html_esc*)
		    bsearch( (const void*)(((char*)&(src[i]))), _qp_he, 65, sizeof(struct _qp_html_esc), _qp_esc_compare);

		    if (p==NULL)
			size++;
		    else
			size+= strlen(p->esc) +2;
		}
	}

    dest = malloc(size+1);

    for (i=0, j=0; src[i]!='\0'; i++)
	{
	    if ((unsigned char)src[i]<128)
		{
		    dest[j++] = src[i];
		}
	    else
		{
		    struct _qp_html_esc	*p = (struct _qp_html_esc*)
		    bsearch( (const void*)(((char*)&(src[i]))), _qp_he, 65, sizeof(struct _qp_html_esc), _qp_esc_compare);

		    if (p==NULL)
			dest[j++] = src[i];
		    else
			{
			    dest[j++] = '&';
			    strcpy( &(dest[j]), p->esc );
			    j+= strlen(p->esc);
			    dest[j++] = ';';
			}
		}
	}

    dest[size] = '\0';

    return dest;
}

/******************************************/

void copy_htmlescaped_query( query_array *qa_dest, query_array *qa_src )
{
    int		i, j;

    (*qa_dest) = _query_array_init( qa_src->n );

    for (i = 0; i<qa_src->n; i++)
	{
	    qa_dest->query[i] = _string_array_init( qa_src->query[i].n );
	    qa_dest->query[i].operand = qa_src->query[i].operand;

	    for (j = 0; j<qa_src->query[i].n; j++)
		{
		    qa_dest->query[i].s[j] = _qp_convert_to_html_escapes( qa_src->query[i].s[j] );
		}
	}
}


// Frigjør data i 'qa':
void destroy_query( query_array *qa )
{
    int		i, j;

    for (i=0; i<qa->n; i++)
	{
	    for (j=0; j<qa->query[i].n; j++)
		{
		    free( qa->query[i].s[j] );	// Minnet her ble allokert av strdup.
		}

	    _string_array_destroy( qa->query[i] );
	}

    _query_array_destroy( *qa );
}

