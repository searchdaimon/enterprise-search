
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "dcontainer.h"


/* int_container: */

static inline int int_compare( container *C, value a, value b )
{
    if (a.i < b.i)
	return -1;
    else if (a.i > b.i)
	return 1;
    else
	return 0;
}


static inline alloc_data int_ap_allocate( container *C, va_list ap )
{
    alloc_data	x;

    x.v.i = va_arg(ap, int);
//    printf("allocated %i\n", x.v.i);

    x.ap = ap;

    return x;
}


static inline void int_deallocate( container *C, value a )
{
}

inline void int_destroy( container *C )
{
    free(C);
}

container* int_container()
{
    container	*C = (container*)malloc(sizeof(container));

    C->compare = int_compare;
    C->ap_allocate = int_ap_allocate;
    C->deallocate = int_deallocate;
    C->destroy = int_destroy;
    C->priv = NULL;

    return C;
}


/* string_container: */

static inline int string_compare( container *C, value a, value b )
{
    return strcmp((char*)a.ptr, (char*)b.ptr);
}


inline alloc_data string_ap_allocate( container *C, va_list ap )
{
    alloc_data	x;

    x.v.ptr = strdup( va_arg(ap, char*) );
//    printf("allocated %s\n", (char*)x.v.ptr);

    x.ap = ap;

    return x;
}


inline void string_deallocate( container *C, value a )
{
    free(a.ptr);
}

inline void string_destroy( container *C )
{
    free(C);
}

container* string_container()
{
    container	*C = (container*)malloc(sizeof(container));

    C->compare = string_compare;
    C->ap_allocate = string_ap_allocate;
    C->deallocate = string_deallocate;
    C->destroy = string_destroy;
    C->priv = NULL;

    return C;
}

/* custom_container: */
/*
alloc_data custom_ap_allocate( container *C, va_list ap )
{
    alloc_data	x;
    int		obj_size = *(int*)(C->priv);
//    void	*ptr = va_arg(ap, void*);

    printf("obj_size %i\n", obj_size);
    x.v.ptr = malloc(obj_size);
    memcpy(x.v.ptr, ap, obj_size);
    ap+= obj_size;
    printf("allocated %.20s\n", (char*)x.v.ptr);

    x.ap = ap;

    return x;
}


void custom_deallocate( container *C, value a )
{
    free(a.ptr);
}

void custom_destroy( container *C )
{
    free(C->priv);
    free(C);
}

container* custom_container(int obj_size, int(*compare)(container*, value, value))
{
    container	*C = (container*)malloc(sizeof(container));

    C->compare = compare;
    C->ap_allocate = custom_ap_allocate;
    C->deallocate = custom_deallocate;
    C->destroy = custom_destroy;
    C->priv = malloc(sizeof(int));
    *(int*)(C->priv) = obj_size;

    return C;
}
*/

/* fancy allocate: */

int ds_compare( container *C, value a, value b )
{
    return C->compare( C, a, b );
}

value allocate( container *C, ... )
{
    value	v;
    va_list	ap;
    alloc_data	ad;

    va_start(ap, C);

    ad = C->ap_allocate( C, ap );
    v = ad.v;
    ap = ad.ap;

    va_end(ap);

    return v;
}

void deallocate( container *C, value v )
{
    C->deallocate(C,v);
}

void destroy( container *C )
{
    C->destroy(C);
}

