%{
#include <stdio.h>

char	worksheets=0, slides=0;
%}
letter		[a-z_]
space		[\ \t\r\n]
%option noyywrap
%x TAG STAG TARGET
%%
\<Relationship			{ BEGIN TAG; }
<TAG,WTAG>\/\>			{ BEGIN INITIAL; }
<TAG>Type=\"[^\"]*worksheet\"	{ if (worksheets) BEGIN STAG; }
<TAG>Type=\"[^\"]*slide\"	{ if (slides) BEGIN STAG; }
<STAG>Target=\"			{ BEGIN TARGET; }
<TARGET>[^\"]*			{ printf("%s\n", yytext); }
<TARGET>\"			{ BEGIN TAG; }
<*>.|\n				{}
%%


int main(int argc, char *argv[])
{
    int		i;

    for (i=1; i<argc; i++)
	{
	    if (!strcmp(argv[i], "--slides"))
		slides=1;
	    if (!strcmp(argv[i], "--worksheets"))
		worksheets=1;
	}

    yylex();

    return 0;
}
