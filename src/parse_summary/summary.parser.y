// (C) Copyright Boitho 2005-2006, Magnus Galåen (magnusga@idi.ntnu.no)

/******************************************
changelog:

Ax: 9. feb 2006
Konvertert fra YACC/Bison til Lemon. "summary.y" skal nå være reentrant.

Runarb 13. des 2005
I steden for å bruke globale verider for title, body, metakeyw, metadesc og *current_buffer her jeg 
laget en "struct bufferformat" med disse i. generate_summary oppreter nå en buffers av denne typen 
og sender den med yyparse.

Hva den heter i yyparse defineres med "#define YYPARSE_PARAM buffers"

Dette får å gjøre honteringen av buffere threadsafe. Uten dette vil man ofte få segfeil i buffer_exit når man 
"kjørerfree( b.data );"

*******************************************/

%include
{
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "summary.parser.common.h"
#include "summary.parser.h"

#define INIT	0
#define	TITLE	1
#define	BODY	2

const int	href_attr=1, name_attr=2, content_attr=4;


typedef struct
{
    unsigned char		*data;
    int				pos, maxsize;
    char			overflow;
} buffer;

struct parseExtra {
	buffer 		title;
	buffer 		body;
	buffer	 	metakeyw;
	buffer 		metadesc;
	char		section;				// summaryParse internal variables
	char		*href_ptr, *name_ptr, *content_ptr;	// summaryParse internal variables
	char		newspan, inspan;
	char		newdiv, indiv;
	char		newhead, endhead, inhead;
};

char* translate(char *s);
void print_with_escapes(char *c, buffer *b);
void print_raw(char *c, buffer *b);
void generate_summary( char text[], int text_size, char **output_title, char **output_body, char **output_metakeywords, char **output_metadescription );
} //%include


%extra_argument {struct parseExtra* pE}

%token_type {Token}
%default_type {Token}
%type attr {int}
%type attrlist {int}

%name summaryParse

%parse_failure
{
	#ifndef NOWARNINGS
    		fprintf(stderr, "summary.y: Parse Failure.\n");
	#endif
}

%stack_overflow
{
    fprintf(stderr, "summary.y: Stack Overflow.\n");
}

main ::= doc.
doc ::= .
doc ::= doc block.
block ::= tag.
block ::= text.

tag ::= starttag.
tag ::= endtag.
tag ::= startendtag.

starttag ::= TAG_START ATTR(Name) attrlist(Attrlist) TAG_STOPP.
	    {
		if (pE->section==INIT && !strcasecmp("title", Name.str)) pE->section = TITLE;
		else if ((pE->section==INIT || pE->section==TITLE) && !strcasecmp("body", Name.str))
		    {
			pE->section = BODY;
		    }
		else if (!strcasecmp("meta", Name.str) && ((Attrlist & name_attr)>0) && ((Attrlist & content_attr)>0))
		    {
			if (!strcasecmp("keywords", pE->name_ptr))
			    {
				print_with_escapes( translate(pE->content_ptr), &(pE->metakeyw) );
			    }
			else if (!strcasecmp("description", pE->name_ptr))
			    {
				print_with_escapes( translate(pE->content_ptr) , &(pE->metadesc) );
			    }
		    }
		else if (pE->section==BODY && !strcasecmp("p", Name.str))
		    pE->newdiv = 1;
		else if (pE->section==BODY && !strcasecmp("div", Name.str))
		    pE->newdiv = 1;
		else if (pE->section==BODY && (Name.str[0]=='h' || Name.str[0]=='H') && (Name.str[1]>='0' && Name.str[1]<='6'))
		    pE->newhead = 1;
		else if (pE->section==BODY && !strcasecmp("table", Name.str))
		    pE->newspan = 1;
		else if (pE->section==BODY && !strcasecmp("tr", Name.str))
		    pE->newspan = 1;
		else if (pE->section==BODY && !strcasecmp("td", Name.str))
		    pE->newspan = 1;
		else if (pE->section==BODY && !strcasecmp("br", Name.str))
		    pE->newspan = 1;
		else if (pE->section==BODY && !strcasecmp("ul", Name.str))
		    pE->newspan = 1;
		else if (pE->section==BODY && !strcasecmp("ol", Name.str))
		    pE->newspan = 1;
		else if (pE->section==BODY && !strcasecmp("li", Name.str))
		    pE->newspan = 1;
		// Bør kanskje være med flere slike tagger?
	    }

endtag ::= ENDTAG_START ATTR(Name) ENDTAG_STOPP.
	    {
		if (pE->section==TITLE && !strcasecmp("title", Name.str))
		    {
			pE->section = BODY;
		    }
		else if (pE->section==BODY && (!strcasecmp("p", Name.str) || !strcasecmp("div", Name.str)))
		    {
			pE->newdiv = 1;
//			print_raw( "\n</p>\n", &(pE->rawbody), 0 );
		    }
		else if (pE->section==BODY && (Name.str[0]=='h' || Name.str[0]=='H') && (Name.str[1]>='0' && Name.str[1]<='6'))
		    {
			if (pE->inhead)
			    {
				pE->endhead = 1;
			    }
			else
			    {
				// Empty header.
				pE->newhead = 0;
				pE->inhead = 0;
			    }
//			print_raw( "</h2>\n", &(pE->rawbody), 0 );
		    }
		else if (pE->section==BODY && (
		    !strcasecmp("table", Name.str)
		    || !strcasecmp("tr", Name.str)
		    || !strcasecmp("td", Name.str)
		    || !strcasecmp("br", Name.str)
		    || !strcasecmp("ul", Name.str)
		    || !strcasecmp("ol", Name.str)
		    || !strcasecmp("li", Name.str)
		    ))
		    {
			pE->newspan = 1;
		    }
	    }

startendtag ::= TAG_START ATTR(Name) attrlist(Attrlist) TAG_ENDTAG_STOPP.
	    {
		if (!strcasecmp("meta", Name.str) && ((Attrlist & name_attr)>0) && ((Attrlist & content_attr)>0))
		    {
			if (!strcasecmp("keywords", pE->name_ptr))
			    {
				print_with_escapes( translate(pE->content_ptr), &(pE->metakeyw) );
			    }
			else if (!strcasecmp("description", pE->name_ptr))
			    {
				print_with_escapes( translate(pE->content_ptr), &(pE->metadesc) );
			    }
		    }
	    }

attrlist(Val) ::= .			{ Val = 0; }
attrlist(Val) ::= attrlist(A) attr(B).	{ Val = A | B; }

attr(Val) ::= ATTR(A) EQUALS TEXTFIELD(T).
	    {
		if (!strcasecmp("href", A.str))
		    {
			pE->href_ptr = T.str;
			Val = href_attr;
			pE->href_ptr++;
			pE->href_ptr[strlen(pE->href_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("name", A.str))
		    {
			pE->name_ptr = T.str;
			Val = name_attr;
			pE->name_ptr++;
			pE->name_ptr[strlen(pE->name_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("content", A.str))
		    {
			pE->content_ptr = T.str;
			Val = content_attr;
			pE->content_ptr++;
			pE->content_ptr[strlen(pE->content_ptr)-1] = '\0';
		    }
		else
		    Val = 0;
	    }
attr(Val) ::= ATTR(A) EQUALS ATTR(B).
	    {
		if (!strcasecmp("href", A.str))
		    {
			pE->href_ptr = B.str;
			Val = href_attr;
		    }
		else if (!strcasecmp("name", A.str))
		    {
			pE->name_ptr = B.str;
			Val = name_attr;
		    }
		else if (!strcasecmp("content", A.str))
		    {
			pE->content_ptr = B.str;
			Val = content_attr;
		    }
		else
		    Val = 0;
	    }
attr(Val) ::= ATTR.			{ Val = 0; }
attr(Val) ::= TEXTFIELD.		{ Val = 0; }
text ::= WORD(W).
	    {
		if (pE->section==INIT)
		    {
			pE->section = BODY;
		    }

		if (pE->section==BODY)
		    {
			// old: print_raw
			if ((pE->newspan || pE->newhead || pE->endhead || pE->newdiv) && pE->inspan)
			    { print_raw( "</span>", &(pE->body) ); pE->newspan = 1; }
			if (pE->endhead)
			    { print_raw( "\n</h2>", &(pE->body) ); pE->inhead = 0; }
			if (pE->newdiv && pE->indiv)
			    { print_raw( "\n</div>", &(pE->body) ); }

			if (pE->newdiv)
			    { print_raw( "\n<div>", &(pE->body) ); pE->indiv = 1; }
			if (pE->newhead)
			    { print_raw( "\n<h2>", &(pE->body) ); pE->inhead = 1; }
			if (pE->newspan)
			    { print_raw( "\n<span>", &(pE->body) ); pE->inspan = 1; }

			pE->endhead = 0;
			pE->newdiv = 0;
			pE->newhead = 0;
			pE->newspan = 0;

			if (W.space)
			    print_raw( " ", &(pE->body) );

			char	*s = translate(W.str);
			print_raw( s, &(pE->body) );
			free(s);
		    }
		else
		    {
		    // Translate escapes first, to ensure no illegal escapes,
		    // then retranslate back to escapes:
		    //print_with_escapes( translate(W.str), &(pE->title) );
			if (W.space)
			    print_raw( " ", &(pE->title) );

			char	*s = translate(W.str);
		        print_raw( s, &(pE->title) );
			free(s);
		    }
	    }
text ::= ESC(W).
	    {
		if (pE->section==INIT)
		    {
			pE->section = BODY;
		    }

		if (pE->section==BODY)
		    {
			// old: print_raw
			if ((pE->newspan || pE->newhead || pE->endhead || pE->newdiv) && pE->inspan)
			    { print_raw( "</span>", &(pE->body) ); pE->newspan = 1; }
			if (pE->endhead)
			    { print_raw( "\n</h2>", &(pE->body) ); pE->inhead = 0; }
			if (pE->newdiv && pE->indiv)
			    { print_raw( "\n</div>", &(pE->body) ); }

			if (pE->newdiv)
			    { print_raw( "\n<div>", &(pE->body) ); pE->indiv = 1; }
			if (pE->newhead)
			    { print_raw( "\n<h2>", &(pE->body) ); pE->inhead = 1; }
			if (pE->newspan)
			    { print_raw( "\n<span>", &(pE->body) ); pE->inspan = 1; }

			pE->endhead = 0;
			pE->newdiv = 0;
			pE->newhead = 0;
			pE->newspan = 0;

			if (W.space)
			    print_raw( " ", &(pE->body) );

			print_with_escapes( translate(W.str), &(pE->body) );
		    }
		else
		    {
		    // Translate escapes first, to ensure no illegal escapes,
		    // then retranslate back to escapes:
			if (W.space)
			    print_raw( " ", &(pE->title) );
			
			print_with_escapes( translate(W.str), &(pE->title) );
		    }
	    }
