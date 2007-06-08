
/**
 *	(C) Copyright 2006, Magnus Galåen
 *
 *	dpair.h: Pair-container.
 */

#ifndef _DPAIR_H_
#define _DPAIR_H_


#include "dcontainer.h"


#define pair(_var) (*((pair_value*)_var.ptr))

/* pair_container: */

typedef struct
{
    value	first, second;
} pair_value;


/**
 *	Returnerer container-struktur for pair basert på de to input-containerne.
 */
container* pair_container( container *A, container *B );


#endif	// _DPAIR_H_
