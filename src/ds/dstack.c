
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dstack.h"


typedef struct
{
    container	*C;
    value	*elem;
    int		size, top;
} stack_container_priv;


inline alloc_data stack_ap_allocate( container *C, va_list ap )
{
    container	*N = C->clone(C);
    alloc_data	x;

    x.v.C = N;
    va_copy(x.ap, ap);

    return x;
}


inline void stack_deallocate( container *C, value a )
{
    destroy(a.C);
}


void stack_destroy( container *C )
{
    stack_container_priv	*S = C->priv;
    int				i;

    for (i=0; i<S->top; i++)
	deallocate( S->C, S->elem[i] );

    free(S->elem);
    destroy(S->C);
    free(S);
    free(C);
}


inline void stack_clear( container *C )
{
    stack_container_priv	*S = C->priv;
    int				i;

    for (i=0; i<S->top; i++)
	deallocate( S->C, S->elem[i] );

    free(S->elem);

    S->top = 0;
    S->size = 64;
    S->elem = malloc(sizeof(value) * S->size);
}


void stack_push( container *C, ... )
{
    va_list			ap;
    alloc_data			ad;
    stack_container_priv	*S = C->priv;

    if (S->top >= S->size)
	{
	    value	*old_elem = S->elem;
	    int		old_size = S->size;

	    S->size*= 2;
	    S->elem = malloc(sizeof(value) * S->size);
	    memcpy( S->elem, old_elem, sizeof(value) * old_size );
	    free(old_elem);
	}

    va_start(ap, C);

    ad = S->C->ap_allocate( S->C, ap );
    S->elem[S->top] = ad.v;
    S->top++;

    va_end(ad.ap);
}

/*
value stack_pop( container *C )
{
    stack_container_priv	*S = C->priv;

    if (S->top==0)
	{
	    fprintf(stderr, "stack_pop: Error! Attempting to pop value from empty stack!\n");

	    return (value)NULL;
	}

    S->top--;
    return S->elem[S->top];
}
*/
void stack_pop( container *C )
{
    stack_container_priv	*S = C->priv;

    if (S->top==0)
	{
	    fprintf(stderr, "stack_pop: Error! Attempting to pop value from empty stack!\n");

	    return;
	}

    S->top--;
    deallocate(S->C, S->elem[S->top]);
}


value stack_peak( container *C )
{
    stack_container_priv	*S = C->priv;

    if (S->top==0)
	{
	    printf("stack_peak: Error! Attempting to peak value from empty stack!\n");

	    return (value)NULL;
	}

    return S->elem[S->top -1];
}

/****/
inline value stack_get( container *C, int id )
{
    return ((stack_container_priv*)C->priv)->elem[id];
}
/****/

int stack_size( container *C )
{
    return ((stack_container_priv*)C->priv)->top;
}


inline container* stack_clone( container *C )
{
    stack_container_priv	*LP = C->priv;
    container			*N = LP->C->clone(LP->C);
    return stack_container(N);
}

inline value stack_copy( container *C, value a )
{
    fprintf(stderr, "Function not implemented yet: stack_copy\n");
    exit(-1);
}

inline void stack_print( container *C, value a )
{
    int		i, p;
    stack_container_priv	*S = a.C->priv;

    p = 0;

    printf("(");
    for (i=0; i<S->top; i++)
	{
	    if (p==0) p = 1;
	    else printf(" ");

	    printv(S->C, S->elem[i]);
	}
    printf(")");
}


container* stack_container( container *C )
{
    container			*S = malloc(sizeof(container));
    stack_container_priv	*SP = malloc(sizeof(stack_container_priv));

    S->compare = NULL;
    S->ap_allocate = stack_ap_allocate;
    S->deallocate = stack_deallocate;
    S->destroy = stack_destroy;
    S->clear = stack_clear;
    S->clone = stack_clone;
    S->copy = stack_copy;
    S->print = stack_print;
    S->priv = SP;

    SP->C = C;
    SP->top = 0;
    SP->size = 64; // We start the stack with a size of 8 units and increase it if necessary.
    SP->elem = malloc(sizeof(value) * SP->size);

    return S;
}
