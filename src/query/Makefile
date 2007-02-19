
CC=/usr/local/bin/gcc -Wall
all:	_query_parser

_query_parser:
	flex -8 -f -Pquery -i query_parser.lex
	$(CC) -c lex.query.c
	$(CC) -c test.c
	$(CC) -o query_parser test.o lex.query.o

clean:
	rm -rf lex.query.c y.tab.c y.tab.h query_parser *.o

.PHONY:	all clean
