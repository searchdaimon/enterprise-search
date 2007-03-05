/**
 *	(C) Copyleft 2006, Magnus Galåen
 *
 *	dstack.h: Stack-container.
 */

#ifndef _DSTACK_H_
#define _DSTACK_H_

#include <stdarg.h>

#include "dcontainer.h"


container* stack_container( container *C );

int stack_size( container *C );
value stack_pop( container *C );
value stack_peak( container *C );
void stack_push( container *C, ... );


#endif	// _DSTACK_H_
