
CC=gcc -O2 -g -m32
yacc=bison

all:
	$(yacc) -d -p bsgp snippet.parser.y
	flex -f -8 -P bsgp -i snippet.parser.l
	$(CC) -c lex.bsgp.c snippet.parser.tab.c
	ar rc libsnippet_generator.a lex.bsgp.o snippet.parser.tab.o
	ranlib libsnippet_generator.a


test:
	$(CC) -o snippet test.c libsnippet_generator.a ../query/lex.query.o ../query/stemmer.o ../ds/libds.a

clean:
	rm -rf libsnippet_generator.a *.o snippet y.tab.c lex.bsgp.c y.tab.h

.PHONY:	all clean test
