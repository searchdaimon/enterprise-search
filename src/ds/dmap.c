
#include <stdio.h>
#include <stdlib.h>

#include "dcontainer.h"
#include "dstack.h"
#include "dmap.h"


typedef struct
{
    container		*Key, *Data;
    _map_node_            *root;
    int                 size;
} map_container_priv;



inline alloc_data map_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    x.ap = ap;

    return x;
}


inline void map_deallocate( container *C, value a )
{
    destroy(a.C);
}


void map_deltree( container *C, _map_node_ *n )
{
    if (n->left_child != NULL)
	map_deltree( C, n->left_child );
    if (n->right_child != NULL)
	map_deltree( C, n->right_child );

    map_container_priv	*MP = C->priv;

    deallocate(MP->Key, n->key);
    deallocate(MP->Data, n->val);
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


inline void map_insert( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    map_container_priv	*MP = C->priv;
    value		key, val;

    va_start(ap, C);
    ad = MP->Key->ap_allocate(MP->Key, ap);
    key = ad.v;
    ap = ad.ap;
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
            return;
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
                    return;
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

/*
    // Red-Black-Tree:
    while (node != *root && node->parent->color == Red)
        {
        }
*/
}



inline void map_insert_value( container *C, value key, value val )
{
    map_container_priv	*MP = C->priv;

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
            return;
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
                    return;
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

container* map_container( container *Key, container *Data )
{
    container			*M = malloc(sizeof(container));
    map_container_priv		*MP = malloc(sizeof(map_container_priv));

    M->compare = NULL;
    M->ap_allocate = map_ap_allocate;
    M->deallocate = map_deallocate;
    M->destroy = map_destroy;
    M->clone = map_clone;
    M->priv = MP;

    MP->Key = Key;
    MP->Data = Data;
    MP->root = NULL;
    MP->size = 0;

    return M;
}
