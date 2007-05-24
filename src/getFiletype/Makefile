
CC=/usr/local/bin/gcc -O2

all:
	flex -f -P getfiletype -i getfiletype.l
	$(CC) -c -o getfiletype.o lex.getfiletype.c

test:
	$(CC) -o getfiletype test.c getfiletype.o ../ds/libds.a

clean:
	rm -rf getfiletype getfiletype.o lex.getfiletype.c
