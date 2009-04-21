/**
 *	(C) Copyright 2007-2009, Magnus Galåen
 *
 *	dqueue.h: Queue-container.
 */

#ifndef _DQUEUE_H_
#define _DQUEUE_H_

#include <stdarg.h>

#include "dcontainer.h"


container* queue_container( container *C );

int queue_size( container *C );
    // Advarsel, verdiene slettes (free) når denne funksjonen kjøres:
void queue_pop( container *C );
value queue_peak( container *C );
value queue_peak_front( container *C, int index_from_first );
value queue_peak_end( container *C, int index_from_last );
void queue_push( container *C, ... );


#endif	// _DQUEUE_H_
