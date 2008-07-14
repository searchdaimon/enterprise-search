
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "dtuple.h"


/* tuple_container: */

typedef struct
{
    int		n;
    container	**C;
} tuple_container_priv;


int tuple_compare( container *C, value a, value b )
{
    int		i, ret;

    for (i=0; i<((tuple_container_priv*)C->priv)->n; i++)
	{
	    ret = ds_compare( ((tuple_container_priv*)C->priv)->C[i], tuple(a).element[i], tuple(b).element[i] );

	    if (ret != 0)
		return ret;
	}

    return 0;
}

alloc_data tuple_ap_allocate( container *C, va_list ap )
{
    alloc_data	zap;
    value	v;
    int		i;
    tuple_container_priv	*T = (tuple_container_priv*)C->priv;
    alloc_data	ad;

    v.ptr = (void*)malloc(sizeof(tuple_value));
    ((tuple_value*)v.ptr)->element = malloc(sizeof(value) * T->n);

    for (i=0; i<T->n; i++)
	{
	    ad = T->C[i]->ap_allocate(T->C[i], ap);
	    ((tuple_value*)v.ptr)->element[i] = ad.v;
	    ap = ad.ap;
	}

    zap.v = v;
    va_copy(zap.ap, ap);

    return zap;
}

void tuple_deallocate( container *C, value v )
{
    int		i;
    tuple_container_priv	*T = (tuple_container_priv*)C->priv;

    for (i=0; i<T->n; i++)
	{
	    T->C[i]->deallocate(T->C[i], ((tuple_value*)v.ptr)->element[i]);
	}

    free(((tuple_value*)v.ptr)->element);
    free(v.ptr);
}

void tuple_destroy( container *C )
{
    int		i;
    tuple_container_priv	*T = (tuple_container_priv*)C->priv;

    for (i=0; i<T->n; i++)
	destroy(T->C[i]);

    free(T->C);
    free(C->priv);
    free(C);
}

inline void tuple_clear( container *C )
{
}

inline container* tuple_clone( container *C )
{
    fprintf(stderr, "Function not implemented yet: tuple_clone\n");
    exit(-1);
/*
    tuple_container_priv	*LP = C->priv;
    container		*N = LP->A->clone(LP->A),
			*M = LP->B->clone(LP->B);
    return tuple_container(N,M);
*/
}


inline value tuple_copy( container *C, value a )
{
    fprintf(stderr, "Function not implemented yet: tuple_copy\n");
    exit(-1);
}

inline void tuple_print( container *C, value v )
{
    int		i;
    printf("(");
    for (i=0; i<((tuple_container_priv*)C->priv)->n; i++)
	{
	    if (i>0) printf(":");
	    print(((tuple_container_priv*)C->priv)->C[i], tuple(v).element[i]);
	}
    printf(")");
}

container* tuple_container( int n, ... )
{
    container	*C = (container*)malloc(sizeof(container));

    C->compare = tuple_compare;
    C->ap_allocate = tuple_ap_allocate;
    C->deallocate = tuple_deallocate;
    C->destroy = tuple_destroy;
    C->clear = tuple_clear;
    C->clone = tuple_clone;
    C->copy = tuple_copy;
    C->print = tuple_print;
    C->priv = (void*)malloc(sizeof(tuple_container_priv));

    va_list	ap;
    int		i;
    va_start(ap, n);

    ((tuple_container_priv*)C->priv)->n = n;
    ((tuple_container_priv*)C->priv)->C = malloc(sizeof(container*) * n);

    for (i=0; i<n; i++)
        ((tuple_container_priv*)C->priv)->C[i] = va_arg(ap, container*);

    va_end(ap);

    return C;
}

