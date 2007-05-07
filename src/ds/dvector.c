
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "dvector.h"

typedef struct
{
    container	*C;
    value	*elem;
    int		size;
    int		_array_size;
} vector_container_priv;



void vector_destroy( container *C )
{
    vector_container_priv	*VP = C->priv;
    int				i;

    for (i=0; i<VP->size; i++)
	deallocate( VP->C, VP->elem[i] );

    free(VP->elem);
    destroy(VP->C);
    free(VP);
    free(C);
}

inline void vector_pushback( container *C, ... )
{
    va_list			ap;
    alloc_data			ad;
    vector_container_priv	*VP = C->priv;

    if (VP->size >= VP->_array_size)
	{
	    value	*old_elem = VP->elem;
	    int		old_size = VP->_array_size;

	    VP->_array_size*= 2;
	    VP->elem = malloc(sizeof(value) * VP->_array_size);
	    memcpy( VP->elem, old_elem, sizeof(value) * old_size );
	    free(old_elem);
	}

    va_start(ap, C);

    ad = VP->C->ap_allocate( VP->C, ap );
    VP->elem[VP->size] = ad.v;
    VP->size++;

    va_end(ad.ap);
}


inline value vector_get( container *C, int id )
{
    return ((vector_container_priv*)C->priv)->elem[id];
}


inline void vector_set( container *C, int id, value v )
{
    ((vector_container_priv*)C->priv)->elem[id] = v;
}


inline int vector_size( container *C )
{
    return ((vector_container_priv*)C->priv)->size;
}



container* vector_container( container *C )
{
    container			*V = malloc(sizeof(container));
    vector_container_priv	*VP = malloc(sizeof(vector_container_priv));

    V->compare = NULL;
    V->ap_allocate = NULL;
    V->deallocate = NULL;
    V->destroy = vector_destroy;
    V->priv = VP;

    VP->C = C;
    VP->size = 0;
    VP->_array_size = 8; // We start the vector with a size of 8 units and increase it if necessary.
    VP->elem = malloc(sizeof(value) * VP->_array_size);

    return V;
}
