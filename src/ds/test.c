
#include <stdio.h>

#include "dcontainer.h"
#include "dpair.h"
#include "dvector.h"
#include "dstack.h"
#include "dqueue.h"
#include "dlist.h"


/* main: */

int main()
{
    container	*L = list_container( int_container() );

    list_pushback(L, 5);
    list_pushfront(L, 3);
    list_insert(L, list_begin(L)->next, 4);
    list_pushback(L, 6);
    list_pushfront(L, 1);
    list_insert(L, list_begin(L)->next, 2);

    list_iterator *it = list_begin(L);

    printf("list(%i): ", list_size(L));
    int		del;
    for (del=0; it!=NULL; del++)
	{
	    list_iterator	*gammel_it = it;
	    printf("%i ", it->x.i);
	    it = it->next;
	    if (del&1) list_erase(L, gammel_it);
	}
    printf(" |  ");
    for (it=list_end(L); it!=NULL; it=it->previous)
	{
	    printf("%i ", it->x.i);
	}
    printf("\n");

    container	*I = int_container();
    value	v;

    v = allocate(I, 1);
    printf("%i\n", v.i);
    deallocate(I, v);
    destroy(I);

    container	*P = pair_container( int_container(), int_container() );

    v = allocate(P, 3, 5);
    printf("%i %i\n", pair(v).first.i, pair(v).second.i);
    deallocate(P, v);
    destroy(P);

    container	*P2 = pair_container( pair_container(int_container(), int_container()), int_container() );

    v = allocate(P2, 4, 5, 6);

    printf("%i %i %i\n",
	pair( pair(v).first).first.i,
	pair( pair(v).first).second.i,
	pair( v).second.i );

    deallocate(P2, v);
    destroy(P2);
/*
    container	*A, *B;
    value	a, b;

    A = pair_container( int_container(), pair_container( int_container(), int_container() ));
    B = pair_container( int_container(), pair_container( int_container(), int_container() ));

    a = allocate(A, 6, 5, 700);
    b = allocate(B, 6, 5, 90);

    int		ret = compare(A, a, b);

    if (ret==0)
	printf("A == B\n");
    else
	printf("A %c B\n", (ret > 0 ? '>' : '<'));
*/

    container	*V = vector_container( pair_container( string_container(), int_container() ) );
    int		i;

    vector_pushback(V, "aa", 5);
    vector_pushback(V, "b", 6);
    vector_pushback(V, "ccc", 7);
    vector_pushback(V, "defghi", 9);

    for (i=0; i<vector_size(V); i++)
	printf("%s %i\n", (char*)pair(vector_get(V, i)).first.ptr,
			pair(vector_get(V, i)).second.i );

    destroy(V);


    container	*S = stack_container( int_container() );

    stack_push(S, 6);
    stack_push(S, 5);
    stack_push(S, 3);
    stack_push(S, 4);
    stack_push(S, 2);
    stack_push(S, 1);
    stack_push(S, -8);

    while (stack_size(S)>0)
	{
	    printf("%i\n", stack_peak(S).i);
	    stack_pop(S);
	}

    destroy(S);
/*
    {
	container	*V = vector_container( int_container() );
	int		i;

	for (i=0; i<4000000; i++)
	    vector_pushback(V, i);

	for (i=0; i<vector_size(V); i++)
	    vector_get(V, i);
    }
*/

    printf("---\n");

    {
        container	*Q = queue_container( string_container() );
	int		i;

        queue_push(Q, "alfa");
        queue_push(Q, "beta");
        queue_push(Q, "3");
        queue_push(Q, "oto");
        queue_push(Q, "gamma");
        queue_push(Q, "nils");

	for (i=0; i<5; i++)
	    {
		printf("%s\n", (char*)queue_peak(Q).ptr);
		queue_pop(Q);
	    }

	queue_push(Q, "dette");
	queue_push(Q, "er");
	queue_push(Q, "noe");
	queue_push(Q, "vi");
	queue_push(Q, "la");
	queue_push(Q, "til");
	queue_push(Q, "helt");
	queue_push(Q, "p");
	queue_push(Q, "slutten");

	while (queue_size(Q)>0)
	    {
		printf("%s\n", (char*)queue_peak(Q).ptr);
		queue_pop(Q);
	    }
    }

    return 0;
}
