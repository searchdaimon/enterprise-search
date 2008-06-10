%{
#include <stdlib.h>
#include <string.h>

#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dlist.h"
#include "../ds/dstack.h"

#include "css_parser_common.h"
#include "bcpm.tab.h"

/**
 * Simple CSS-parser for detecting font properties.
 *
 * (C) SearchDaimon 2007, Magnus Galåen (magnusga@idi.ntnu.no)
 */


static inline void savestring( char *text, yyscan_t scanner );

/*
name		{nmchar}+
nmstart		[_a-z]|{nonascii}|{escape}
nonascii	[^\0-\177]
unicode		\\[0-9a-f]{1,6}(\r\n|[ \n\r\t\f])?
escape		{unicode}|\\[^\n\r\f0-9a-f]
nmchar		[_a-z0-9-]|{nonascii}|{escape}
num		[0-9]+|[0-9]*\.[0-9]+
string		{string1}|{string2}
string1		\"([^\n\r\f\\"]|\\{nl}|{escape})*\"
string2		\'([^\n\r\f\\']|\\{nl}|{escape})*\'
invalid		{invalid1}|{invalid2}
invalid1	\"([^\n\r\f\\"]|\\{nl}|{escape})*
invalid2	\'([^\n\r\f\\']|\\{nl}|{escape})*
nl		\n|\r\n|\r|\f
w		[ \t\r\n\f]*

ident		[a-z0-9_\-]

*/
%}

adent		[a-z0-9_\-]
hex		[0-9a-f]
blank		[\ \t\r\n\f]
ident		[-]?{nmstart}{nmchar}*
nmstart		[_a-z]|{nonascii}|{escape}
nonascii	[^\0-\177]
unicode		\\[0-9a-f]{1,6}(\r\n|[ \n\r\t\f])?
escape		{unicode}|\\[^\n\r\f0-9a-f]
nmchar		[_a-z0-9-]|{nonascii}|{escape}
num		-?[0-9]+|[0-9]*\.[0-9]+
string		{string1}|{string2}
string1		\"([^\n\r\f\\"]|\\{nl}|{escape})*\"
string2		\'([^\n\r\f\\']|\\{nl}|{escape})*\'
nl		\n|\r\n|\r|\f
w		[ \t\r\n\f]*
%option	nomain noyywrap reentrant bison-bridge
%x	COMMENT BLOCK
%%
@[^;]*;				{} /* Ignore at-rules */
<*>\<\!--|--\>			{}
<BLOCK>(color|background-color|background)	{
//				    ((struct bcpm_yy_extra*)yyget_extra(yyscanner))->color_ident = 1;
				    savestring(yytext, yyscanner);
				    return COLOR_IDENT;
				}
<BLOCK>(font|font-size)		{
				    savestring(yytext, yyscanner);
				    return FONTSZ_IDENT;
				}
<BLOCK>visibility		{
				    savestring(yytext, yyscanner);
				    return VISBLTY_IDENT;
				}
<BLOCK>\!important		{}
<INITIAL,BLOCK>{ident}		{
				    savestring(yytext, yyscanner);
				    return IDENT;
				}
<BLOCK>{num}			{ savestring(yytext, yyscanner); return NUMBER; }
<BLOCK>{num}%			{ savestring(yytext, yyscanner); return PERCENTAGE; }
<BLOCK>{num}px			{ savestring(yytext, yyscanner); return PX_DIMENSION; }
<BLOCK>{num}{ident}		{ savestring(yytext, yyscanner); return DIMENSION; }
<BLOCK>\#{ident}|\#{hex}+	{ savestring(yytext, yyscanner); return HASH; }
<BLOCK>url\({w}{string}{w}\)|url\({w}([!#$%&*-~]|{nonascii}|{escape})*{w}\)	{ savestring(yytext, yyscanner); return URI; }
<BLOCK>{ident}\(	{
			    savestring(yytext, yyscanner);
			    ((struct bcpm_yy_extra*)yyget_extra(yyscanner))->in_function = 1;
			    return FUNCTION_BEGIN;
			}
<BLOCK>\)		{
			    struct bcpm_yy_extra	*ce = yyget_extra(yyscanner);

			    if (ce->in_function)
				{
				    ce->in_function = 0;
				    return FUNCTION_END;
				}
//			    else
//				printf("INVALID: )\n");
			}
<INITIAL>\#{ident}	{
			    savestring(yytext, yyscanner);
			    return IDSELECTOR;
			}
<INITIAL>\.{ident}	{
			    savestring(yytext, yyscanner);
			    return CLASSELECTOR;
			}
<INITIAL>\:{ident}	{
			    savestring(yytext, yyscanner);
			    return PSEUDOCLASS;
			}
<INITIAL>\{		{
			    BEGIN BLOCK;
//			    struct bcpm_yy_extra	*ce = yyget_extra(yyscanner);

//			    printf("\033[1;34mbrace_start: %s\033[0m\n", yytext);
			    return BRACE_START;
			}
<BLOCK>:		{
//			    struct bcpm_yy_extra	*ce = yyget_extra(yyscanner);

//			    printf("\033[1;34mcolon: %s\033[0m\n", yytext);
			    return COLON;
			}
<BLOCK>;		{
//			    struct bcpm_yy_extra	*ce = yyget_extra(yyscanner);

//			    printf("\033[1;34msemicolon: %s\033[0m\n", yytext);
			    return SEMICOLON;
			}
<INITIAL,BLOCK>,			{ /*printf("\033[1;34mcomma: %s\033[0m\n", yytext);*/ return COMMA; }
<INITIAL,BLOCK>@			{ /*printf("at: %s\n", yytext); return AT;*/ }
<BLOCK>\}		{
			    BEGIN INITIAL;
//			    struct bcpm_yy_extra	*ce = yyget_extra(yyscanner);

//			    printf("\033[1;34mbrace_stop: %s\033[0m\n", yytext);
			    return BRACE_STOP;
			}
    /* '[^']*'			{ /*printf("\033[1;34mstring: %s\033[0m\n", yytext); savestring(yytext, yyscanner); return STRING; } */
    /* \"[^\"]*\"		{ /*printf("\033[1;34mstring: %s\033[0m\n", yytext); savestring(yytext, yyscanner); return STRING; } */
<INITIAL,BLOCK>{string1}		{ savestring(yytext, yyscanner); return STRING; }
<INITIAL,BLOCK>{string2}		{ savestring(yytext, yyscanner); return STRING; }
<INITIAL>\>		{ return CHILD; }
<INITIAL>{blank}+	{
			    struct bcpm_yy_extra	*ce = yyget_extra(yyscanner);

			    if (ce->inside_selector)
				return BLANK;
			}
<BLOCK>{blank}+		{
//			    struct bcpm_yy_extra	*ce = yyget_extra(yyscanner);
			}
<INITIAL>\/\*		{ ((struct bcpm_yy_extra*)yyget_extra(yyscanner))->state = INITIAL; BEGIN COMMENT; }
<BLOCK>\/\*		{ ((struct bcpm_yy_extra*)yyget_extra(yyscanner))->state = BLOCK; BEGIN COMMENT; }
<COMMENT>\*\/		{ BEGIN ((struct bcpm_yy_extra*)yyget_extra(yyscanner))->state; }
<COMMENT>.|\n		{}
<*>.|\n			{ /*printf("INVALID: %s\n", yytext);*/ }
%%


static inline void savestring( char *text, yyscan_t scanner )
{
    struct bcpm_yy_extra	*ce = yyget_extra(scanner);

    ce->string++;
    if (ce->string==32) ce->string = 0;

    strncpy(ce->text[ce->string], text, 511);
    *bcpmget_lval(scanner) = (int)ce->text[ce->string];
}

