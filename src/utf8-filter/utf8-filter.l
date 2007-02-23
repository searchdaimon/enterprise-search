%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _ubuf
{
    int		pos;
    char	*buf;
};

#define YY_EXTRA_TYPE	struct _ubuf*

%}

utf-8-1b	[\\!"#$%'()*+,\-\./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~\<\>]
utf-8-2b	[\300-\337][\200-\277]
utf-8-3b	[\340-\357][\200-\277][\200-\277]
utf-8-4b	[\360-\367][\200-\277][\200-\277][\200-\277]
%option nomain noyywrap reentrant
%%
({utf-8-2b}|{utf-8-3b}|{utf-8-4b})+	{
	int	i;
	struct _ubuf	*data = yyget_extra(yyscanner);

	for (i=0; yytext[i]!='\0'; i++)
	    data->buf[data->pos++] = yytext[i];
    }
.|\n					{
	struct _ubuf	*data = yyget_extra(yyscanner);

	if (((unsigned char)yytext[0])<128)
	    data->buf[data->pos++] = yytext[0];
    }
%%


char* utf8_filter( char *str )
{
    struct _ubuf	*data = malloc(sizeof(struct _ubuf));
    yyscan_t		scanner;
    int			len = strlen(str);
    char		*output;

    data->pos = 0;
    data->buf = malloc(len+1);

    yylex_init( &scanner );
    yyset_extra( data, scanner );
    YY_BUFFER_STATE	bs = yy_scan_bytes( str, len, scanner );

    yylex(scanner);

    yy_delete_buffer(bs, scanner);
    yylex_destroy(scanner);

    data->buf[data->pos] = '\0';
    output = strdup(data->buf);

    free(data->buf);
    free(data);

    return output;
}
