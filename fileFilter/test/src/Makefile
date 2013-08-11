CFLAGS=	-W -Wall -g -O0


All: test-infinite-loop test-infinite-loop-and-data test-normal test-abort

test-infinite-loop: test-infinite-loop.c
	gcc $(CFLAGS) -o ../test-infinite-loop test-infinite-loop.c

test-infinite-loop-and-data: test-infinite-loop-and-data.c
	gcc $(CFLAGS) -o ../test-infinite-loop-and-data test-infinite-loop-and-data.c

test-normal: test-normal.c
	gcc $(CFLAGS) -o ../test-normal test-normal.c

test-abort: test-abort.c
	gcc $(CFLAGS) -o ../test-abort test-abort.c
