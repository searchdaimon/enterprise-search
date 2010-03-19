
#include <stdio.h>
#include <stdlib.h>

#include "dcontainer.h"
#include "dstack.h"
#include "dset.h"


typedef struct
{
    container		*Key;
    _set_node_		*root;
    int			size;
} set_container_priv;



inline alloc_data set_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    va_copy(x.ap, ap);

    return x;
}


inline void set_deallocate( container *C, value a )
{
    destroy(a.C);
}


void set_deltree( container *C, _set_node_ *n )
{
    if (n->left_child != NULL)
	set_deltree( C, n->left_child );
    if (n->right_child != NULL)
	set_deltree( C, n->right_child );

    set_container_priv	*MP = C->priv;

    deallocate(MP->Key, n->key);
    free(n);
}


void set_destroy( container *C )
{
    set_container_priv	*MP = C->priv;

    if (MP->root!=NULL) set_deltree( C, MP->root );
    destroy( MP->Key );
    free(MP);
    free(C);
}


void set_clear( container *C )
{
    set_container_priv	*MP = C->priv;

    if (MP->root!=NULL) set_deltree( C, MP->root );

    MP->root = NULL;
    MP->size = 0;
}


inline iterator set_insert( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    set_container_priv	*MP = C->priv;
    value		key;
    iterator		it;

    va_start(ap, C);
    ad = MP->Key->ap_allocate(MP->Key, ap);
    key = ad.v;
    va_end(ad.ap);

    if (MP->size == 0)
        {
            MP->root = malloc(sizeof(_set_node_));
            MP->root->color = Black;
            MP->root->parent = NULL;
            MP->root->left_child = NULL;
            MP->root->right_child = NULL;
	    MP->root->key = key;
            MP->size++;
//            printf("set_insert: Root is %i\n", key.i);

	    it.node = MP->root;
	    it.valid = 1;
            return it;
        }

    _set_node_		*node = MP->root, *last_node = MP->root;
//    printf("set_insert: Adding %i\n", key.i);

    while (node!=NULL)
        {
    	    int		cmp = MP->Key->compare( MP->Key, node->key, key );

	    if (!cmp)
                {
		    // For now, do nothing if the key already exists.
		    // It is possible to expand this to include multiset etc.
//        	    printf("set_insert: Conflict with %i\n", key.i);

		    MP->Key->deallocate(MP->Key, key);
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


    node = malloc(sizeof(_set_node_));
    node->color = Red;
    node->parent = last_node;
    node->left_child = NULL;
    node->right_child = NULL;
    node->key = key;

    if (MP->Key->compare( MP->Key, last_node->key, key ) > 0)
	{
//	    printf("set_insert: Inserting %i at left.\n", key.i);
    	    last_node->left_child = node;
    	}
    else
	{
//	    printf("set_insert: Inserting %i at right.\n", key.i);
    	    last_node->right_child = node;
        }

    MP->size++;

    it.node = node;
    it.valid = 1;
    return it;
/*
    // Red-Black-Tree:
    while (node != *root && node->parent->color == Red)
        {
        }
*/
}



inline iterator set_insert_value( container *C, value key )
{
    set_container_priv	*MP = C->priv;
    iterator		it;

    if (MP->size == 0)
        {
            MP->root = malloc(sizeof(_set_node_));
            MP->root->color = Black;
            MP->root->parent = NULL;
            MP->root->left_child = NULL;
            MP->root->right_child = NULL;
	    MP->root->key = key;
            MP->size++;
//            printf("set_insert: Root is %i\n", key.i);

	    it.node = MP->root;
	    it.valid = 1;
            return it;
        }

    _set_node_		*node = MP->root, *last_node = MP->root;
//    printf("set_insert: Adding %i\n", key.i);

    while (node!=NULL)
        {
    	    int		cmp = MP->Key->compare( MP->Key, node->key, key );

	    if (!cmp)
                {
		    // For now, do nothing if the key already exists.
		    // It is possible to expand this to include multiset etc.
//        	    printf("set_insert: Conflict with %i\n", key.i);
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

    node = malloc(sizeof(_set_node_));
    node->color = Red;
    node->parent = last_node;
    node->left_child = NULL;
    node->right_child = NULL;
    node->key = key;

    if (MP->Key->compare( MP->Key, last_node->key, key ) > 0)
	{
//	    printf("set_insert: Inserting %i at left.\n", key.i);
    	    last_node->left_child = node;
    	}
    else
	{
//	    printf("set_insert: Inserting %i at right.\n", key.i);
    	    last_node->right_child = node;
        }

    MP->size++;

    it.node = node;
    it.valid = 1;
    return it;
}


inline void set_remove( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    set_container_priv	*MP = C->priv;
    value		key;


    if (MP->size == 0) return;

    va_start(ap, C);
    ad = MP->Key->ap_allocate(MP->Key, ap);
    key = ad.v;
    va_end(ad.ap);

    _set_node_		*node = MP->root;

    while (node!=NULL)
        {
    	    int		cmp = MP->Key->compare( MP->Key, node->key, key );

	    if (!cmp)
                {
		    // Remove existing key:

		    _set_node_		*z, *x, *y;
		    z = node;

		    if (z->left_child == NULL || z->right_child == NULL)
			y = node;
		    else
			// y = Tree-Successor(z);
			{
			    y = z->right_child;
			    while (y->left_child!=NULL)
				y = y->left_child;
			}

		    if (y->left_child != NULL)
			x = y->left_child;
		    else
			x = y->right_child;

		    if (x != NULL)
			x->parent = y->parent;

		    if (y->parent == NULL)
			{
			    MP->root = x;
			}
		    else
			{
			    if (y == y->parent->left_child)
				y->parent->left_child = x;
			    else
				y->parent->right_child = x;
			}

		    if (y != z)
			{
			    MP->Key->deallocate(MP->Key, z->key);
			    z->key = y->key;
			    free(y);
			}
		    else
			{
			    MP->Key->deallocate(MP->Key, y->key);
			    free(y);
			}

		    MP->size--;
		    MP->Key->deallocate(MP->Key, key);
                    return;
                }

            if (cmp > 0)
                node = node->left_child;
            else
                node = node->right_child;
        }

    MP->Key->deallocate(MP->Key, key);
}




inline iterator set_next( const iterator old_it )
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
			if (((_set_node_*)it.node)->left_child != NULL)
			    {
				it.node = ((_set_node_*)it.node)->left_child;
				s = 0;
			    }
			break;
		    case 1:
			s = 2;
			return it;
		    case 2:
			s = 3;
			if (((_set_node_*)it.node)->right_child != NULL)
			    {
				it.node = ((_set_node_*)it.node)->right_child;
				s = 0;
			    }
			break;
		    default:
			if (((_set_node_*)it.node)->parent != NULL && it.node == ((_set_node_*)it.node)->parent->left_child)
			    s = 1;
			else // if (((_set_node_*)it.node)->parent != NULL && it.node == ((_set_node_*)it.node)->parent->right_child)
			    s = 3;

			it.node = ((_set_node_*)it.node)->parent;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator set_previous( const iterator old_it )
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
			if (((_set_node_*)it.node)->right_child != NULL)
			    {
				it.node = ((_set_node_*)it.node)->right_child;
				s = 0;
			    }
			break;
		    case 1:
			s = 2;
			return it;
		    case 2:
			s = 3;
			if (((_set_node_*)it.node)->left_child != NULL)
			    {
				it.node = ((_set_node_*)it.node)->left_child;
				s = 0;
			    }
			break;
		    default:
			if (((_set_node_*)it.node)->parent != NULL && it.node == ((_set_node_*)it.node)->parent->right_child)
			    s = 1;
			else // if (((_set_node_*)it.node)->parent != NULL && it.node == ((_set_node_*)it.node)->parent->left_child)
			    s = 3;

			it.node = ((_set_node_*)it.node)->parent;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator set_begin( container *C )
{
    set_container_priv	*MP = C->priv;
    iterator		it;

    if (MP->size==0)
	{
	    it.valid = 0;
	    it.node = NULL;
	    return it;
	}

    it.node = MP->root;
    while (((_set_node_*)it.node)->left_child != NULL)
	it.node = ((_set_node_*)it.node)->left_child;;
    it.valid = 1;

    return it;
}


inline iterator set_end( container *C )
{
    set_container_priv	*MP = C->priv;
    iterator		it;

    if (MP->size==0)
	{
	    it.valid = 0;
	    it.node = NULL;
	    return it;
	}

    it.node = MP->root;
    while (((_set_node_*)it.node)->right_child != NULL)
	it.node = ((_set_node_*)it.node)->right_child;
    it.valid = 1;

    return it;
}



inline iterator set_find_value( container *C, value key )
{
    set_container_priv	*MP = C->priv;
    iterator		it;

    it.node = ((set_container_priv*)C->priv)->root;

    while (it.node!=NULL)
	{
	    int		cmp = MP->Key->compare(MP->Key, ((_set_node_*)it.node)->key, key);

	    if (cmp==0)
		{
		    it.valid = 1;
		    return it;
		}
	    if (cmp>0)
		{
    		    it.node = ((_set_node_*)it.node)->left_child;
		}
	    else
		{
		    it.node = ((_set_node_*)it.node)->right_child;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator set_find( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    set_container_priv	*MP = C->priv;
    value		key;
    iterator		it;

    va_start(ap, C);
    ad = MP->Key->ap_allocate(MP->Key, ap);
    key = ad.v;
    va_end(ap);

    it = set_find_value( C, key );

    deallocate(MP->Key, key);

    return it;
}


inline int set_size( container *C )
{
    return ((set_container_priv*)C->priv)->size;
}


inline container* _set_clone( container *C )
{
    set_container_priv	*LP = C->priv;
    container		*N = LP->Key->clone(LP->Key);

    return set_container(N);
}

inline value set_copy( container *C, value a )
{
    fprintf(stderr, "Function not implemented yet: set_copy\n");

    exit(-1);
}

inline void set_print( container *C, value a )
{
    int		i=0;
    iterator	it = set_begin(a.C);

    printf("(");
    for (; it.valid; it=set_next(it))
	{
	    if (i==0) i++;
	    else printf(" ");

	    printv(((set_container_priv*)C->priv)->Key, set_key(it));
	}
    printf(")");
}

inline void set_bprint( container *C, buffer *B, char *delim, value a )
{
    int		i=0;
    iterator	it = set_begin(a.C);

    for (; it.valid; it=set_next(it))
	{
	    if (i==0) i++;
	    else bprintf(B, "%s", delim);

	    bprintv(((set_container_priv*)C->priv)->Key, B, delim, set_key(it));
	}
}


container* set_container( container *Key )
{
    container			*M = malloc(sizeof(container));
    set_container_priv		*MP = malloc(sizeof(set_container_priv));

    M->compare = NULL;
    M->ap_allocate = set_ap_allocate;
    M->deallocate = set_deallocate;
    M->destroy = set_destroy;
    M->clear = set_clear;
    M->clone = _set_clone;
    M->copy = set_copy;
    M->print = set_print;
    M->bprint = set_bprint;
    M->priv = MP;

    MP->Key = Key;
    MP->root = NULL;
    MP->size = 0;

    return M;
}



inline iterator2 _set_next2( iterator2 it2 )
{
    iterator	it;
    it.node = it2.node;
    it.valid = it2.valid;
    it = set_next(it);
    it2.node = it.node;
    it2.valid = it.valid;

    return it2;
}

inline iterator2 _set_previous2( iterator2 it2 )
{
    iterator	it;
    it.node = it2.node;
    it.valid = it2.valid;
    it = set_previous(it);
    it2.node = it.node;
    it2.valid = it.valid;

    return it2;
}

inline value _set_get_key( iterator2 it2 )
{
    iterator	it;
    it.node = it2.node;
    it.valid = it2.valid;

    return set_key(it);
}

inline value _set_get_value( iterator2 it2 )
{
    value	v;
    v.i = 0;

    return v;
}

inline void _set_insert( container *C, value v )
{
    set_insert(C, v);
}

inline int _set_compare_keys( container *C, value a, value b )
{
    //printf("comparing %s and %s\n", a.str, b.str);
    set_container_priv	*MP = C->priv;
    return MP->Key->compare( MP->Key, a, b );
}

inline iterator2 set_begin2( container *C )
{
    iterator		it = set_begin(C);
    iterator2		it2;

    it2.node = it.node;
    it2.valid = it.valid;
    it2.next = _set_next2;
    it2.get_key = _set_get_key;
    it2.get_value = _set_get_value;
    it2.compare_keys = _set_compare_keys;
    it2.C = C;
    it2.insert = _set_insert;

    return it2;
}


inline iterator2 set_end2( container *C )
{
    iterator		it = set_end(C);
    iterator2		it2;

    it2.node = it.node;
    it2.valid = it.valid;
    it2.next = _set_previous2;
    it2.get_key = _set_get_key;
    it2.get_value = _set_get_value;
    it2.compare_keys = _set_compare_keys;
    it2.C = C;
    it2.insert = _set_insert;

    return it2;
}
