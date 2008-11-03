
#include <stdio.h>
#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dvector.h"
#include "../ds/dmap.h"
#include "../ds/dset.h"
#include "../ds/dmultimap.h"
#include "../ds/dtuple.h"
#include "../common/bprint.h"
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
		    default: fprintf(stderr, "attr_makexml: Warning! Undefined operand in 'add_to_query'.\n"); return -1;
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

char* attribute_generate_value_string(char *prefix, char *param[], int size)
{
    buffer	*B = buffer_init(-1);
    int		i;

    if (size > 0) bprintf(B, "%s\"%s", prefix, param[0]);
    for (i=1; i<size; i++) bprintf(B, "/%s", param[i]);
    if (size > 0) bprintf(B, "\"");

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


va_list va_attribute_count_add( int count, container *attributes, int argc, va_list ap )
{
    int		i;
    char	*id = va_arg(ap, char*);
    container	*subattr;

    iterator	it = map_find(attributes, id);
    if (it.valid)
	{
	    subattr = pair(map_val(it)).first.ptr;

	    container	*M = pair(map_val(it)).second.ptr;
	    it = map_find(M, count);
	    if (it.valid)
		{
		    map_val(it).i++;
		}
	    else
		{
		    map_insert(M, count, 1);
		}
	    //for (i=0; i<len; i++)
		//((int*)pair(map_val(it)).second.ptr)[i]+= count[i];
	}
    else
	{
	    subattr = map_container( string_container(), pair_container( ptr_container(), ptr_container() ) );

	    container	*M = map_container( int_container(), int_container() );
	    map_insert(M, count, 1);
	    //int		*C = malloc(sizeof(int)*len);
	    //for (i=0; i<len; i++) C[i] = count[i];
	    map_insert(attributes, id, subattr, M);
	}

    if (argc > 1) ap = va_attribute_count_add( count, subattr, argc-1, ap );

    return ap;
}

void attribute_count_add( int count, container *attributes, int argc, ... )
{
    va_list		ap;

    va_start(ap, argc);
    ap = va_attribute_count_add( count, attributes, argc, ap );
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

void attribute_count_print( container *attributes, int attrib_count, int indent )
{
    iterator		it = map_begin(attributes);

    for (; it.valid; it=map_next(it))
	{
	    int		i;

	    for (i=0; i<indent; i++) printf(" ");
	    printf("%s (?)\n", (char*)map_key(it).ptr);
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
	    if (it.valid && set_key(it).i == i)
		{
		    it = set_next(it);
		    if (qa->query[i].operand == QUERY_ATTRIBUTE) filter+= 1<<j;
		    if (qa->query[i].operand == QUERY_GROUP
			|| qa->query[i].operand == QUERY_FILETYPE) filter+= 1<<(len-1);
		}
	    else if (is_file_ext && (qa->query[i].operand == QUERY_FILETYPE
		|| qa->query[i].operand == QUERY_GROUP)) filter+= 1<<(len-1);

	    if (qa->query[i].operand == QUERY_ATTRIBUTE) j++;
	}

    int		alle = 0;
    for (i=0; i<len; i++) alle+= 1<<i;

    filter = alle - filter;

    int		hits = 0;
    it = map_begin(count);
    for (; it.valid; it=map_next(it))
	{
	    if ((map_key(it).i & filter) == filter)
		hits+= map_val(it).i;
	}

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
			    if (tuple(vector_get(items, j)).element[5].i) is_file_ext = 1;
			multimap_insert(M, (char*)tuple(vector_get(items, j)).element[1].ptr,
			    (char*)tuple(vector_get(items, j)).element[0].ptr,
			    tuple(vector_get(items, j)).element[3].i,
			    attrib_count_hits(qa, tuple(vector_get(items, j)).element[2].ptr, attrib_count, remove, tuple(vector_get(items, j)).element[5].i),
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
		    || qa->query[i].operand == QUERY_FILETYPE) set_insert(remove, i);
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
*/
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
			    bprintf(B, "%.*s", hit_split-pos, str);
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
*/
		    printed++;
		}
	}

    free(q);
    destroy(M);
    return printed;
}

struct attribute_temp_res attribute_generate_xml_subpattern_recurse(container *attributes, int attrib_count, struct fte_data *getfiletypep, int indent, int sort, char sort_reverse, container *attr_query, query_array *qa)
{
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

	    if (map_size(pair(map_val(it_m1)).first.ptr) > 0)
		{
		    struct attribute_temp_res	res = attribute_generate_xml_subpattern_recurse(pair(map_val(it_m1)).first.ptr, attrib_count, getfiletypep, indent+2, sort, sort_reverse, attr_query, qa);
		    int		is_selected = 0;
		    int		expanded = 0;
		    char	*icon = NULL;

		    BINDENT; bprintf(B, "<group key=\"%s\"", vector_get(attr_query,0).ptr);
		    char	*t = attribute_generate_value_string_from_vector(" value=", attr_query);
		    bprintf(B, "%s", t);
		    free(t);
		    bprintf(B, " name=\"%s\"", (char*)map_key(it_m1).ptr);
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
		}
	    else
		{
		    // Spesialtilfelle for 'filetype':
		    if ((vector_size(attr_query)==3 && !strcmp("group", vector_get(attr_query,0).ptr))
			|| !strcmp("filetype", vector_get(attr_query,0).ptr))
			{
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
			    */
			    //destroy(remove);
			    //hit_split = B->pos;
			    //bprintf(B, "\" />\n");
			}
		    else
			{
			    //item.key = strdup((char*)vector_get(attr_query,0).ptr);
			    //item.value = attribute_generate_value_string_from_vector("", attr_query);
			    //item.name = strdup((char*)map_key(it_m1).ptr);
			    //item.query = ;

			    BINDENT; bprintf(B, "<item key=\"%s\"", vector_get(attr_query,0).ptr);
			    char	*t = attribute_generate_value_string_from_vector(" value=", attr_query);
			    bprintf(B, "%s", t);
			    free(t);
			    bprintf(B, " name=\"%s\"", (char*)map_key(it_m1).ptr);
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


struct attribute_temp_res attribute_generate_xml_recurse(container *attributes, int attrib_count, struct attr_group *parent, struct fte_data *getfiletypep, int indent, query_array *qa)
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

			    for (j=0; j<S->size; j++)
				{
				    it_m1 = map_find(subattrp, S->select[j]);
				    if (it_m1.valid) subattrp = pair(map_val(it_m1)).first.ptr;
				    else break;
				    vector_pushback(attr_query, S->select[j]);
				}

			    if (it_m1.valid)
				{
				    if (map_size(subattrp) > 0)
					{
    					    struct attribute_temp_res	res = attribute_generate_xml_subpattern_recurse(subattrp, attrib_count, getfiletypep, indent, parent->sort, parent->flags & sort_reverse, attr_query, qa);
					    //attribute_print_xml_sorted(res.items, indent, B, parent->sort, parent->flags & sort_reverse);


					    if (parent->flags & flat_expand && vector_size(attr_query) == 1)
						{
/***/
//		    struct attribute_temp_res	res = attribute_generate_xml_subpattern_recurse(pair(map_val(it_m1)).first.ptr, attrib_count, getfiletypep, indent+2, sort, sort_reverse, attr_query, qa);
						    int		is_selected = 0;
						    int		expanded = 0;
						    int		split = -1, hit_split = -1;
						    buffer	*B = buffer_init(-1);
//		    char	*icon = NULL;

						    BINDENT; bprintf(B, "<group key=\"%s\"", vector_get(attr_query,0).ptr);
						    //char	*t = attribute_generate_value_string_from_vector(" value=", attr_query);
						    //bprintf(B, "%s", t);
						    //free(t);
						    bprintf(B, " name=\"%s\"", vector_get(attr_query,0).ptr);
//						    bprintf(B, " name=\"%s\"", pair(map_val(it_m1)).second.ptr);
						    set_insert(R.remove, is_selected=add_to_query(B, qa, QUERY_ATTRIBUTE, vector_get(attr_query,0).ptr, &split) );
/*
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

		    if (icon==NULL) icon = getfiletypep->default_icon;
		    bprintf(B, " icon=\"%s\"", icon);
*/
						    bprintf(B, " hits=\"");
						    hit_split = B->pos;

						    int	child_selected = !attribute_is_empty_remove(res.remove);

		    //bprintf(B, "\" expanded=\"%s\">\n", expanded ? "true" : "false");
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

//		    iterator	it_sr = set_begin(res.remove);
//		    for (; it_sr.valid; it_sr=set_next(it_sr))
//			set_insert(R.remove, set_key(it_sr).i);

//		    for (i=0; i<vector_size(res.items); i++) free(tuple(vector_get(res.items,i)).element[0].ptr);

//		    destroy(res.items);
//		    destroy(res.remove);
						    BINDENT; bprintf(B, "</group>\n");
/***/
						    vector_pushback(R.items, buffer_exit(B), (char*)map_key(it_m1).ptr, pair(map_val(it_m1)).second.ptr, split, hit_split, expanded);
						}
					    else
						{
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
					    buffer	*B = buffer_init(-1);
					    int		split = -1, hit_split = -1;

					    BINDENT; bprintf(B, "<item key=\"%s\"", S->select[0]);
					    bprintf(B, "%s", attribute_generate_value_string(" value=", &(S->select[1]), S->size-1) );
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
						    bprintf(B, " name=\"%s\"", S->select[S->size-1]);
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

					    vector_pushback(R.items, buffer_exit(B), (char*)map_key(it_m1).ptr, "", split, hit_split, 0);
					}

				    //R.hits+= pair(map_val(it_m1)).second.i;
				}
			    else	// Zero hits:
				{
				    if (parent->flags & show_empty)
					{
					    buffer	*B = buffer_init(-1);
					    int		split = -1;

					    BINDENT; bprintf(B, "<item key=\"%s\"", S->select[0]);
					    bprintf(B, "%s", attribute_generate_value_string(" value=", &(S->select[1]), S->size-1) );
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
						    bprintf(B, " name=\"%s\"", S->select[S->size-1]);
						    char	*s = attribute_generate_key_value_string_from_vector2(attr_query);
//						    bprintf(B, " query=\'%s%s\'", querystr, s);
//						    set_insert(R.remove, query_remove_word(qa, QUERY_ATTRIBUTE, s));
//						    bprintf(B, " query=\'");
						    set_insert(R.remove, add_to_query(B, qa, QUERY_ATTRIBUTE, s, &split) );
//						    bprintf(B, "\'");
						}
					    bprintf(B, " hits=\"0\" />\n");
					    vector_pushback(R.items, buffer_exit(B), S->select[S->size-1], NULL, split, -1, 0);
					}
				}

			    destroy(attr_query);
			    break;
			}
		    case item_group:
			{
			    struct attr_group		*G = pair(vector_get(parent->child,i)).second.ptr;
			    struct attribute_temp_res	res = attribute_generate_xml_recurse(attributes, attrib_count, G, getfiletypep, indent+2, qa);
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


char* attribute_generate_xml(container *attributes, int attrib_count, attr_conf *showattrp, struct fte_data *getfiletypep, query_array *qa)
{
    int		i;
    buffer	*B = buffer_init(-1);
/*
    query_array	*qa = malloc(sizeof(query_array));
    get_query(querystr, strlen(querystr), qa);
    char	s[1024];
    sprint_query_array(s, 1024, qa);
    printf("\n%s\n", s);
*/
    struct attribute_temp_res	R;
    R = attribute_generate_xml_recurse(attributes, attrib_count, showattrp, getfiletypep, 2, qa);

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

    return buffer_exit(B);
}

