%{

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../common/search_automaton.h"
#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dlist.h"
#include "../ds/dmap.h"
#include "../ds/dvector.h"
#include "css_parser_common.h"
#include "css_parser.h"


// --- fra flex:
typedef void* yyscan_t;
typedef struct bcpm_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE bcpm_scan_bytes( const char *bytes, int len, yyscan_t yyscanner );
struct bcpm_yy_extra *bcpmget_extra( yyscan_t yyscanner );
// ---



#ifdef FULL_PARSING
typedef struct
{
    char	*property;
    container	*values;
} css_declaration;
#endif

struct bcpm_intern_data
{
    iterator	last_selector;
    container	*values;
#ifdef FULL_PARSING
    int		last_value;
    container	*declarations;
#endif
    container	*selectors;
    int		crnt_ruleno;
    css_selector *crnt_selector;
    int		color_ident, color_val,
		fontsz_ident, fontsz_val,
		visblty_ident,
		crnt_bk, crnt_col, crnt_fsz, crnt_hidden;
};


char		*color_names[] = {"black", "green", "silver", "lime", "gray", "olive", "white", "yellow", "maroon", "navy", "red", "blue", "purple", "teal", "fuchsia", "aqua", "orange"};
int		color_values[] = {0x000000, 0x008000, 0xc0c0c0, 0x00ff00, 0x808080, 0x808000, 0xffffff, 0xffff00, 0x800000, 0x000080, 0xff0000, 0x0000ff, 0x800080, 0x008080, 0xff00ff, 0x00ffff, 0xffa500};
const int	color_names_size = 17;
automaton	*color_names_automaton = NULL;

char		*fontsz_names[] = {"xx-small", "x-small", "small", "medium", "large", "x-large", "xx-large"};
int		fontsz_values[] = {9,10,13,16,18,24,32};
const int	fontsz_names_size = 7;
automaton	*fontsz_names_automaton = NULL;


void delete_selector_node(css_selector *s);


%}

%pure-parser
%parse-param { struct bcpm_intern_data *data }
%parse-param { yyscan_t yyscanner }
%lex-param { yyscan_t yyscanner }
%token IDENT BRACE_START BRACE_STOP COLON SEMICOLON COMMA AT STRING IDSELECTOR CLASSELECTOR PSEUDOCLASS HASH NUMBER PERCENTAGE DIMENSION URI FUNCTION_BEGIN FUNCTION_END BLANK CHILD COLOR_IDENT INT FONTSZ_IDENT PX_DIMENSION PT_DIMENSION EM_DIMENSION EX_DIMENSION VISBLTY_IDENT

%%
statement	:
		| statement ruleset
		| statement BLANK
		;
ruleset		: selector_list BRACE_START declaration_set BRACE_STOP
		{
#ifdef FULL_PARSING
		    if (vector_size(data->declarations) > data->last_value)
#else
		    if (data->crnt_col>=0 || data->crnt_bk>=0 || data->crnt_fsz>=0 || data->crnt_hidden)
#endif
			{
			    iterator		it;

			    if (data->last_selector.valid)
				it = list_next(data->last_selector);
			    else
				it = list_begin(data->selectors);

#ifdef FULL_PARSING
			    int		start = data->last_value, stop = vector_size(data->declarations);
#endif
			    for (; it.valid; it=list_next(it))
				{
				    css_selector	*s = list_val(it).ptr;

#ifdef DEBUG
				    printf("\033[1;33m%i\033[0m ", s->ruleno);
#endif
#ifdef FULL_PARSING
				    s->decl_start = start;
				    s->decl_stop = stop;
#endif
				    s->color = data->crnt_col;
				    s->background = data->crnt_bk;
				    s->fontsize = data->crnt_fsz;
				    s->hidden = data->crnt_hidden;

				    iterator	sit;
				    sit = list_end(s->pattern);

				    if (sit.valid && ((css_token*)list_val(sit).ptr)->type == css_descendant)
					{
					    free(list_val(sit).ptr);
					    list_erase(s->pattern, sit);
					}
				}

			    data->crnt_col = -1;
			    data->crnt_bk = -1;
			    data->crnt_fsz = -1;
			    data->crnt_hidden = 0;
			    data->last_selector = list_end(data->selectors);

#ifdef FULL_PARSING
			    data->last_value = vector_size(data->declarations);
#endif
			}
		    else
			{
			    iterator	it;

			    if (data->last_selector.valid)
				it = list_next(data->last_selector);
			    else
				it = list_begin(data->selectors);

			    for (; it.valid;)
				{
#ifdef DEBUG
				    printf("\033[0;34m%i\033[0m ", ((css_selector*)list_val(it).ptr)->ruleno);
#endif

				    delete_selector_node(list_val(it).ptr);

				    iterator	gammel_it = it;
				    it = list_next(it);
				    list_erase(data->selectors, gammel_it);
				}

			    data->last_selector = list_end(data->selectors);
			}
		}
		;
selector_list	:
		{
		    struct bcpm_yy_extra	*ce = bcpmget_extra(yyscanner);
		    ce->inside_selector = 1;

		    data->crnt_selector = malloc(sizeof(css_selector));
		    data->crnt_selector->ruleno = data->crnt_ruleno++;
		    data->crnt_selector->pattern = list_container( ptr_container() );
		    list_pushback(data->selectors, data->crnt_selector);
		}
		| selector_list selector
		;
selector	: IDENT
		{
		    css_token	*ct = malloc(sizeof(css_token));

		    ct->type = css_tag;
		    ct->str = strdup((char*)$1);
		    list_pushback(data->crnt_selector->pattern, ct);
		}
		| IDSELECTOR
		{
		    css_token	*ct = malloc(sizeof(css_token));

		    ct->type = css_id;
		    ct->str = strdup(&(((char*)$1)[1]));
		    list_pushback(data->crnt_selector->pattern, ct);
		}
		| CLASSELECTOR
		{
		    css_token	*ct = malloc(sizeof(css_token));

		    ct->type = css_class;
		    ct->str = strdup(&(((char*)$1)[1]));
		    list_pushback(data->crnt_selector->pattern, ct);
		}
		| PSEUDOCLASS
		{
		}
		| BLANK
		{
		    if (list_size(data->crnt_selector->pattern) > 0)
			{
			    css_token	*ct = malloc(sizeof(css_token));

			    ct->type = css_descendant;
			    ct->str = NULL;
			    list_pushback(data->crnt_selector->pattern, ct);
			}
		}
		| CHILD
		{
		    css_token	*ct = malloc(sizeof(css_token));

		    ct->type = css_child;
		    ct->str = NULL;
		    list_pushback(data->crnt_selector->pattern, ct);
		}
		| COMMA
		{
		    data->crnt_selector = malloc(sizeof(css_selector));
		    data->crnt_selector->ruleno = data->crnt_ruleno++;
		    data->crnt_selector->pattern = list_container( ptr_container() );
		    list_pushback(data->selectors, data->crnt_selector);
		}
		;
declaration_set	:
		{
		    struct bcpm_yy_extra	*ce = bcpmget_extra(yyscanner);
		    ce->inside_selector = 0;
		}
		| declaration dec_semicolon
		;
dec_semicolon	:
		| SEMICOLON declaration_set
		;
declaration	: property COLON value value_list
		{
#ifdef FULL_PARSING
		    css_declaration	*decl = malloc(sizeof(css_declaration));
		    decl->property = strdup((char*)$1);
		    decl->values = data->values;
		    vector_pushback(data->declarations, (void*)decl);
		    data->values = data->values->clone(data->values);
#endif

#ifdef DEBUG_2
		    if (data->color_ident && data->color_val>=0)
			printf("%s: (\033[0;34m%.6x\033[0m)\n", (char*)$1, data->color_val);
		    if (data->fontsz_ident && data->fontsz_val>=0)
			printf("font-size: (\033[0;34m%i\033[0m)\n", data->fontsz_val);
#endif

		    if (data->color_ident)
			{
			    if (!strcasecmp("color", (char*)$1))
				{
				    data->crnt_col = data->color_val;
				}
			    else
				{
				    data->crnt_bk = data->color_val;
				}
			}

		    if (data->fontsz_ident)
			data->crnt_fsz = data->fontsz_val;
		}
		;
property	: IDENT
		{
		    data->color_ident = 0;
		    data->fontsz_ident = 0;
		    data->visblty_ident = 0;
		}
		| COLOR_IDENT
		{
		    data->color_ident = 1;
		    data->fontsz_ident = 0;
		    data->visblty_ident = 0;
		    data->color_val = -1;
		}
		| FONTSZ_IDENT
		{
		    data->color_ident = 0;
		    data->fontsz_ident = 1;
		    data->visblty_ident = 0;
		    data->fontsz_val = -1;
		}
		| VISBLTY_IDENT
		{
		    data->color_ident = 0;
		    data->fontsz_ident = 0;
		    data->visblty_ident = 1;
		}
		;
value_list	:
		| value_list value
		;
value		: IDENT
		{
		    if (data->color_ident)
			{
			    int		hit = search_automaton(color_names_automaton, (char*)$1);

			    if (hit>=0)
				{
				    int		color = color_values[hit];
//				    printf("color: (\033[0;34m%.6x\033[0m)\n", color);
				    data->color_val = color;
				}
			}
		    else if (data->fontsz_ident)
			{
			    int		hit = search_automaton(fontsz_names_automaton, (char*)$1);

			    if (hit>=0)
				{
    				    data->fontsz_val = fontsz_values[hit];
				}
			}
		    else if (data->visblty_ident)
			{
			    if (!strcmp("hidden", (char*)$1))
				data->crnt_hidden = 1;
			}

		    list_pushback(data->values, IDENT, $1);
		}
		| NUMBER
		{
		    if (data->fontsz_ident)
			{
			    data->fontsz_val = atoi((char*)$1);
			}

		    list_pushback(data->values, NUMBER, $1);
		}
		| PERCENTAGE
		{
		    list_pushback(data->values, PERCENTAGE, $1);
		}
		| PX_DIMENSION
		{
		    if (data->fontsz_ident)
			{
			    data->fontsz_val = atoi((char*)$1);
			}

		    list_pushback(data->values, PX_DIMENSION, $1);
		}
		| DIMENSION
		{
		    list_pushback(data->values, DIMENSION, $1);
		}
		| HASH
		{
		    if (data->color_ident)
			{
			    int		len = strlen((char*)$1);
			    int		color=0;

			    if (len==4)
				{
				    int		tmp;
				    tmp = strtol(&(((char*)$1)[1]), NULL, 16);
				    color = ((tmp&3840)*(4096+256)) + ((tmp&240)*(256+16)) + ((tmp&15)*(16+1));
				}
			    else
				{
				    color = strtol(&(((char*)$1)[1]), NULL, 16);
				}
//			    printf("color: (\033[0;34m%.6x\033[0m)\n", color);
			    data->color_val = color;
			}

		    list_pushback(data->values, HASH, $1);
		}
		| URI
		{
		    list_pushback(data->values, URI, $1);
		}
		| STRING
		{
		    list_pushback(data->values, STRING, $1);
		}
		| COMMA
		| function
		;
function	: FUNCTION_BEGIN value_list FUNCTION_END
		{
//		    printf("function: %s\n", (char*)$1);
		    if (data->color_ident && !strcasecmp("rgb(", (char*)$1))
			{
			    iterator	it = list_end(data->values);
			    int		i, color=0;

			    for (i=0; i<3 && it.valid; i++)
				{
				    int		val=0;

				    if (pair(list_val(it)).first.i == NUMBER)
					{
					    val = atoi((char*)pair(list_val(it)).second.ptr);
					}
				    else if (pair(list_val(it)).first.i == PERCENTAGE)
					{
		    			    val = (atoi((char*)pair(list_val(it)).second.ptr)*255)/100;
					}

				    if (val<0) val=0;
				    else if (val>255) val=255;
				    color+= val << (8*i);

				    iterator	gammel_it = it;
				    it = list_previous(it);
				    list_erase(data->values, gammel_it);
				}
//			    printf("color: (\033[0;34m%.6x\033[0m) (rgb)\n", color);
			    data->color_val = color;
//			    list_pushback(data->values, INT, color);
			}
		}
		;
%%

void delete_selector_node(css_selector *s)
{
    iterator		sit = list_begin(s->pattern);

    for (; sit.valid; sit=list_next(sit))
	{
	    css_token	*t = list_val(sit).ptr;

	    if (t->str!=NULL) free(t->str);
	    free(t);
	}

    destroy(s->pattern);
    free(s);
}


void destroy_selectors(css_selector_block *selector_block)
{
    iterator	it = list_begin(selector_block->selectors);
    for (; it.valid; it=list_next(it))
	{
	    delete_selector_node(list_val(it).ptr);
	}

    destroy(selector_block->selectors);
    destroy(selector_block->tag_selectors);
    destroy(selector_block->class_selectors);
    destroy(selector_block->id_selectors);
    free(selector_block);
}


void css_parser_init()
{
    color_names_automaton = build_automaton( color_names_size, (unsigned char**)color_names );
    fontsz_names_automaton = build_automaton( fontsz_names_size, (unsigned char**)fontsz_names );
}


void css_parser_exit()
{
    free_automaton(fontsz_names_automaton);
    free_automaton(color_names_automaton);
    fontsz_names_automaton = NULL;
    color_names_automaton = NULL;
}


css_selector_block* css_parser_run( char text[], int textsize )
{
    assert(color_names_automaton != NULL);

    struct bcpm_yy_extra	*ce = malloc(sizeof(struct bcpm_yy_extra));
    struct bcpm_intern_data	*data = malloc(sizeof(struct bcpm_intern_data));

    ce->state = 0;
    ce->in_function = 0;
    ce->inside_selector = 0;
    ce->string = 0;

    data->values = list_container( pair_container( int_container(), string_container() ) );
#ifdef FULL_PARSING
    data->declarations = vector_container( ptr_container() );
    data->last_value = 0;
#endif
    data->selectors = list_container( ptr_container() );
    data->last_selector.valid = 0;
    data->crnt_ruleno = 0;
    data->crnt_selector = NULL;
    data->crnt_col = -1;
    data->crnt_bk = -1;
    data->crnt_fsz = -1;
    data->crnt_hidden = 0;

    yyscan_t		scanner;
    bcpmlex_init(&scanner);
    bcpmset_extra(ce, scanner);
    YY_BUFFER_STATE	bs = bcpm_scan_bytes(text, textsize, scanner);
    int			yv;

    while ((yv=bcpmparse(data, scanner))!=0)
	{
	    #ifndef NOWARNINGS
	    printf("Skipping css-code...\n");
	    #endif
	    // On error, skip entire css, and return zero:
	    bcpm_delete_buffer(bs, scanner);
	    bcpmlex_destroy(scanner);

	    free(ce);

	    destroy(data->values);
	    free(data);

	    return NULL;
	}

//    while (bcpmlex(scanner));

#ifdef DEBUG
    printf("\n");
#endif

    bcpm_delete_buffer(bs, scanner);
    bcpmlex_destroy(scanner);

#ifdef SHOWRULES
    {
	iterator	it = list_begin(data->selectors);

	for (; it.valid; it=list_next(it))
	    {
		css_selector	*s = list_val(it).ptr;

		printf("%.2i: ", s->ruleno);

		iterator	sit = list_begin(s->pattern);

		for (; sit.valid; sit=list_next(sit))
		    {
			css_token	*token = list_val(sit).ptr;

		        switch (token->type)
			    {
			        case css_tag:
				    printf("(tag)");
				    break;
				case css_class:
				    printf("(class)");
				    break;
				case css_id:
				    printf("(id)");
				    break;
				case css_descendant:
				    printf("(desc)");
				    break;
				case css_child:
				    printf("(child)");
				    break;
				case css_sibling:
				    printf("(sibling)");
				    break;
				default:
				    printf("(unknown)");
			    }

			if (token->str != NULL)
			    printf("(\033[1;36m%s\033[0m)", token->str);
			printf(" ");
		    }

    		printf("\n");

		int		i;

		printf("    ");
		if (s->color >= 0)
		    printf("\033[1;34mcolor\033[0m: %.6x; ", s->color);
		if (s->background >= 0)
		    printf("\033[1;34mbackground\033[0m: %.6x; ", s->background);
		if (s->fontsize >= 0)
		    printf("\033[1;34mfont-size\033[0m: %i; ", s->fontsize);
		printf("\n");
#ifdef FULL_PARSING
		printf("    ");
		for (i=s->decl_start; i<s->decl_stop; i++)
		    {
			css_declaration	*decl = vector_get(data->declarations, i).ptr;
			printf("\033[1;34m%s\033[0m:", decl->property);

			iterator			vit = list_begin(decl->values);
			for (; vit.valid; vit=list_next(vit))
			    switch (pair(list_val(vit)).first.i)
				{
				    case INT:
					printf(" 0x%6x", pair(list_val(vit)).second.i);
					break;
				    default:
					printf(" %s", (char*)pair(list_val(vit)).second.ptr);
				}
			printf("; ");
		    }
		printf("\n---\n");
#endif	// FULL_PARSING
	    }
    }
#endif	// SHOW_RULES

    free(ce);

#ifdef FULL_PARSING
    int		i;

    for (i=0; i<vector_size(data->declarations); i++)
	{
	    css_declaration	*decl = vector_get(data->declarations, i).ptr;
	    free(decl->property);
	    destroy(decl->values);
	    free(decl);
	}

    destroy(data->declarations);
#endif

    destroy(data->values);

    css_selector_block	*selector_block = malloc(sizeof(css_selector_block));

    selector_block->selectors = data->selectors;
    selector_block->tag_selectors = map_container( string_container(), list_container( ptr_container() ) );
    selector_block->class_selectors = map_container( string_container(), list_container( ptr_container() ) );
    selector_block->id_selectors = map_container( string_container(), list_container( ptr_container() ) );

    // Sort selectors in tag, class and id-containers:
    iterator	it = list_begin(selector_block->selectors);

    for (; it.valid; it=list_next(it))
	{
	    css_selector	*selector = list_val(it).ptr;

	    if (list_size(selector->pattern) == 0)
		{
		    #ifdef DEBUG
		    printf("Invalid pattern\n");
		    #endif
		    continue;
		}

	    css_token		*token = list_val(list_end(selector->pattern)).ptr;
	    container		*C = NULL;

	    switch (token->type)
		{
		    case css_tag:
//			printf("tag ");
			C = selector_block->tag_selectors;
			break;
		    case css_class:
//			printf("class ");
			C = selector_block->class_selectors;
			break;
		    case css_id:
//			printf("id ");
			C = selector_block->id_selectors;
			break;
//		    default:
//			printf("??? ");
		}
//	    fflush(stdout);

	    if (C!=NULL)
		{
		    iterator	sit = map_find( C, token->str );
		    if (!sit.valid)
			{
		    	    map_insert( C, token->str );  // list legges til automatisk
			    sit = map_find( C, token->str );
			}

		    container	*L = map_val(sit).C;
		    list_pushback( L, selector );
		}
	}
//    printf("\nfinished\n");

    free(data);

    return selector_block;
}


yyerror( struct bcpm_intern_data *data, void *yyscan_t, char *s )
{
	#ifndef NOWARNINGS
    		printf("parse_error(css): %s\n", s);
	#endif
}


int parse_color( char *color )
{
    if (color==NULL || color[0]=='\0') return -1;

    assert(color_names_automaton!=NULL);

    int		ret = -1;

    if (color[0]=='#')
	{
	    if (strlen(color)==4)
		{
            	    int         tmp;
            	    tmp = strtol(&(color[1]), NULL, 16);
            	    ret = ((tmp&3840)*(4096+256)) + ((tmp&240)*(256+16)) + ((tmp&15)*(16+1));
		}
	    else
		ret = strtol(&(color[1]), NULL, 16);
	}
    else if (!strncmp(color, "rgb(", 4))
	{
	    int		r, g, b;
	    char	*ptr = color, *nptr;

	    while (*ptr!='\0' && (*ptr<'0' || *ptr>'9')) ptr++;
	    r = strtol(ptr, &nptr, 10);
	    ptr = nptr;
	    while (*ptr!='\0' && (*ptr<'0' || *ptr>'9')) ptr++;
	    g = strtol(ptr, &nptr, 10);
	    ptr = nptr;
	    while (*ptr!='\0' && (*ptr<'0' || *ptr>'9')) ptr++;
	    b = strtol(ptr, &nptr, 10);
	    ptr = nptr;

	    ret = ((r&255)<<16) + ((g&255)<<8) + (b&255);
	}
    else
	{
	    int		hit = search_automaton(color_names_automaton, color);

	    if (hit>=0) ret = color_values[hit];
	}

    return ret;
}

