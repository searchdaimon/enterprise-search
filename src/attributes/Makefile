
CFLAGS?=	-O2
yacc=bison

include ../../mk/setup.mk

all:
	$(yacc) -d -b rac -p rac show_attributes.y
	flex -f -8 -P rac -i show_attributes.l
	$(CC) -c lex.rac.c
	$(CC) -c rac.tab.c
	$(yacc) -d -b adf -p adf attribute_descriptions.y
	flex -f -8 -P adf -i attribute_descriptions.l
	$(CC) -c lex.adf.c
	$(CC) -c adf.tab.c
	$(CC) -c attr_makexml.c
	ar rc libshow_attr.a lex.rac.o rac.tab.o lex.adf.o adf.tab.o attr_makexml.o
	ranlib libshow_attr.a
	cp libshow_attr.a $(LIBDIR)

test:
	$(CC) -o show_attributes test.c libshow_attr.a $(LIBDIR)/libds.a

clean:
	rm -rf show_attributes rac.tab.[ch] lex.rac.c rac.output adf.tab.[ch] lex.adf.c adf.output *.o libshow_attr.a
