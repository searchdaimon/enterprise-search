%{

#include <stdio.h>
#include <string.h>

#include "adf.tab.h"

%}

blank		[\ \t\r\f\n]
id		[a-z_]
%option nomain noyywrap reentrant bison-bridge
%%
\#[^\n]*\n		{ /* Ignore comments. */ }
lang			{ return LANG_ID; }
key			{ return KEY_ID; }
value			{ return VALUE_ID; }
name			{ return NAME_ID; }
icon			{ return ICON_ID; }
\=			{ return EQUALS_ID; }
\;			{ return SEMICOLON_ID; }
\,			{ return COMMA_ID; }
\{			{ return BRACKET_BEGIN; }
\}			{ return BRACKET_CLOSE; }
\"[^\"]*\"		{
			    yytext[strlen(yytext)-1] = '\0';
			    *adfget_lval(yyscanner) = (int)&(yytext[1]);
			    return STRING_ID;
			}
.|\n			{}
%%
