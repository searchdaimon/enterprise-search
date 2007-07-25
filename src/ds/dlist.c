
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "dlist.h"



typedef struct
{
    container		*C;
    _list_node_		*head, *tail;
    int			size;
} list_container_priv;



inline alloc_data list_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    x.ap = ap;

    return x;
}


inline void list_deallocate( container *C, value a )
{
    destroy(a.C);
}


void list_destroy( container *C )
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
	    _list_node_		*previous = list_node(it)->previous;

	    if (previous!=NULL) previous->next = m;
	    m->previous = previous;
	    m->next = it.node;
	    list_node(it)->previous = m;
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

    if (list_node(it)->previous==NULL)
	{
	    LP->head = list_node(it)->next;
	}
    else
	{
	    list_node(it)->previous->next = list_node(it)->next;
	}

    if (list_node(it)->next==NULL)
	{
	    LP->tail = list_node(it)->previous;
	}
    else
	{
	    list_node(it)->next->previous = list_node(it)->previous;
	}

    LP->size--;
}


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

    it.node = list_node(old_it)->next;
    if (it.node == NULL) it.valid = 0;
    else it.valid = 1;

    return it;
}


inline iterator list_previous( const iterator old_it )
{
    iterator	it;

    it.node = list_node(old_it)->previous;
    if (it.node == NULL) it.valid = 0;
    else it.valid = 1;

    return it;
}


inline int list_size( container *C )
{
    return ((list_container_priv*)C->priv)->size;
}


inline container* list_clone( container *C )
{
    list_container_priv	*LP = C->priv;
    container		*N = LP->C->clone(LP->C);
    return list_container(N);
}


inline value list_copy( container *C, value a )
{
    value	v;
    assert(1==2);
//    container	*N = list_clone(C);
    //!!!
    return v;
}


container* list_container( container *C )
{
    container			*L = malloc(sizeof(container));
    list_container_priv		*LP = malloc(sizeof(list_container_priv));

    L->compare = NULL;
    L->ap_allocate = list_ap_allocate;
    L->deallocate = list_deallocate;
    L->destroy = list_destroy;
    L->clone = list_clone;
    L->priv = LP;

    LP->C = C;
    LP->size = 0;
    LP->head = NULL;
    LP->tail = NULL;

    return L;
}
