
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


int stack_size( container *C )
{
    return ((stack_container_priv*)C->priv)->top;
}


container* stack_container( container *C )
{
    container			*S = malloc(sizeof(container));
    stack_container_priv	*SP = malloc(sizeof(stack_container_priv));

    S->compare = NULL;
    S->ap_allocate = NULL;
    S->deallocate = NULL;
    S->destroy = stack_destroy;
    S->priv = SP;

    SP->C = C;
    SP->top = 0;
    SP->size = 64; // We start the stack with a size of 8 units and increase it if necessary.
    SP->elem = malloc(sizeof(value) * SP->size);

    return S;
}
