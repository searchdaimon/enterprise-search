
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dcontainer.h"
#include "dmultimap.h"


typedef struct
{
    container		*Key, *Data;
    _multimap_node_	*root;
    int                 size;
} multimap_container_priv;



inline alloc_data multimap_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    va_copy(x.ap, ap);

    return x;
}


inline void multimap_deallocate( container *C, value a )
{
    destroy(a.C);
}


void multimap_deltree( container *C, _multimap_node_ *n )
{
    multimap_container_priv	*MP = C->priv;

    deallocate(MP->Key, n->key);
    deallocate(MP->Data, n->val);

    if (n->left_child != NULL)
	multimap_deltree( C, n->left_child );
    if (n->right_child != NULL)
	multimap_deltree( C, n->right_child );

    free(n);
}


void multimap_destroy( container *C )
{
    multimap_container_priv	*MP = C->priv;

    if (MP->root!=NULL) multimap_deltree( C, MP->root );
    destroy( MP->Key );
    destroy( MP->Data );
    free(MP);
    free(C);
}


void multimap_clear( container *C )
{
    multimap_container_priv	*MP = C->priv;

    if (MP->root!=NULL) multimap_deltree( C, MP->root );

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


int multimap_verify_tree( container *C, _multimap_node_ *node )
{
    if (node == NULL) return 0;

    multimap_container_priv *MP = C->priv;

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

    multimap_verify_tree(C, node->left_child);
    multimap_verify_tree(C, node->right_child);
}

int multimap_check_tree( container *C, container *hash, _multimap_node_ *node )
{
    multimap_container_priv *MP = C->priv;

    if (node == NULL) return 0;

    printf("%s_%c ", get_hash(hash, (int)node), (node->color==Black ? 'B':'R'));

    if (node->left_child == NULL) printf("(--- |");
    else printf("(%s_%c |", get_hash(hash, (int)node->left_child), (node->left_child->color==Black ? 'B':'R') );

    if (node->right_child == NULL) printf("| ---)\n");
    else printf("| %s_%c)\n", get_hash(hash, (int)node->right_child), (node->right_child->color==Black ? 'B':'R') );

    multimap_check_tree(C, hash, node->left_child);
    multimap_check_tree(C, hash, node->right_child);
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
    else black1 = multimap_check_tree(C, node->left_child);

    if (node->right_child == NULL) black2 = 1;
    else black2 = multimap_check_tree(C, node->right_child);

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

static inline void multimap_rotate_left( multimap_container_priv *MP, _multimap_node_ *x )
{
    _multimap_node_		*y;

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

static inline void multimap_rotate_right( multimap_container_priv *MP, _multimap_node_ *y )
{
    _multimap_node_		*x;

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


inline iterator multimap_insert( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    multimap_container_priv	*MP = C->priv;
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
            MP->root = malloc(sizeof(_multimap_node_));
            MP->root->color = Black;
            MP->root->parent = NULL;
            MP->root->left_child = NULL;
            MP->root->right_child = NULL;
	    MP->root->key = key;
	    MP->root->val = val;
            MP->size++;
//            printf("multimap_insert: Root is %i\n", key.i);

	    it.node = MP->root;
	    it.valid = 1;
            return it;
        }

    _multimap_node_		*node = MP->root, *last_node = MP->root;
//    printf("multimap_insert: Adding %i\n", key.i);

    while (node!=NULL)
        {
    	    int		cmp = MP->Key->compare( MP->Key, node->key, key );
/*
	    if (!cmp)
                {
		    // For now, do nothing if the key already exists.
		    // It is possible to expand this to include multimap etc.
//        	    printf("multimap_insert: Conflict with %i\n", key.i);
		    MP->Key->deallocate(MP->Key, key);
		    MP->Data->deallocate(MP->Data, val);

		    it.node = NULL;
		    it.valid = 0;
                    return it;
                }
*/
            last_node = node;

            if (cmp > 0)
//		{printf("left\n");
                node = node->left_child;
            else
//		{printf("right\n");
                node = node->right_child;
        }

    node = malloc(sizeof(_multimap_node_));
    node->color = Red;
    node->parent = last_node;
    node->left_child = NULL;
    node->right_child = NULL;
    node->key = key;
    node->val = val;

    if (MP->Key->compare( MP->Key, last_node->key, key ) > 0)
	{
//	    printf("multimap_insert: Inserting %i at left.\n", key.i);
    	    last_node->left_child = node;
    	}
    else
	{
//	    printf("multimap_insert: Inserting %i at right.\n", key.i);
    	    last_node->right_child = node;
        }

    MP->size++;
    it.node = node;
    it.valid = 1;

    return it;
}



inline iterator multimap_insert_value( container *C, value key, value val )
{
    multimap_container_priv	*MP = C->priv;
    iterator		it;
//    container		*hash = vector_container( pair_container( int_container(), string_container() ) );

    if (MP->size == 0)
        {
            MP->root = malloc(sizeof(_multimap_node_));
            MP->root->color = Black;
            MP->root->parent = NULL;
            MP->root->left_child = NULL;
            MP->root->right_child = NULL;
	    MP->root->key = key;
	    MP->root->val = val;
            MP->size++;
//            printf("multimap_insert: Root is %i\n", key.i);

	    it.node = MP->root;
	    it.valid = 1;
            return it;
        }

    _multimap_node_		*node = MP->root, *last_node = MP->root;
//    printf("multimap_insert: Adding %i\n", key.i);

    while (node!=NULL)
        {
    	    int		cmp = MP->Key->compare( MP->Key, node->key, key );
/*
	    if (!cmp)
                {
		    // For now, do nothing if the key already exists.
		    // It is possible to expand this to include multimap etc.
//        	    printf("multimap_insert: Conflict with %i\n", key.i);
		    it.node = NULL;
		    it.valid = 0;
                    return it;
                }
*/
            last_node = node;

            if (cmp > 0)
//		{printf("left\n");
                node = node->left_child;
            else
//		{printf("right\n");
                node = node->right_child;
        }

    node = malloc(sizeof(_multimap_node_));
    node->color = Red;
    node->parent = last_node;
    node->left_child = NULL;
    node->right_child = NULL;
    node->key = key;
    node->val = val;

    if (MP->Key->compare( MP->Key, last_node->key, key ) > 0)
	{
//	    printf("multimap_insert: Inserting %i at left.\n", key.i);
    	    last_node->left_child = node;
    	}
    else
	{
//	    printf("multimap_insert: Inserting %i at right.\n", key.i);
    	    last_node->right_child = node;
        }

    MP->size++;
    it.node = node;
    it.valid = 1;

    return it;
}




inline iterator multimap_next( const iterator old_it )
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
			if (((_multimap_node_*)it.node)->left_child != NULL)
			    {
				it.node = ((_multimap_node_*)it.node)->left_child;
				s = 0;
			    }
			break;
		    case 1:
			s = 2;
			return it;
		    case 2:
			s = 3;
			if (((_multimap_node_*)it.node)->right_child != NULL)
			    {
				it.node = ((_multimap_node_*)it.node)->right_child;
				s = 0;
			    }
			break;
		    default:
			if (((_multimap_node_*)it.node)->parent != NULL && it.node == ((_multimap_node_*)it.node)->parent->left_child)
			    s = 1;
			else // if (((_multimap_node_*)it.node)->parent != NULL && it.node == ((_multimap_node_*)it.node)->parent->right_child)
			    s = 3;

			it.node = ((_multimap_node_*)it.node)->parent;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator multimap_previous( const iterator old_it )
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
			if (((_multimap_node_*)it.node)->right_child != NULL)
			    {
				it.node = ((_multimap_node_*)it.node)->right_child;
				s = 0;
			    }
			break;
		    case 1:
			s = 2;
			return it;
		    case 2:
			s = 3;
			if (((_multimap_node_*)it.node)->left_child != NULL)
			    {
				it.node = ((_multimap_node_*)it.node)->left_child;
				s = 0;
			    }
			break;
		    default:
			if (((_multimap_node_*)it.node)->parent != NULL && it.node == ((_multimap_node_*)it.node)->parent->right_child)
			    s = 1;
			else // if (((_multimap_node_*)it.node)->parent != NULL && it.node == ((_multimap_node_*)it.node)->parent->left_child)
			    s = 3;

			it.node = ((_multimap_node_*)it.node)->parent;
		}
	}

    it.valid = 0;
    return it;
}


inline iterator multimap_begin( container *C )
{
    multimap_container_priv	*MP = C->priv;
    iterator		it;

    if (MP->size==0)
	{
	    it.valid = 0;
	    it.node = NULL;
	    return it;
	}

    it.node = MP->root;
    while (((_multimap_node_*)it.node)->left_child != NULL)
	it.node = ((_multimap_node_*)it.node)->left_child;;
    it.valid = 1;

    return it;
}


inline iterator multimap_end( container *C )
{
    multimap_container_priv	*MP = C->priv;
    iterator		it;

    if (MP->size==0)
	{
	    it.valid = 0;
	    it.node = NULL;
	    return it;
	}

    it.node = MP->root;
    while (((_multimap_node_*)it.node)->right_child != NULL)
	it.node = ((_multimap_node_*)it.node)->right_child;
    it.valid = 1;

    return it;
}

/*
inline iterator multimap_find_value( container *C, value key )
{
    multimap_container_priv	*MP = C->priv;
    iterator		it;

    it.node = ((multimap_container_priv*)C->priv)->root;

    while (it.node!=NULL)
	{
	    int		cmp = MP->Key->compare(MP->Key, ((_multimap_node_*)it.node)->key, key);

	    if (cmp==0)
		{
		    it.valid = 1;
		    return it;
		}
	    if (cmp>0)
		{
    		    it.node = ((_multimap_node_*)it.node)->left_child;
		}
	    else
		{
		    it.node = ((_multimap_node_*)it.node)->right_child;
		}
	}

    it.valid = 0;
    return it;
}
*/
/*
inline iterator multimap_find( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    multimap_container_priv	*MP = C->priv;
    value		key;
    iterator		it;

    va_start(ap, C);
    ad = MP->Key->ap_allocate(MP->Key, ap);
    key = ad.v;
    va_end(ap);

    it = multimap_find_value( C, key );

    deallocate(MP->Key, key);

    return it;
}
*/

inline int multimap_size( container *C )
{
    return ((multimap_container_priv*)C->priv)->size;
}


inline container* multimap_clone( container *C )
{
    multimap_container_priv	*LP = C->priv;
    container		*N = LP->Key->clone(LP->Key),
			*M = LP->Data->clone(LP->Data);
    return multimap_container(N,M);
}

inline value multimap_copy( container *C, value a )
{
    fprintf(stderr, "Function not implemented yet: multimap_copy\n");

    exit(-1);
}

inline void multimap_print( container *C, value a )
{
    int		i=0;
    iterator	it = multimap_begin(a.C);

    printf("(");
    for (; it.valid; it=multimap_next(it))
	{
	    if (i==0) i++;
	    else printf(" ");

	    printv(((multimap_container_priv*)C->priv)->Key, multimap_key(it));
	    printf(":");
	    printv(((multimap_container_priv*)C->priv)->Data, multimap_val(it));
	}
    printf(")");
}


container* multimap_container( container *Key, container *Data )
{
    container			*M = malloc(sizeof(container));
    multimap_container_priv		*MP = malloc(sizeof(multimap_container_priv));

    M->compare = NULL;
    M->ap_allocate = multimap_ap_allocate;
    M->deallocate = multimap_deallocate;
    M->destroy = multimap_destroy;
    M->clear = multimap_clear;
    M->clone = multimap_clone;
    M->copy = multimap_copy;
    M->print = multimap_print;
    M->priv = MP;

    MP->Key = Key;
    MP->Data = Data;
    MP->root = NULL;
    MP->size = 0;

    return M;
}
