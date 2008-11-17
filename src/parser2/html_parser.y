%{
// (C) Copyright Boitho 2004-2008, Magnus Galåen (magnusga@idi.ntnu.no)

/*
Changelog

Juli 2008 (Magnus):
    Fjerner ugyldig charsets, men sparer det som er gyldig.

August 2007 (Magnus):
    Parser nå css og detekterer cloaking.

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
#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dstack.h"
#include "../ds/dlist.h"
#include "../ds/dmap.h"
#include "css_parser.h"


// --- fra flex:
typedef void* yyscan_t;
typedef struct bhpm_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE bhpm_scan_bytes( const char *bytes, int len, yyscan_t yyscanner );
struct bhpm_yy_extra *bhpmget_extra( yyscan_t yyscanner );
// ---


struct tag_info
{
    int		tag;
    int		color, background, fontsize;
    char	hidden;
    char	*tag_name, *tag_id, *tag_class;
};

// Data for bison-parseren:
struct bhpm_intern_data
{
    char	*url;	// DEBUG
    char	abort;

    char	*page_uri, *page_path, *page_rel_path;
    int		page_url_len;

    char	*attr[16], *val[16];
    int		num_attr;

    int		color, background, fontsize;
    char	hidden;

    container	*tag_list;
};


char* create_full_link( char *url, int page_url_len, char *page_uri, char *page_path );

void lexwords(unsigned char *s);


char is_invisible( int color, int background, int fontsize );
void delete_tag_info(struct tag_info *ptr);
int parse_color( char *color );


char		*meta_attr[] = {"keywords","description","author","redirect"};
enum		{ meta_keywords=0, meta_description, meta_author, meta_redirect };
const int	meta_attr_size = 4;
automaton	*meta_attr_automaton = NULL;

char		*tag_attr[] = {"color","text","bgcolor","size","style","id","class"};
enum		{ attr_color=0, attr_text, attr_bgcolor, attr_size, attr_style, attr_id, attr_class };
const int	tag_attr_size = 7;
automaton	*tag_attr_automaton = NULL;

//void temp_func_css(struct bhpm_yy_extra *he, int hit, struct tag_info *ptr, char *arg_2, struct bhpm_intern_data *data);

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
		struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
		int			hit = search_automaton(tags_automaton, (char*)$2);

//		printf("tag:%s\n", (char*)$2);

		if (hit>=0)
		    {
			if (tag_flags[hit] & tagf_space)
			    he->space = 1;

			if (!(tag_flags[hit] & tagf_empty))
			    {
				struct tag_info	*ptr = malloc(sizeof(struct tag_info));

				ptr->tag = hit;
				ptr->tag_name = NULL;
				ptr->tag_id = NULL;
				ptr->tag_class = NULL;
				ptr->color = -1;
				ptr->background = -1;
				ptr->fontsize = -1;
				ptr->hidden = 0;

		    	        list_pushback(data->tag_list, (void*)ptr);

//				temp_func_css(he, hit, ptr, (char*)$2, data);

				if (he->css_selector_block!=NULL)
				    {
					int		color=-1, background=-1, fontsize=-1;
					int		i;

					// Find matching selectors:
					iterator	selector_it[3];

					ptr->tag_name = strdup((char*)$2);
					selector_it[0] = map_find(he->css_selector_block->tag_selectors, ptr->tag_name);
					selector_it[1].valid = 0;
					selector_it[2].valid = 0;

					for (i=0; i<data->num_attr; i++)
					    {
						if (data->val[i]!=NULL)
						    {
							int	attr_val = search_automaton(tag_attr_automaton, data->attr[i]);

							if (attr_val==attr_class)
							    {
								ptr->tag_class = strdup(data->val[i]);
				    			        selector_it[1] = map_find(he->css_selector_block->class_selectors, ptr->tag_class);
							    }
							else if (attr_val==attr_id)
							    {
								ptr->tag_id = strdup(data->val[i]);
								selector_it[2] = map_find(he->css_selector_block->id_selectors, ptr->tag_id);
							    }
						    }
					    }

					// Test for all selector-patterns ending with same tag, class or id:
					for (i=0; i<3; i++)
					    {
						iterator	css_it;

						if (selector_it[i].valid)
						    css_it = list_begin(map_val(selector_it[i]).C);
						else
						    css_it.valid = 0;

						// Test current selector for match:
						for (; css_it.valid; css_it=list_next(css_it))
						    {
							css_selector	*selector = list_val(css_it).ptr;
							iterator	rule_it = list_begin(selector->pattern);
							iterator	tag_it = list_begin(data->tag_list);

							for (; tag_it.valid && rule_it.valid; tag_it=list_next(tag_it))
							    {
								struct tag_info	*ptr = list_val(tag_it).ptr;
								css_token	*token = list_val(rule_it).ptr;
								char		match = 1;

								if (token->type == css_child || token->type == css_descendant)
								    {
									rule_it=list_next(rule_it);
									if (rule_it.valid) token = list_val(rule_it).ptr;
								    }

								for (; match && rule_it.valid;)
								    {
									match = 0;
									switch (token->type)
									    {
										case css_tag:
										    if (ptr->tag_name!=NULL && !strcasecmp(token->str, ptr->tag_name))
											match = 1;
										    break;
										case css_class:
										    if (ptr->tag_class!=NULL && !strcasecmp(token->str, ptr->tag_class))
											match = 1;
										    break;
										case css_id:
										    if (ptr->tag_id!=NULL && !strcasecmp(token->str, ptr->tag_id))
											match = 1;
										    break;
									    }

									if (match)
									    {
										rule_it=list_next(rule_it);
										if (rule_it.valid) token = list_val(rule_it).ptr;
									    }
								    }
							    }

							if (!rule_it.valid)
							    {
								if (selector->color >= 0) color = selector->color;
								if (selector->background >= 0) background = selector->background;
								if (selector->fontsize >= 0) fontsize = selector->fontsize;
								if (selector->hidden) ptr->hidden = 1;
							    }
						    }
					    }

					ptr->color = color;
					ptr->background = background;
					ptr->fontsize = fontsize;

					if (color>=0) data->color = color;
					if (background>=0) data->background = background;
					if (fontsize>=0) data->fontsize = fontsize;
				    } // end if (..css..)

				int		i;
				int		color=-1, background=-1, fontsize=-1;

				for (i=0; i<data->num_attr; i++)
				    {
					if (data->val[i]!=NULL)
					    {
						int	attr_val = search_automaton(tag_attr_automaton, data->attr[i]);

//				    		if (!strcasecmp(data->attr[i], "color") || (hit==tag_body && !strcasecmp(data->attr[i], "text")))
						if (attr_val==attr_color || (hit==tag_body && attr_val==attr_text))
					    // basefont, font
				    		    {
							color = parse_color(data->val[i]);
						    }
//						else if (!strcasecmp(data->attr[i], "bgcolor"))
						else if (attr_val==attr_bgcolor)
					    // body, table, th, td, tr
						    {
							background = parse_color(data->val[i]);
						    }
//						else if (hit==tag_font && !strcasecmp(data->attr[i], "size"))
						else if (hit==tag_font && attr_val==attr_size)
						    { // 1==10, 2==13, 3==16, 4==18, 5==24, 6==32, 7==48
							int	size_mod[7] = {10,13,16,18,24,32,48};
							int	k;

							k = atoi(data->val[i]);
							if (k>=1 && k<=7)
							    fontsize = size_mod[k-1];
						    }
//				    	        else if (!strcasecmp(data->attr[i], "style"))
						else if (attr_val==attr_style)
						    {
//							{blank}*{ident}+{blank}*:{blank}*{value}+{blank}*;?
							int	p = 0, n, r, j;
							char	*T = data->val[i];
							char	*ident, *value[16];

							while (T[p]!='\0')
							    {
								while (T[p]!='\0' && (T[p]==' ' || T[p]=='\t' || T[p]=='\r' || T[p]=='\n' || T[p]=='\f')) p++;
								ident = &(T[p]);
								while (T[p]!='\0' && !(T[p]==' ' || T[p]=='\t' || T[p]=='\r' || T[p]=='\n' || T[p]=='\f' || T[p]==':')) p++;
								n = p;
								while (T[p]!='\0' && (T[p]==' ' || T[p]=='\t' || T[p]=='\r' || T[p]=='\n' || T[p]=='\f' || T[p]==':')) p++;
								T[n] = '\0';

								j = 0;
								while (T[p]!='\0' && T[p]!=';')
								    {
									value[j] = &(T[p]);
									while (T[p]!='\0' && T[p]!=';' && T[p]!=' ' && T[p]!='\t' && T[p]!='\r' && T[p]!='\n' && T[p]!='\f') p++;
									r = p;
									while (T[p]!='\0' && T[p]!=';' && (T[p]==' ' || T[p]=='\t' || T[p]=='\r' || T[p]=='\n' || T[p]=='\f')) p++;
									T[r] = '\0';
									if (j<16) j++;
								    }

								if (T[p]==';') p++;

								if (strlen(ident)>0 && j>0 && strlen(value[0])>0)
								    {
									int	z;
									int	tmp;

									if (!strcmp(ident, "font-size") || !strcmp(ident, "font"))
									    {
										tmp = -1;

										for (z=0; z<j; z++)
										    {
											if (value[z][0]>='0' && value[z][0]<='9')
											    {
												int	len = strlen(value[z]);
												if ((value[z][len-1]>='0' && value[z][len-1]<='9')
												    || (value[z][len-2]=='p' && value[z][len-1]=='x'))
											    	    tmp = atoi(value[z]);
											    }
											else if (!strcmp(value[z], "xx-small"))
											    tmp = 9;
											else if (!strcmp(value[z], "x-small"))
											    tmp = 10;
											else if (!strcmp(value[z], "small"))
											    tmp = 13;
											else if (!strcmp(value[z], "medium"))
											    tmp = 16;
											else if (!strcmp(value[z], "large"))
											    tmp = 18;
											else if (!strcmp(value[z], "x-large"))
											    tmp = 24;
											else if (!strcmp(value[z], "xx-large"))
											    tmp = 32;

											if (tmp>=0) fontsize = tmp;
										    }
									    }
									else if (!strcmp(ident, "color"))
									    {
										tmp = -1;
										for (z=0; z<j; z++)
										    {
											tmp = parse_color(value[z]);
											if (tmp>=0) color = tmp;
										    }
									    }
									else if (!strcmp(ident, "background") || !strcmp(ident, "background-color"))
									    {
										tmp = -1;
										for (z=0; z<j; z++)
										    {
											tmp = parse_color(value[z]);
											if (tmp>=0) background = tmp;
										    }
									    }
									else if (!strcmp(ident, "visibility"))
									    {
										for (z=0; z<j; z++)
										    if (!strcmp(value[z], "hidden"))
											{
											    ptr->hidden = 1;
											    break;
											}
									    }
								    }
							    }
						    }
					    }
				    }

				if (ptr->hidden) data->hidden++;

				if (color>=0 || background>=0 || fontsize>=0 || data->hidden>0)
				    {
					if (color>=0)
					    {
						data->color = color;
						ptr->color = color;
					    }
					if (background>=0)
					    {
						data->background = background;
						ptr->background = background;
					    }
					if (fontsize>=0)
					    {
						data->fontsize = fontsize;
						ptr->fontsize = fontsize;
					    }

					if (data->hidden) he->invisible_text = 1;
					else he->invisible_text = is_invisible(data->color, data->background, data->fontsize);
				    }
			    } // end if (.. tagf_empty)


//		    printf("(%s,%i)", (char*)$2, hit); fflush(stdout);
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
						    he->nlink = 1;
//							clean(data->val[i]);

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
			        char			*content = NULL;
				enum parsed_unit	pu = pu_none;
				char			http_content_type = 0;

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
					else if (!strcasecmp(data->attr[i], "http-equiv") && data->val[i] != NULL)
					    {
						if (!strcasecmp(data->val[i], "Content-Type"))
						    {
							http_content_type = 1;
						    }
					    }
				    }

				if (http_content_type && content!=NULL)
				    {
					char	*ptr = strstr(content, "charset=");

					if (ptr != NULL)
					    {
						// Husk: Oppdater j når det legges til charsets.
						// Husk: Sjekk også startendtag.
						int	j = 0;
						char	charset_match = 0;
						char	*allowed_charsets[] = {"iso-8859", "iso8859", "utf-8", "latin1", "windows-1252", "us-ascii", "macintosh"};

						while (ptr[0] != '=') ptr++;
						ptr++;

						for (j=0; j<7 && !charset_match; j++)
						    if (!strncasecmp(ptr, allowed_charsets[j], strlen(allowed_charsets[j])))
							charset_match = 1;

						if (!charset_match)
						    {
							fprintf(stderr, "html_parser: Warning! Illegal charset (%s), skipping text.\n", content);
							he->illegal_charset = 1;
						    }
						else
						    {
//							fprintf(stderr, "html_parser: Charset %s.\n", content);
							he->illegal_charset = 0;
						    }
/*
						while (ptr[j]!='\0' && (
						    (ptr[j]>='A' && ptr[j]<='Z')
						    || (ptr[j]>='a' && ptr[j]<='z')
						    || (ptr[j]>='0' && ptr[j]<='9')
						    || ptr[j]=='-' || ptr[j]=='_'))
						    j++;

						ptr[j] = '\0';
						printf("CHARSET IS '%s' !!!\n", ptr);
*/
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
//				printf("\n\033[0;7mtitle\033[0m\n");
			        he->title = 1;
				he->title_nr++;
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

		    } // end if (hit>=0)
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

		if (hit>=0 && list_size(data->tag_list)>1)
		    {
//			printf("\033[1;34m</%s>\033[0m\n", (char*)$2);

			int		i;

			iterator	it = list_end(data->tag_list);
			for (; it.valid; it=list_previous(it))
			    {
				struct tag_info	*ptr = list_val(it).ptr;

				if (ptr->tag == hit)
				    {
//					printf("deleted: ");
					if (tag_flags[ptr->tag] & tagf_block)
					    {
						iterator	gammel_it;
						for (; it.valid;)
						    {
//							printf("%s ", ((struct tag_info*)list_val(it).ptr)->tag_name);
							delete_tag_info(list_val(it).ptr);
							gammel_it = it;
							it=list_next(it);
							list_erase(data->tag_list, gammel_it);
						    }
					    }
					else
					    {
//						printf("%s", ptr->tag_name);
						delete_tag_info(list_val(it).ptr);
						list_erase(data->tag_list, it);
					    }
//					printf("\n");
					break;
				    }
			    }

			data->color = -1;
			data->background = -1;
			data->fontsize = -1;
			data->hidden = 0;
			it = list_end(data->tag_list);
			for (; it.valid; it=list_previous(it))
			    {
				struct tag_info	*ptr = list_val(it).ptr;

				if (data->color < 0 && ptr->color >= 0) data->color = ptr->color;
				if (data->background < 0 && ptr->background >= 0) data->background = ptr->background;
				if (data->fontsize < 0 && ptr->fontsize >= 0) data->fontsize = ptr->fontsize;
				if (ptr->hidden) data->hidden++;
			    }

			if (data->hidden>0) he->invisible_text = 1;
			else he->invisible_text = is_invisible(data->color, data->background, data->fontsize);
		    }
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
			char			*content = NULL;
			enum parsed_unit	pu = pu_none;
			char			http_content_type = 0;

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
				else if (!strcasecmp(data->attr[i], "http-equiv") && data->val[i] != NULL)
				    {
					if (!strcasecmp(data->val[i], "Content-Type"))
					    {
						http_content_type = 1;
					    }
				    }
			    }

			if (http_content_type && content!=NULL)
			    {
				char	*ptr = strstr(content, "charset=");

				if (ptr != NULL)
				    {
					int	j = 0;
					char	charset_match = 0;
					char	*allowed_charsets[] = {"iso-8859", "iso8859", "utf-8", "latin1", "windows-1252", "us-ascii", "macintosh"};

					while (ptr[0] != '=') ptr++;
					ptr++;

					for (j=0; j<7 && !charset_match; j++)
					    if (!strncasecmp(ptr, allowed_charsets[j], strlen(allowed_charsets[j])))
						charset_match = 1;

					if (!charset_match)
					    {
						fprintf(stderr, "html_parser: Warning! Illegal charset (%s), skipping text.\n", content);
						he->illegal_charset = 1;
					    }
					else
					    {
//						fprintf(stderr, "html_parser: Charset %s.\n", content);
						he->illegal_charset = 0;
					    }
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

/*
void temp_func_css(struct bhpm_yy_extra *he, int hit, struct tag_info *ptr, char *arg_2, struct bhpm_intern_data *data)
{
				if (he->css_selector_block!=NULL)
				    {
					int		color=-1, background=-1, fontsize=-1;
					int		i;

					// Find matching selectors:
					iterator	selector_it[3];

					ptr->tag_name = strdup(arg_2);
					selector_it[0] = map_find(he->css_selector_block->tag_selectors, ptr->tag_name);
					selector_it[1].valid = 0;
					selector_it[2].valid = 0;

					for (i=0; i<data->num_attr; i++)
					    {
						if (data->val[i]!=NULL)
						    {
							int	attr_val = search_automaton(tag_attr_automaton, data->attr[i]);

							if (attr_val==attr_class)
							    {
								ptr->tag_class = strdup(data->val[i]);
				    			        selector_it[1] = map_find(he->css_selector_block->class_selectors, ptr->tag_class);
							    }
							else if (attr_val==attr_id)
							    {
								ptr->tag_id = strdup(data->val[i]);
								selector_it[2] = map_find(he->css_selector_block->id_selectors, ptr->tag_id);
							    }
						    }
					    }

					// Test for all selector-patterns ending with same tag, class or id:
					for (i=0; i<3; i++)
					    {
						iterator	css_it;

						if (selector_it[i].valid)
						    css_it = list_begin(map_val(selector_it[i]).C);
						else
						    css_it.valid = 0;

						// Test current selector for match:
						for (; css_it.valid; css_it=list_next(css_it))
						    {
							css_selector	*selector = list_val(css_it).ptr;
							iterator	rule_it = list_begin(selector->pattern);
							iterator	tag_it = list_begin(data->tag_list);

							for (; tag_it.valid && rule_it.valid; tag_it=list_next(tag_it))
							    {
								struct tag_info	*ptr = list_val(tag_it).ptr;
								css_token	*token = list_val(rule_it).ptr;
								char		match = 1;

								if (token->type == css_child || token->type == css_descendant)
								    {
									rule_it=list_next(rule_it);
									if (rule_it.valid) token = list_val(rule_it).ptr;
								    }

								for (; match && rule_it.valid;)
								    {
									match = 0;
									switch (token->type)
									    {
										case css_tag:
										    if (ptr->tag_name!=NULL && !strcasecmp(token->str, ptr->tag_name))
											match = 1;
										    break;
										case css_class:
										    if (ptr->tag_class!=NULL && !strcasecmp(token->str, ptr->tag_class))
											match = 1;
										    break;
										case css_id:
										    if (ptr->tag_id!=NULL && !strcasecmp(token->str, ptr->tag_id))
											match = 1;
										    break;
									    }

									if (match)
									    {
										rule_it=list_next(rule_it);
										if (rule_it.valid) token = list_val(rule_it).ptr;
									    }
								    }
							    }

							if (!rule_it.valid)
							    {
								if (selector->color >= 0) color = selector->color;
								if (selector->background >= 0) background = selector->background;
								if (selector->fontsize >= 0) fontsize = selector->fontsize;
								if (selector->hidden) ptr->hidden = 1;
							    }
						    }
					    }

					ptr->color = color;
					ptr->background = background;
					ptr->fontsize = fontsize;

					if (color>=0) data->color = color;
					if (background>=0) data->background = background;
					if (fontsize>=0) data->fontsize = fontsize;
				    } // end if (..css..)
}
*/

void delete_tag_info(struct tag_info *ptr)
{
    free(ptr->tag_name);
    free(ptr->tag_id);
    free(ptr->tag_class);
    free(ptr);
}


char is_invisible( int color, int background, int fontsize )
{
    int		diff, r, g, b;

    r = abs( ((color>>16)&0xff) - ((background>>16)&0xff) );
    g = abs( ((color>>8)&0xff) - ((background>>8)&0xff) );
    b = abs( ((color)&0xff) - ((background)&0xff) );

    diff = r+g+b;

    return ((diff < 48) || (fontsize<8));
}


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
	//fprintf(stderr, "html_parser: Warning! page_uri==NULL\n");
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
    fprintf(stderr, "html_parser: init()\n");
    // Removed elements: noframes, noscript, script, style, select, textarea, object

    tags_automaton = build_automaton( tags_size, (unsigned char**)tags );
    meta_attr_automaton = build_automaton( meta_attr_size, (unsigned char**)meta_attr );
    tag_attr_automaton = build_automaton( tag_attr_size, (unsigned char**)tag_attr );
//    color_names_automaton = build_automaton( color_names_size, (unsigned char**)color_names );
//    text_containers_automaton = build_automaton( text_containers_size, (unsigned char**)text_containers );

    css_parser_init();
}

void html_parser_exit()
{
    fprintf(stderr, "html_parser: exit()\n");
    css_parser_exit();
//    free_automaton(text_containers_automaton);
//    free_automaton(color_names_automaton);
    free_automaton(tag_attr_automaton);
    free_automaton(meta_attr_automaton);
    free_automaton(tags_automaton);
}


//void run_html_parser( char *url, char text[], int textsize, void (*fn)(char*,int,enum parsed_unit,enum parsed_unit_flag) )
void html_parser_run( char *url, char text[], int textsize, char **output_title, char **output_body,
    void (*fn)(char*,int,enum parsed_unit,enum parsed_unit_flag,void* wordlist), void* wordlist )
{
    fprintf(stderr, "html_parser: run(\"%s\")\n", url);
    assert(tags_automaton!=NULL);	// Gir feilmelding dersom man har glemt å kjøre html_parser_init().

    struct bhpm_yy_extra	*he = malloc(sizeof(struct bhpm_yy_extra));
    struct bhpm_intern_data	*data = malloc(sizeof(struct bhpm_intern_data));

    data->abort = 0;
    data->url = strdup(url);	// DEBUG

#ifdef PARSEERR
    {
	int i;
	for (i=0; i<201; i++)
	    he->last[i] = '\0';
    }
#endif

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
    he->title_nr = 0;
    he->alink = 0;
    he->nlink = 0;
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
    he->inlink = 0;

    he->css_selector_block = NULL;

    he->Btitle = buffer_init( 10240 );
    he->Bbody = buffer_init( 16384 + textsize*2 );

    data->tag_list = list_container( ptr_container() );

    struct tag_info	*ptr = malloc(sizeof(struct tag_info));
    ptr->tag = -1;
    ptr->color = 0x000000;
    ptr->background = 0xffffff;
    ptr->fontsize = 16;	// medium
    ptr->hidden = 0;
    ptr->tag_name = strdup("");
    ptr->tag_class = strdup("");
    ptr->tag_id = strdup("");

    list_pushback( data->tag_list, ptr );
    data->color = ptr->color;
    data->background = ptr->background;
    data->fontsize = ptr->fontsize;
    data->hidden = 0;

//    data->font_list = list_container( pair_container( string_container(), pair_container( int_container(), int_container() ) ) );
//    list_pushback( data->font_list, "", 0x000000, 0xffffff );

//    data->crnt_txtcol = 0x000000;
//    data->crnt_bgcol = 0xffffff;
    he->invisible_text = 0;
    he->illegal_charset = 0;

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

    if (he->inlink) bprintf(he->Bbody, "</link>");
    if (he->inspan) bprintf(he->Bbody, "</span>\n");
    if (he->inhead) bprintf(he->Bbody, "  </h2>\n");
    if (he->indiv) bprintf(he->Bbody, "</div>\n");

    bhpm_delete_buffer( bs, scanner );
    bhpmlex_destroy( scanner );

    if (he->Bbody->overflow)
	data->abort = 1;

    if (data->abort)	// On error
	{
            #ifndef NOWARNINGS
            fprintf(stderr, "html_parser: Document error, content was:\n");
            fprintf(stderr, "html_parser: --- Content begin ---\n");
            fprintf(stderr, "%s\n", text);
            fprintf(stderr, "html_parser: --- Content end ---\n");
            #endif
	    // Magnus: Fjernet buffer_abort(). Den teksten som allerede har blitt indeksert, bÃ¸r finnes.
	    *output_title = buffer_exit( he->Btitle );
	    *output_body = buffer_exit( he->Bbody );
	}
    else
	{
	    *output_title = buffer_exit( he->Btitle );
	    *output_body = buffer_exit( he->Bbody );
	}

//    destroy(data->font_list);
    iterator	it = list_begin(data->tag_list);
    for (; it.valid; it=list_next(it))
	{
	    delete_tag_info(list_val(it).ptr);
	}

    free(data->url);	// DEBUG

    destroy(data->tag_list);
    free(data->page_uri);
    free(data->page_path);
    free(data->page_rel_path);
    free(data);

    if (he->css_selector_block!=NULL)
	destroy_selectors(he->css_selector_block);
    free(he);
}


yyerror( struct bhpm_intern_data *data, void *yyscan_t, char *s )
{
	#ifndef NOWARNINGS
    		fprintf(stderr, "html_parser: Parse error! %s\n", s);
	#endif
}
