/**
 *	(C) Copyright 2006-2007, Magnus Galåen
 *
 *	dmap.h: Map-container.
 */

#ifndef _DMAP_H_
#define _DMAP_H_

#include <stdarg.h>

#include "dcontainer.h"


#define map_val(_var) (((_map_node_*)(_var).node)->val)
#define map_key(_var) (((_map_node_*)(_var).node)->key)


typedef struct _map_node_ _map_node_;
typedef struct map_iterator map_iterator;

struct _map_node_
{
    enum { Red, Black } color;
    _map_node_		*parent, *left_child, *right_child;
    value		key, val;
};



container* map_container( container *Key, container *Data );

extern inline int map_size( container *C );

extern inline iterator map_begin( container *C );
extern inline iterator map_end( container *C );

extern inline iterator map_next( const iterator old_it );
extern inline iterator map_previous( const iterator old_it );

extern inline iterator map_find( container *C, ... );
extern inline iterator map_find_value( container *C, value key );

extern inline iterator map_insert( container *C, ... );
extern inline iterator map_insert_value( container *C, value key, value data );


#endif	// _DMAP_H_
