
CC=/usr/local/bin/gcc -Wall
all:	_query_parser

_query_parser:
	flex -8 -f -Pquery -i query_parser.lex
	$(CC) -c lex.query.c
	flex -8 -f -Pthesaurus -i read_thesaurus.lex
	$(CC) -c lex.thesaurus.c
	$(CC) -c test.c
	$(CC) -o query_parser test.o lex.thesaurus.o lex.query.o ../ds/libds.a

#_read_thesaurus:
#	flex -8 -f -Pthesaurus -i read_thesaurus.lex
#	$(CC) -c lex.thesaurus.c
#	$(CC) -c test_expand_query.c
#	$(CC) -o expand_query test_expand_query.o lex.thesaurus.o lex.query.o ../ds/libds.a

clean:
	rm -rf lex.query.c y.tab.c y.tab.h query_parser *.o

.PHONY:	all clean
