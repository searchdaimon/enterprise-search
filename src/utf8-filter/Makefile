
CC=gcc -g

all:
	flex -f -8 -Pu8fl utf8-filter.l
	$(CC) -O2 -c lex.u8fl.c
	$(CC) -O2 -o utf8-filter test.c lex.u8fl.o

clean:
	rm -rf utf8-filter lex.u8fl.c lex.u8fl.o

.PHONY:	all clean
