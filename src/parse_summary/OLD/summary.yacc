%{
// (C) Copyright Boitho 2005, Magnus Galåen (magnusga@idi.ntnu.no)

/******************************************
changelog:
Runarb 13. des 2005
I steden for å bruke globale verider for title, body, metakeyw, metadesc og *current_buffer her jeg 
laget en "struct bufferformat" med disse i. generate_summary oppreter nå en buffers av denne typen 
og sender den med yyparse.

Hva den heter i yyparse defineres med "#define YYPARSE_PARAM buffers"

Dette får å gjøre honteringen av buffere threadsafe. Uten dette vil man ofte få segfeil i buffer_exit når man 
"kjørerfree( b.data );"

*******************************************/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//navn på en parameter  yyparse skal ta inn http://dinosaur.compilertools.net/bison/bison_7.html#SEC65
#define YYPARSE_PARAM buffers

#include "summary_common.h"
#include "summary.h"

#define INIT	0
#define	TITLE	1
#define	BODY	2


char	section;
char	*href_ptr=NULL, *name_ptr=NULL, *content_ptr=NULL;
int	href_attr=1, name_attr=2, content_attr=4;

typedef struct
{
    char	*data;
    int		pos, maxsize;
    char	overflow;
} buffer;

// pruker ikke globala variabler foo dette. Sender i steden med en sturft av typen "struct bufferformat"
//buffer		title, body, metakeyw, metadesc;
//buffer		*current_buffer;

struct bufferformat {
	buffer 		title;
	buffer 		body;
	buffer	 	metakeyw;
	buffer 		metadesc;
	buffer          *current_buffer;	
};

char* translate(char *s);
void print_with_escapes(char *c,struct bufferformat *buffers);


%}

%token WORD TAG_START TAG_STOPP TAG_ENDTAG_STOPP ENDTAG_START ENDTAG_STOPP ATTR EQUALS TEXTFIELD

%%
doc	:
	| doc block
	;
block	: tag
	| text
	;
tag	: starttag
	| endtag
	| startendtag
	;
starttag	: TAG_START ATTR attrlist TAG_STOPP
	    {
		if (section==INIT && !strcasecmp("title",(char*)$2)) section = TITLE;
		else if ((section==INIT||section==TITLE) && !strcasecmp("body",(char*)$2))
		    {
			section = BODY;
			//current_buffer = &buffers.body;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }
		else if (!strcasecmp("meta",(char*)$2) && (($3 & name_attr)>0) && (($3 & content_attr)>0))
		    {
			if (!strcasecmp("keywords",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metakeyw;
				print_with_escapes( translate(content_ptr),(struct bufferformat *) buffers );
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
			else if (!strcasecmp("description",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metadesc;
				print_with_escapes( translate(content_ptr) , (struct bufferformat *) buffers);
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
		    }
	    }
	;
endtag	: ENDTAG_START ATTR ENDTAG_STOPP
	    {
		if (section==TITLE && !strcasecmp("title",(char*)$2))
		    {
			section = BODY;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }
	    }
	;
startendtag	: TAG_START ATTR attrlist TAG_ENDTAG_STOPP
	    {
		if (!strcasecmp("meta",(char*)$2) && (($3 & name_attr)>0) && (($3 & content_attr)>0))
		    {
			if (!strcasecmp("keywords",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metakeyw;
				print_with_escapes( translate(content_ptr), (struct bufferformat *) buffers );
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
			else if (!strcasecmp("description",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metadesc;
				print_with_escapes( translate(content_ptr),(struct bufferformat *) buffers);
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
		    }
	    }
	;
attrlist :
	    { $$ = 0; }
	| attrlist attr
	    { $$ = $1 | $2; }
	;
attr	: ATTR EQUALS TEXTFIELD
	    {
		if (!strcasecmp("href",(char*)$1))
		    {
			href_ptr = (char*)$3;
			$$ = href_attr;
			href_ptr++;
			href_ptr[strlen(href_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("name",(char*)$1))
		    {
			name_ptr = (char*)$3;
			$$ = name_attr;
			name_ptr++;
			name_ptr[strlen(name_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("content",(char*)$1))
		    {
			content_ptr = (char*)$3;
			$$ = content_attr;
			content_ptr++;
			content_ptr[strlen(content_ptr)-1] = '\0';
		    }
		else
		    $$ = 0;
	    }
	| ATTR EQUALS ATTR
	    {
		if (!strcasecmp("href",(char*)$1))
		    {
			href_ptr = (char*)$3;
			$$ = href_attr;
		    }
		else if (!strcasecmp("name",(char*)$1))
		    {
			name_ptr = (char*)$3;
			$$ = name_attr;
		    }
		else if (!strcasecmp("content",(char*)$1))
		    {
			content_ptr = (char*)$3;
			$$ = content_attr;
		    }
		else
		    $$ = 0;
	    }
	| ATTR
	    { $$ = 0; }
	| TEXTFIELD
	    { $$ = 0; }
	;
text	: WORD
	    {
		if (section==INIT)
		    {
			section = BODY;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }

		// Translate escapes first, to ensure no illegal escapes,
		// then retranslate back to escapes:
		print_with_escapes( translate((char*)$1),(struct bufferformat *) buffers );
	    }
	;
%%

extern FILE *yyin;
extern int linenr;



struct trans_tab
{
    char	*escape, translation;
};

struct trans_tab tt[130] = {
    {"#178",'²'},{"#179",'³'},{"#185",'¹'},{"#192",'À'},{"#193",'Á'},{"#194",'Â'},{"#195",'Ã'},{"#196",'Ä'},
    {"#197",'Å'},{"#198",'Æ'},{"#199",'Ç'},{"#200",'È'},{"#201",'É'},{"#202",'Ê'},{"#203",'Ë'},{"#204",'Ì'},
    {"#205",'Í'},{"#206",'Î'},{"#207",'Ï'},{"#208",'Ð'},{"#209",'Ñ'},{"#210",'Ò'},{"#211",'Ó'},{"#212",'Ô'},
    {"#213",'Õ'},{"#214",'Ö'},{"#216",'Ø'},{"#217",'Ù'},{"#218",'Ú'},{"#219",'Û'},{"#220",'Ü'},{"#221",'Ý'},
    {"#222",'Þ'},{"#223",'ß'},{"#224",'à'},{"#225",'á'},{"#226",'â'},{"#227",'ã'},{"#228",'ä'},{"#229",'å'},
    {"#230",'æ'},{"#231",'ç'},{"#232",'è'},{"#233",'é'},{"#234",'ê'},{"#235",'ë'},{"#236",'ì'},{"#237",'í'},
    {"#238",'î'},{"#239",'ï'},{"#240",'ð'},{"#241",'ñ'},{"#242",'ò'},{"#243",'ó'},{"#244",'ô'},{"#245",'õ'},
    {"#246",'ö'},{"#248",'ø'},{"#249",'ù'},{"#250",'ú'},{"#251",'û'},{"#252",'ü'},{"#253",'ý'},{"#254",'þ'},
    {"#255",'ÿ'},{"AElig",'Æ'},{"Aacute",'Á'},{"Acirc",'Â'},{"Agrave",'À'},{"Aring",'Å'},{"Atilde",'Ã'},{"Auml",'Ä'},
    {"Ccedil",'Ç'},{"ETH",'Ð'},{"Eacute",'É'},{"Ecirc",'Ê'},{"Egrave",'È'},{"Euml",'Ë'},{"Iacute",'Í'},{"Icirc",'Î'},
    {"Igrave",'Ì'},{"Iuml",'Ï'},{"Ntilde",'Ñ'},{"Oacute",'Ó'},{"Ocirc",'Ô'},{"Ograve",'Ò'},{"Oslash",'Ø'},{"Otilde",'Õ'},
    {"Ouml",'Ö'},{"THORN",'Þ'},{"Uacute",'Ú'},{"Ucirc",'Û'},{"Ugrave",'Ù'},{"Uuml",'Ü'},{"Yacute",'Ý'},{"aacute",'á'},
    {"acirc",'â'},{"aelig",'æ'},{"agrave",'à'},{"aring",'å'},{"atilde",'ã'},{"auml",'ä'},{"ccedil",'ç'},{"eacute",'é'},
    {"ecirc",'ê'},{"egrave",'è'},{"eth",'ð'},{"euml",'ë'},{"iacute",'í'},{"icirc",'î'},{"igrave",'ì'},{"iuml",'ï'},
    {"ntilde",'ñ'},{"oacute",'ó'},{"ocirc",'ô'},{"ograve",'ò'},{"oslash",'ø'},{"otilde",'õ'},{"ouml",'ö'},{"sup1",'¹'},
    {"sup2",'²'},{"sup3",'³'},{"szlig",'ß'},{"thorn",'þ'},{"uacute",'ú'},{"ucirc",'û'},{"ugrave",'ù'},{"uuml",'ü'},
    {"yacute",'ý'},{"yuml",'ÿ'}};


int compare(const void *a, const void *b)
{
    return strncmp( (char*)a, ((struct trans_tab*)b)->escape, strlen(((struct trans_tab*)b)->escape) );
}

// Translate escapes in string:
char* translate(char *s)
{
    char	*d = (char*)malloc(strlen(s)+1);
    int		i, j, k;
    char	replace;

    for (i=0, j=0; s[j]!='\0';)
	switch (s[j])
	    {
		case '&':
		    replace = 0;

		    if (s[j+1]!='\0')
			{
			    struct trans_tab	*code = (struct trans_tab*)bsearch(&(s[j+1]),tt,130,sizeof(struct trans_tab),compare);

			    if (code!=NULL)
				{
				    replace = 1;
				    d[i++] = code->translation;
				    j+= strlen(code->escape)+1;
				    if (s[j]==';') j++;
				}
			}

		    if (!replace)
			{
			    d[i++] = '&';
			    j++;
			}

		    break;
		default:
		    d[i++] = s[j++];
	    }

end:
    d[i] = '\0';
    return d;
}


struct html_esc
{
    char	c, *esc;
};

struct html_esc he[65] = {
    {'²',"sup2"},{'³',"sup3"},{'¹',"sup1"},{'À',"Agrave"},{'Á',"Aacute"},{'Â',"Acirc"},{'Ã',"Atilde"},{'Ä',"Auml"},
    {'Å',"Aring"},{'Æ',"AElig"},{'Ç',"Ccedil"},{'È',"Egrave"},{'É',"Eacute"},{'Ê',"Ecirc"},{'Ë',"Euml"},{'Ì',"Igrave"},
    {'Í',"Iacute"},{'Î',"Icirc"},{'Ï',"Iuml"},{'Ð',"ETH"},{'Ñ',"Ntilde"},{'Ò',"Ograve"},{'Ó',"Oacute"},{'Ô',"Ocirc"},
    {'Õ',"Otilde"},{'Ö',"Ouml"},{'Ø',"Oslash"},{'Ù',"Ugrave"},{'Ú',"Uacute"},{'Û',"Ucirc"},{'Ü',"Uuml"},{'Ý',"Yacute"},
    {'Þ',"THORN"},{'ß',"szlig"},{'à',"agrave"},{'á',"aacute"},{'â',"acirc"},{'ã',"atilde"},{'ä',"auml"},{'å',"aring"},
    {'æ',"aelig"},{'ç',"ccedil"},{'è',"egrave"},{'é',"eacute"},{'ê',"ecirc"},{'ë',"euml"},{'ì',"igrave"},{'í',"iacute"},
    {'î',"icirc"},{'ï',"iuml"},{'ð',"eth"},{'ñ',"ntilde"},{'ò',"ograve"},{'ó',"oacute"},{'ô',"ocirc"},{'õ',"otilde"},
    {'ö',"ouml"},{'ø',"oslash"},{'ù',"ugrave"},{'ú',"uacute"},{'û',"ucirc"},{'ü',"uuml"},{'ý',"yacute"},{'þ',"thorn"},
    {'ÿ',"yuml"}};


int esc_compare(const void *a, const void *b)
{
    if (*((char*)a) < ((struct html_esc*)b)->c) return -1;
    if (*((char*)a) > ((struct html_esc*)b)->c) return +1;
    return 0;
}

void print2buffer(struct bufferformat *buffers,const char *fmt, ...)
{
    if (((struct bufferformat *) buffers)->current_buffer->overflow) return;

    va_list	ap;

    va_start(ap, fmt);
    int	len_printed = vsnprintf(&(((struct bufferformat *) buffers)->current_buffer->data[((struct bufferformat *) buffers)->current_buffer->pos]), ((struct bufferformat *) buffers)->current_buffer->maxsize - ((struct bufferformat *) buffers)->current_buffer->pos - 1, fmt, ap);

    ((struct bufferformat *) buffers)->current_buffer->pos+= len_printed;

//    if (section==TITLE) title_size+= len_printed;
//    else body_size+= len_printed;

    if (((struct bufferformat *) buffers)->current_buffer->pos >= ((struct bufferformat *) buffers)->current_buffer->maxsize - 1) ((struct bufferformat *) buffers)->current_buffer->overflow = 1;
}


void print_with_escapes(char *c,struct bufferformat *buffers)
{
    int		i;

    if (((struct bufferformat *) buffers)->current_buffer->pos > 0) print2buffer((struct bufferformat *) buffers," ");

    for (i=0; c[i]!='\0'; i++)
	if ((unsigned char)c[i]<128)
	    print2buffer((struct bufferformat *) buffers,"%c", c[i]);
	else
	    {
		struct html_esc	*p = (struct html_esc*)
		bsearch( (const void*)(((char*)&(c[i]))), he, 65, sizeof(struct html_esc), esc_compare);

		if (p==NULL)
		    print2buffer((struct bufferformat *) buffers,"%c", c[i]);
		else
		    print2buffer((struct bufferformat *) buffers,"&%s;", p->esc);
	    }

    free( c );
}

buffer buffer_init( int _maxsize )
{
    buffer	b;

    b.overflow = 0;
    b.pos = 0;
    b.maxsize = _maxsize;
    b.data = (char*)malloc(b.maxsize);

    return b;
}

char* buffer_exit( buffer b )
{
    char	*output;

    output = (char*)malloc(b.pos+1);
    memcpy( output, &(b.data[0]), b.pos );
    output[b.pos] = '\0';
    free( b.data );

    return output;
}

void generate_summary( char text[], int text_size, char **output_title, char **output_body, char **output_metakeywords, char **output_metadescription )
{
    // Set global variables for lexer:
    custom_input = text;
    custom_pos = 0;
    custom_size = text_size;

    // Set variables for yacc-er:
    section = INIT;

    // We accept output summaries of a size up to double the original textsize.
    // (Although on almost all occations the size will shrink).
//    maxsize = text_size*2;

    // Allocate output_buffer:
//    output_buffer = (char*)malloc(maxsize);
//    title_size = body_size = pos = 0;
//    overflow = 0;	// To prevent corrupt inputfiles to overflow output_buffer.

    // Fields 'title', 'meta keywords' and 'meta description', will only keep first 10240 bytes,
    // field body will only keep up to double original textsize (should be enough for all ordinary documents).

    struct bufferformat buffers;

    buffers.title = buffer_init( 10240 );
    buffers.body = buffer_init( text_size*2 );
    buffers.metakeyw = buffer_init( 10240 );
    buffers.metadesc = buffer_init( 10240 );

    buffers.current_buffer = &buffers.title;

    // Run parser:
    do
	{
	    yyparse((void*)&buffers);
	}
    while (custom_pos<custom_size);

    (*output_title) = buffer_exit( buffers.title );
    (*output_body) = buffer_exit( buffers.body );
    (*output_metakeywords) = buffer_exit( buffers.metakeyw );
    (*output_metadescription) = buffer_exit( buffers.metadesc );
}


yyerror( char *s )
{
    //fprintf( stderr, "parse_error %s on line %i\n", s, linenr );
}
