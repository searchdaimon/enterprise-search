%{
#include <stdio.h>

char		space=0;
%}
letter		[a-z_]
space		[\ \t\r\n]
%option noyywrap
%x TAG DCMNT SCMNT
%%
\<				{ space=1; BEGIN TAG; }
<TAG>\"				{ BEGIN DCMNT; }
<TAG>\'				{ BEGIN SCMNT; }
<TAG>\\\>			{}
<DCMNT>\\\"			{}
<SCMNT>\\\'			{}
<DCMNT>\"			{ BEGIN TAG; }
<SCMNT>\'			{ BEGIN TAG; }
<TAG>\>				{ BEGIN INITIAL; }
.|\n				{ if (space) { printf(" "); space=0; } printf("%s", yytext); }
<*>.|\n				{}
%%


int main()
{
    yylex();

    return 0;
}
