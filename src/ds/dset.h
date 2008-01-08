/**
 *	(C) Copyright 2006-2008, Magnus Galåen
 *
 *	dset.h: Set-container.
 */

#ifndef _DSET_H_
#define _DSET_H_

#include <stdarg.h>

#include "dcontainer.h"


#define set_key(_var) (((_set_node_*)(_var).node)->key)


typedef struct _set_node_ _set_node_;
typedef struct set_iterator set_iterator;

struct _set_node_
{
    enum { Red, Black } color;
    _set_node_		*parent, *left_child, *right_child;
    value		key;
};



container* set_container( container *Key );

extern inline int set_size( container *C );

extern inline iterator set_begin( container *C );
extern inline iterator set_end( container *C );

extern inline iterator set_next( const iterator old_it );
extern inline iterator set_previous( const iterator old_it );

extern inline iterator set_find( container *C, ... );
extern inline iterator set_find_value( container *C, value key );

extern inline iterator set_insert( container *C, ... );
extern inline iterator set_insert_value( container *C, value key );


#endif	// _DSET_H_
