%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
%}

utf-8-1b	[\\!"#$%'()*+,\-\./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~\<\>]
utf-8-2b	[\300-\337][\200-\277]
utf-8-3b	[\340-\357][\200-\277][\200-\277]
utf-8-4b	[\360-\367][\200-\277][\200-\277][\200-\277]
%option	noyywrap
%%
({utf-8-2b}|{utf-8-3b}|{utf-8-4b})+	{ printf("%s", yytext); }
.|\n					{ if (((unsigned char)yytext[0])<128) printf("%s", yytext); }
%%


int main( int argc, char *argv[] )
{
    yylex();
}
