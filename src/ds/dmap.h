/**
 *	(C) Copyleft 2006, Magnus Galåen
 *
 *	dmap.h: Map-container.
 */

#ifndef _DMAP_H_
#define _DMAP_H_

#include <stdarg.h>

#include "dcontainer.h"


container* map_container( container *C );

int map_size( container *Key, container *Value );
value map_find( container *C, value key );
void map_insert( container *C, ... );


#endif	// _DMAP_H_

/*
#ifndef _C_MAP_H_
#define _C_MAP_H_


#include "c_container.h"
#include "c_stack.h"

typedef struct map_node map_node;
typedef struct map_root map_root;
typedef struct map_iterator map_iterator;

struct map_node
{
    enum { Red, Black } color;
    map_node		*parent, *left_child, *right_child;
    data_c		key, data;
};

struct map_root
{
    map_node            *root;
    int                 size;
    container		*Ckey, *Cdata;
};

struct map_iterator
{
    map_node		*current;
    stack		*status;
};


map_root* map_allocate( container *Ckey, container *Cdata );
void map_deallocate( map_root *M );

void map_insert( map_root *M, data_c key, data_c data );
map_node* map_find( map_root *M, data_c key );

map_iterator* map_it_allocate( map_root *M );
void map_it_deallocate( map_iterator *it );
void map_it_next( map_iterator *it );


#endif	// _C_MAP_H_
*/
