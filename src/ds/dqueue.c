
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dqueue.h"


typedef struct
{
    container	*C;
    value	*elem;
    int		size, head, tail, num_elems;
} queue_container_priv;


void queue_destroy( container *C )
{
    queue_container_priv	*Q = C->priv;
//    int				i;

    while (Q->num_elems>0)
	queue_pop(C);
/*
    for (i=0; i<Q->size; i++)
	deallocate( Q->C, Q->elem[i] );
*/
    free(Q->elem);
    destroy(Q->C);
    free(Q);
    free(C);
}


void queue_push( container *C, ... )
{
    va_list			ap;
    alloc_data			ad;
    queue_container_priv	*Q = C->priv;

    if (Q->num_elems >= Q->size)
	{
	    value	*old_elem = Q->elem;
	    int		old_size = Q->size;

	    assert( Q->tail == Q->head );
//	    printf(" (oldsize:%i, newsize:%i) ", Q->size, Q->size*2); fflush(stdout);

	    Q->size*= 2;
	    Q->elem = malloc(sizeof(value) * Q->size);
	    memcpy( Q->elem, old_elem, sizeof(value) * Q->tail );
	    memcpy( Q->elem + Q->tail + old_size, old_elem + Q->head, sizeof(value) * (old_size - Q->head) );
	    Q->head = Q->tail + old_size;
	    free(old_elem);
	}

//    printf(" (push;num_elems:%i) ", Q->num_elems); fflush(stdout);

    va_start(ap, C);

    ad = Q->C->ap_allocate( Q->C, ap );
    Q->elem[Q->tail] = ad.v;
    Q->tail++;
    if (Q->tail >= Q->size) Q->tail = 0;
    Q->num_elems++;

    va_end(ad.ap);
}


void queue_pop( container *C )
{
    queue_container_priv	*Q = C->priv;
//    value			v;

    if (Q->num_elems==0)
	{
	    fprintf(stderr, "queue_pop: Error! Attempting to pop value from empty queue!\n");
//	    return (value)NULL;
	    return;
	}

    deallocate(Q->C, Q->elem[Q->head]);

    Q->head++;
    if (Q->head >= Q->size) Q->head = 0;

    Q->num_elems--;

//    printf(" (pop;num_elems:%i) ", Q->num_elems); fflush(stdout);
//    return v;
}


value queue_peak( container *C )
{
    queue_container_priv	*Q = C->priv;

    if (Q->num_elems==0)
	{
	    fprintf(stderr, "queue_peak: Error! Attempting to peak value from empty queue!\n");

	    return (value)NULL;
	}

    return Q->elem[Q->head];
}


int queue_size( container *C )
{
    return ((queue_container_priv*)C->priv)->num_elems;
}


container* queue_container( container *C )
{
    container			*QX = malloc(sizeof(container));
    queue_container_priv	*QP = malloc(sizeof(queue_container_priv));

    QX->compare = NULL;
    QX->ap_allocate = NULL;
    QX->deallocate = NULL;
    QX->destroy = queue_destroy;
    QX->priv = QP;

    QP->C = C;
    QP->head = QP->tail = QP->num_elems = 0;
    QP->size = 64; // We start the queue with a size of 64 units and increase it if necessary.
    QP->elem = malloc(sizeof(value) * QP->size);

    return QX;
}
