
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>

#include "dstack.h"
#include "dmap.h"

typedef struct map_node map_node;
typedef struct map_iterator map_iterator;

struct map_node
{
    enum { Red, Black } color;
    map_node		*parent, *left_child, *right_child;
    value		key, data;
};

typedef struct
{
    container		*Key, *Data;
    map_node            *root;
    int                 size;
} container_map_priv;

struct map_iterator
{
    map_node		*current;
    stack		*status;
};



void map_deltree( map_node *n )
{
    if (n->left_child != NULL)
	map_deltree( n->left_child );
    if (n->right_child != NULL)
	map_deltree( n->right_child );

    free( n );
}

void map_deallocate( map_root *M )
{
    map_deltree( M->root );
    free( M );
}

void map_insert( container *C, ... )
{
    container_map_priv	*M = C->priv;
    map_node		*it = M->root, *last_it = M->root;

    if (M->root == NULL)
        {
            M->root = malloc(sizeof(map_node));
            M->root->color = Black;
            M->root->parent = NULL;
            M->root->left_child = NULL;
            M->root->right_child = NULL;
            M->root->key = key;
            M->root->data = data;
            M->size++;
//            printf("map_insert: Root is %i\n", M->root->key.i);

            return;
        }

    while (it!=NULL)
        {
    	    int		cmp = M->Ckey->compare( it->key, key );

	    if (!cmp)
                {
//            	    it->data = M->et->data_conflict( it->data, data );
		    // For now, do nothing if the key already exists.
		    // It is possible to expand this to include multimap etc.
//        	    printf("map_insert: Conflict with %i\n", key.i);
                    return;
                }

            last_it = it;

            if (cmp > 0)
                it = it->left_child;
            else
                it = it->right_child;
        }

    it = (map_node*)malloc(sizeof(map_node));
    it->color = Red;
    it->parent = last_it;
    it->left_child = NULL;
    it->right_child = NULL;
    it->key = key;
    it->data = data;

    if (M->Ckey->compare( last_it->key, key ) > 0)
	{
//	    printf("map_insert: Inserting %i at left.\n", key.i);
    	    last_it->left_child = it;
    	}
    else
	{
//	    printf("map_insert: Inserting %i at right.\n", key.i);
    	    last_it->right_child = it;
        }

    M->size++;

/*
    // Red-Black-Tree:
    while (it != *root && it->parent->color == Red)
        {
        }
*/
}

/**/
void stack_push( container *C, ... )
{
    va_list			ap;
    alloc_data			ad;
    stack_container_priv	*S = C->priv;

    if (S->top >= S->size)
	{
	    value	*old_elem = S->elem;
	    int		old_size = S->size;

	    S->size*= 2;
	    S->elem = malloc(sizeof(value) * S->size);
	    memcpy( S->elem, old_elem, sizeof(value) * old_size );
	    free(old_elem);
	}

    va_start(ap, C);

    ad = S->C->ap_allocate( S->C, ap );
    S->elem[S->top] = ad.v;
    S->top++;

    va_end(ad.ap);
}

/**/

map_iterator* map_it_allocate( map_root *M )
{
    map_iterator	*it = (map_iterator*)malloc(sizeof(map_iterator));

    it->current = M->root;

    if (it->current == NULL)
	{
	    it->status = NULL;
	    return it;
	}

    it->status = stack_allocate( int_container() );
    stack_push( it->status, i2d(0) );

    map_it_next( it );

    return it;
}


void map_it_deallocate( map_iterator *it )
{
    if (it->status != NULL)
	stack_deallocate( it->status );

    free(it);
}


void map_it_next( map_iterator *it )
{
    while (it->current != NULL)
	{
	    int		s = stack_pop( it->status ).i;

	    switch (s)
		{
		    case 0:
		        stack_push( it->status, i2d(1) );
			if (it->current->left_child != NULL)
			    {
	    			it->current = it->current->left_child;
			        stack_push( it->status, i2d(0) );
	    		    }
	    		break;
	    	    case 1:
		        stack_push( it->status, i2d(2) );
	    		return;
	    	    case 2:
		        stack_push( it->status, i2d(3) );
			if (it->current->right_child != NULL)
			    {
				it->current = it->current->right_child;
			        stack_push( it->status, i2d(0) );
			    }
			break;
		    default:
			it->current = it->current->parent;
		}
	}
}


map_node* map_find( map_root *M, data_c key )
{
    map_node		*n = M->root;

    while (n!=NULL)
	{
	    int		cmp = M->Ckey->compare(n->key, key);

	    if (cmp==0)
		return n;
	    if (cmp>0)
		n = n->left_child;
	    else
		n = n->right_child;
	}

    return n;
}


int main()
{
    char	*a = "hubba bubba", *b="kaffe er drikke", *c="hadet", *d="bra";

    map_root	*M = map_allocate( str_container(), int_container() );

    map_insert( M, s2d(a), i2d(1) );
    map_insert( M, s2d(b), i2d(1) );
    map_insert( M, s2d(c), i2d(0) );
    map_insert( M, s2d(d), i2d(1) );
    map_insert( M, s2d(a), i2d(0) );
    map_insert( M, s2d(c), i2d(1) );

    map_iterator	*it = map_it_allocate( M );

    while (it->current != NULL)
	{
	    printf("%s (%s)\n", (char*)it->current->key.ptr, (it->current->data.i > 0 ? "true" : "false"));
	    map_it_next( it );
	}

    map_it_deallocate( it );

    map_deallocate( M );

    M = map_allocate( int_container(), str_container() );
    map_insert( M, i2d(8), s2d("alfa") );
    map_insert( M, i2d(19), s2d("bravo") );
    map_insert( M, i2d(2), s2d("charlie") );
    map_insert( M, i2d(7), s2d("delta") );
    map_insert( M, i2d(3), s2d("ekko") );
    map_insert( M, i2d(1), s2d("foxtrot") );
    map_insert( M, i2d(17), s2d("gamma") );
    map_insert( M, i2d(10), s2d("hohoho") );

    it = map_it_allocate( M );

    while (it->current != NULL)
	{
	    printf("%i (%s)\n", it->current->key.i, (char*)it->current->data.ptr );
	    map_it_next( it );
	}

    printf("---\n");

    map_node	*n;
    n = map_find( M, i2d(17) );
    printf("%i (%s)\n", n->key.i, (char*)n->data.ptr);
    n = map_find( M, i2d(7) );
    printf("%i (%s)\n", n->key.i, (char*)n->data.ptr);
    n = map_find( M, i2d(10) );
    printf("%i (%s)\n", n->key.i, (char*)n->data.ptr);
    n = map_find( M, i2d(13) );
    if (n==NULL) printf("NULL\n");

    map_it_deallocate( it );

    map_deallocate( M );
}



container* map_container( container *Key, container *Data )
{
    container			*MC = malloc(sizeof(container));
    map_container_priv		*M = malloc(sizeof(map_container_priv));

    MC->compare = NULL;
    MC->ap_allocate = NULL;
    MC->deallocate = NULL;
    MC->destroy = map_destroy;
    MC->priv = M;

    M->Key = Key;
    M->Data = Data;
    M->root = NULL;
    M->size = 0;

    return MC;
}
