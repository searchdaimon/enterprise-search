
#include <stdio.h>
#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dvector.h"
#include "../ds/dmap.h"
#include "../ds/dset.h"
#include "../ds/dmultimap.h"
#include "../ds/dtuple.h"
#include "../common/bprint.h"
#include "../common/xml.h"
#include "../query/query_parser.h"
#include "attr_makexml.h"


/*
int query_remove_word(query_array *qa, char operand, char *_word)
{
    int		i;
    char	*word = strchr(_word, ':');

    if (word==NULL) word = _word;
    else word++;
    if (word[0]=='\"') word++;
    if (word[strlen(word)-1]=='\"') word[strlen(word)-1] = '\0';

    printf("TEST %c: %s\n", operand, word);

    for (i=0; i<qa->n; i++)
	{
	    if (qa->query[i].operand == operand && qa->query[i].n == 1)
		{
		    if (!strcasecmp(qa->query[i].s[0], word))
			{
			    printf("REMOVE %i (%s ...)\n", i, qa->query[i].s[0]);
			    return i;
			}
		}
	}

    return -1;
}
*/
/*
inline int add_to_query(buffer *B, query_array *qa, char operand, char *word, int *split)
{
    int		i;
    int		exists = -1;

    //printf("TEST %c: %s\n", operand, word);

    for (i=0; i<qa->n; i++)
	{
	    if (qa->query[i].operand == operand && qa->query[i].n == 1)
		{
		    if (!strcasecmp(qa->query[i].s[0], word))
			{
			    //printf("REMOVE %i (%s ...)\n", i, qa->query[i].s[0]);
			    exists = i;
			    break;
			}
		}
	}

    bprintf(B, " query=\'");
    *split = B->pos;
    //bsprint_query_with_remove(B, NULL, qa);

    //if (exists == -1)
	{
	    switch (operand)
		{
		    case QUERY_GROUP: bprintf(B, " group:\""); break;
		    case QUERY_FILETYPE: bprintf(B, " filetype:\""); break;
		    case QUERY_ATTRIBUTE: bprintf(B, " attribute:\""); break;
		    default: fprintf(stderr, "attr_makexml: Warning! Undefined operand in 'add_to_query'.\n"); break;
		}
	    bprintf(B, "%s\"", word);
	}

    bprintf(B, "\'");

    if (exists >= 0) bprintf(B, " selected=\"true\"");

    return exists;
}

/*
int add_to_query(buffer *B, query_array *qa, char operand, char *word)
{
    int		i;
    int		exists = -1;

//    printf("TEST %c: %s\n", operand, word);

    for (i=0; i<qa->n; i++)
	{
	    if (qa->query[i].operand == operand && qa->query[i].n == 1)
		{
		    if (!strcasecmp(qa->query[i].s[0], word))
			{
//			    printf("REMOVE %i (%s ...)\n", i, qa->query[i].s[0]);
			    exists = i;
			    break;
			}
		}
	}

    bprintf(B, " query=\'");
    bsprint_query_with_remove(B, NULL, qa);

    if (exists == -1)
	{
	    switch (operand)
		{
		    case QUERY_GROUP: bprintf(B, " group:\""); break;
		    case QUERY_FILETYPE: bprintf(B, " filetype:\""); break;
		    case QUERY_ATTRIBUTE: bprintf(B, " attribute:\""); break;
		    default: fprintf(stderr, "attr_makexml: Warning! Undefined operand in 'add_to_query'.\n"); return -1;
		}
	    bprintf(B, "%s\"", word);
	}

    bprintf(B, "\'");

    if (exists >= 0) bprintf(B, " selected=\"true\"");

    return exists;
}
*/

/*
int query_remove_phrase(query_array *qa, char operand, container *V)
{
    int		i, j;

    printf("TEST %c: ", operand);
    println(V);

    for (i=0; i<qa->n; i++)
	{
	    if (qa->query[i].operand == operand)
		{
		    char	match = 1;
		    for (j=0; j<qa->query[i].n && match; j++)
			if (strcasecmp(qa->query[i].s[j], (char*)vector_get(V,j).ptr))
			    match = 0;

		    if (match) printf("REMOVE %i (%s ...)\n", i, qa->query[i].s[0]);
		    if (match) return i;
		}
	}

    return -1;
}
*/
/*
char* attribute_generate_value_string(char *prefix, char *param[], int size)
{
    buffer	*B = buffer_init(-1);
    int		i;

    if (size > 0) bprintf(B, "%s\"%s", prefix, param[0]);
    for (i=1; i<size; i++) bprintf(B, "/%s", param[i]);
    if (size > 0) bprintf(B, "\"");

    return buffer_exit(B);
}

char* attribute_generate_value(char *param[], int size)
{
    buffer	*B = buffer_init(-1);
    int		i;

    if (size > 0) bprintf(B, "%s", param[0]);
    for (i=1; i<size; i++) bprintf(B, "/%s", param[i]);

    return buffer_exit(B);
}

char* attribute_generate_key_value_string_from_vector(char *prefix, container *param)
{
    buffer	*B = buffer_init(-1);
    int		i;

    if (vector_size(param) > 0) bprintf(B, "%s\"%s", prefix, vector_get(param,0).ptr);
    if (vector_size(param) > 1) bprintf(B, "=%s", vector_get(param,1).ptr);
    for (i=2; i<vector_size(param); i++) bprintf(B, "/%s", vector_get(param,i).ptr);
    if (vector_size(param) > 0) bprintf(B, "\"");

    return buffer_exit(B);
}

char* attribute_generate_key_value_string_from_vector2(container *param)
{
    buffer	*B = buffer_init(-1);
    int		i;

    if (vector_size(param) > 0) bprintf(B, "%s", vector_get(param,0).ptr);
    if (vector_size(param) > 1) bprintf(B, "=%s", vector_get(param,1).ptr);
    for (i=2; i<vector_size(param); i++) bprintf(B, "/%s", vector_get(param,i).ptr);

    return buffer_exit(B);
}

char* attribute_generate_value_string_from_vector(char *prefix, container *param)
{
    buffer	*B = buffer_init(-1);
    int		i;

    if (vector_size(param) > 1) bprintf(B, "%s\"%s", prefix, vector_get(param,1).ptr);
    for (i=2; i<vector_size(param); i++) bprintf(B, "/%s", vector_get(param,i).ptr);
    if (vector_size(param) > 1) bprintf(B, "\"");

    return buffer_exit(B);
}

char* attribute_generate_value_from_vector(container *param)
{
    buffer	*B = buffer_init(-1);
    int		i;

    if (vector_size(param) > 1) bprintf(B, "%s", vector_get(param,1).ptr);
    for (i=2; i<vector_size(param); i++) bprintf(B, "/%s", vector_get(param,i).ptr);

    return buffer_exit(B);
}
*/
int	ant[10];

void attribute_init_count()
{
    int		i;
    for (i=0; i<10; i++) ant[i] = 0;
}

void attribute_finish_count()
{
    int		i;
    for (i=0; i<10; i++)
	if (ant[i]>0) {
		#ifdef DEBUG
		    printf("\t%i:%i", i, ant[i]);
		#endif
	}
    #ifdef DEBUG
    	printf("\n");
    #endif
}


va_list va_attribute_count_add( int size, int count, container *attributes, int argc, va_list ap )
{
    ant[0]++;
    int		i;
    char	*id = va_arg(ap, char*);
    container	*subattr;

    iterator	it = map_find(attributes, id);
    if (it.valid)
	{
	    ant[1]++;
	    subattr = pair(map_val(it)).first.ptr;

	    container	*M = pair(map_val(it)).second.ptr;
	    it = map_find(M, count);
	    if (it.valid)
		{
		    ant[2]++;
		    map_val(it).i+= size;
		}
	    else
		{
		    ant[3]++;
		    map_insert(M, count, size);
		}
	    //for (i=0; i<len; i++)
		//((int*)pair(map_val(it)).second.ptr)[i]+= count[i];
	}
    else
	{
	    ant[4]++;
	    subattr = map_container( string_container(), pair_container( ptr_container(), ptr_container() ) );

	    container	*M = map_container( int_container(), int_container() );
	    map_insert(M, count, size);
	    //int		*C = malloc(sizeof(int)*len);
	    //for (i=0; i<len; i++) C[i] = count[i];
	    map_insert(attributes, id, subattr, M);
	}

    if (argc > 1) ap = va_attribute_count_add( size, count, subattr, argc-1, ap );

    return ap;
}

void attribute_count_add( int size, int count, container *attributes, int argc, ... )
{
    ant[5]++;
    va_list		ap;

    va_start(ap, argc);
    ap = va_attribute_count_add( size, count, attributes, argc, ap );
    va_end(ap);
}

void attribute_destroy_recursive( container *attributes )
{
    iterator	it = map_begin(attributes);

    for (; it.valid; it=map_next(it))
	{
	    destroy(pair(map_val(it)).second.C);
	    attribute_destroy_recursive( pair(map_val(it)).first.C );
	}

    destroy(attributes);
}

/*
struct attribute_temp
{
};

struct attribute_hash attribute_new()
{
}

void attribute_count_add( int count, container *attributes, int argc, ... )
void attribute_add( struct attribute_temp *temp, int count, char *arg1, char *arg2, char *arg3 )
{
}

container* attribute_build( )
{
}
*/

void attribute_count_print( container *attributes, int attrib_count, int indent )
{
    iterator		it = map_begin(attributes);

    for (; it.valid; it=map_next(it))
	{
	    int		i;

	    for (i=0; i<indent; i++) printf(" ");
	    printf("%s\n", (char*)map_key(it).ptr);
	    /*
	    for (i=0; i<attrib_count; i++)
		{
		    if (i>0) printf(", ");
		    printf("%i", ((int*)pair(map_val(it)).second.ptr)[i]);
		}
	    printf(")\n");
	    */

	    attribute_count_print(pair(map_val(it)).first.C, attrib_count, indent+2);
	}
}

/*
int attrib_count_hits(query_array *qa, container *count, int len, container *remove, int is_file_ext)
{
    if (count==NULL) return 0;

    int		i, j;
    int		sum = 0;
    iterator	it;
    int		filter = 0;

    if (remove==NULL) it.valid = 0;
    else it = set_begin(remove);

    if (it.valid && set_key(it).i == -1)
	it = set_next(it);

    for (i=0,j=0; i<qa->n; i++)
	{
	    printf("\"%s\"", qa->query[i].s[0]);
	    if (it.valid && set_key(it).i == i)
		{
		    it = set_next(it);
		    if (qa->query[i].operand == QUERY_ATTRIBUTE) {filter+= 1<<j;printf("(f:attr)");}
		    if (qa->query[i].operand == QUERY_GROUP
			|| qa->query[i].operand == QUERY_FILETYPE) {filter+= 1<<(len-1);printf("(f:group)");}
		}
	    else if (is_file_ext && (qa->query[i].operand == QUERY_FILETYPE
		|| qa->query[i].operand == QUERY_GROUP)) {filter+= 1<<(len-1);printf("(f:group)");}

	    if (qa->query[i].operand == QUERY_ATTRIBUTE) {j++;printf("(j++)");}
	}

    int		alle = 0;
    for (i=0; i<len; i++) alle+= 1<<i;

    filter = alle - filter;
    printf("filter:");
    for (i=0; i<len; i++) printf("%c", (filter & (1<<i) ? '1':'0'));

    int		hits = 0;
    int		__total__ = 0;
    it = map_begin(count);
    for (; it.valid; it=map_next(it))
	{
	    __total__++;
	    if ((map_key(it).i & filter) == filter)
		hits+= map_val(it).i;
	}
    printf("\tmatch:%i/%i\n", hits, __total__);

    return hits;
}

int attribute_is_empty_remove( container *remove )
{
    if (remove==NULL || set_size(remove)==0) return 1;

    if (set_key(set_begin(remove)).i==-1 && set_size(remove)==1) return 1;

    return 0;
}


#define BINDENT	{ int x; for (x=0; x<indent; x++) bprintf(B, " "); }
struct attribute_temp_res
{
    int		hits;
    container	*items, *remove;
};

int attribute_print_xml_sorted(container *items, int indent, buffer *B, int sort, char sort_reverse, query_array *qa, container *remove, int attrib_count)
{
    int		j;
    container	*M = NULL;
    char	reverse = sort_reverse;
    int		is_file_ext = 0;
    int		printed = 0;

    switch (sort)
	{
	    case sort_none:
		{
		    M = multimap_container(int_container(), tuple_container(4, string_container(), int_container(), int_container(), int_container() ) );
		    for (j=0; j<vector_size(items); j++)
			{
			    printf("  * %s ", (char*)tuple(vector_get(items, j)).element[1].ptr);
			    if (tuple(vector_get(items, j)).element[5].i) is_file_ext = 1;
			multimap_insert(M, 0, (char*)tuple(vector_get(items, j)).element[0].ptr,
			    tuple(vector_get(items, j)).element[3].i,
			    attrib_count_hits(qa, tuple(vector_get(items, j)).element[2].ptr, attrib_count, remove, tuple(vector_get(items, j)).element[5].i),
			    tuple(vector_get(items, j)).element[4].i);
			}

		    break;
		}
	    case sort_hits:
		{
		    M = multimap_container(int_container(), tuple_container(4, string_container(), int_container(), int_container(), int_container() ) );
		    for (j=0; j<vector_size(items); j++)
			{
			    printf("  * %s ", (char*)tuple(vector_get(items, j)).element[1].ptr);
			    int		hits = attrib_count_hits(qa, tuple(vector_get(items, j)).element[2].ptr, attrib_count, remove, tuple(vector_get(items, j)).element[5].i);
			    if (tuple(vector_get(items, j)).element[5].i) is_file_ext = 1;

			multimap_insert(M, hits, //tuple(vector_get(items, j)).element[2].i,
			    (char*)tuple(vector_get(items, j)).element[0].ptr,
			    tuple(vector_get(items, j)).element[3].i,
			    hits,
			    tuple(vector_get(items, j)).element[4].i);
			}

		    reverse = (reverse ? 0 : 1);
		    break;
		}
	    case sort_alpha:
		{
		    M = multimap_container(string_container(), tuple_container(4, string_container(), int_container(), int_container(), int_container() ) );
		    for (j=0; j<vector_size(items); j++)
			{
			    printf("  * %s ", (char*)tuple(vector_get(items, j)).element[1].ptr);
			    int		hits = attrib_count_hits(qa, tuple(vector_get(items, j)).element[2].ptr, attrib_count, remove, tuple(vector_get(items, j)).element[5].i);
			    if (tuple(vector_get(items, j)).element[5].i) is_file_ext = 1;

			multimap_insert(M, (char*)tuple(vector_get(items, j)).element[1].ptr,
			    (char*)tuple(vector_get(items, j)).element[0].ptr,
			    tuple(vector_get(items, j)).element[3].i,
			    hits,
			    tuple(vector_get(items, j)).element[4].i);
			}

		    break;
		}
	}

//    BINDENT;
//    bprintf(B, "<!-- <item name=\"alle\" query=\'");
//    bprintf(B, "<!--    QUERY=\'");
//    bsprint_query_with_remove(B, remove, qa);
//    bprintf(B, "\'    /> -->\n");

    if (is_file_ext)
	{
	    int	i;

	    for (i=0; i<qa->n; i++)
		if (qa->query[i].operand == QUERY_GROUP
		    || qa->query[i].operand == QUERY_FILETYPE
		    || qa->query[i].operand == QUERY_ATTRIBUTE) set_insert(remove, i);
	}

    buffer	*B2 = buffer_init(-1);
    bsprint_query_with_remove(B2, remove, qa);
/*
    if (!bsprint_query_with_remove(B2, remove, qa))
	{
	    printf("FALSEFALSEFALSE\n");
	    bprintf(B2, " XXXXX ");
	    //if (remove==NULL) break;
	    iterator	it_r = set_begin(remove);
	    if (it_r.valid && set_key(it_r).i==-1) it_r = set_next(it_r);
	    if (it_r.valid)
		{
		    it_r = set_next(it_r);
		    container	*new_remove = set_container( int_container() );
		    for (; it_r.valid; it_r=set_next(it_r))
			set_insert(new_remove, set_key(it_r).i);

		    buffer_abort(B2);
		    B2 = buffer_init(-1);
		    bprintf(B2, "  <FANCYQUERY>  ");
		    bsprint_query_with_remove(B2, new_remove, qa);
		    bprintf(B2, "  </FANCYQUERY>  ");

		    destroy(new_remove);
		}
	    else bprintf(B2, "  W0000t???  ");
	}
    else
	printf("TRUETRUETRUE\n");
*-
    char	*q = buffer_exit(B2);

    iterator	it_m2;
    if (!reverse)
	{
	    it_m2 = multimap_begin(M);
	    for (; it_m2.valid; it_m2=multimap_next(it_m2))
		{
		    char	*str = (char*)tuple(multimap_val(it_m2)).element[0].ptr;
		    int		split = tuple(multimap_val(it_m2)).element[1].i;
		    int		hits = tuple(multimap_val(it_m2)).element[2].i;
		    int		hit_split = tuple(multimap_val(it_m2)).element[3].i;
		    int		pos = 0;

		    if (hit_split>=0 && hits==0) continue;

		    if (split >= 0)
			{
			    bprintf(B, "%.*s", split, str);
			    bprintf(B, "%s", q);
			    pos = split;
			}

		    if (hit_split >= 0)
			{
			    bprintf(B, "%.*s", hit_split-pos, &(str[pos]));
			    bprintf(B, "%i", hits);
			    pos = hit_split;
			}

		    bprintf(B, "%s", &(str[pos]));
		    printed++;
		}
	}
    else
    	{
	    it_m2 = multimap_end(M);
	    for (; it_m2.valid; it_m2=multimap_previous(it_m2))
		{
		    char	*str = (char*)tuple(multimap_val(it_m2)).element[0].ptr;
		    int		split = tuple(multimap_val(it_m2)).element[1].i;
		    int		hits = tuple(multimap_val(it_m2)).element[2].i;
		    int		hit_split = tuple(multimap_val(it_m2)).element[3].i;
		    int		pos = 0;

		    if (hit_split>=0 && hits==0) continue;

		    if (split >= 0)
			{
			    bprintf(B, "%.*s", split, str);
			    bprintf(B, "%s", q);
			    pos = split;
			}

		    if (hit_split >= 0)
			{
			    bprintf(B, "%.*s", hit_split-pos, &(str[pos]));
			    bprintf(B, "%i", hits);
			    pos = hit_split;
			}

		    bprintf(B, "%s", &(str[pos]));
/*
		    char	*str = (char*)pair(multimap_val(it_m2)).first.ptr;
		    int		split = pair(multimap_val(it_m2)).second.i;

		    if (split >= 0)
			{
			    bprintf(B, "%.*s", split, str);
			    bprintf(B, "%s", q);
			    bprintf(B, "%s", &(str[split]));
			}
		    else
			{
			    bprintf(B, "%s", str);
			}
*--
		    printed++;
		}
	}

    free(q);
    destroy(M);
    return printed;
}
*/
static inline int attribute_description(struct adf_data *attrdescrp, char *key, char *value, char **description, char **icon)
{
    int		success = 0;
    if (key==NULL || key[0]=='\0') return 0;

    if (value==NULL || value[0]=='\0')
	{
	    success = adf_get_key_descr(attrdescrp, "nbo", key, description, icon);
	}
    else
	{
	    success = adf_get_val_descr(attrdescrp, "nbo", key, value, description, icon);
	}

    return success;
}
/*
struct attribute_temp_res attribute_generate_xml_subpattern_recurse(container *attributes, int attrib_count,
		    struct fte_data *getfiletypep, struct adf_data *attrdescrp, int indent, int sort, char sort_reverse,
		    container *attr_query, query_array *qa, container *hide)
{
    printf("      children\n");
    int		i;
    iterator	it_m1 = map_begin(attributes);
    struct attribute_temp_res	R;

    R.hits = 0;
    R.items = vector_container( tuple_container(6, ptr_container(), string_container(), ptr_container(), int_container(), int_container(), int_container()) );
    R.remove = set_container( int_container() );

    for (; it_m1.valid; it_m1=map_next(it_m1))
	{
	    int		is_file_ext = 0;
	    buffer	*B = buffer_init(-1);
	    int		split = -1;
	    int		hit_split = -1;
	    vector_pushback(attr_query, (char*)map_key(it_m1).ptr);
	    printf("        %s", (char*)map_key(it_m1).ptr);
	    int		is_hidden = 0;
	    for (i=0; i<vector_size(hide) && !is_hidden; i++)
		{
		    int		j;
		    container	*hide_elem = vector_get(hide, i).C;

		    for (j=0; j<vector_size(hide_elem); j++)
			{
			    if (strcasecmp(vector_get(attr_query,j).ptr, vector_get(hide_elem,j).ptr)) break;
			}

		    if (j>=vector_size(hide_elem)) is_hidden = 1;
		}

	    if (is_hidden)
		{
		    printf(" (hidden)\n");
		    vector_remove_last(attr_query);
		    continue;
		}

	    if (map_size(pair(map_val(it_m1)).first.ptr) > 0)
		{
		    printf(" [more children]\n");
		    struct attribute_temp_res	res = attribute_generate_xml_subpattern_recurse(pair(map_val(it_m1)).first.ptr, attrib_count, getfiletypep, attrdescrp, indent+2, sort, sort_reverse, attr_query, qa, hide);
		    int		is_selected = 0;
		    int		expanded = 0;
		    char	*icon = NULL;

		    BINDENT; bprintf(B, "<group key=\"%s\"", vector_get(attr_query,0).ptr);
		    char	*value = attribute_generate_value_from_vector(attr_query);
		    //char	*t = attribute_generate_value_string_from_vector(" value=", attr_query);
		    if (strlen(value)>0) bprintf(B, " value=\"%s\"", value);
		    // Spesialtilfelle for (file)group:
		    if (!strcmp("group", vector_get(attr_query,0).ptr)
			&& (!strcasecmp(fte_getdefaultgroup(getfiletypep, "nbo", &icon), (char*)map_key(it_m1).ptr)
			    || fte_groupid(getfiletypep, "nbo", (char*)map_key(it_m1).ptr, &icon) >= 0))
			{
//			    bprintf(B, " query=\'%s group:\"%s\"\'", querystr, (char*)map_key(it_m1).ptr);
//			    set_insert(R.remove, query_remove_word(qa, QUERY_GROUP, (char*)map_key(it_m1).ptr));
//			    bprintf(B, " query=\'");
			    //set_insert(R.remove, add_to_query(B, qa, QUERY_GROUP, (char*)map_key(it_m1).ptr) );
			    set_insert(R.remove, is_selected=add_to_query(B, qa, QUERY_GROUP, (char*)map_key(it_m1).ptr, &split) );
//			    bprintf(B, "\'");
			}
		    else
			{
//			    char	*s = attribute_generate_key_value_string_from_vector(" attribute:", attr_query);
//			    bprintf(B, " query=\'%s%s\'", querystr, s);
//			    set_insert(R.remove, query_remove_word(qa, QUERY_ATTRIBUTE, s));
			    char	*s = attribute_generate_key_value_string_from_vector2(attr_query);
//			    bprintf(B, " query=\'");
			    set_insert(R.remove, is_selected=add_to_query(B, qa, QUERY_ATTRIBUTE, s, &split) );
//			    bprintf(B, "\'");
			}

		    //bprintf(B, " name=\"%s\"", (char*)map_key(it_m1).ptr
		    // Attribute descriptions overskriver fileext descriptions.
		    char	*description;
		    if (attribute_description(attrdescrp, vector_get(attr_query,0).ptr, value, &description, &icon))
			bprintf(B, " name=\"%s\"", description);
		    else
			bprintf(B, " name=\"%s\"", (char*)map_key(it_m1).ptr);
		    //free(t);
		    free(value);

		    if (icon==NULL) icon = getfiletypep->default_icon;
		    bprintf(B, " icon=\"%s\"", icon);
		    bprintf(B, " hits=\"");
		    hit_split = B->pos;

		    int	child_selected = !attribute_is_empty_remove(res.remove);

		    //bprintf(B, "\" expanded=\"%s\">\n", expanded ? "true" : "false");
		    set_insert(res.remove, is_selected);
		    buffer	*B2 = buffer_init(-1);
		    int	printed = attribute_print_xml_sorted(res.items, indent+2, B2, sort, sort_reverse, qa, res.remove, attrib_count);

		    //if ((is_selected>=0 && printed>1) || child_selected)
		    if (child_selected)
			{
			    expanded = 1;
			}

		    bprintf(B, "\" expanded=\"%s\">\n", expanded ? "true" : "false");
		    char	*u = buffer_exit(B2);
		    bprintf(B, "%s", u);
		    free(u);

		    iterator	it_sr = set_begin(res.remove);
		    for (; it_sr.valid; it_sr=set_next(it_sr))
			set_insert(R.remove, set_key(it_sr).i);

		    for (i=0; i<vector_size(res.items); i++) free(tuple(vector_get(res.items,i)).element[0].ptr);

		    destroy(res.items);
		    destroy(res.remove);
		    BINDENT; bprintf(B, "</group>\n");
		    printf("        [/more children]\n");
		}
	    else
		{
		    printf(" (no children)");
		    // Spesialtilfelle for 'filetype':
		    if ((vector_size(attr_query)==3 && !strcmp("group", vector_get(attr_query,0).ptr))
			|| !strcmp("filetype", vector_get(attr_query,0).ptr))
			{
			    printf(" (filetype)\n");
			    char	*group, *descr, *icon;
			    fte_getdescription(getfiletypep, "nbo", (char*)map_key(it_m1).ptr, &group, &descr, &icon);

			    is_file_ext = 1;
			    BINDENT; bprintf(B, "<item key=\"filetype\"");
			    bprintf(B, " value=\"%s\"", (char*)map_key(it_m1).ptr);
			    bprintf(B, " name=\"%s\"", descr);
			    bprintf(B, " icon=\"%s\"", icon);
//			    bprintf(B, " query=\'%s filetype:\"%s\"\'", querystr, (char*)map_key(it_m1).ptr);
//			    set_insert(R.remove, query_remove_word(qa, QUERY_FILETYPE, (char*)map_key(it_m1).ptr));
//			    bprintf(B, " query=\'");
			    set_insert(R.remove, add_to_query(B, qa, QUERY_FILETYPE, (char*)map_key(it_m1).ptr, &split) );
//			    bprintf(B, "\'");
			    //bprintf(B, " hits=\"%i\" />\n", pair(map_val(it_m1)).second.i);
			    //container	*remove = set_container( int_container() );
			    /*
			    for (i=0; i<qa->n; i++)
				if (qa->query[i].operand == QUERY_FILETYPE
				    || qa->query[i].operand == QUERY_GROUP) set_insert(R.remove, i);
			    bprintf(B, " hits=\"%i\" />\n",
				attrib_count_hits(qa, pair(map_val(it_m1)).second.ptr, attrib_count, R.remove));
			    *--
			    //destroy(remove);
			    //hit_split = B->pos;
			    //bprintf(B, "\" />\n");
			}
		    else
			{
			    printf(" (attribute)\n");
			    //item.key = strdup((char*)vector_get(attr_query,0).ptr);
			    //item.value = attribute_generate_value_string_from_vector("", attr_query);
			    //item.name = strdup((char*)map_key(it_m1).ptr);
			    //item.query = ;
			    char	*description;
			    char	*icon = NULL;

			    BINDENT; bprintf(B, "<item key=\"%s\"", vector_get(attr_query,0).ptr);
			    char	*value = attribute_generate_value_from_vector(attr_query);
			    if (strlen(value)>0) bprintf(B, " value=\"%s\"", value);
//			    bprintf(B, " name=\"%s\"", (char*)map_key(it_m1).ptr);
			    if (attribute_description(attrdescrp, vector_get(attr_query,0).ptr, value, &description, &icon))
				bprintf(B, " name=\"%s\"", description);
			    else
				bprintf(B, " name=\"%s\"", (char*)map_key(it_m1).ptr);

			    free(value);
			    if (icon!=NULL) bprintf(B, " icon=\"%s\"", icon);
//			    char	*s = attribute_generate_key_value_string_from_vector(" attribute:", attr_query);
//			    bprintf(B, " query=\'%s%s\'", querystr, s);
//			    set_insert(R.remove, query_remove_word(qa, QUERY_ATTRIBUTE, s));
			    char	*s = attribute_generate_key_value_string_from_vector2(attr_query);
//			    bprintf(B, " query=\'");
			    set_insert(R.remove, add_to_query(B, qa, QUERY_ATTRIBUTE, s, &split) );
			    free(s);
//			    bprintf(B, "\'");
			    //bprintf(B, " hits=\"%i\" />\n", pair(map_val(it_m1)).second.i);
			}

		    bprintf(B, " hits=\"");
		    hit_split = B->pos;
		    bprintf(B, "\" />\n");
		}

	    vector_remove_last(attr_query);
	    vector_pushback(R.items, buffer_exit(B), (char*)map_key(it_m1).ptr, pair(map_val(it_m1)).second.ptr, split, hit_split, is_file_ext);
	}

    return R;
}


struct attribute_temp_res attribute_generate_xml_recurse(container *attributes, int attrib_count, struct attr_group *parent, struct fte_data *getfiletypep, struct adf_data *attrdescrp, int indent, query_array *qa)
{
    int		i, j;
    struct attribute_temp_res	R;

    R.hits = 0;
    R.items = vector_container( tuple_container(6, ptr_container(), string_container(), ptr_container(), int_container(), int_container(), int_container() ) );
    R.remove = set_container( int_container() );

    for (i=0; i<vector_size(parent->child); i++)
	{
	    switch (pair(vector_get(parent->child,i)).first.i)
	        {
		    case item_select:
		        {
			    struct attr_select	*S = pair(vector_get(parent->child,i)).second.ptr;

			    container	*subattrp = attributes;
			    container	*attr_query = vector_container( string_container() );
			    iterator	it_m1;
			    it_m1.valid = 0;

			    printf("Select ");
			    for (j=0; j<S->size; j++)
				{
				    if (j>0) printf("/");
				    printf("%s", S->select[j]);
				    it_m1 = map_find(subattrp, S->select[j]);
				    if (it_m1.valid) subattrp = pair(map_val(it_m1)).first.ptr;
				    else break;
				    vector_pushback(attr_query, S->select[j]);
				}
			    printf("\n");

			    if (it_m1.valid)
				{
				    if (map_size(subattrp) > 0)
					{
					    printf("  valid/1\n");
    					    struct attribute_temp_res	res = attribute_generate_xml_subpattern_recurse(subattrp, attrib_count, getfiletypep, attrdescrp, indent, parent->sort, parent->flags & sort_reverse, attr_query, qa, parent->hide);
					    //attribute_print_xml_sorted(res.items, indent, B, parent->sort, parent->flags & sort_reverse);


					    if (parent->flags & build_groups && vector_size(attr_query) == 1)
						{
						    printf("    building groups\n");
/***---
//		    struct attribute_temp_res	res = attribute_generate_xml_subpattern_recurse(pair(map_val(it_m1)).first.ptr, attrib_count, getfiletypep, indent+2, sort, sort_reverse, attr_query, qa);
						    int		is_selected = 0;
						    int		expanded = 0;
						    int		split = -1, hit_split = -1;
						    buffer	*B = buffer_init(-1);
						    char	*description, *icon = NULL;

						    BINDENT; bprintf(B, "<group key=\"%s\"", vector_get(attr_query,0).ptr);
						    //char	*t = attribute_generate_value_string_from_vector(" value=", attr_query);
						    //bprintf(B, "%s", t);
						    //free(t);
						    if (attribute_description(attrdescrp, vector_get(attr_query,0).ptr, NULL, &description, &icon))
							bprintf(B, " name=\"%s\"", description);
						    else
							bprintf(B, " name=\"%s\"", pair(map_val(it_m1)).second.ptr);
						    if (icon!=NULL) bprintf(B, " icon=\"%s\"", icon);
//						    bprintf(B, " name=\"%s\"", pair(map_val(it_m1)).second.ptr);
						    set_insert(R.remove, is_selected=add_to_query(B, qa, QUERY_ATTRIBUTE, vector_get(attr_query,0).ptr, &split) );

						    bprintf(B, " hits=\"");
						    hit_split = B->pos;

						    int	child_selected = !attribute_is_empty_remove(res.remove);

						    set_insert(res.remove, is_selected);
						    buffer	*B2 = buffer_init(-1);
						    int	printed = attribute_print_xml_sorted(res.items, indent+2, B2, parent->sort, parent->flags & sort_reverse, qa, res.remove, attrib_count);

						    //if ((is_selected>=0 && printed>1) || child_selected)
						    if (child_selected)
							{
							    expanded = 1;
							}

						    bprintf(B, "\" expanded=\"%s\">\n", expanded ? "true" : "false");
						    char	*u = buffer_exit(B2);
						    bprintf(B, "%s", u);
						    free(u);

						    BINDENT; bprintf(B, "</group>\n");

						    //vector_pushback(R.items, buffer_exit(B), (char*)map_key(it_m1).ptr, pair(map_val(it_m1)).second.ptr, split, hit_split, expanded);
						    vector_pushback(R.items, buffer_exit(B), (char*)map_key(it_m1).ptr, pair(map_val(it_m1)).second.ptr, split, hit_split, 1);
						}
					    else
						{
						    printf("    proceeding\n");
						    for (j=0; j<vector_size(res.items); j++)
							{
						    	    value	v = vector_get(res.items,j);
						    	    vector_pushback(R.items, tuple(v).element[0].ptr, tuple(v).element[1].ptr, tuple(v).element[2].ptr, tuple(v).element[3].i, tuple(v).element[4].i, tuple(v).element[5].i);
							}
						}

					    iterator	it_sr = set_begin(res.remove);
					    for (; it_sr.valid; it_sr=set_next(it_sr))
						set_insert(R.remove, set_key(it_sr).i);

					    destroy(res.items);
					    destroy(res.remove);
					}
				    else
					{
					    printf("  valid/0\n");
					    buffer	*B = buffer_init(-1);
					    int		split = -1, hit_split = -1;

					    BINDENT; bprintf(B, "<item key=\"%s\"", S->select[0]);
					    char	*value = attribute_generate_value(&(S->select[1]), S->size-1);
					    if (strlen(value)>0) bprintf(B, " value=\"%s\"", value);
					    // Spesialtilfelle for 'filetype':
					    if (!strcmp("filetype", S->select[0]))
						{
						    char	*group, *descr, *icon;
						    fte_getdescription(getfiletypep, "nbo", S->select[S->size-1], &group, &descr, &icon);
						    bprintf(B, " name=\"%s\"", descr);
						    bprintf(B, " icon=\"%s\"", icon);
//						    bprintf(B, " query=\'");
						    //add_to_query(B, qa, R.remove, QUERY_FILETYPE, S->select[S->size-1]);
						    set_insert(R.remove, add_to_query(B, qa, QUERY_FILETYPE, S->select[S->size-1], &split) );
//						    bprintf(B, "\'");
						    //set_insert(R.remove, query_remove_word(qa, QUERY_FILETYPE, S->select[S->size-1]));
						}
					    else
						{
						    char	*description, *icon = NULL;
						    //bprintf(B, " name=\"%s\"", S->select[S->size-1]);
						    if (attribute_description(attrdescrp, S->select[S->size-1], value, &description, &icon))
							bprintf(B, " name=\"%s\"", description);
						    else
							bprintf(B, " name=\"%s\"", S->select[S->size-1]);
						    if (icon!=NULL) bprintf(B, " icon=\"%s\"", icon);
						    char	*s = attribute_generate_key_value_string_from_vector2(attr_query);
//						    bprintf(B, " query=\'%s%s\'", querystr, s);
//						    set_insert(R.remove, query_remove_word(qa, QUERY_ATTRIBUTE, s));
//						    bprintf(B, " query=\'");
						    //add_to_query(B, qa, R.remove, QUERY_ATTRIBUTE, s);
						    set_insert(R.remove, add_to_query(B, qa, QUERY_ATTRIBUTE, s, &split) );
//						    bprintf(B, "\'");
						}
					    bprintf(B, " hits=\"");
					    hit_split = B->pos;
					    bprintf(B, "\" />\n");
					    free(value);

					    vector_pushback(R.items, buffer_exit(B), (char*)map_key(it_m1).ptr, "", split, hit_split, 0);
					}

				    //R.hits+= pair(map_val(it_m1)).second.i;
				}
			    else	// Zero hits:
				{
				    printf("  invalid/0\n");
				    if (parent->flags & show_empty)
					{
					    buffer	*B = buffer_init(-1);
					    int		split = -1;

					    BINDENT; bprintf(B, "<item key=\"%s\"", S->select[0]);
					    //bprintf(B, "%s", attribute_generate_value_string(" value=", &(S->select[1]), S->size-1) );
					    char	*value = attribute_generate_value(&(S->select[1]), S->size-1);
					    if (strlen(value)>0) bprintf(B, " value=\"%s\"", value);
					    // Spesialtilfelle for 'filetype':
					    if (!strcmp("filetype", S->select[0]))
						{
						    char	*group, *descr, *icon;
						    //int	ret =
						    fte_getdescription(getfiletypep, "nbo", S->select[S->size-1], &group, &descr, &icon);
						    //if (ret&255)
						    bprintf(B, " name=\"%s\"", descr);
						    bprintf(B, " icon=\"%s\"", icon);
//						    bprintf(B, " query=\'%s filetype:%s\'", querystr, S->select[S->size-1]);
//						    set_insert(R.remove, query_remove_word(qa, QUERY_FILETYPE, S->select[S->size-1]));
//						    bprintf(B, " query=\'");
						    //add_to_query(B, qa, R.remove, QUERY_FILETYPE, S->select[S->size-1]);
						    set_insert(R.remove, add_to_query(B, qa, QUERY_FILETYPE, S->select[S->size-1], &split) );
//						    bprintf(B, "\'");
						}
					    else
						{
						    char	*description, *icon = NULL;
						    //bprintf(B, " name=\"%s\"", S->select[S->size-1]);
						    if (attribute_description(attrdescrp, S->select[S->size-1], value, &description, &icon))
							bprintf(B, " name=\"%s\"", description);
						    else
							bprintf(B, " name=\"%s\"", S->select[S->size-1]);
						    if (icon!=NULL) bprintf(B, " icon=\"%s\"", icon);
						    char	*s = attribute_generate_key_value_string_from_vector2(attr_query);
//						    bprintf(B, " query=\'%s%s\'", querystr, s);
//						    set_insert(R.remove, query_remove_word(qa, QUERY_ATTRIBUTE, s));
//						    bprintf(B, " query=\'");
						    set_insert(R.remove, add_to_query(B, qa, QUERY_ATTRIBUTE, s, &split) );
//						    bprintf(B, "\'");
						}
					    bprintf(B, " hits=\"0\" />\n");
					    free(value);
					    vector_pushback(R.items, buffer_exit(B), S->select[S->size-1], NULL, split, -1, 0);
					}
				}

			    destroy(attr_query);
			    break;
			}
		    case item_group:
			{
			    struct attr_group		*G = pair(vector_get(parent->child,i)).second.ptr;
			    struct attribute_temp_res	res = attribute_generate_xml_recurse(attributes, attrib_count, G, getfiletypep, attrdescrp, indent+2, qa);
			    buffer	*B = buffer_init(-1);

			    BINDENT; bprintf(B, "<group name=\"%s\" query=\'", G->name);
			    bsprint_query_with_remove(B, res.remove, qa);
			    bprintf(B, "\' expanded=\"");
			    if (G->flags&is_expanded || !attribute_is_empty_remove(res.remove))
				bprintf(B, "true");
			    else
				bprintf(B, "false");
			    bprintf(B, "\">\n");
			    attribute_print_xml_sorted(res.items, indent+2, B, G->sort, G->flags & sort_reverse, qa, res.remove, attrib_count);
			    BINDENT; bprintf(B, "</group>\n");

			    //vector_pushback(R.items, buffer_exit(B), G->name, res.hits, -1);
			    vector_pushback(R.items, buffer_exit(B), G->name, NULL, -1, -1, 0);

			    for (j=0; j<vector_size(res.items); j++) free(tuple(vector_get(res.items,j)).element[0].ptr);
			    destroy(res.items);

			    iterator	it_sr = set_begin(res.remove);
			    for (; it_sr.valid; it_sr=set_next(it_sr))
				set_insert(R.remove, set_key(it_sr).i);

			    destroy(res.remove);
//			    printf("<group selected=\"%i\" type=\"%s\" query=\"item:&quot;contacts/Dogbert&quot;\" />\n",
//				0, );
			    break;
			}
		    case item_import:
			{
			    struct attr_import	*I = pair(vector_get(parent->child,i)).second.ptr;
			    break;
			}
		}
	}

    return R;
}
*/

struct _attr_tree_
{
    char	*name, *key, *value, *icon;
    char	*querystr;
    container	*query_param;
    container	*count;
    container	*children;
    int		selected;
    char	selected_descendant, expanded, show_empty;
    int		container_id;
    int		hits;
    enum attr_sort_enum sort;
    char	sort_reverse, free_name, free_value;
    int		max_items;
};

struct _attr_ret_
{
    container	*C;
    char	selected_descendant;
};

struct _attr_query_element_
{
    char	*w[10];
    char	selected;
    int		query_pos;
    int		filter_id;
};

struct _attr_tree_* _attribute_tree_malloc_()
{
    struct _attr_tree_	*this_item = malloc(sizeof(struct _attr_tree_));
    this_item->name = NULL;
    this_item->key = NULL;
    this_item->value = NULL;
    this_item->icon = NULL;
    this_item->free_name = 0;
    this_item->free_value = 0;
    this_item->query_param = NULL;
    this_item->count = NULL;
    this_item->children = NULL;
    this_item->selected = -1;
    this_item->selected_descendant = 0;
    this_item->expanded = 0;
    this_item->container_id = 0;
    this_item->hits = 0;
    this_item->show_empty = 1;
    this_item->sort = sort_none;
    this_item->sort_reverse = 0;
    this_item->max_items = -1;

    return this_item;
}
/*
void _attribute_tree_free_( struct _attr_tree_ *this_item )
{
    if (this_item->name != NULL) free(this_item->name);
    if (this_item->query_param != NULL) destroy(this_item->query_param);
    if (this_item->count != NULL) free(this_item->count);
    if (this_item->children != NULL) foreach _attribute_tree_free_(child);
}
*/

int _attribute_is_selected_(container *A, container *param, int container_id)
{
    int		i, j;
    for (i=0; i<vector_size(A); i++)
	{
	    struct _attr_query_element_ *vquery = (struct _attr_query_element_*)vector_get(A,i).ptr;
	    char selected = 1;
	    for (j=0; j<10 && j<vector_size(param) && selected; j++)
		{
    		    if (vquery->w[j]==NULL) selected = 0;
		    else if (strcasecmp(vquery->w[j], (char*)vector_get(param,j).ptr)) selected = 0;
		}

	    if (j<10 && vquery->w[j]!=NULL) selected = 0;

	    // filetype/X == group/Y/X
	    if (!selected && vquery->w[0]!=NULL && !strcasecmp(vquery->w[0], "filetype")
		&& !strcasecmp((char*)vector_get(param,0).ptr, "group")
		&& vquery->w[1]!=NULL && vector_size(param)>=3
		&& !strcasecmp(vquery->w[1], (char*)vector_get(param,2).ptr))
		{
		    selected = 1;
		}

	    if (selected)
		{
		    vquery->selected = container_id;
		    return vquery->filter_id;
		}
	}

    return -1;
}


struct _attr_ret_ _attribute_add_children_(container *attributes, container *attr_query, container *hide, container *A, int container_id)
{
    int		i, j;
    container	*list_items = vector_container( ptr_container() );
    iterator	it_m1 = map_begin(attributes);
    int		has_selected_child = 0;

    for (; it_m1.valid; it_m1=map_next(it_m1))
	{
	    vector_pushback(attr_query, (char*)map_key(it_m1).ptr);

	    int		is_hidden = 0;
	    for (i=0; i<vector_size(hide) && !is_hidden; i++)
		{
		    container	*hide_elem = vector_get(hide, i).C;

		    for (j=0; j<vector_size(hide_elem); j++)
			{
			    if (strcasecmp(vector_get(attr_query,j).ptr, vector_get(hide_elem,j).ptr)) break;
			}

		    if (j>=vector_size(hide_elem)) is_hidden = 1;
		}

	    if (is_hidden)
		{
		    vector_remove_last(attr_query);
		    continue;
		}

	    struct _attr_tree_	*this_item = _attribute_tree_malloc_();
	    int val = _attribute_is_selected_(A, attr_query, container_id);
	    if (val>=0) this_item->selected = val;

	    if (map_size(pair(map_val(it_m1)).first.ptr) > 0)
		{
		    struct _attr_ret_ ret = _attribute_add_children_(pair(map_val(it_m1)).first.ptr, attr_query, hide, A, container_id);
		    if (ret.selected_descendant)
			{
			    this_item->selected_descendant = 1;
			    has_selected_child = 1;
			}
		    this_item->children = ret.C;
		    this_item->query_param = vector_container( string_container() );
		    for (j=0; j<vector_size(attr_query); j++)
			vector_pushback(this_item->query_param, vector_get(attr_query, j).ptr);
		    //this_item->name = ;
		    this_item->count = pair(map_val(it_m1)).second.ptr;
		}
	    else
		{
		    this_item->query_param = vector_container( string_container() );
		    for (j=0; j<vector_size(attr_query); j++)
			vector_pushback(this_item->query_param, vector_get(attr_query, j).ptr);
		    //this_item->name = ;
		    this_item->count = pair(map_val(it_m1)).second.ptr;
		}

	    vector_remove_last(attr_query);

	    if (this_item->name == NULL && this_item->query_param == NULL && this_item->count == NULL && this_item->children == NULL)
		{
		    free(this_item);
		}
	    else
		{
		    vector_pushback(list_items, this_item);
		    if (this_item->selected >= 0) has_selected_child = 1;
		}
	}

    struct _attr_ret_	ret;
    ret.C = list_items;
    ret.selected_descendant = has_selected_child;
    return ret;
}



struct _attr_ret_ _attribute_build_tree_(container *attributes, struct attr_group *parent, container *A, int *container_id)
{
    int		i, j;
    container	*list_items = vector_container( ptr_container() );
    int		has_selected_child = 0;

    for (i=0; i<vector_size(parent->child); i++)
	{
	    struct _attr_tree_	*this_item = _attribute_tree_malloc_();

	    switch (pair(vector_get(parent->child,i)).first.i)
	        {
		    case item_select:
		        {
			    struct attr_select	*S = pair(vector_get(parent->child,i)).second.ptr;

			    container	*subattrp = attributes;
			    container	*attr_query = vector_container( string_container() );
			    iterator	it_m1;
			    it_m1.valid = 0;

			    //printf("Select ");
			    for (j=0; j<S->size; j++)
				{
				    //if (j>0) printf("/");
				    //printf("%s", S->select[j]);
				    it_m1 = map_find(subattrp, S->select[j]);
				    if (it_m1.valid) subattrp = pair(map_val(it_m1)).first.ptr;
				    else break;
				    vector_pushback(attr_query, S->select[j]);
				}
			    //printf("\n");

			    if (it_m1.valid)
				{
				    this_item->query_param = attr_query;
				    this_item->count = pair(map_val(it_m1)).second.ptr;
				    this_item->sort = parent->sort;
				    this_item->sort_reverse = parent->flags & sort_reverse;
				    int val = _attribute_is_selected_(A, attr_query, *container_id);
				    if (val>=0) this_item->selected = val;

				    if (map_size(subattrp) > 0)
					{
					    //printf("  Generate subpattern children\n");
					    struct _attr_ret_ ret = _attribute_add_children_(subattrp, attr_query, parent->hide, A, *container_id);
					    if (ret.selected_descendant)
						{
						    this_item->selected_descendant = 1;
						    has_selected_child = 1;
						}

					    if (S->flags & build_groups)
						{
						    this_item->children = ret.C;
						}
					    else
						{
						    for (j=0; j<vector_size(ret.C); j++)
							vector_pushback(list_items, vector_get(ret.C,j).ptr);
						    this_item->query_param = NULL;
						    this_item->count = NULL;
						    this_item->children = NULL;
						    this_item->name = NULL;
						}

					    if (this_item->children == NULL) destroy(ret.C);
					}
				    //else
					//{
					    //printf("  No children\n");
					//}

				}
			    else	// Zero hits:
				{
				    //printf("  Zero hits\n");
				    /*
				    if (parent->flags & show_empty)
					{
					    this_item->query_param = attr_query;
					    int val = _attribute_is_selected_(A, attr_query, *container_id);
					    if (val>=0) this_item->selected = val;
					    this_item->show_empty = 1;
					    //printf("    (show anyway)\n");
					}
				    */
				}

			    if (this_item->query_param == NULL) destroy(attr_query);
			    break;
			}
		    case item_group:
			{
			    //printf("Creating new container {\n");
			    struct attr_group		*G = pair(vector_get(parent->child,i)).second.ptr;
			    this_item->name = strdup(G->name);
			    this_item->free_name = 1;
			    this_item->expanded = G->flags & is_expanded;
			    this_item->sort = G->sort;
			    this_item->sort_reverse = G->flags & sort_reverse;
			    this_item->show_empty = G->flags & show_empty;
			    this_item->max_items = G->max_items;

			    (*container_id)++; // NB! Will fail miserably for recursive container-groups.
			    this_item->container_id = *container_id;
			    //printf("  Container #id: %i\n", this_item->container_id);
			    struct _attr_ret_ ret = _attribute_build_tree_(attributes, G, A, container_id);
			    if (ret.selected_descendant)
				{
				    this_item->selected_descendant = 1;
				    has_selected_child = 1;
				}

			    this_item->children = ret.C;

			    //printf("}\n");
			    break;
			}
		    case item_import:
			{
			    struct attr_import	*I = pair(vector_get(parent->child,i)).second.ptr;
			    break;
			}
		}

	    if (this_item->name == NULL && this_item->query_param == NULL && this_item->count == NULL && this_item->children == NULL)
		{
		    free(this_item);
		}
	    else
		{
		    vector_pushback(list_items, this_item);
		    if (this_item->selected >= 0) has_selected_child = 1;
		}
	}

    struct _attr_ret_	ret;
    ret.C = list_items;
    ret.selected_descendant = has_selected_child;
    return ret;
}


int _attribute_count_hits_(container *count, int filter, int group_filter_id)
{
    if (count==NULL) return 0;

    int		i;
    int		len = group_filter_id+1;
    int		alle = 0;

    for (i=0; i<len; i++) alle+= 1<<i;

    int		inverse = alle - filter;
    //printf("filter:");
    //for (i=0; i<len; i++) printf("%c", (inverse & (1<<i) ? '1':'0'));
    //printf("\n");

    int		hits = 0;
    int		__total__ = 0;
    iterator	it;

    it = map_begin(count);
    for (; it.valid; it=map_next(it))
	{
	    //printf("       ");
	    //for (i=0; i<len; i++) printf("%c", ((alle - map_key(it).i) & (1<<i) ? '1':'0'));

	    __total__++;
	    if (((alle - map_key(it).i) & inverse) == 0)
		{
		    hits+= map_val(it).i;
		    //printf("  <count>\n");
		}
	    //else printf("  <----->\n");
	}
    //printf("       match:%i/%i\n", hits, __total__);

    return hits;
}


int _attribute_build_items_(container *X, container *A, query_array *qa, int default_filter, int group_filter_id, struct fte_data *getfiletypep, struct adf_data *attrdescrp)
{
    if (X==NULL) return 0;

    int		i, j;
    int		total_hits = 0;

    for (i=0; i<vector_size(X); i++)
	{
	    struct _attr_tree_	*item = vector_get(X, i).ptr;
	    int			filter = default_filter;

	    // Container? Recalculate qa:
	    if (item->container_id > 0)
		{
		    // NB! Overlappende selecteds i forskjellige groups kan feile.
		    //printf("Recalculating qa for container %i\n", item->container_id);
		    filter = 0;
		    for (j=0; j<vector_size(A); j++)
			{
			    struct _attr_query_element_ *vquery = (struct _attr_query_element_*)vector_get(A,j).ptr;

			    if (!(vquery->selected > 0 && vquery->selected != item->container_id))
				{
				    //printf("  qarg: %i\n", vquery->query_pos);
				    qa->query[vquery->query_pos].hide = 1;
				    filter|= (1<<vquery->filter_id);
				}
			    //else if (vquery->selected == 0) filter|= (1<<vquery->filter_id);
			}
		}

	    /*
	    if (item->selected>=0)
		{
		    filter|= (1<<item->selected);	// Skulle bli samme om filtrert eller ikke?
							// må -un-filtreres for rekursivitet
		}
	    */

	    // Construct querystring:
	    buffer	*B = buffer_init(-1);
	    bsprint_query_with_remove(B, NULL, qa, 1);

	    if (item->query_param!=NULL && vector_size(item->query_param)>0 && item->selected<0)
		{
		    if (!strcasecmp((char*)vector_get(item->query_param,0).ptr, "group"))
			{
			    if (vector_size(item->query_param)==3)
				bprintf(B, " filetype:\"%s\"", (char*)vector_get(item->query_param,2).ptr);
			    else if (vector_size(item->query_param)==2)
				bprintf(B, " group:\"%s\"", (char*)vector_get(item->query_param,1).ptr);
			}
		    else
			{
			    bprintf(B, " attribute:\"");
			    for (j=0; j<vector_size(item->query_param); j++)
				{
				    if (j==1) bprintf(B, "=");
				    else if (j>1) bprintf(B, "/");
				    bprintf(B, "%s", (char*)vector_get(item->query_param,j).ptr);
				}
			    bprintf(B, "\"");
			}
		}
	    item->querystr = buffer_exit(B);

	    // Calculate hits:
	    int hits = 0;

	    if (item->children!=NULL) // Recurse:
		hits = _attribute_build_items_(item->children, A, qa, filter, group_filter_id, getfiletypep, attrdescrp);

	    if (item->count==NULL)
		item->hits = hits;
	    else
		item->hits = _attribute_count_hits_(item->count, filter, group_filter_id);

	    total_hits+= item->hits;

	    if (item->container_id > 0)
		{
		    // NB! Overlappende selecteds i forskjellige groups kan feile.
		    for (j=0; j<vector_size(A); j++)
			{
			    struct _attr_query_element_ *vquery = (struct _attr_query_element_*)vector_get(A,j).ptr;
			    if (vquery->selected == item->container_id)
				{
				    qa->query[vquery->query_pos].hide = 0;
				}
			}
		}

	    // Set name, key, etc.

	    if (item->query_param!=NULL && vector_size(item->query_param)>0)
		{
		    int		key_type = 0;

		    if (!strcasecmp((char*)vector_get(item->query_param,0).ptr, "group"))
			{
			    if (vector_size(item->query_param)==3)
				{
				    key_type = 1;
				    item->key = "filetype";
				    item->value = (char*)vector_get(item->query_param,2).ptr;
				}
			    else if (vector_size(item->query_param)==2)
				{
				    key_type = 2;
				    item->key = "group";
				    item->value = (char*)vector_get(item->query_param,1).ptr;
				}
			}
		    else if (!strcasecmp((char*)vector_get(item->query_param,0).ptr, "filetype"))
			{
			    if (vector_size(item->query_param)==2)
				{
				    key_type = 1;
				    item->key = "filetype";
				    item->value = (char*)vector_get(item->query_param,1).ptr;
				}
			}
		    else
			{
			    item->key = (char*)vector_get(item->query_param, 0).ptr;
			    if (vector_size(item->query_param)>1)
				{
				    buffer	*vb = buffer_init(-1);

				    bprintf(vb, "%s", (char*)vector_get(item->query_param, 1).ptr);
				    for (j=2; j<vector_size(item->query_param); j++)
					bprintf(vb, "/%s", (char*)vector_get(item->query_param, j).ptr);

				    item->value = buffer_exit(vb);
				    item->free_value = 1;
				}
			}

		    if (key_type == 1 && item->value!=NULL) //filetype
			{
			    char	*group;
			    if ((fte_getdescription(getfiletypep, "nbo", item->value, &group, &(item->name), &(item->icon)) & 255) == 0)
				{
				    item->name = strdup(item->name);
				    item->free_name = 1;
				}
			}
		    else if (key_type == 2 && item->value!=NULL) //group
			{
			    fte_groupid(getfiletypep, "nbo", item->value, &(item->icon));
			}
		    else if (item->key!=NULL) //attribute
			{
			    attribute_description(attrdescrp, item->key, item->value, &(item->name), &(item->icon));
			}
		}

	    if (item->name==NULL)
		{
		    if (item->value!=NULL) item->name = item->value;
		    else item->name = item->key;
		}
	}

    return total_hits;
}


void _attribute_sort_items_(container **X, enum attr_sort_enum sort, char sort_reverse)
{
    if (*X == NULL) return;

    int		i, j;
    container	*N = NULL;

    /*
    printf("sort: ");
    switch (sort)
    {
	case sort_none: printf("sort_none"); break;
	case sort_hits: printf("sort_hits"); break;
	case sort_alpha: printf("sort_alpha"); break;
    }
    if (sort_reverse) printf(" (reverse)");
    printf("\n");

    for (i=0; i<vector_size(*X); i++)
	{
	    struct _attr_tree_	*item = vector_get(*X, i).ptr;
	    printf("  %.8x name:%s hits:%i\n", (int)X, item->name, item->hits);
	}
    */

    if (sort == sort_hits) N = multimap_container( int_container(), ptr_container() );
    else if (sort == sort_alpha) N = multimap_container( string_container(), ptr_container() );

    for (i=0; i<vector_size(*X); i++)
	{
	    struct _attr_tree_	*item = vector_get(*X, i).ptr;

	    if (item->children != NULL)
		_attribute_sort_items_(&(item->children), item->sort, item->sort_reverse);

	    if (sort == sort_hits) multimap_insert(N, item->hits, item);
	    else if (sort == sort_alpha)
		{
		    if (item->name!=NULL) multimap_insert(N, item->name, item);
		    else if (item->value!=NULL) multimap_insert(N, item->name, item);
		    else if (item->name!=NULL) multimap_insert(N, item->name, item);
		    else multimap_insert(N, item->name, item);
		}
	}

    if (!sort_reverse && sort != sort_hits && sort != sort_alpha) return;

    container	*Y = vector_container( ptr_container() );
    iterator	it;

    if ((sort == sort_hits && sort_reverse) || (sort == sort_alpha && !sort_reverse))
	{
	    it = multimap_begin(N);
	    for (; it.valid; it=multimap_next(it))
		vector_pushback(Y, map_val(it).ptr);
	}
    else
	{
	    if (sort != sort_hits && sort != sort_alpha)
		{
		    for (i=vector_size(*X)-1; i>=0; i--)
			vector_pushback(Y, vector_get(*X, i).ptr);
		}
	    else
		{
		    it = multimap_end(N);
		    for (; it.valid; it=multimap_previous(it))
			vector_pushback(Y, map_val(it).ptr);
		}
	}

    if (N != NULL) destroy(N);
    destroy(*X);
    *X = Y;

    /*
    for (i=0; i<vector_size(*X); i++)
	{
	    struct _attr_tree_	*item = vector_get(*X, i).ptr;
	    printf("  %.8x name:%s hits:%i\n", (int)X, item->name, item->hits);
	}
    */

    return;
}


void _attribute_print_and_delete_tree_(buffer *bout, container *X, int indent, int max_items)
{
    if (X==NULL) return;

    int		i, j;

    for (i=0; i<vector_size(X); i++)
	{
	    struct _attr_tree_	*item = vector_get(X, i).ptr;

	    //if (item->container_id == 0 && ((item->hits == 0 && !item->show_empty) || indent==-1))
	    //if (item->children!=NULL) bprintf(bout, "show_empty=%s", item->show_empty ? "true" : "false");
	    if ((item->container_id == 0 && (item->hits == 0 || indent==-1)) || (item->hits==0 && !item->show_empty))
		{
		    if (item->children!=NULL)
			{
			    _attribute_print_and_delete_tree_(bout, item->children, -1, item->max_items);
			}

		    if (item->free_value) free(item->value);
		    if (item->free_name) free(item->name);
		    if (item->querystr != NULL) free(item->querystr);
		    destroy(item->children);
		    destroy(item->query_param);
		    free(item);
		    continue;
		}


	    if (max_items > 0 && i>=max_items)
		{
		    if (i==max_items)
			{
			    for (j=0; j<indent; j++) bprintf(bout, " ");
			    bprintf(bout, "<item name=\"...\" />\n");
			}
		}
	    else
		{
		    char buf[1024];

		    for (j=0; j<indent; j++) bprintf(bout, " ");
		    if (item->children!=NULL) bprintf(bout, "<group");
		    else bprintf(bout, "<item");

		    if (item->key!=NULL) bprintf(bout, " key=\"%s\"", xml_escape_uri(item->key, buf, sizeof(buf)));
		    if (item->value!=NULL) bprintf(bout, " value=\"%s\"", xml_escape_uri(item->value, buf, sizeof(buf)));
		    if (item->name!=NULL) bprintf(bout, " name=\"%s\"", xml_escape_attr(item->name, buf, sizeof(buf)));
		    if (item->icon!=NULL) bprintf(bout, " icon=\"%s\"", xml_escape_uri(item->icon, buf, sizeof(buf)));

		    if (item->querystr!=NULL) bprintf(bout, " query=\"%s\"", xml_escape_uri(item->querystr, buf, sizeof(buf)));
		    if (item->selected >= 0) bprintf(bout, " selected=\"true\"");
		    bprintf(bout, " expanded=\"%s\"", item->selected_descendant || item->expanded ? "true":"false");
		    if (item->container_id == 0) bprintf(bout, " hits=\"%i\"", item->hits);

		    if (item->children!=NULL)
			{
			    bprintf(bout, ">\n");
			    _attribute_print_and_delete_tree_(bout, item->children, indent+4, item->max_items);
			    for (j=0; j<indent; j++) bprintf(bout, " ");
			    bprintf(bout, "</group>\n");
			}
		    else bprintf(bout, " />\n");
		}

	    if (item->free_value) free(item->value);
	    if (item->free_name) free(item->name);
	    if (item->querystr != NULL) free(item->querystr);
	    destroy(item->children);
	    destroy(item->query_param);
	    free(item);
	}
}



container* _attribute_dissect_query_(query_array *qa, int group_filter_id)
{
    int		i;
    int		exists = -1;
    container	*A = vector_container( ptr_container() );
    int		acount = 0;

    for (i=0; i<qa->n; i++)
	if (qa->query[i].n == 1)
	    {
		if (qa->query[i].operand == QUERY_ATTRIBUTE
		    || qa->query[i].operand == QUERY_GROUP
		    || qa->query[i].operand == QUERY_FILETYPE)
		    {
			struct _attr_query_element_ *vquery = malloc(sizeof(struct _attr_query_element_));
			int	j;
			for (j=0; j<10; j++) vquery->w[j] = NULL;
			j = 0;
			vquery->selected = 0;
			vquery->query_pos = i;

			if (qa->query[i].operand == QUERY_GROUP)
			    {
				vquery->w[j++] = strdup("group");
				vquery->filter_id = group_filter_id;
			    }
			if (qa->query[i].operand == QUERY_FILETYPE)
			    {
				vquery->w[j++] = strdup("filetype");
				vquery->filter_id = group_filter_id;
			    }
			else vquery->filter_id = acount++;

			char	*s = strdup(qa->query[i].s[0]);
			char	*ptrptr;
			char	*token;
			token = strtok_r(s, "=", &ptrptr);
			vquery->w[j++] = strdup(token);

			while (j<10)
			    {
				token = strtok_r(NULL, "/", &ptrptr);
				if (token == NULL) break;
				vquery->w[j++] = strdup(token);
			    }
			free(s);

			vector_pushback(A, vquery);
		    }
	    }

    return A;
}



char* attribute_generate_xml(container *attributes, int attrib_count, attr_conf *showattrp,
			struct fte_data *getfiletypep, struct adf_data *attrdescrp, query_array *qa)
{
    int		i, j;
    //buffer	*B = buffer_init(-1);
/*
    query_array	*qa = malloc(sizeof(query_array));
    get_query(querystr, strlen(querystr), qa);
    char	s[1024];
    sprint_query_array(s, 1024, qa);
    printf("\n%s\n", s);
*/
    //struct attribute_temp_res	R;
    //R = attribute_generate_xml_recurse(attributes, attrib_count, showattrp, getfiletypep, attrdescrp, 2, qa);

    /*** *** ***/
    container *A = _attribute_dissect_query_(qa, attrib_count-1);
    int *container_id = malloc(sizeof(int));
    *container_id = 0;
    struct _attr_ret_ ret = _attribute_build_tree_(attributes, showattrp, A, container_id);

    int		default_filter = 0;
    printf("default filter: %2x\ngroup filter_id: %i\n", default_filter, attrib_count-1);
    //_attribute_build_querystr_(ret.C, qa);
    //_attribute_calculate_hits_(ret.C, default_filter, attrib_count-1);
    _attribute_build_items_(ret.C, A, qa, default_filter, attrib_count-1, getfiletypep, attrdescrp);
    _attribute_sort_items_(&ret.C, showattrp->sort, showattrp->flags & sort_reverse);

    buffer	*bout = buffer_init(-1);
    bprintf(bout, "<navigation query=\"");

    for (i=0; i<vector_size(A); i++)
	{
	    struct _attr_query_element_ *vquery = (struct _attr_query_element_*)vector_get(A,i).ptr;
	    if (vquery->selected)
		{
		    qa->query[vquery->query_pos].hide = 1;
		}
	    else default_filter|= (1<<vquery->filter_id);
	}

    // Hide dato from "all"-query:
    for (i=0; i<qa->n; i++)
	if (qa->query[i].operand == QUERY_DATE)
	    qa->query[i].hide = 1;

    bsprint_query_with_remove(bout, NULL, qa, 1);
    bprintf(bout, "\">\n");

    _attribute_print_and_delete_tree_(bout, ret.C, 4, showattrp->max_items);

    //bprintf(bout, "</navigation>\n");	// Denne legges på i dispatcher, sammen med dato-feltet.

    free(container_id);
    destroy(ret.C);

    for (i=0; i<vector_size(A); i++)
	{
	    struct _attr_query_element_ *vquery = vector_get(A,i).ptr;
	    for (j=0; j<10; j++) if (vquery->w[j]!=NULL) free(vquery->w[j]);
	    free(vquery);
	}
    destroy(A);

    char	*out = buffer_exit(bout);
    printf("%s\n", out);
    return out;
    /*** *** ***/
/*
    for (i=0; i<qa->n; i++)
	if (qa->query[i].operand == QUERY_DATE)
	    set_insert(R.remove, i);

    bprintf(B, "<navigation query=\'");
    bsprint_query_with_remove(B, R.remove, qa);
    bprintf(B, "\'>\n");
    attribute_print_xml_sorted(R.items, 2, B, showattrp->sort, showattrp->flags & sort_reverse, qa, R.remove, attrib_count);
    //bprintf(B, "</navigation>\n");

//    free(qa);

    for (i=0; i<vector_size(R.items); i++) free(tuple(vector_get(R.items,i)).element[0].ptr);
    destroy(R.items);
    destroy(R.remove);

    return buffer_exit(B);*/
}

