
/**
 *	(C) Copyright 2006-2008, Magnus Galåen
 *
 *	dtuple.h: n-tuple-container.
 */

#ifndef _DTUPLE_H_
#define _DTUPLE_H_


#include "dcontainer.h"


#define tuple(_var) (*((tuple_value*)_var.ptr))

/* tuple_container: */

typedef struct
{
    value	*element;
} tuple_value;


/**
 *	Returns the container struct for a tuple based on the input containers.
 */
container* tuple_container( int n, ... );


#endif	// _DTUPLE_H_
