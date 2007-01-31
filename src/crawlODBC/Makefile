
CC=gcc -g

all:
	$(CC) -c dcontainer.c
	$(CC) -c dpair.c
	$(CC) -c dvector.c
	$(CC) -o odbc_crawl odbc_crawl.c -lodbc dcontainer.o dpair.o dvector.o

test:
	gcc -g -o test_connect test_connect.c -lodbc

clean:
	rm -rf dcontainer.o dpair.o dvector.o odbc_crawl test_connect

.PHONY:	all clean
