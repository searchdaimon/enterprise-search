
CC=/usr/local/bin/gcc -Wall -O2

all:
	$(CC) -c dcontainer.c
	$(CC) -c dpair.c
	$(CC) -c dvector.c
	$(CC) -c dstack.c
	$(CC) -c dqueue.c
	ar rc libds.a dcontainer.o dpair.o dvector.o dstack.o dqueue.o
	ranlib libds.a

test:
	$(CC) -o test test.c libds.a

clean:
	rm -rf dcontainer.o dpair.o dvector.o dstack.o dqueue.o test libds.a

.PHONY:	all clean
