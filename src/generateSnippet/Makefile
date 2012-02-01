
CFLAGS?=	-O2
yacc=bison

include ../../mk/setup.mk

all:
	$(yacc) -d -p bsgp snippet.parser.y
	flex -f -8 -P bsgp -i snippet.parser.l
	$(CC) -c lex.bsgp.c snippet.parser.tab.c
	ar rc libsnippet_generator.a lex.bsgp.o snippet.parser.tab.o
	ranlib libsnippet_generator.a
	cp libsnippet_generator.a $(LIBDIR)


test:
	$(CC) -o snippet test.c libsnippet_generator.a ../query/lex.query.o ../query/stemmer.o $(LIBDIR)/libds.a ../common/xml.c

clean:
	rm -rf libsnippet_generator.a *.o snippet y.tab.c lex.bsgp.c y.tab.h

.PHONY:	all clean test
