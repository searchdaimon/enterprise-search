
CC=/usr/local/bin/gcc -O2 -Wall
CPP=/usr/local/bin/g++ -O2 -Wall
all:	_query_parser build_thesaurus

_query_parser:
	flex -8 -f -Pquery -i query_parser.lex
	$(CC) -c lex.query.c
	$(CC) -c stemmer.c
	$(CC) -c test.c
	$(CC) -o query_parser test.o lex.query.o stemmer.o ../ds/libds.a

build_thesaurus:
	$(CPP) -o build_thesaurus_from_ordbanken build_thesaurus_from_ordbanken.cpp

clean:
	rm -rf lex.query.c y.tab.c y.tab.h query_parser build_thesaurus_from_ordbanken gmon.out *.o

.PHONY:	all clean
