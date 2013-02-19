
CC=gcc -O2 -Wall -g
CPP=g++ -O2 -Wall -g

include ../../mk/setup.mk

all:	_query_parser

_query_parser:
	flex -8 -f -Pquery -i query_parser.lex
	$(CC) -c lex.query.c
	$(CC) -c stemmer.c

test:
	$(CC) -c test.c
	$(CC) -o query_parser test.o lex.query.o stemmer.o ../ds/libds.a ../common/xml.c

build_thesaurus:
# Kjør 'src/query/build_thesaurus_from_ordbanken data/ordbanken/fullform_bm.txt data/thesaurus.text data/thesaurus.id'
# når thesaurus-en skal bygges.
	$(CPP) -o build_thesaurus_from_ordbanken build_thesaurus_from_ordbanken.cpp

clean:
	rm -rf lex.query.c y.tab.c y.tab.h query_parser build_thesaurus_from_ordbanken gmon.out *.o

.PHONY:	all clean
