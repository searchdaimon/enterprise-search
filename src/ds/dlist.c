
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "dlist.h"




inline alloc_data list_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    va_copy(x.ap, ap);

    return x;
}


inline void list_deallocate( container *C, value a )
{
    destroy(a.C);
}


void _list_destroy( container *C )
{
    list_container_priv	*LP = C->priv;
    _list_node_		*item;

    item = LP->head;
    for (item=LP->head; item!=NULL;)
	{
	    _list_node_		*crnt = item;
	    item = item->next;

	    deallocate( LP->C, crnt->val );
	    free(crnt);
	}

    destroy(LP->C);
    free(LP);
    free(C);
}


void list_clear( container *C )
{
    list_container_priv	*LP = C->priv;
    _list_node_		*item;

    item = LP->head;
    for (item=LP->head; item!=NULL;)
	{
	    _list_node_		*crnt = item;
	    item = item->next;

	    deallocate( LP->C, crnt->val );
	    free(crnt);
	}

    LP->size = 0;
    LP->head = NULL;
    LP->tail = NULL;
}


inline void list_empty( container *C )
{
    list_container_priv	*LP = C->priv;
    _list_node_		*item;

    item = LP->head;
    for (item=LP->head; item!=NULL;)
	{
	    _list_node_		*crnt = item;
	    item = item->next;

	    deallocate( LP->C, crnt->val );
	    free(crnt);
	}

    LP->head = NULL;
    LP->tail = NULL;
    LP->size = 0;
}


inline void list_pushback( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    list_container_priv	*LP = C->priv;

    if (LP->size==0)
	{
	    LP->tail = malloc(sizeof(_list_node_));
	    LP->tail->previous = NULL;
	    LP->head = LP->tail;
	    LP->head->next = NULL;
	    LP->head->previous = NULL;
	}
    else if (LP->size==1)
	{
	    LP->tail = malloc(sizeof(_list_node_));
	    LP->tail->previous = LP->head;
	    LP->head->next = LP->tail;
	}
    else
	{
	    _list_node_		*previous = LP->tail;
	    LP->tail = malloc(sizeof(_list_node_));
	    LP->tail->previous = previous;
	    previous->next = LP->tail;
	}

    va_start(ap, C);

    ad = LP->C->ap_allocate( LP->C, ap );
    LP->tail->val = ad.v;
    LP->tail->next = NULL;
    LP->size++;

    va_end(ad.ap);
}


inline void list_pushback_value( container *C, value v )
{
    list_container_priv	*LP = C->priv;

    if (LP->size==0)
	{
	    LP->tail = malloc(sizeof(_list_node_));
	    LP->tail->previous = NULL;
	    LP->head = LP->tail;
	    LP->head->next = NULL;
	    LP->head->previous = NULL;
	}
    else if (LP->size==1)
	{
	    LP->tail = malloc(sizeof(_list_node_));
	    LP->tail->previous = LP->head;
	    LP->head->next = LP->tail;
	}
    else
	{
	    _list_node_		*previous = LP->tail;
	    LP->tail = malloc(sizeof(_list_node_));
	    LP->tail->previous = previous;
	    previous->next = LP->tail;
	}

    LP->tail->val = v;
    LP->tail->next = NULL;
    LP->size++;
}


inline void list_pushfront( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    list_container_priv	*LP = C->priv;

    if (LP->size==0)
	{
	    LP->head = malloc(sizeof(_list_node_));
	    LP->head->next = NULL;
	    LP->tail = LP->head;
	    LP->tail->next = NULL;
	    LP->tail->previous = NULL;
	}
    else if (LP->size==1)
	{
	    LP->head = malloc(sizeof(_list_node_));
	    LP->head->next = LP->tail;
	    LP->tail->previous = LP->head;
	}
    else
	{
	    _list_node_		*next = LP->head;
	    LP->head = malloc(sizeof(_list_node_));
	    LP->head->next = next;
	    next->previous = LP->head;
	}

    va_start(ap, C);

    ad = LP->C->ap_allocate( LP->C, ap );
    LP->head->val = ad.v;
    LP->head->previous = NULL;
    LP->size++;

    va_end(ad.ap);
}


inline void list_insert( container *C, const iterator it, ... )
{
    va_list		ap;
    alloc_data		ad;
    list_container_priv	*LP = C->priv;
    _list_node_		*m = malloc(sizeof(_list_node_));

    if (LP->size==0)
	{
	    LP->head = m;
	    LP->head->next = NULL;
	    LP->head->previous = NULL;
	    LP->tail = m;
	    LP->tail->next = NULL;
	    LP->tail->previous = NULL;
	}
    else
	{
	    _list_node_		*previous = ((_list_node_*)it.node)->previous;

	    if (previous!=NULL) previous->next = m;
	    m->previous = previous;
	    m->next = it.node;
	    ((_list_node_*)it.node)->previous = m;
	}

    va_start(ap, it);

    ad = LP->C->ap_allocate( LP->C, ap );
    m->val = ad.v;
    LP->size++;

    va_end(ad.ap);
}


inline void list_erase( container *C, const iterator it )
{
    list_container_priv	*LP = C->priv;

    if (((_list_node_*)it.node)->previous==NULL)
	{
	    LP->head = ((_list_node_*)it.node)->next;
	}
    else
	{
	    ((_list_node_*)it.node)->previous->next = ((_list_node_*)it.node)->next;
	}

    if (((_list_node_*)it.node)->next==NULL)
	{
	    LP->tail = ((_list_node_*)it.node)->previous;
	}
    else
	{
	    ((_list_node_*)it.node)->next->previous = ((_list_node_*)it.node)->previous;
	}

    free(it.node);
    LP->size--;
}

/*
inline iterator list_begin( container *C )
{
    iterator	it;

    it.node = ((list_container_priv*)C->priv)->head;
    it.valid = (it.node==NULL ? 0 : 1);

    return it;
}

inline iterator list_end( container *C )
{
    iterator	it;

    it.node = ((list_container_priv*)C->priv)->tail;
    it.valid = (it.node==NULL ? 0 : 1);

    return it;
}


inline iterator list_next( const iterator old_it )
{
    iterator	it;

    it.node = ((_list_node_*)old_it.node)->next;
    if (it.node == NULL) it.valid = 0;
    else it.valid = 1;

    return it;
}


inline iterator list_previous( const iterator old_it )
{
    iterator	it;

    it.node = ((_list_node_*)old_it.node)->previous;
    if (it.node == NULL) it.valid = 0;
    else it.valid = 1;

    return it;
}


inline int list_size( container *C )
{
    return ((list_container_priv*)C->priv)->size;
}
*/

inline container* list_clone( container *C )
{
    list_container_priv	*LP = C->priv;
    container		*N = LP->C->clone(LP->C);
    return list_container(N);
}


inline value list_copy( container *C, value a )
{
    value	v;

    fprintf(stderr, "Function not implemented yet: list_copy\n");
    exit(-1);
//    assert(1==2);
//    container	*N = list_clone(C);
    //!!!
    return v;
}


inline void list_print( container *C, value a )
{
    int		i=0;
    iterator	it = list_begin(a.C);

    printf("(");
    for (; it.valid; it=list_next(it))
	{
	    if (i==0) i++;
	    else printf(" ");

	    print(((list_container_priv*)C->priv)->C, list_val(it));
	}
    printf(")");
}


container* list_container( container *C )
{
    container			*L = malloc(sizeof(container));
    list_container_priv		*LP = malloc(sizeof(list_container_priv));

    L->compare = NULL;
    L->ap_allocate = list_ap_allocate;
    L->deallocate = list_deallocate;
    L->destroy = _list_destroy;
    L->clear = list_clear;
    L->clone = list_clone;
    L->copy = list_copy;
    L->print = list_print;
    L->priv = LP;

    LP->C = C;
    LP->size = 0;
    LP->head = NULL;
    LP->tail = NULL;

    return L;
}
