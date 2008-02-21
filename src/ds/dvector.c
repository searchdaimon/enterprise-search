
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



inline int vector_compare( container *C, value a, value b )
{
    vector_container_priv	*AP = a.C->priv;
    int		val;
    int		i;
    int		size;

//    cmp_count++;

    size = vector_size(a.C);
    if (vector_size(b.C) < size) size = vector_size(b.C);

    for (i=0; i<size; i++)
	{
	    val = AP->C->compare(AP->C, vector_get(a.C,i), vector_get(b.C,i));

	    if (val!=0) return val;
	}

    if (vector_size(a.C) > size) return 1;
    else if (vector_size(b.C) > size) return -1;

    return 0;
}


inline alloc_data vector_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    va_copy(x.ap, ap);

    return x;
}


inline void vector_deallocate( container *C, value a )
{
    destroy(a.C);
}


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


inline void vector_clear( container *C )
{
    vector_container_priv	*VP = C->priv;
    int				i;

    for (i=0; i<VP->size; i++)
	deallocate( VP->C, VP->elem[i] );

    free(VP->elem);

    VP->size = 0;
    VP->_array_size = 8;
    VP->elem = malloc(sizeof(value) * VP->_array_size);
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


inline void vector_pushback_value( container *C, value val )
{
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

    VP->elem[VP->size] = val;
    VP->size++;
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


inline container* vector_clone( container *C )
{
    vector_container_priv	*LP = C->priv;
    container			*N = LP->C->clone(LP->C);
    return vector_container(N);
}

inline value vector_copy( container *C, value a )
{
    fprintf(stderr, "Function not implemented yet: vector_copy\n");
    exit(-1);
}

inline void vector_print( container *C, value a )
{
    int		i;

    printf("[");
    for (i=0; i<vector_size(a.C); i++)
	{
	    if (i>0) printf(" ");

	    print(((vector_container_priv*)C->priv)->C, vector_get(a.C,i));
	}
    printf("]");
}


container* vector_container( container *C )
{
    container			*V = malloc(sizeof(container));
    vector_container_priv	*VP = malloc(sizeof(vector_container_priv));

    V->compare = vector_compare;
    V->ap_allocate = vector_ap_allocate;
    V->deallocate = vector_deallocate;
    V->destroy = vector_destroy;
    V->clear = vector_clear;
    V->clone = vector_clone;
    V->copy = vector_copy;
    V->print = vector_print;
    V->priv = VP;

    VP->C = C;
    VP->size = 0;
    VP->_array_size = 8; // We start the vector with a size of 8 units and increase it if necessary.
    VP->elem = malloc(sizeof(value) * VP->_array_size);

    return V;
}
