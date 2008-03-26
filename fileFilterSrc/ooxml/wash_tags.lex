%{
%}
letter		[a-z_]
space		[\ \t\r\n]
%option noyywrap
%x TAG DCMNT SCMNT
%%
\<{space}*{letter}+		{ BEGIN TAG; printf("%s", yytext); }
\<{space}*\/{space}*{letter}+	{ BEGIN TAG; printf("%s", yytext); }
<TAG>\"				{ BEGIN DCMNT; }
<TAG>\'				{ BEGIN SCMNT; }
<TAG>\\\>			{}
<DCMNT>\\\"			{}
<SCMNT>\\\'			{}
<DCMNT>\"			{ BEGIN TAG; }
<SCMNT>\'			{ BEGIN TAG; }
<TAG>\/{space}*\>		{ BEGIN INITIAL; printf("%s", yytext); }
<TAG>\>				{ BEGIN INITIAL; printf("%s", yytext); }
.|\n				{ printf("%s", yytext); }
<*>.|\n				{}
%%


int main()
{
    yylex();

    return 0;
}
