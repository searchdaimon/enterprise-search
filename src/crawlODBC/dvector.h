/**
 *	(C) Copyleft 2006, Magnus Galåen
 *
 *	dvector.h: Vector-container.
 */

#ifndef _DVECTOR_H_
#define _DVECTOR_H_

#include <stdarg.h>

#include "dcontainer.h"


container* vector_container( container *C );

extern inline int vector_size( container *C );
//value vector_get( container *C, int id );
extern inline void vector_pushback( container *C, ... );


extern inline value vector_get( container *C, int id );
/*
{
    return ((vector_container_priv*)C->priv)->elem[id];
}
*/
#endif	// _DVECTOR_H_
