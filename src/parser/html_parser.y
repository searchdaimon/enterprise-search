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

#include "../common/bstr.h"
#include "html_parser_common.h"
#include "html_parser.h"
#include "search_automaton.h"

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

char* bhpm_translate(char *s);
void clean(char *s);
void lexwords(char *s);

char		*ca_taglist[] = {"a","base","h1","h2","h3","h4","h5","h6","meta","title"};
enum		{ tag_a=0, tag_base, tag_h1, tag_h2, tag_h3, tag_h4, tag_h5, tag_h6, tag_meta, tag_title };
const int	ca_taglist_size = 10;
automaton	*sa_taglist = NULL;

char		*meta_taglist[] = {"keywords","description","author","redirect"};
enum		{ meta_keywords=0, meta_description, meta_author, meta_redirect };
const int	meta_taglist_size = 4;
automaton	*sa_meta_taglist = NULL;

char		*ca_spacetags[] = {"br","button","center","div","h1","h2","h3","h4","h5","h6","hr","img","label","map","p","table"};
const int	ca_spacetags_size = 16;
automaton	*sa_spacetags = NULL;


char		*nh_tags[] = {"h1","h2","h3","h4","h5","h6"};
const int	nh_tags_size = 6;
automaton	*nha_tags = NULL;

char		*nd_tags[] = {"div","ol","p","table","ul"};
const int	nd_tags_size = 5;
automaton	*nda_tags = NULL;

char		*ns_tags[] = {"br","li","td","th","tr"};
const int	ns_tags_size = 5;
automaton	*nsa_tags = NULL;

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
		int			hit;
		struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);

		if (search_automaton(sa_spacetags, (char*)$2) != -1)
		    he->space = 1;

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

		switch (hit=search_automaton(sa_taglist, (char*)$2))
		    {
			case tag_a:
			    {
				int	i;

				for (i=0; i<data->num_attr; i++)
				    {
					if (!strcasecmp(data->attr[i], "href") && data->val[i]!=NULL)
					    {
						he->alink = 1;
						clean(data->val[i]);

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
						clean(data->val[i]);
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

			    printf("meta:");

			    for (i=0; i<data->num_attr; i++)
				{
				    if (!strcasecmp(data->attr[i], "content"))
					content = data->val[i];

				    if (!strcasecmp(data->attr[i], "name") && data->val[i] != NULL)
					switch (search_automaton(sa_meta_taglist, data->val[i]))
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
				    lexwords(content);
				    he->user_fn( bhpm_translate(content), 0, pu, puf_none, he->wordlist );
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

		if (search_automaton(nha_tags, (char*)$2) != -1)
			he->newhead = 1;
		if (search_automaton(nda_tags, (char*)$2) != -1)
			he->newdiv = 1;
		if (search_automaton(nsa_tags, (char*)$2) != -1)
			he->newspan = 1;
	    }
	;
endtag	: ENDTAG_START ATTR ENDTAG_STOPP
//	    { printf("endtag: %s\n", $2); }
	    {
		int	hit;
		struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);

		hit = search_automaton(sa_taglist, (char*)$2);

		if (hit==tag_title)	he->title = 0;
		else if (hit>=tag_h1 && hit<=tag_h6)	he->h = 0;
		else if (hit==tag_a)	he->alink = 0;

		//!!!
		if (search_automaton(nha_tags, (char*)$2) != -1)
			he->newendhead = 1;
		if (search_automaton(nda_tags, (char*)$2) != -1)
			he->newdiv = 1;
		if (search_automaton(nsa_tags, (char*)$2) != -1)
			he->newspan = 1;
	    }
	;
startendtag	: TAG_START ATTR attrlist TAG_ENDTAG_STOPP
//	    { printf("start/end-tag: %s\n", $2); }
	    {
//		printf("\nStart/end-tag: %s\n", $2);
		int	hit;
		struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);

		if (search_automaton(sa_spacetags, (char*)$2) != -1)
		    he->space = 1;

		hit = search_automaton(sa_taglist, (char*)$2);

		if (hit==tag_base)
		    {
			int	i;

			for (i=0; i<data->num_attr; i++)
			    {
				if (!strcasecmp(data->attr[i], "href") && data->val[i]!=NULL)
				    {
					clean(data->val[i]);
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

				if (!strcasecmp(data->attr[i], "name") && data->val[i] != NULL)
				    switch (search_automaton(sa_meta_taglist, data->val[i]))
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
				lexwords(content);
				he->user_fn( bhpm_translate(content), 0, pu, puf_none, he->wordlist );
			    }
		    }

		if (search_automaton(nha_tags, (char*)$2) != -1)
			he->newhead = 1;
		if (search_automaton(nda_tags, (char*)$2) != -1)
			he->newdiv = 1;
		if (search_automaton(nsa_tags, (char*)$2) != -1)
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


void clean(char *s)	// Clean textfield for backslashes.
{
    int		i, j;

    for (i=0,j=0; s[i]!='\0'; i++)
        if (s[i]!=0x5c)	// Backslash.
	    s[j++] = s[i];

    s[j] = '\0';
}

// Denne bør oppdateres:
void lexwords(char *s)	// Lex for words in textfield.
{
    int		i, j;
    char	word=0;

    //    word		[0-9a-z'ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþ]
    for (i=0,j=0; s[i]!='\0'; i++)
	if ((s[i]>='0') && (s[i]<='9')
	    || (s[i]>='A') && (s[i]<='Z')
	    || (s[i]>='a') && (s[i]<='z')
	    || (s[i]>='À') && (s[i]<='ß')
	    || (s[i]>='à') && (s[i]<='þ')
	    || s[i]==0x27)
	    {
		s[j++] = s[i];
		word = 1;
	    }
	else
	    {
		if (word) s[j++] = ' ';
		word = 0;
	    }

    s[j] = '\0';
}



struct trans_tab
{
    char	*escape, translation;
};

struct trans_tab bhpm_tt[130] = {
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


int bhpm_compare(const void *a, const void *b)
{
    return strncmp( (char*)a, ((struct trans_tab*)b)->escape, strlen(((struct trans_tab*)b)->escape) );
}

// Translate escapes in string:
char* bhpm_translate(char *s)
{
//    char	*d = (char*)malloc(strlen(s)+1);
    char	*d = s;		// @@HACK: Test for memleak
    int		i, j, k;
    char	replace;

    for (i=0, j=0; s[j]!='\0';)
	switch (s[j])
	    {
		case '&':
		    replace = 0;

		    if (s[j+1]!='\0')
			{
			    struct trans_tab	*code = (struct trans_tab*)bsearch(&(s[j+1]),bhpm_tt,130,sizeof(struct trans_tab),bhpm_compare);

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
//rikit konvertering til små bokstaver. Støtter også æøå
int bhpm_btolower(int c) {

        c = (unsigned char)c;


        if ((c >= 65) && (c <= 90)) {
                //A-Z
                c = c+32;
        }
        else if (c >= 192 && (c <=221)) {
                //øæå og andre utviede chars over 127
                c = c+32;
        }

        return c;
}


void convert_string_to_lowercase( unsigned char *c )
{
    int		i;

    for (i=0; c[i]!='\0'; i++)
	c[i] = (unsigned char)bhpm_btolower((int)c[i]);
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
	    convert_string_to_lowercase( (unsigned char*)(*uri) );
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

    if (uri==NULL)
	{
	    strcpy(new_url, page_uri);

	    if (path!=NULL)
		strcat( new_url, path );
	    else if (page_path!=NULL)
		strcat( new_url, page_path );

	    if (rel_path!=NULL)
		strcat( new_url, rel_path );
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
    sa_taglist = build_automaton( ca_taglist_size, (unsigned char**)ca_taglist );
    sa_meta_taglist = build_automaton( meta_taglist_size, (unsigned char**)meta_taglist );
    sa_spacetags = build_automaton( ca_spacetags_size, (unsigned char**)ca_spacetags );

    nha_tags = build_automaton( nh_tags_size, (unsigned char**)nh_tags );
    nda_tags = build_automaton( nd_tags_size, (unsigned char**)nd_tags );
    nsa_tags = build_automaton( ns_tags_size, (unsigned char**)ns_tags );
}

void html_parser_exit()
{
    free_automaton(nsa_tags);
    free_automaton(nda_tags);
    free_automaton(nha_tags);

    free_automaton(sa_spacetags);
    free_automaton(sa_meta_taglist);
    free_automaton(sa_taglist);
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
