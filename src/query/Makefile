
CC=/usr/local/bin/gcc -g -Wall
all:	_read_thesaurus _query_parser

_read_thesaurus:
	flex -8 -f -Pthesaurus -i read_thesaurus.lex
	$(CC) -c lex.thesaurus.c
	$(CC) -c test_expand_query.c
	$(CC) -o expand_query test_expand_query.o lex.thesaurus.o ../ds/libds.a

_query_parser:
	flex -8 -f -Pquery -i query_parser.lex
	$(CC) -c lex.query.c
	$(CC) -c test.c
	$(CC) -o query_parser test.o lex.query.o

clean:
	rm -rf lex.query.c y.tab.c y.tab.h query_parser *.o

.PHONY:	all clean
