%{
#include <stdio.h>
#include "y.tab.h"

extern yylval;

int lineno=1;
%}

character	[_0-9A-Za-z]
%option	nomain noyywrap
%x	COMMENT SINGLEQUOTE DOUBLEQUOTE
%%
\#			{ BEGIN COMMENT; } // Ignore comments.
[ \t\r]*		{} // Ignore blanks.
{character}+		{ yylval = (int)strdup(yytext);	return TEXT; }
\=			{				return EQUALS; }
\'			{ BEGIN SINGLEQUOTE; }
\"			{ BEGIN DOUBLEQUOTE; }
<SINGLEQUOTE>[^\']*	{ yylval = (int)strdup(yytext);	return TEXT; }
<DOUBLEQUOTE>[^\"]*	{ yylval = (int)strdup(yytext);	return TEXT; }
<SINGLEQUOTE>\'		{ BEGIN INITIAL; }
<DOUBLEQUOTE>\"		{ BEGIN INITIAL; }
\n			{ lineno++;			return EOLN; }
<COMMENT>\n		{ lineno++;	BEGIN INITIAL;	return EOLN; }
<COMMENT>.		{} // Ignore comments.
.			{ printf("Error reading config: parse error at line %i.\n", lineno); exit(-1); }
%%
