%{
/**
 *	(C) Copyright Searchdaimon AS 2008-2009, Magnus Galåen (mg@searchdaimon.com)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dvector.h"
#include "../ds/dmap.h"

#include "../getFiletype/identify_extension.h"
#include "show_attributes.h"
#include "show_attributes_common.h"


// --- fra flex:
typedef void* yyscan_t;
typedef struct rac_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE rac_scan_bytes( const char *bytes, int len, yyscan_t yyscanner );
struct rac_yy_extra *racget_extra( yyscan_t yyscanner );
//YY_BUFFER_STATE racscan_bytes( const char *bytes, int len, yyscan_t yyscanner );

// ---



/*
void print_strings( container *V )
{
    int i;
    for (i=0; i<vector_size(V) && i<10; i++)
        {
            if (i>0) printf(" ");
            printf("%s", vector_get(V,i).ptr);
        }

    if (i<vector_size(V)) printf(" ...");
}
*/
typedef struct item item;

struct item
{
    container	*child;
    container	*hide;
    item	*parent;
    // group/select/import data:
    enum attr_item_type type;
    char	*id;
    container	*parameters;
    char	flags;
    enum attr_sort_enum	sort;
    int		max_items;
};


struct rac_yacc_data
{
    container	*S;
    item	*current_item;
    int		failed;
};

%}

%pure-parser
%parse-param { struct rac_yacc_data *data }
%parse-param { yyscan_t yyscanner }
%lex-param { yyscan_t yyscanner }
%token EXPANDED_ID GROUP_ID IMPORT_ID NAME_ID SELECT_ID SORT_ID SHOW_EMPTY_ID EQUALS_ID PARANTES_BEGIN PARANTES_CLOSE BRACKET_BEGIN BRACKET_CLOSE STRING_ID SORT_REVERSE_ID HIDE_ID SHOW_MAX_ID
//%token SHOW_DUPLICATES_ID EXPANDED_ID FROM_ID GROUP_ID IMPORT_ID NAME_ID SELECT_ID SORT_ID SHOW_EMPTY_ID EQUALS_ID PARANTES_BEGIN PARANTES_CLOSE BRACKET_BEGIN BRACKET_CLOSE STRING_ID SORT_REVERSE_ID HIDE_ID SHOW_MAX_ID

%%
doc	:
	{
//	    printf("doc\n");
	}
	| doc group
	| doc select
	| doc sort
	| doc show_max
	;
group	: GROUP_ID PARANTES_BEGIN STRING_ID PARANTES_CLOSE BRACKET_BEGIN block BRACKET_CLOSE
	{
//	    printf("group(\"%s\")\n", (char*)$3);

	    data->current_item->id = (char*)$3;
	    data->current_item = data->current_item->parent;
	}
	;
block	:
	{
//	    printf("new block\n");
//	    data->current_item = ;
	    item	*new_item;

	    new_item = malloc(sizeof(item));
	    new_item->child = vector_container( ptr_container() );
	    new_item->hide = vector_container( ptr_container() );
	    new_item->parent = data->current_item;
	    new_item->type = item_group;
	    new_item->id = NULL;
	    new_item->parameters = vector_container( string_container() );
	    new_item->flags = show_empty;
	    new_item->sort = -1;
	    new_item->max_items = data->current_item->max_items;
	    vector_pushback(data->current_item->child, new_item);

	    data->current_item = new_item;
	}
	| block group
	| block select
/*
	| block SHOW_DUPLICATES_ID EQUALS_ID STRING_ID
	{
//	    printf("show.duplicates = %s\n", (char*)$4);
//	    data->current_item->flags|= show_duplicates;
	    if (!strcasecmp((const char*)$4, "true"))
		data->current_item->flags|= show_duplicates;
	    else
		data->current_item->flags&= 0xffff - show_duplicates;
	}
*/
	| block EXPANDED_ID EQUALS_ID STRING_ID
	{
//	    printf("expanded = %s\n", (char*)$4);
//	    data->current_item->flags|= is_expanded;
	    if (!strcasecmp((const char*)$4, "true"))
		data->current_item->flags|= is_expanded;
	    else
		data->current_item->flags&= 0xffff - is_expanded;
	}
	| block IMPORT_ID PARANTES_BEGIN strings PARANTES_CLOSE
	{
	    item	*new_item;

	    new_item = malloc(sizeof(item));
	    new_item->child = NULL;
	    new_item->parent = data->current_item;
	    new_item->hide = NULL;
	    new_item->type = item_import;
//	    new_item->id = strdup(vector_get(data->S,0).ptr);
	    new_item->id = vector_get(data->S,0).ptr;
	    new_item->parameters = data->S;
	    vector_pushback(data->current_item->child, new_item);

	    data->S = vector_container( string_container() );
//	    printf("import = ");
//	    print_strings(data->S);
//	    printf("\n");
	}
	| block NAME_ID PARANTES_BEGIN STRING_ID PARANTES_CLOSE EQUALS_ID STRING_ID
	{
//	    printf("name(%s) = %s\n", (char*)$4, (char*)$7);
	    vector_pushback(data->current_item->parameters, $4);
	    vector_pushback(data->current_item->parameters, $7);
	}
	| block sort
	| block show_max
	| block SHOW_EMPTY_ID EQUALS_ID STRING_ID
	{
//	    printf("show.empty = %s\n", (char*)$4);
	    if (!strcasecmp((const char*)$4, "true"))
		data->current_item->flags|= show_empty;
	    else
		data->current_item->flags&= 0xffff - show_empty;
	}
	| block SORT_REVERSE_ID EQUALS_ID STRING_ID
	{
//	    printf("show.empty = %s\n", (char*)$4);
	    if (!strcasecmp((const char*)$4, "true"))
		data->current_item->flags|= sort_reverse;
	    else
		data->current_item->flags&= 0xffff - sort_reverse;
	}
//	| block BUILD_GROUPS_ID EQUALS_ID STRING_ID
//	{
//	    printf("show.empty = %s\n", (char*)$4);
//	    if (!strcasecmp((const char*)$4, "true"))
//		data->current_item->flags|= build_groups;
//	    else
//		data->current_item->flags&= 0xffff - build_groups;
//	}
	| block HIDE_ID PARANTES_BEGIN strings PARANTES_CLOSE
	{
	    vector_pushback(data->current_item->hide, data->S);
	    data->S = vector_container( string_container() );
	}
	;
select	: SELECT_ID PARANTES_BEGIN strings PARANTES_CLOSE
	{
	    item	*new_item;

	    new_item = malloc(sizeof(item));
	    new_item->child = NULL;
	    new_item->hide = NULL;
	    new_item->parent = data->current_item;
	    new_item->type = item_select;
	    new_item->id = NULL;
	    if (!strcmp((char*)vector_get(data->S, vector_size(data->S)-1).ptr, "*"))
		{
		    vector_remove_last(data->S);
		    new_item->flags = show_empty;
		}
	    else new_item->flags = build_groups | show_empty;

	    new_item->parameters = data->S;
	    vector_pushback(data->current_item->child, new_item);

//	    printf("select( ");
//	    print_strings(data->S);
	    data->S = vector_container( string_container() );
//	    printf(" )\n");
	}
/*
	| SELECT_ID PARANTES_BEGIN strings FROM_ID strings PARANTES_CLOSE
	{
	    item	*new_item;

	    new_item = malloc(sizeof(item));
	    new_item->child = NULL;
	    new_item->parent = data->current_item;
	    new_item->type = item_select;
	    new_item->id = NULL;
	    new_item->parameters = data->S;
	    new_item->flags = $3;
	    vector_pushback(data->current_item->child, new_item);

//	    printf("select( ");

//	    int i;
//	    for (i=0; i<vector_size(data->S); i++)
//	        {
//		    if (i==$3) printf(" FROM");
//	            if (i>0) printf(" ");
//	            printf("%s", vector_get(data->S,i).ptr);
//	        }

	    data->S = vector_container( string_container() );

//	    printf(" )\n");
	}
*/
	;
sort	: SORT_ID EQUALS_ID STRING_ID
	{
//	    printf("sort = %s\n", (char*)$3);
	    if (!strcasecmp((const char*)$3, "none"))
	        data->current_item->sort = sort_none;
	    else if (!strcasecmp((const char*)$3, "hits"))
	        data->current_item->sort = sort_hits;
	    else if (!strcasecmp((const char*)$3, "alphabetic")
//		|| !strcasecmp((const char*)$3, "ascending")
		|| !strcasecmp((const char*)$3, "alpha"))
	        data->current_item->sort = sort_alpha;
//	    else if (!strcasecmp((const char*)$3, "desc")
//		|| !strcasecmp((const char*)$3, "descending"))
//	        data->current_item->sort = sort_desc;
	    else
		{
		    bprintf(racget_extra(yyscanner)->Bwarnings, "line %i: Unknown sort parameter \"%s\", using default (sort=hits).\n", racget_extra(yyscanner)->line, (char*)$3);
		    data->current_item->sort = sort_hits;
		}
	}
	;
show_max : SHOW_MAX_ID EQUALS_ID STRING_ID
	{
	    data->current_item->max_items = strtol((const char*)$3, NULL, 10);
	    if (errno == EINVAL || errno == ERANGE) data->current_item->max_items = -1;
	}
strings	:
	| strings STRING_ID
	{
	    vector_pushback(data->S, $2);
	    $$ = vector_size(data->S);
	}
	;
%%


void print_recurse_items(item *I, int indent);
static container* recurse_items(item *I, int sort_inherit);
static void recursive_delete(struct attr_group *a);


// Returnerer warnings.
attr_conf* show_attributes_init(char *text, char **warnings, int *failed)
{
	*failed = 0;

    struct rac_yy_extra		*re = malloc(sizeof(struct rac_yy_extra));
    struct rac_yacc_data	*data = malloc(sizeof(struct rac_yacc_data));
    data->failed = 0;

    re->ptr = 0;
    re->last_ptr = 0;
    re->line = 1;
    re->Bwarnings = buffer_init(16384);

    data->S = vector_container( string_container() );
    data->current_item = malloc(sizeof(item));
    data->current_item->id = NULL;
    data->current_item->type = item_group;
    data->current_item->flags = show_empty;
    data->current_item->sort = sort_none;
    data->current_item->max_items = -1;
    data->current_item->child = vector_container( ptr_container() );
    data->current_item->hide = vector_container( ptr_container() );
    data->current_item->parameters = vector_container( string_container() );
    data->current_item->parent = NULL;

    yyscan_t		scanner;

    raclex_init(&scanner);
    racset_extra(re, scanner);
    //racset_in(fyyin, scanner);
    YY_BUFFER_STATE bs = rac_scan_bytes(text, strlen(text), scanner);


    racparse(data, scanner);


    rac_delete_buffer(bs, scanner);
    raclex_destroy(scanner);

    //fclose(fyyin);

    print_recurse_items(data->current_item, 0);

    attr_conf	*ac = malloc(sizeof(attr_conf));
    ac->sort = data->current_item->sort;
    ac->max_items = data->current_item->max_items;
    ac->flags = data->current_item->flags;
    ac->child = recurse_items(data->current_item, sort_hits);
    ac->hide = data->current_item->hide;
    ac->name = strdup("<root>");
    ac->alt_names = NULL;

    *warnings = buffer_exit(re->Bwarnings);

    free(re);
//    destroy(data->current_item->child);
    destroy(data->S);
    destroy(data->current_item->parameters);
    *failed = data->failed;
    free(data->current_item);
    free(data);

    return ac;
}


void show_attributes_destroy( attr_conf *ac )
{
    recursive_delete((struct attr_group*)ac);
}

static void recursive_delete(struct attr_group *a)
{
    int		i, j;

    free(a->name);
    destroy(a->alt_names);

    for (i=0; i<vector_size(a->hide); i++)
	{
	    destroy( vector_get(a->hide, i).ptr );
	}
    destroy(a->hide);

    for (i=0; i<vector_size(a->child); i++)
	{
	    switch (pair(vector_get(a->child,i)).first.i)
	        {
		    case item_select:
		        {
			    struct attr_select	*S = pair(vector_get(a->child,i)).second.ptr;
			    for (j=0; j<S->size; j++)
				free(S->select[j]);
			    free(S->select);
			    free(S);
			    break;
			}
		    case item_import:
			{
			    struct attr_import	*I = pair(vector_get(a->child,i)).second.ptr;
			    for (j=0; j<I->size; j++)
				free(I->import[j]);
			    free(I->import);
			    free(I);
			    break;
			}
		    case item_group:
			{
			    struct attr_group	*G = pair(vector_get(a->child,i)).second.ptr;
			    recursive_delete(G);
			    break;
			}
		}
	}

    destroy(a->child);
    free(a);
}


static container* recurse_items(item *parent, int sort_inherit)
{
    if (parent->child == NULL) return NULL;

    int		i, j;
    container	*ret = vector_container( pair_container( int_container(), ptr_container() ) );

    for (i=0; i<vector_size(parent->child); i++)
        {
	    item	*I = vector_get(parent->child, i).ptr;

	    if (I->type == item_select)
		{
		    struct attr_select	*S = malloc(sizeof(struct attr_select));

		    S->size = vector_size(I->parameters);
		    S->flags = I->flags;
		    S->select = malloc(sizeof(char*) * S->size);

		    for (j=0; j<S->size; j++)
			S->select[j] = strdup(vector_get(I->parameters,j).ptr);
/*
		    S->from = malloc(sizeof(char) * (vector_size(I->parameters) - I->flags));

		    for (; j<vector_size(I->parameters); j++)
			S->from[j-I->flags] = strdup(vector_get(I->parameters,j).ptr);
*/
		    vector_pushback(ret, item_select, S);
		}
	    else if (I->type == item_import)
		{
		    struct attr_import	*V = malloc(sizeof(struct attr_import));

		    V->size = vector_size(I->parameters);
		    V->import = malloc(sizeof(char*) * V->size);

		    for (j=0; j<V->size; j++)
			V->import[j] = strdup(vector_get(I->parameters,j).ptr);

		    vector_pushback(ret, item_import, V);
		}
	    else if (I->type == item_group)
		{
		    struct attr_group	*G = malloc(sizeof(struct attr_group));

		    G->name = strdup(I->id);
		    G->flags = I->flags;
		    G->sort = (I->sort == -1 ? sort_inherit : I->sort);
		    G->max_items = I->max_items;
		    G->hide = I->hide;

		    G->alt_names = map_container( string_container(), string_container() );
		    for (j=0; j<vector_size(I->parameters); j+=2)
			{
			    map_insert(G->alt_names, vector_get(I->parameters,j).ptr, vector_get(I->parameters,j+1).ptr);
			}

		    G->child = recurse_items(I, G->sort);

		    vector_pushback(ret, item_group, G);
		}

	    destroy(I->parameters);
	    free(I);
	}

    destroy(parent->child);

    return ret;
}


void print_recurse_items(item *I, int indent)
{
    int		i, j;

    if (I->type == item_group) printf("+ group");
    else if (I->type == item_select) printf("select");
    else if (I->type == item_import) printf("import");
    if (I->id != NULL) printf("(%s)", I->id);

    if (I->type == item_select)
	{
//	    for (i=0; i<I->flags && i<vector_size(I->parameters); i++)
//		printf(" %s", vector_get(I->parameters, i).ptr);

//	    if (I->flags == 0) printf(" *");

//	    printf(" FROM");

	    for (i=0; i<vector_size(I->parameters); i++)
		{
		    if (i==0) printf(" ");
		    else printf("/");
		    printf("%s", vector_get(I->parameters, i).ptr);
		}
	}
    else if (I->type == item_import)
	{
	    for (i=0; i<vector_size(I->parameters); i++)
		printf(" %s", vector_get(I->parameters, i).ptr);
	}

    printf("\n");

    if (I->type == item_group)
	{
	    for (i=0; i<vector_size(I->parameters); i+=2)
		{
		    for (j=0; j<indent*3+2; j++)
			if (j%3==2) printf("|");
			else printf(" ");

    	    	    printf("name(%s): %s\n", vector_get(I->parameters, i).ptr, vector_get(I->parameters, i+1).ptr);
		}

	    for (j=0; j<indent*3+2; j++)
		if (j%3==2) printf("|");
		else printf(" ");
	    printf("sort:");
	    switch (I->sort)
		{
		    case sort_none: printf("none"); break;
		    case sort_hits: printf("hits"); break;
		    case sort_alpha: printf("alphabetic"); break;
		}
	    if (I->flags & sort_reverse) printf(" (reverse)");
	    if (I->flags & build_groups) printf(" (build groups)");
	    printf("\n");

	    if (I->flags & is_expanded)
		{
		    for (j=0; j<indent*3+2; j++)
			if (j%3==2) printf("|");
			else printf(" ");
		    printf("expanded\n");
		}

	    if (I->flags & show_empty)
		{
		    for (j=0; j<indent*3+2; j++)
			if (j%3==2) printf("|");
			else printf(" ");
		    printf("show.empty\n");
		}
/*
	    if (I->flags & show_duplicates)
		{
		    for (j=0; j<indent*3+2; j++)
			if (j%3==2) printf("|");
			else printf(" ");
		    printf("show.duplicates\n");
		}
*/
	    if (I->max_items > 0)
		{
		    for (j=0; j<indent*3+2; j++)
			if (j%3==2) printf("|");
			else printf(" ");
		    printf("show.max = %i\n", I->max_items);
		}
	}

    if (I->hide != NULL)
	{
	    for (i=0; i<vector_size(I->hide); i++)
		{
		    for (j=0; j<indent*3+2; j++)
			if (j%3==2) printf("|");
			else printf(" ");
		    printf("hide");
		    container	*hide = vector_get(I->hide, i).ptr;
		    for (j=0; j<vector_size(hide); j++)
			{
			    if (j==0) printf(" ");
			    else printf("/");
			    printf("%s", vector_get(hide, j).ptr);
			}
		    printf("\n");
		}
	}

    if (I->child == NULL) return;

    for (i=0; i<vector_size(I->child); i++)
        {
	    for (j=0; j<indent*3+2; j++)
		if (j%3==2) printf("|");
		else printf(" ");
	    print_recurse_items(vector_get(I->child, i).ptr, indent+1);
	}
}


racerror( struct rac_yacc_data *data, void *yyscan_t, char *s )
{
    fprintf(stderr, "rac: Parse error (%s)\n", s);
    data->failed = 1;
}

