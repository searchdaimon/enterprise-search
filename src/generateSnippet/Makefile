
CC=/usr/local/bin/gcc -g

all:
	yacc -d -p bsgp snippet.parser.y
	flex -f -8 -P bsgp -i snippet.parser.l
	$(CC) -c lex.bsgp.c y.tab.c
	ar rc libsnippet_generator.a lex.bsgp.o y.tab.o
	ranlib libsnippet_generator.a


test:
	$(CC) -o snippet test.c libsnippet_generator.a ../ds/libds.a ../query/lex.query.o

clean:
	rm -rf libsnippet_generator.a *.o snippet y.tab.c lex.bsgp.c y.tab.h

.PHONY:	all clean
