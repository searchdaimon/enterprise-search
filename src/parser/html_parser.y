%{
// (C) Copyright Boitho 2004-2007, Magnus Galåen (magnusga@idi.ntnu.no)

/*
Changelog

Januar 2007 (Magnus):
    Kildekoden er oppgradert til reentrant, og spytter i tillegg ut tekst-summary.

Runar 21.des
Håndtering av at page_uri er  NULL en skjelden gang i create_full_link

*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "../common/utf8-strings.h"
#include "../common/search_automaton.h"
#include "html_parser_common.h"
#include "html_parser.h"
#include "html_parser_tags.h"

// --- fra flex:
typedef void* yyscan_t;
typedef struct bhpm_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE bhpm_scan_bytes( const char *bytes, int len, yyscan_t yyscanner );
struct bhpm_yy_extra *bhpmget_extra( yyscan_t yyscanner );
// ---


// Data for bison-parseren:
struct bhpm_intern_data
{
    char	*page_uri, *page_path, *page_rel_path;
    int		page_url_len;

    char	*attr[16], *val[16];
    int		num_attr;
};


char* create_full_link( char *url, int page_url_len, char *page_uri, char *page_path );

void lexwords(unsigned char *s);


char		*meta_attr[] = {"keywords","description","author","redirect"};
enum		{ meta_keywords=0, meta_description, meta_author, meta_redirect };
const int	meta_attr_size = 4;
automaton	*meta_attr_automaton = NULL;


%}

%pure-parser
%parse-param { struct bhpm_intern_data *data }
%parse-param { yyscan_t yyscanner }
%lex-param { yyscan_t yyscanner }
%token WORD ESC TAG_START TAG_STOPP TAG_ENDTAG_STOPP ENDTAG_START ENDTAG_STOPP ATTR EQUALS TEXTFIELD

%%
doc	:
	| doc tag
	;
tag	: starttag
	| endtag
	| startendtag
	;
starttag	: TAG_START ATTR attrlist TAG_STOPP
	    {
//		printf("\nStarttag: %s\n", $2);
		struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
		int			hit = search_automaton(tags_automaton, (char*)$2);

		if ((hit>=0) && (tag_flags[hit] & tagf_space))
		    he->space = 1;
/*
		if (he->wordcount < 25)
		    {
		printf("\033[1;33m<%s", (char*)$2);
		int j;
		for (j=0; j<data->num_attr; j++)
		    {
			printf(" (%s)", data->attr[j]);
			if (data->val[j] != NULL)
			    printf("={%s}", data->val[j]);
		    }
		printf(">\033[0m\n");
		    }
*/
		switch (hit)
		    {
			case tag_a:
			    {
				int	i;

				for (i=0; i<data->num_attr; i++)
				    {
					if (!strcasecmp(data->attr[i], "href") && data->val[i]!=NULL)
					    {
						he->alink = 1;
//						clean(data->val[i]);

						char	*temp_link = create_full_link(data->val[i], data->page_url_len, data->page_uri, data->page_path );
						he->user_fn( temp_link, he->linkcount++, pu_link, puf_none, he->wordlist );	// new link
						free( temp_link );
					    }
				    }
			    }
			    break;
			case tag_base:
			    {
				int	i;

				for (i=0; i<data->num_attr; i++)
				    {
					if (!strcasecmp(data->attr[i], "href") && data->val[i]!=NULL)
					    {
						he->user_fn( data->val[i], 0, pu_baselink, puf_none, he->wordlist );
					    }
				    }
			    }
			    break;
			case tag_h1: case tag_h2: case tag_h3: case tag_h4: case tag_h5: case tag_h6:
			    he->h = hit -tag_h1 +1;
			    break;
			case tag_meta:
			    {
			    int			i;
			    char		*content = NULL;
			    enum parsed_unit	pu = pu_none;

			    for (i=0; i<data->num_attr; i++)
				{
				    if (!strcasecmp(data->attr[i], "content"))
					content = data->val[i];
				    else if (!strcasecmp(data->attr[i], "name") && data->val[i] != NULL)
					switch (search_automaton(meta_attr_automaton, data->val[i]))
					    {
						case meta_keywords:
						    pu = pu_meta_keywords;
						    break;
						case meta_description:
						    pu = pu_meta_description;
						    break;
						case meta_author:
						    pu = pu_meta_author;
						    break;
						case meta_redirect:
						    pu = pu_meta_redirect;
						    break;
					    }
				}

			    if (pu != pu_none && content != NULL)
				{
				    lexwords((unsigned char*)content);
				    he->user_fn( content, 0, pu, puf_none, he->wordlist );
				}
			    }
			    break;
			case tag_title:
//			    printf("\n\033[0;7mtitle\033[0m\n");
			    he->title = 1;
			    break;
		    }

		if (hit!=tag_title)
		    he->title = 0;

		if (tag_flags[hit] & tagf_head)
			he->newhead = 1;
		else if (tag_flags[hit] & tagf_div)
			he->newdiv = 1;
		else if (tag_flags[hit] & tagf_span)
			he->newspan = 1;
	    }
	;
endtag	: ENDTAG_START ATTR ENDTAG_STOPP
//	    { printf("endtag: %s\n", $2); }
	    {
		struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
		int			hit = search_automaton(tags_automaton, (char*)$2);

		if (hit==tag_title)	he->title = 0;
		else if ((hit>=0) && (tag_flags[hit] & tagf_head))	he->h = 0;
		else if (hit==tag_a)	he->alink = 0;


		if (tag_flags[hit] & tagf_head)
			he->newendhead = 1;
		else if (tag_flags[hit] & tagf_div)
			he->newdiv = 1;
		else if (tag_flags[hit] & tagf_span)
			he->newspan = 1;
	    }
	;
startendtag	: TAG_START ATTR attrlist TAG_ENDTAG_STOPP
//	    { printf("start/end-tag: %s\n", $2); }
	    {
//		printf("\nStart/end-tag: %s\n", $2);
		struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
		int			hit = search_automaton(tags_automaton, (char*)$2);

		if ((hit>=0) && (tag_flags[hit] & tagf_space))
		    he->space = 1;

		if (hit==tag_base)
		    {
			int	i;

			for (i=0; i<data->num_attr; i++)
			    {
				if (!strcasecmp(data->attr[i], "href") && data->val[i]!=NULL)
				    {
					he->user_fn( data->val[i], 0, pu_baselink, puf_none, he->wordlist );
				    }
			    }
		    }
		else if (hit==tag_meta)
		    {
			int			i;
			char		*content = NULL;
			enum parsed_unit	pu = pu_none;

			for (i=0; i<data->num_attr; i++)
			    {
				if (!strcasecmp(data->attr[i], "content"))
				    content = data->val[i];
				else if (!strcasecmp(data->attr[i], "name") && data->val[i] != NULL)
				    switch (search_automaton(meta_attr_automaton, data->val[i]))
					{
					    case meta_keywords:
						pu = pu_meta_keywords;
						break;
					    case meta_description:
						pu = pu_meta_description;
						break;
					    case meta_author:
						pu = pu_meta_author;
						break;
					    case meta_redirect:
						pu = pu_meta_redirect;
						break;
					}
			    }

			if (pu != pu_none && content != NULL)
			    {
				lexwords((unsigned char*)content);
				he->user_fn( content, 0, pu, puf_none, he->wordlist );
			    }
		    }

		if (tag_flags[hit] & tagf_head)
			he->newendhead = 1;
		else if (tag_flags[hit] & tagf_div)
			he->newdiv = 1;
		else if (tag_flags[hit] & tagf_span)
			he->newspan = 1;
	    }
	;
attrlist :
	    { data->num_attr = 0; }
	| attrlist attr
	    {}
	;
attr	: ATTR EQUALS TEXTFIELD
	    {
		if (data->num_attr < 16)
		    {
			data->attr[data->num_attr] = (char*)$1;
			data->val[data->num_attr] = (char*)$3;
			data->num_attr++;
		    }
	    }
	| ATTR EQUALS ATTR
	    {
		if (data->num_attr < 16)
		    {
			data->attr[data->num_attr] = (char*)$1;
			data->val[data->num_attr] = (char*)$3;
			data->num_attr++;
		    }
	    }
	| ATTR
	    {
		if (data->num_attr < 16)
		    {
			data->attr[data->num_attr] = (char*)$1;
			data->val[data->num_attr] = NULL;
			data->num_attr++;
		    }
	    }
	| TEXTFIELD
	    {
		if (data->num_attr < 16)
		    {
			data->attr[data->num_attr] = (char*)$1;
			data->val[data->num_attr] = NULL;
			data->num_attr++;
		    }
	    }
	;
%%


void lexwords(unsigned char *s)	// Lex for words in textfield.
{
    int		i, j;
    char	word=0;
    char	esc=0;

    //    word		[0-9a-z'ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþ]
    for (i=0,j=0; s[i]!='\0'; i++)
	if (!esc && ((s[i]>='0' && s[i]<='9')
	    || (s[i]>='A' && s[i]<='Z')
	    || (s[i]>='a' && s[i]<='z')
	    || (s[i]==0xc3 && s[i+1]>=0x80 && s[i+1]<=0x9e)
	    || (s[i]==0xc3 && s[i+1]>=0xa0 && s[i+1]<=0xbe)
	    || s[i]==0x27))
	    {
		s[j++] = s[i];
		word = 1;
	    }
	else if (s[i]=='&')
	    esc=1;
	else if (s[i]==';')
	    esc=0;
	else
	    {
		if (word) s[j++] = ' ';
		word = 0;
	    }

    s[j] = '\0';
}


// in: url
// out: uri, path, rel_path	NB: minne må frigjøres!!
void url_split( char *url, char **uri, char **path, char **rel_path )
{
    int		i=0;
    int		pos_uri_end=0, pos_path_end=0;
    int		len = strlen(url);

    if (len>=8 && !strncmp( url, "http://", 7))
	{
	    for (i=8; i<len && url[i]!='/'; i++);
	    pos_uri_end = i;
	    (*uri) = (char*)strndup(url, i);
	    convert_to_lowercase( (unsigned char*)(*uri) );
	}
    else
	{
	    (*uri) = NULL;
	}

    if (len>i && url[i]=='/')
	{
	    for (; i<len && url[i]!='?'; i++)
		if (url[i]=='/') pos_path_end = i;
	    pos_path_end++;

	    (*path) = (char*)strndup(&(url[pos_uri_end]), pos_path_end - pos_uri_end);

	    if (pos_path_end<len)
	        (*rel_path) = (char*)strndup(&(url[pos_path_end]), len - pos_path_end);
	    else
		(*rel_path) = NULL;
	}
    else
	{
	    (*path) = NULL;
	    if (len>i)
		(*rel_path) = (char*)strndup(&(url[i]), len-i);
	    else
		(*rel_path) = NULL;
	}
}

// NB: minne må frigjøres!
char* create_full_link( char *url, int page_url_len, char *page_uri, char *page_path )
{
    char	*new_url = (char*)malloc( page_url_len + strlen(url) +1 ); // Largest possible output.
    char	*uri, *path, *rel_path;

    url_split( url, &uri, &path, &rel_path );

    // Runar: honterer feil der page_uri er NULL (komer dette av at urlen ikke 
    // begynner på http:// ?. url_split splitt ser vel etter http:// og setter den til NULL
    // hvis det mangler)
    if (page_uri == NULL) {
	#ifndef NOWARNINGS
	printf("page_uri is NULL!\n");
	#endif
	new_url[0] = '\0';
	return new_url;
    }

//    printf("\nlink: %s %s %s\n", uri, path, rel_path);

    if (uri==NULL)
	{
	    strcpy(new_url, page_uri);

	    if (path==NULL && page_path!=NULL && rel_path!=NULL)
		{
		    int		i = 0, j;
		    int		backsteps=0;

		    while (1)
			{
			    if (!strncmp(&(rel_path[i]), "../", 3))
				{
				    i+= 3;
				    backsteps++;
				}
			    else if (!strncmp(&(rel_path[i]), "./", 2))
				{
				    i+= 2;
				}
			    else break;
			}

//		    printf("Backsteps: %i\n", backsteps);

		    backsteps++;
		    for (j=strlen(page_path)-1; backsteps>0 && j>0; j--)
			{
//			    printf("%c", page_path[j]);
			    if (page_path[j]=='/')
				{
				    backsteps--;
				    if (backsteps==0) break;
				}
			}
//		    printf("\n");

		    strncat( new_url, page_path, j+1 );
		    strcat( new_url, &(rel_path[i]) );
		}
	    else
		{
		    if (path!=NULL)
			strcat( new_url, path );
		    else if (page_path!=NULL)
			strcat( new_url, page_path );

		    if (rel_path!=NULL)
			strcat( new_url, rel_path );
		}
	}
    else
	{
	    strcpy(new_url, uri);

	    if (path!=NULL)
		strcat( new_url, path );

	    if (rel_path!=NULL)
		strcat( new_url, rel_path );
	}

    free(uri);
    free(path);
    free(rel_path);
    return new_url;
}

void html_parser_init()
{
    tags_automaton = build_automaton( tags_size, (unsigned char**)tags );
    meta_attr_automaton = build_automaton( meta_attr_size, (unsigned char**)meta_attr );
}

void html_parser_exit()
{
    free_automaton(meta_attr_automaton);
    free_automaton(tags_automaton);
}


//void run_html_parser( char *url, char text[], int textsize, void (*fn)(char*,int,enum parsed_unit,enum parsed_unit_flag) )
void html_parser_run( char *url, char text[], int textsize, char **output_title, char **output_body,
    void (*fn)(char*,int,enum parsed_unit,enum parsed_unit_flag,void* wordlist), void* wordlist )
{
    struct bhpm_yy_extra	*he = malloc(sizeof(struct bhpm_yy_extra));
    struct bhpm_intern_data	*data = malloc(sizeof(struct bhpm_intern_data));

    he->slen = 0;
    he->tt = -1;
    he->stringtop = -1;
    he->flush = 0;

    he->wordlist = wordlist;

    // Save URL of current document:
    url_split( url, &data->page_uri, &data->page_path, &data->page_rel_path );
    data->page_url_len = strlen(url);

    // Set variables for yacc-er:
    he->user_fn = fn;

    he->title = 0;
    he->alink = 0;
    he->wordcount = 0;
    he->linkcount = 0;
    he->h = 0;

    he->newhead = 0;
    he->newdiv = 1;
    he->newspan = 0;
    he->inhead = 0;
    he->indiv = 0;
    he->inspan = 0;
    he->newendhead = 0;

    he->Btitle = buffer_init( 10240 );
    he->Bbody = buffer_init( textsize*2 );

    // Run parser:
    yyscan_t	scanner;
    int		yv;

    bhpmlex_init( &scanner );
    bhpmset_extra( he, scanner );

    YY_BUFFER_STATE	bs = bhpm_scan_bytes( text, textsize, scanner );

    he->space = 0;
    while ((yv = bhpmparse(data, scanner)) != 0)
	{
//	    printf("."); fflush(stdout);
//	    if (yv==WORD || yv==ESC)
//		he->space = 0;
	}

    if (he->inspan) bprintf(he->Bbody, "</span>\n");
    if (he->inhead) bprintf(he->Bbody, "  </h2>\n");
    if (he->indiv) bprintf(he->Bbody, "</div>\n");

    bhpm_delete_buffer( bs, scanner );
    bhpmlex_destroy( scanner );


    *output_title = buffer_exit( he->Btitle );
    *output_body = buffer_exit( he->Bbody );

    free(data->page_uri);
    free(data->page_path);
    free(data->page_rel_path);
    free(data);
    free(he);
}


yyerror( struct bhpm_intern_data *data, void *yyscan_t, char *s )
{
	#ifndef NOWARNINGS
    		printf("parse_error: %s\n", s);
	#endif
}
