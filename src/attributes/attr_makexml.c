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
#include "../common/define.h"
#include "../query/query_parser.h"
#include "attr_makexml.h"
#include "../ccan/json/json.h"

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


void va_attribute_count_add( int size, int count, container *attributes, int argc, va_list *ap )
{
    ant[0]++;
    int		i;
    char	*id = va_arg(*ap, char*);
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

    if (argc > 1) va_attribute_count_add( size, count, subattr, argc-1, ap );
}

void attribute_count_add( int size, int count, container *attributes, int argc, ... )
{
    ant[5]++;
    va_list		ap;

    va_start(ap, argc);
    va_attribute_count_add( size, count, attributes, argc, &ap );
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


static inline int attribute_description(struct adf_data *attrdescrp, char *key, char *value, char **description, char **icon)
{
    int		success = 0;
    if (key==NULL || key[0]=='\0') return 0;

    if (value==NULL || value[0]=='\0')
	{
	    success = adf_get_key_descr(attrdescrp, "eng", key, description, icon);
	}
    else
	{
	    success = adf_get_val_descr(attrdescrp, "eng", key, value, description, icon);
	}

    return success;
}


struct _attr_tree_
{
    char	*name, *key, *value, *icon, *version;
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
    int		max_sub_items;
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
    this_item->version = NULL;
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

    return this_item;
}


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
					//    printf("  No children\n");
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
			    this_item->max_sub_items = G->max_sub_items;
printf("max_sub_items: %d\n", this_item->max_sub_items);
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
	    bsprint_query_with_remove(B, NULL, qa, 0);

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
			    if ((fte_getdescription(getfiletypep, "eng", item->value, &group, &(item->name), &(item->icon), &(item->version)) & 255) == 0)
			    //if ((fte_getdescription(getfiletypep, "nbo", item->value, &group, &(item->name), &(item->icon)) & 255) == 0)
				{
				    item->name = strdup(item->name);
				    item->free_name = 1;
				}
			}
		    else if (key_type == 2 && item->value!=NULL) //group
			{
			    fte_groupid(getfiletypep, "eng", item->value, &(item->icon));
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


void _attribute_print_and_delete_tree_xml_(buffer *bout, container *X, int indent, int max_items, int max_sub_items, int depth)
{
    if (X==NULL) return;

    int		i, j;

    ++depth;

    for (i=0; i<vector_size(X); i++)
	{
	    struct _attr_tree_	*item = vector_get(X, i).ptr;

	    if ((item->container_id == 0 && (item->hits == 0 || indent==-1)) || (item->hits==0 && !item->show_empty))
		{
		    if (item->children!=NULL)
			{
			    _attribute_print_and_delete_tree_xml_(bout, item->children, -1, max_items, max_sub_items, depth);
			}

		    if (item->free_value) free(item->value);
		    if (item->free_name) free(item->name);
		    if (item->querystr != NULL) free(item->querystr);
		    destroy(item->children);
		    destroy(item->query_param);
		    free(item);
		    continue;
		}

	    if ((depth!=3 && max_items>0 && i>=max_items) || (depth==3 && max_sub_items>0 && i>=max_sub_items))
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
		    if (item->version!=NULL) bprintf(bout, " version=\"%s\"", xml_escape_attr(item->version, buf, sizeof(buf)));

		    if (item->querystr!=NULL) bprintf(bout, " query=\"%s\"", xml_escape_uri(item->querystr, buf, sizeof(buf)));
		    if (item->selected >= 0) bprintf(bout, " selected=\"true\"");
		    bprintf(bout, " expanded=\"%s\"", item->selected_descendant || item->expanded ? "true":"false");
		    if (item->container_id == 0) bprintf(bout, " hits=\"%i\"", item->hits);
		    if (item->sort==sort_alpha)
			{
			    if (item->sort_reverse)
				bprintf(bout, " sort=\"alpha_reversed\"");
			    else
				bprintf(bout, " sort=\"alpha\"");
			}

		    if (item->children!=NULL)
			{
			    bprintf(bout, ">\n");
			    _attribute_print_and_delete_tree_xml_(bout, item->children, indent+4, max_items, max_sub_items, depth);
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

void _attribute_print_and_delete_tree_json_(container *X, int indent, int max_items, int max_sub_items, int depth, JsonNode *root)
{
    if (X==NULL) return;

    ++depth;

    int		i, j;
    JsonNode *jsonitems = NULL;
    int isgroup = 0;

    jsonitems = json_mkarray();


    for (i=0; i<vector_size(X); i++)
	{
        JsonNode *jsonitem = json_mkobject();
	    struct _attr_tree_	*item = vector_get(X, i).ptr;

	    if ((item->container_id == 0 && (item->hits == 0 || indent==-1)) || (item->hits==0 && !item->show_empty))
		{
		    if (item->children!=NULL)
			{
			    _attribute_print_and_delete_tree_json_(item->children, -1, max_items, max_sub_items, depth, jsonitem);
			}

		    if (item->free_value) free(item->value);
		    if (item->free_name) free(item->name);
		    if (item->querystr != NULL) free(item->querystr);
		    destroy(item->children);
		    destroy(item->query_param);
		    free(item);
		    continue;
		}



	    if (!((depth!=3 && max_items>0 && i>=max_items) || (depth==3 && max_sub_items>0 && i>=max_sub_items)))
		{
		    char buf[1024];

		    if (item->children!=NULL) {
                isgroup = 1;
		    }
		    else { 
                isgroup = 0;
		    }


		    //if (item->key!=NULL) json_append_member(jsonitem, "key", json_mkstring(item->key) ); 
		    //if (item->value!=NULL) json_append_member(jsonitem, "value", json_mkstring(item->value) ); 
		    if (item->name!=NULL) json_append_member(jsonitem, "name", json_mkstring(item->name) ); 
		    if (item->icon!=NULL) json_append_member(jsonitem, "icon", json_mkstring(item->icon) );
		    if (item->querystr!=NULL) json_append_member(jsonitem, "query", json_mkstring(item->querystr) );
            if (item->container_id == 0) json_append_member(jsonitem, "hits", json_mknumber(item->hits) );

		    if (item->children!=NULL)
			{
			    _attribute_print_and_delete_tree_json_(item->children, indent+4, max_items, max_sub_items, depth, jsonitem);

			}
		}

	    if (item->free_value) free(item->value);
	    if (item->free_name) free(item->name);
	    if (item->querystr != NULL) free(item->querystr);
	    destroy(item->children);
	    destroy(item->query_param);
	    free(item);

        json_append_element(jsonitems, jsonitem );
	}


    if (isgroup==1)
        json_append_member(root, "group", jsonitems );
    else
        json_append_member(root, "items", jsonitems );
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
			struct fte_data *getfiletypep, struct adf_data *attrdescrp, query_array *qa, int outformat)
{
    int		i, j;


    /*** *** ***/
    container *A = _attribute_dissect_query_(qa, attrib_count-1);
    int *container_id = malloc(sizeof(int));
    *container_id = 0;
    struct _attr_ret_ ret = _attribute_build_tree_(attributes, showattrp, A, container_id);

    int		default_filter = 0;

    _attribute_build_items_(ret.C, A, qa, default_filter, attrib_count-1, getfiletypep, attrdescrp);
    _attribute_sort_items_(&ret.C, showattrp->sort, showattrp->flags & sort_reverse);

    buffer	*bout = buffer_init(-1);

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

    if (outformat==_OUT_FOMRAT_SD_JSON) {
        JsonNode *root = json_mkobject();
        _attribute_print_and_delete_tree_json_(ret.C, 4, showattrp->max_items, showattrp->max_sub_items, 0, root);
        char *tmps = json_stringify(root, "\t");
        bprintf(bout, tmps);
	    free(tmps);
    }
    else {
        bprintf(bout, "<navigation query=\"");
        bsprint_query_with_remove(bout, NULL, qa, 1);
        bprintf(bout, "\">\n");

        _attribute_print_and_delete_tree_xml_(bout, ret.C, 4, showattrp->max_items, showattrp->max_sub_items, 0);
    }

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
    #ifdef DEBUG
        printf("%s\n", out);
    #endif

    return out;
    /*** *** ***/

}

