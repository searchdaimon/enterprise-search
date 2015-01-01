
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "dcontainer.h"
#include "../common/bprint.h"


/* int_container: */
#ifndef DEBUG
static inline 
#endif
int int_compare( container *C, value a, value b )
{
    if (a.i < b.i)
	return -1;
    else if (a.i > b.i)
	return 1;
    else
	return 0;
}

#ifndef DEBUG
static inline 
#endif
alloc_data int_ap_allocate( container *C, va_list ap )
{
    alloc_data	x;

    x.v.i = va_arg(ap, int);
//    printf("allocated %i\n", x.v.i);

    va_copy(x.ap, ap);

    return x;
}

#ifndef DEBUG
static inline 
#endif
void int_deallocate( container *C, value a )
{
}

#ifndef DEBUG
inline 
#endif
void int_destroy( container *C )
{
    free(C);
}

#ifndef DEBUG
inline 
#endif
void int_clear( container *C )
{
}

#ifndef DEBUG
inline 
#endif
container* int_clone( container *C )
{
    return int_container();
}

#ifndef DEBUG
inline 
#endif
value int_copy( container *C, value a )
{
    return a;
}

#ifndef DEBUG
inline 
#endif
void int_print( container *C, value a )
{
    printf("%i", a.i);
}

container* int_container()
{
    container	*C = (container*)malloc(sizeof(container));

    C->compare = int_compare;
    C->ap_allocate = int_ap_allocate;
    C->deallocate = int_deallocate;
    C->destroy = int_destroy;
    C->clear = int_clear;
    C->clone = int_clone;
    C->copy = int_copy;
    C->print = int_print;
    C->priv = NULL;

    return C;
}


/* string_container: */
#ifndef DEBUG
static inline 
#endif
int string_compare( container *C, value a, value b )
{
    return strcmp((char*)a.ptr, (char*)b.ptr);
}

#ifndef DEBUG
inline 
#endif
alloc_data string_ap_allocate( container *C, va_list ap )
{
    alloc_data	x;
    char	*c = va_arg(ap, char*);

    if (c==NULL)
	x.v.ptr = NULL;
    else
	x.v.ptr = strdup(c);
//    if (SC_DEBUG) printf(" [allocated %s] ", (char*)x.v.ptr);

    va_copy(x.ap,ap);

    return x;
}

#ifndef DEBUG
inline 
#endif
void string_deallocate( container *C, value a )
{
    if (a.ptr!=NULL)
        free(a.ptr);
}

inline void string_destroy( container *C )
{
    free(C);
}

inline void string_clear( container *C )
{
}

inline container* string_clone( container *C )
{
    return string_container();
}

inline value string_copy( container *C, value a )
{
    value	v;
    v.ptr = strdup(a.ptr);
    return v;
}

inline void string_print( container *C, value a )
{
    printf("\"%s\"", (char*)a.ptr);
}

inline void string_bprint( container *C, buffer *B, char *delim, value a )
{
    bprintf(B, "%s", (char*)a.ptr);
}

container* string_container()
{
    container	*C = (container*)malloc(sizeof(container));

    C->compare = string_compare;
    C->ap_allocate = string_ap_allocate;
    C->deallocate = string_deallocate;
    C->destroy = string_destroy;
    C->clear = string_clear;
    C->clone = string_clone;
    C->copy = string_copy;
    C->print = string_print;
    C->bprint = string_bprint;
    C->priv = NULL;

    return C;
}


/* ptr_container: */
#ifndef DEBUG
static inline 
#endif
int ptr_compare( container *C, value a, value b )
{
    if (a.i < b.i)
	return -1;
    else if (a.i > b.i)
	return 1;
    else
	return 0;
}

#ifndef DEBUG
static inline 
#endif
alloc_data ptr_ap_allocate( container *C, va_list ap )
{
    alloc_data	x;

    x.v.ptr = va_arg(ap, void*);
//    printf("allocated %i\n", x.v.i);

    va_copy(x.ap, ap);

    return x;
}


static inline void ptr_deallocate( container *C, value a )
{
}

inline void ptr_destroy( container *C )
{
    free(C);
}

inline void ptr_clear( container *C )
{
}

inline container* ptr_clone( container *C )
{
    return ptr_container();
}

inline value ptr_copy( container *C, value a )
{
    return a;
}

inline void ptr_print( container *C, value a )
{
    printf("(ptr:%x)", a.i);
}

container* ptr_container()
{
    container	*C = (container*)malloc(sizeof(container));

    C->compare = ptr_compare;
    C->ap_allocate = ptr_ap_allocate;
    C->deallocate = ptr_deallocate;
    C->destroy = ptr_destroy;
    C->clear = ptr_clear;
    C->clone = ptr_clone;
    C->copy = ptr_copy;
    C->print = ptr_print;
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
    va_copy(ap, ad.ap);

    va_end(ap);

    return v;
}

void deallocate( container *C, value v )
{
    C->deallocate(C,v);
}

void destroy( container *C )
{
    if (C!=NULL) C->destroy(C);
}

void clear( container *C )
{
    C->clear(C);
}

inline void print( container *C )
{
    C->print(C, container_value(C));
}

inline void printv( container *C, value v )
{
    C->print(C, v);
}

inline void println( container *C )
{
    C->print(C, container_value(C));
    printf("\n");
}

char* asprint( container *C, char *delim )
{
    buffer	*B = buffer_init(-1);

    C->bprint(C, B, delim, container_value(C));

    return buffer_exit(B);
}

inline void bprintv( container *C, buffer *B, char *delim, value v )
{
    C->bprint(C, B, delim, v);
}

/*
void destroy_iterator( iterator *it )
{
    it->destroy(it);
}
*/

inline container* ds_intersection( iterator2 a, iterator2 b )
{
    container	*C = a.C;
    container	*R = C->clone(C);

    while (a.valid && b.valid)
	{
	    int cmp = a.compare_keys(C, ds_key(a), ds_key(b));

	    if (cmp==0)
		{
		    a.insert(R, ds_key(a));
		    ds_next(a);
		    ds_next(b);
		}
	    else if (cmp < 0)
		{
		    ds_next(a);
		}
	    else
		{
		    ds_next(b);
		}
	}

    return R;
}


inline container* ds_union( iterator2 a, iterator2 b )
{
    container	*C = a.C;
    container	*R = C->clone(C);

    while (a.valid && b.valid)
	{
	    int cmp = a.compare_keys(C, ds_key(a), ds_key(b));

	    if (cmp==0)
		{
		    a.insert(R, ds_key(a));
		    ds_next(a);
		    ds_next(b);
		}
	    else if (cmp < 0)
		{
		    a.insert(R, ds_key(a));
		    ds_next(a);
		}
	    else
		{
		    a.insert(R, ds_key(b));
		    ds_next(b);
		}
	}

    for (;a.valid; ds_next(a)) a.insert(R, ds_key(a));
    for (;b.valid; ds_next(b)) a.insert(R, ds_key(b));

    return R;
}

