%{

#include "snippet.parser.common.h"
#include "y.tab.h"

static inline void bsgp_newstring( const char* str, yyscan_t scanner );

%}

legalchar       [a-z0-9_'`]
blank		[\ \t\n\r\f]
parantes	[\(\[\{\}\]\)]
utf-8-2b        [\300-\337][\200-\277]
utf-8-3b        [\340-\357][\200-\277][\200-\277]
utf-8-4b        [\360-\367][\200-\277][\200-\277][\200-\277]
%option nomain noyywrap reentrant bison-bridge
%%
\<div\>				{ return DIV_START; }
\<\/div\>			{ return DIV_END; }
\<h2\>				{ return HEAD_START; }
\<\/h2\>			{ return HEAD_END; }
\<span\>			{ return SPAN_START; }
\<\/span\>			{ return SPAN_END; }
(\&quot;|\&amp;|\&gt;|\&lt;)+	{ bsgp_newstring(yytext, yyscanner); return OTHER; }
({legalchar}|{utf-8-2b}|{utf-8-3b}|{utf-8-4b})+	{ bsgp_newstring(yytext, yyscanner); return WORD; }
{blank}+			{ (yyget_extra(yyscanner))->space = 1; }
[\.!?:;,]+			{ bsgp_newstring(yytext, yyscanner); return EOS; }
{parantes}			{ bsgp_newstring(yytext, yyscanner); return PARANTES; }
<*>.				{ bsgp_newstring(yytext, yyscanner); return OTHER; }
%%


static inline void bsgp_newstring( const char* str, yyscan_t scanner )
{
    struct bsgp_yy_extra	*he = yyget_extra(scanner);

    he->stringtop++;
    if (he->stringtop>=32) he->stringtop = 0;

    strncpy( he->stringcircle[he->stringtop], str, bsgp_maxNewString );
    he->stringcircle[he->stringtop][bsgp_maxNewString] = '\0';		// I tilfelle str er for lang.

    *bsgpget_lval(scanner) = (int)he->stringcircle[he->stringtop];
}

