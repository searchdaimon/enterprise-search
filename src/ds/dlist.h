/**
 *	(C) Copyright 2007, Magnus Galåen
 *
 *	dlist.h: List-container.
 */

#ifndef _DLIST_H_
#define _DLIST_H_

#include <stdarg.h>

#include "dcontainer.h"


#define list_node(_var) ((_list_node_*)(_var).node)


/*
typedef struct list_iterator list_iterator;

struct list_iterator
{
    value		val;
    list_iterator	*previous, *next;
};
*/
typedef struct _list_node_ _list_node_;

struct _list_node_
{
    value		val;
    _list_node_		*previous, *next;
};


container* list_container( container *C );

extern inline int list_size( container *C );

extern inline void list_pushback( container *C, ... );
extern inline void list_pushfront( container *C, ... );
extern inline void list_insert( container *C, const iterator it, ... );

extern inline void list_pushback_value( container *C, value v );

extern inline void list_erase( container *C, const iterator it );
extern inline void list_empty( container *C );

extern inline iterator list_begin( container *C );
extern inline iterator list_end( container *C );
extern inline iterator list_next( const iterator old_it );
extern inline iterator list_previous( const iterator old_it );



#endif	// _DLIST_H_
