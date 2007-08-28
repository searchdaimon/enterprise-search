
/**
 *	(C) Copyright 2006-2007, Magnus Galåen
 *
 *	dcontainer.h: Basic containers.
 */

#ifndef _DCONTAINER_H_
#define _DCONTAINER_H_


#include <stdarg.h>
#include <string.h>

//#define ex(_type,_var) (*((_type##_value*)_var.ptr))

/*
    La oss kalle datastrukturer for containere.
    Containere skal kunne inneholde andre containere.
    Maa ha allocate og deallocate.
    Og compare.
*/

typedef struct container container;
typedef struct iterator iterator;

typedef union
{
    int		i;
    char	c;
    double	d;
    void	*ptr;
    char	*str;
    container	*C;
} value;

typedef struct alloc_data
{
    value	v;
    va_list	ap;
} alloc_data;


struct container
{
    int		(*compare)( container *C, value a, value b );
    alloc_data	(*ap_allocate)( container *C, va_list ap );
    void	(*deallocate)( container *C, value a );
    void	(*destroy)( container *C );
    container*	(*clone)( container *C );
    value	(*copy)( container *C, value a );
    void	*priv;
};

struct iterator
{
    void	*node;
    int		valid;
};

/* fancy allocate: */

int ds_compare( container *C, value a, value b );

value allocate( container *C, ... );

void deallocate( container *C, value v );

void destroy( container *C );

static inline value copy( container *C, value v )
{
    return C->copy( C, v );
}

//void destroy_iterator( iterator *it );


/* int_container: */

container* int_container();

/* string_container: */

container* string_container();

container* ptr_container();

/* custom_container: */

//container* custom_container(int obj_size, int(*compare)(container*, value, value));


static inline value int_value( int val )
{
    value	v;
    v.i = val;
    return v;
}


static inline value string_value( char *val )
{
    value	v;
    v.ptr = strdup(val);
    return v;
}


static inline value ptr_value( void *val )
{
    value	v;
    v.ptr = val;
    return v;
}


static inline value container_value( container *val )
{
    value	v;
    v.C = val;
    return v;
}


#endif	// _DCONTAINER_H_
