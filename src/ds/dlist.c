
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "dlist.h"


typedef struct
{
    container		*C;
    list_iterator	*head, *tail;
    int			size;
} list_container_priv;



void list_destroy( container *C )
{
    list_container_priv	*LP = C->priv;
    list_iterator	*item;

    item = LP->head;
    for (item=LP->head; item!=NULL;)
	{
	    list_iterator	*crnt = item;
	    item = item->next;

	    deallocate( LP->C, crnt->x );
	    free(crnt);
	}

    destroy(LP->C);
    free(LP);
    free(C);
}

inline void list_pushback( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    list_container_priv	*LP = C->priv;

    if (LP->size==0)
	{
	    LP->tail = malloc(sizeof(list_iterator));
	    LP->tail->previous = NULL;
	    LP->head = LP->tail;
	    LP->head->next = NULL;
	    LP->head->previous = NULL;
	}
    else if (LP->size==1)
	{
	    LP->tail = malloc(sizeof(list_iterator));
	    LP->tail->previous = LP->head;
	    LP->head->next = LP->tail;
	}
    else
	{
	    list_iterator	*previous = LP->tail;
	    LP->tail = malloc(sizeof(list_iterator));
	    LP->tail->previous = previous;
	    previous->next = LP->tail;
	}

    va_start(ap, C);

    ad = LP->C->ap_allocate( LP->C, ap );
    LP->tail->x = ad.v;
    LP->tail->next = NULL;
    LP->size++;

    va_end(ad.ap);
}


inline void list_pushfront( container *C, ... )
{
    va_list		ap;
    alloc_data		ad;
    list_container_priv	*LP = C->priv;

    if (LP->size==0)
	{
	    LP->head = malloc(sizeof(list_iterator));
	    LP->head->next = NULL;
	    LP->tail = LP->head;
	    LP->tail->next = NULL;
	    LP->tail->previous = NULL;
	}
    else if (LP->size==1)
	{
	    LP->head = malloc(sizeof(list_iterator));
	    LP->head->next = LP->tail;
	    LP->tail->previous = LP->head;
	}
    else
	{
	    list_iterator	*next = LP->head;
	    LP->head = malloc(sizeof(list_iterator));
	    LP->head->next = next;
	    next->previous = LP->head;
	}

    va_start(ap, C);

    ad = LP->C->ap_allocate( LP->C, ap );
    LP->head->x = ad.v;
    LP->head->previous = NULL;
    LP->size++;

    va_end(ad.ap);
}


inline void list_insert( container *C, list_iterator *l, ... )
{
    va_list		ap;
    alloc_data		ad;
    list_container_priv	*LP = C->priv;
    list_iterator	*m = malloc(sizeof(list_iterator));

    if (l==NULL)
	{
	    assert(LP->size==0);
	    LP->head = m;
	    LP->head->next = NULL;
	    LP->head->previous = NULL;
	    LP->tail = m;
	    LP->tail->next = NULL;
	    LP->tail->previous = NULL;
	}
    else
	{
	    list_iterator	*previous = l->previous;

	    if (previous!=NULL) previous->next = m;
	    m->previous = previous;
	    m->next = l;
	    l->previous = m;
	}

    va_start(ap, l);

    ad = LP->C->ap_allocate( LP->C, ap );
    m->x = ad.v;
    LP->size++;

    va_end(ad.ap);
}


inline void list_erase( container *C, list_iterator *l )
{
    list_container_priv	*LP = C->priv;

    if (l->previous==NULL)
	{
	    LP->head = l->next;
	}
    else
	{
	    l->previous->next = l->next;
	}

    if (l->next==NULL)
	{
	    LP->tail = l->previous;
	}
    else
	{
	    l->next->previous = l->previous;
	}

    LP->size--;
}


inline list_iterator* list_begin( container *C )
{
    return ((list_container_priv*)C->priv)->head;
}

inline list_iterator* list_end( container *C )
{
    return ((list_container_priv*)C->priv)->tail;
}


inline int list_size( container *C )
{
    return ((list_container_priv*)C->priv)->size;
}



container* list_container( container *C )
{
    container			*L = malloc(sizeof(container));
    list_container_priv		*LP = malloc(sizeof(list_container_priv));

    L->compare = NULL;
    L->ap_allocate = NULL;
    L->deallocate = NULL;
    L->destroy = list_destroy;
    L->priv = LP;

    LP->C = C;
    LP->size = 0;
    LP->head = NULL;
    LP->tail = NULL;

    return L;
}
