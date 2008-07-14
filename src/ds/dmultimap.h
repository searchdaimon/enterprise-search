/**
 *	(C) Copyright 2006-2008, Magnus Galåen
 *
 *	dmultimap.h: Multimap-container.
 */

#ifndef _DMULTIMAP_H_
#define _DMULTIMAP_H_

#include <stdarg.h>

#include "dcontainer.h"


#define multimap_val(_var) (((_multimap_node_*)(_var).node)->val)
#define multimap_key(_var) (((_multimap_node_*)(_var).node)->key)


typedef struct _multimap_node_ _multimap_node_;
typedef struct multimap_iterator multimap_iterator;

#ifndef _REDBLACK_ENUM_
#define _REDBLACK_ENUM_
typedef enum { Red, Black } enum_redblack;
#endif


struct _multimap_node_
{
    enum_redblack	color;
    _multimap_node_	*parent, *left_child, *right_child;
    value		key, val;
};


container* multimap_container( container *Key, container *Data );

extern inline int multimap_size( container *C );

extern inline iterator multimap_begin( container *C );
extern inline iterator multimap_end( container *C );

extern inline iterator multimap_next( const iterator old_it );
extern inline iterator multimap_previous( const iterator old_it );

//extern inline iterator multimap_find( container *C, ... );
//extern inline iterator multimap_find_value( container *C, value key );

extern inline iterator multimap_insert( container *C, ... );
extern inline iterator multimap_insert_value( container *C, value key, value data );


#endif	// _DMULTIMAP_H_
