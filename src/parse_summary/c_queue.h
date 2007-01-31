
#ifndef _C_QUEUE_H_
#define _C_QUEUE_H_

struct
{
	int	match, pos, rpos;
} typedef match_data;

match_data Match_data( int match, int pos, int rpos )
{
    match_data	md;

    md.match = match;
    md.pos = pos;
    md.rpos = rpos;

    return md;
}

// Type of variable:
#define _C_QUEUE_TYPE	match_data
// To use queues with more than one variable, cut'n'paste and change prefix of functions.

#include <assert.h>

struct
{
    int			top, bottom, size;
    int			_array_size;
    _C_QUEUE_TYPE	*elem;
} typedef queue;

queue* queue_new();
void queue_destroy( queue *Q );
void queue_push( queue *Q, _C_QUEUE_TYPE e );
_C_QUEUE_TYPE queue_pop( queue *Q );

#define _INIT_ARRAY_SIZE	128


queue* queue_new()
{
    queue	*Q = (queue*)malloc(sizeof(queue));

    Q->top = 0;
    Q->bottom = 0;
    Q->size = 0;

    Q->_array_size = _INIT_ARRAY_SIZE;

    Q->elem = (_C_QUEUE_TYPE*)malloc(sizeof(_C_QUEUE_TYPE) * Q->_array_size);

    return Q;
}

void queue_destroy( queue *Q )
{
    while (Q->size>0) queue_pop( Q );

    free(Q->elem);
    free(Q);
}

void queue_push( queue *Q, _C_QUEUE_TYPE e )
{
    if (Q->size == Q->_array_size-1)
	{
//	    printf("doubles %i %i\n", Q->top, Q->bottom);
	    _C_QUEUE_TYPE	*replace_elem;
	    int			i, j;

	    replace_elem = (_C_QUEUE_TYPE*)malloc(sizeof(_C_QUEUE_TYPE) * Q->_array_size*2);

	    if (Q->top < Q->bottom)
		{
		    for (i=Q->top; i<Q->bottom; i++)
			replace_elem[i - Q->top] = Q->elem[i];
		}
	    else
		{
		    for (i=Q->top; i<Q->_array_size; i++)
			replace_elem[i - Q->top] = Q->elem[i];

		    j = Q->_array_size - Q->top;

		    for (i=0; i<Q->bottom; i++)
			replace_elem[j+i] = Q->elem[i];
		}

	    Q->top = 0;
	    Q->bottom = Q->size;

	    free(Q->elem);
	    Q->_array_size*= 2;
	    Q->elem = replace_elem;
	}

    Q->elem[Q->bottom] = e;
    Q->bottom++;
    if (Q->bottom >= Q->_array_size) Q->bottom = 0;
    Q->size++;
}


_C_QUEUE_TYPE queue_pop( queue *Q )
{
    _C_QUEUE_TYPE	e;

    assert(Q->size != 0);		// Dette vil kræsje programmet om køa allerede er tom.

    e = Q->elem[Q->top];
    Q->top++;
    if (Q->top >= Q->_array_size) Q->top = 0;
    Q->size--;

    return e;
}

#endif	// _C_QUEUE_H_
