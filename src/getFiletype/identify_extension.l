%{

#include <stdio.h>
#include <string.h>

#include "fte.tab.h"

/*
#include <stdio.h>

#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dvector.h"

#include "../common/search_automaton.h"

#include "getfiletype.h"



struct readconf_data
{
    int		lineno, lang_size, group_size, ext_size;
    container	*lang;
    container	*group[], *descr[];
    container	*ext, *ext2group;
};

char	*lang[lang_size] = {"", "no_NB"};
char	*group[lang_size][group_size];
char	*ext[ext_size];
int	ext2group[ext_size];
char	*descr[lang_size][ext_size];
*/
//#define YY_EXTRA_TYPE	struct readconf_data*

%}

blank		[\ \t\r\f\n]
id		[a-z_]
%option nomain noyywrap reentrant bison-bridge
%%
\#[^\n]*\n		{ /* Ignore comments. */ }
lang			{ return LANG_ID; }
default			{ return DEFAULT_ID; }
group			{ return GROUP_ID; }
postfix			{ return POSTFIX_ID; }
file			{ return FILE_ID; }
name			{ return NAME_ID; }
ext			{ return EXT_ID; }
version			{ return VERSION_ID; }
\=			{ return EQUALS_ID; }
\;			{ return SEMICOLON_ID; }
\,			{ return COMMA_ID; }
\{			{ return BRACKET_BEGIN; }
\}			{ return BRACKET_CLOSE; }
\"[^\"]*\"		{
			    yytext[strlen(yytext)-1] = '\0';
			    *fteget_lval(yyscanner) = (int)&(yytext[1]);
			    return STRING_ID;
			}
.|\n			{}
%%
