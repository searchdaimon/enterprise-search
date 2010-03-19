
CC=gcc -Wall -O3 -g -m32

all:
	$(CC) -c dcontainer.c
	$(CC) -c dpair.c
	$(CC) -c dtuple.c
	$(CC) -c dvector.c
	$(CC) -c dstack.c
	$(CC) -c dqueue.c
	$(CC) -c dlist.c
	$(CC) -c dmap.c
	$(CC) -c dmultimap.c
	$(CC) -c dset.c
	ar rc libds.a dcontainer.o dpair.o dtuple.o dvector.o dstack.o dqueue.o dlist.o dmap.o dmultimap.o dset.o
	ranlib libds.a

test:
	$(CC) -o test test.c libds.a

clean:
	rm -rf dcontainer.o dpair.o dtuple.o dvector.o dstack.o dqueue.o dlist.o dmap.o dmultimap.o dset.o test libds.a

.PHONY:	all clean test
