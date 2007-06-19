/**
 *	(C) Copyright 2007, Magnus Galåen
 *
 *	dlist.h: List-container.
 */

#ifndef _DLIST_H_
#define _DLIST_H_

#include <stdarg.h>

#include "dcontainer.h"


typedef struct list_iterator list_iterator;

struct list_iterator
{
    value		x;
    list_iterator	*previous, *next;
};


container* list_container( container *C );

extern inline int list_size( container *C );

extern inline void list_pushback( container *C, ... );
extern inline void list_pushfront( container *C, ... );
extern inline void list_insert( container *C, list_iterator *l, ... );

extern inline void list_erase( container *C, list_iterator *l );

extern inline list_iterator* list_begin( container *C );
extern inline list_iterator* list_end( container *C );



#endif	// _DLIST_H_
