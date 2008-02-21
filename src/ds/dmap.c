
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dcontainer.h"
#include "dpair.h"
#include "dstack.h"
#include "dvector.h"
#include "dmap.h"

//#define RED_BLACK

typedef struct
{
    container		*Key, *Data;
    _map_node_		*root;
    int                 size;
} map_container_priv;



inline alloc_data map_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    va_copy(x.ap, ap);

    return x;
}


inline void map_deallocate( container *C, value a )
{
    destroy(a.C);
}


void map_deltree( container *C, _map_node_ *n )
{
    map_container_priv	*MP = C->priv;

    deallocate(MP->Key, n->key);
    deallocate(MP->Data, n->val);

    if (n->left_child != NULL)
	map_deltree( C, n->left_child );
    if (n->right_child != NULL)
	map_deltree( C, n->right_child );

    free(n);
}


void map_destroy( container *C )
{
    map_container_priv	*MP = C->priv;

    if (MP->root!=NULL) map_deltree( C, MP->root );
    destroy( MP->Key );
    destroy( MP->Data );
    free(MP);
    free(C);
}


void map_clear( container *C )
{
    map_container_priv	*MP = C->priv;

    if (MP->root!=NULL) map_deltree( C, MP->root );

    MP->root = NULL;
    MP->size = 0;
}

/*
char* get_hash(container *hash, int v)
{
    int		i;

    for (i=0; i<vector_size(hash); i++)
	{
	    int		hash_key = pair(vector_get(hash,i)).first.i;
	    char	*hash_val = (char*)pair(vector_get(hash,i)).second.ptr;

	    if (v == hash_key)
		return hash_val;
	}

    char	*s = malloc(5);
    sprintf(s, "%.1X", i);

    vector_pushback(hash, v, s);
    return s;
}


int map_verify_tree( container *C, _map_node_ *node )
{
    if (node == NULL) return 0;

    map_container_priv *MP = C->priv;

    int		cmp1, cmp2;

    if (node->left_child == NULL) cmp1 = 1;
    else cmp1 = MP->Key->compare( MP->Key, node->key, node->left_child->key );

    if (node->right_child == NULL) cmp2 = -1;
    else cmp2 = MP->Key->compare( MP->Key, node->key, node->right_child->key );

    if (cmp1<=0 || cmp2>=0)
	{
	    printf("map: invalid tree!\n");
	    exit(-1);
	}

    map_verify_tree(C, node->left_child);
    map_verify_tree(C, node->right_child);
}

int map_check_tree( container *C, container *hash, _map_node_ *node )
{
    map_container_priv *MP = C->priv;

    if (node == NULL) return 0;

    printf("%s_%c ", get_hash(hash, (int)node), (node->color==Black ? 'B':'R'));

    if (node->left_child == NULL) printf("(--- |");
    else printf("(%s_%c |", get_hash(hash, (int)node->left_child), (node->left_child->color==Black ? 'B':'R') );

    if (node->right_child == NULL) printf("| ---)\n");
    else printf("| %s_%c)\n", get_hash(hash, (int)node->right_child), (node->right_child->color==Black ? 'B':'R') );

    map_check_tree(C, hash, node->left_child);
    map_check_tree(C, hash, node->right_child);
**
    if (node->color == Red)
	{
	    if ((node->left_child!=NULL && node->left_child->color!=Black)
		|| (node->right_child!=NULL && node->right_child->color!=Black))
		{
		    printf("map: Red nodes should have only black children.\n");
		    exit(-1);
		}
	}

    if (node->left_child == NULL && node->right_child == NULL)
	{
	    // Leaf
	    return 1;
	}

    int		black1, black2;

    if (node->left_child == NULL) black1 = 1;
    else black1 = map_check_tree(C, node->left_child);

    if (node->right_child == NULL) black2 = 1;
    else black2 = map_check_tree(C, node->right_child);

    if (black1 != black2)
	{
	    printf("map: #blacks differ: %i %i\n", black1, black2);
	    exit(-1);
	}

    if (node->color == Black)
	return black1 + 1;

    return black1;
**
}
*/

static inline void map_rotate_left( map_container_priv *MP, _map_node_ *x )
{
    _map_node_		*y;

    assert(x->right_child != NULL);
    y = x->right_child;
    x->right_child = y->left_child;
    if (y->left_child != NULL)
	y->left_child->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL)
	MP->root = y;
    else if (x == x->parent->left_child)
	x->parent->left_child = y;
    else
	x->parent->right_child = y;
    y->left_child = x;
    x->parent = y;
}

static inline void map_rotate_right( map_container_priv *MP, _map_node_ *y )
{
    _map_node_		*x;

    assert(y->left_child != NULL);
    x = y->left_child;
    y->left_child = x->right_child;
    if (x->right_child != NULL)
	x->right_child->parent = y;
    x->parent = y->parent;
    if (y->parent == NULL)
	MP->root = x;
    else if (y == y->parent->left_child)
	y->parent->left_child = x;
    else
	y->parent->right_child = x;
    x->right_child = y;
    y->parent = x;
}


inline iterator map_insert( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    map_container_priv	*MP = C->priv;
    value		key, val;
    iterator		it;
//    container		*hash = vector_container( pair_container( int_container(), string_container() ) );

    va_start(ap, C);
    ad = MP->Key->ap_allocate(MP->Key, ap);
    key = ad.v;
    va_copy(ap, ad.ap);
    ad = MP->Data->ap_allocate(MP->Data, ap);
    val = ad.v;
    va_end(ad.ap);

    if (MP->size == 0)
        {
            MP->root = malloc(sizeof(_map_node_));
            MP->root->color = Black;
            MP->root->parent = NULL;
            MP->root->left_child = NULL;
            MP->root->right_child = NULL;
	    MP->root->key = key;
	    MP->root->val = val;
            MP->size++;
//            printf("map_insert: Root is %i\n", key.i);

	    it.node = MP->root;
	    it.valid = 1;
            return it;
        }

    _map_node_		*node = MP->root, *last_node = MP->root;
//    printf("map_insert: Adding %i\n", key.i);

    while (node!=NULL)
        {
    	    int		cmp = MP->Key->compare( MP->Key, node->key, key );

	    if (!cmp)
                {
		    // For now, do nothing if the key already exists.
		    // It is possible to expand this to include multimap etc.
//        	    printf("map_insert: Conflict with %i\n", key.i);
		    MP->Key->deallocate(MP->Key, key);
		    MP->Data->deallocate(MP->Data, val);

		    it.node = NULL;
		    it.valid = 0;
                    return it;
                }

            last_node = node;

            if (cmp > 0)
//		{printf("left\n");
                node = node->left_child;
            else
//		{printf("right\n");
                node = node->right_child;
        }

    node = malloc(sizeof(_map_node_));
    node->color = Red;
    node->parent = last_node;
    node->left_child = NULL;
    node->right_child = NULL;
    node->key = key;
    node->val = val;

    if (MP->Key->compare( MP->Key, last_node->key, key ) > 0)
	{
//	    printf("map_insert: Inserting %i at left.\n", key.i);
    	    last_node->left_child = node;
    	}
    else
	{
//	    printf("map_insert: Inserting %i at right.\n", key.i);
    	    last_node->right_child = node;
        }

    MP->size++;
    it.node = node;
    it.valid = 1;

#ifdef RED_BLACK
    // Red-Black Tree:
    node->color = Red;

    while (node != MP->root && node->parent->color == Red)
	{
	    assert(node->parent->parent != NULL);
	    if (node->parent == node->parent->parent->left_child)
		{
		    _map_node_		*y = node->parent->parent->right_child;

//		    if (y==NULL) break;

//		    if (y!=NULL && y->color == Red)
		    if (y!=NULL && y->color == Red)
			{
			    node->parent->color = Black;
			    y->color = Black;
			    node->parent->parent->color = Red;
			    node = node->parent->parent;
			}
		    else
			{
			    if (node == node->parent->right_child)
				{
				    node = node->parent;
				    map_rotate_left(MP, node);
				}

			    node->parent->color = Black;
			    node->parent->parent->color = Red;
			    map_rotate_right(MP, node->parent->parent);
			}
		}
	    else
		{
		    _map_node_		*y = node->parent->parent->left_child;

		    if (y!=NULL && y->color == Red)
			{
			    node->parent->color = Black;
			    y->color = Black;
			    node->parent->parent->color = Red;
			    node = node->parent->parent;
			}
		    else
			{
			    if (node == node->parent->left_child)
				{
				    node = node->parent;
				    map_rotate_right(MP, node);
				}

			    node->parent->color = Black;
			    node->parent->parent->color = Red;
			    map_rotate_left(MP, node->parent->parent);
			}
		}
	}

    MP->root->color = Black;

//    map_check_tree(C, hash, MP->root);
//    map_verify_tree(C, MP->root);
//    printf("."); fflush(stdout);
//    printf("\n");
#endif

    return it;
}



inline iterator map_insert_value( container *C, value key, value val )
{
    map_container_priv	*MP = C->priv;
    iterator		it;
//    container		*hash = vector_container( pair_container( int_container(), string_container() ) );

    if (MP->size == 0)
        {
            MP->root = malloc(sizeof(_map_node_));
            MP->root->color = Black;
            MP->root->parent = NULL;
            MP->root->left_child = NULL;
            MP->root->right_child = NULL;
	    MP->root->key = key;
	    MP->root->val = val;
            MP->size++;
//            printf("map_insert: Root is %i\n", key.i);

	    it.node = MP->root;
	    it.valid = 1;
            return it;
        }

    _map_node_		*node = MP->root, *last_node = MP->root;
//    printf("map_insert: Adding %i\n", key.i);

    while (node!=NULL)
        {
    	    int		cmp = MP->Key->compare( MP->Key, node->key, key );

	    if (!cmp)
                {
		    // For now, do nothing if the key already exists.
		    // It is possible to expand this to include multimap etc.
//        	    printf("map_insert: Conflict with %i\n", key.i);
		    it.node = NULL;
		    it.valid = 0;
                    return it;
                }

            last_node = node;

            if (cmp > 0)
//		{printf("left\n");
                node = node->left_child;
            else
//		{printf("right\n");
                node = node->right_child;
        }

    node = malloc(sizeof(_map_node_));
    node->color = Red;
    node->parent = last_node;
    node->left_child = NULL;
    node->right_child = NULL;
    node->key = key;
    node->val = val;

    if (MP->Key->compare( MP->Key, last_node->key, key ) > 0)
	{
//	    printf("map_insert: Inserting %i at left.\n", key.i);
    	    last_node->left_child = node;
    	}
    else
	{
//	    printf("map_insert: Inserting %i at right.\n", key.i);
    	    last_node->right_child = node;
        }

    MP->size++;
    it.node = node;
    it.valid = 1;

#ifdef RED_BLACK
    // Red-Black Tree:
    node->color = Red;

    while (node != MP->root && node->parent->color == Red)
	{
	    assert(node->parent->parent != NULL);
	    if (node->parent == node->parent->parent->left_child)
		{
		    _map_node_		*y = node->parent->parent->right_child;

//		    if (y==NULL) break;

//		    if (y!=NULL && y->color == Red)
		    if (y!=NULL && y->color == Red)
			{
			    node->parent->color = Black;
			    y->color = Black;
			    node->parent->parent->color = Red;
			    node = node->parent->parent;
			}
		    else
			{
			    if (node == node->parent->right_child)
				{
				    node = node->parent;
				    map_rotate_left(MP, node);
				}

			    node->parent->color = Black;
			    node->parent->parent->color = Red;
			    map_rotate_right(MP, node->parent->parent);
			}
		}
	    else
		{
		    _map_node_		*y = node->parent->parent->left_child;

		    if (y!=NULL && y->color == Red)
			{
			    node->parent->color = Black;
			    y->color = Black;
			    node->parent->parent->color = Red;
			    node = node->parent->parent;
			}
		    else
			{
			    if (node == node->parent->left_child)
				{
				    node = node->parent;
				    map_rotate_right(MP, node);
				}

			    node->parent->color = Black;
			    node->parent->parent->color = Red;
			    map_rotate_left(MP, node->parent->parent);
			}
		}
	}

    MP->root->color = Black;
//    map_check_tree(C, hash, MP->root);
//    map_verify_tree(C, MP->root);
//    printf("."); fflush(stdout);
//    printf("\n");
#endif

    return it;
}




inline iterator map_next( const iterator old_it )
{
    iterator	it = old_it;
    int		s = 2;

    it.valid = 1;

    while (it.node != NULL)
	{
	    switch (s)
		{
		    case 0:
			s = 1;
			if (((_map_node_*)it.node)->left_child != NULL)
			    {
				it.node = ((_map_node_*)it.node)->left_child;
				s = 0;
			    }
			break;
		    case 1:
			s = 2;
			return it;
		    case 2:
			s = 3;
			if (((_map_node_*)it.node)->right_child != NULL)
			    {
				it.node = ((_map_node_*)it.node)->right_child;
				s = 0;
			    }
			break;
		    default:
			if (((_map_node_*)it.node)->parent != NULL && it.node == ((_map_node_*)it.node)->parent->left_child)
			    s = 1;
			else // if (((_map_node_*)it.node)->parent != NULL && it.node == ((_map_node_*)it.node)->parent->right_child)
			    s = 3;

			it.node = ((_map_node_*)it.node)->parent;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator map_previous( const iterator old_it )
{
    iterator	it = old_it;
    int		s = 2;

    it.valid = 1;

    while (it.node != NULL)
	{
	    switch (s)
		{
		    case 0:
			s = 1;
			if (((_map_node_*)it.node)->right_child != NULL)
			    {
				it.node = ((_map_node_*)it.node)->right_child;
				s = 0;
			    }
			break;
		    case 1:
			s = 2;
			return it;
		    case 2:
			s = 3;
			if (((_map_node_*)it.node)->left_child != NULL)
			    {
				it.node = ((_map_node_*)it.node)->left_child;
				s = 0;
			    }
			break;
		    default:
			if (((_map_node_*)it.node)->parent != NULL && it.node == ((_map_node_*)it.node)->parent->right_child)
			    s = 1;
			else // if (((_map_node_*)it.node)->parent != NULL && it.node == ((_map_node_*)it.node)->parent->left_child)
			    s = 3;

			it.node = ((_map_node_*)it.node)->parent;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator map_begin( container *C )
{
    map_container_priv	*MP = C->priv;
    iterator		it;

    if (MP->size==0)
	{
	    it.valid = 0;
	    it.node = NULL;
	    return it;
	}

    it.node = MP->root;
    while (((_map_node_*)it.node)->left_child != NULL)
	it.node = ((_map_node_*)it.node)->left_child;;
    it.valid = 1;

    return it;
}


inline iterator map_end( container *C )
{
    map_container_priv	*MP = C->priv;
    iterator		it;

    if (MP->size==0)
	{
	    it.valid = 0;
	    it.node = NULL;
	    return it;
	}

    it.node = MP->root;
    while (((_map_node_*)it.node)->right_child != NULL)
	it.node = ((_map_node_*)it.node)->right_child;
    it.valid = 1;

    return it;
}


inline iterator map_find_value( container *C, value key )
{
    map_container_priv	*MP = C->priv;
    iterator		it;

    it.node = ((map_container_priv*)C->priv)->root;

    while (it.node!=NULL)
	{
	    int		cmp = MP->Key->compare(MP->Key, ((_map_node_*)it.node)->key, key);

	    if (cmp==0)
		{
		    it.valid = 1;
		    return it;
		}
	    if (cmp>0)
		{
    		    it.node = ((_map_node_*)it.node)->left_child;
		}
	    else
		{
		    it.node = ((_map_node_*)it.node)->right_child;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator map_find( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    map_container_priv	*MP = C->priv;
    value		key;
    iterator		it;

    va_start(ap, C);
    ad = MP->Key->ap_allocate(MP->Key, ap);
    key = ad.v;
    va_end(ap);

    it = map_find_value( C, key );

    deallocate(MP->Key, key);

    return it;
}


inline int map_size( container *C )
{
    return ((map_container_priv*)C->priv)->size;
}


inline container* map_clone( container *C )
{
    map_container_priv	*LP = C->priv;
    container		*N = LP->Key->clone(LP->Key),
			*M = LP->Data->clone(LP->Data);
    return map_container(N,M);
}

inline value map_copy( container *C, value a )
{
    fprintf(stderr, "Function not implemented yet: map_copy\n");

    exit(-1);
}

inline void map_print( container *C, value a )
{
    int		i=0;
    iterator	it = map_begin(a.C);

    printf("(");
    for (; it.valid; it=map_next(it))
	{
	    if (i==0) i++;
	    else printf(" ");

	    print(((map_container_priv*)C->priv)->Key, map_key(it));
	    printf(":");
	    print(((map_container_priv*)C->priv)->Data, map_val(it));
	}
    printf(")");
}


container* map_container( container *Key, container *Data )
{
    container			*M = malloc(sizeof(container));
    map_container_priv		*MP = malloc(sizeof(map_container_priv));

    M->compare = NULL;
    M->ap_allocate = map_ap_allocate;
    M->deallocate = map_deallocate;
    M->destroy = map_destroy;
    M->clear = map_clear;
    M->clone = map_clone;
    M->copy = map_copy;
    M->print = map_print;
    M->priv = MP;

    MP->Key = Key;
    MP->Data = Data;
    MP->root = NULL;
    MP->size = 0;

    return M;
}
