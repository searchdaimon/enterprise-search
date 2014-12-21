
CFLAGS= -O2 -g
# Enable build warinings
CFLAGS+= -Wall -W -Wno-unused-parameter


yacc=bison

include ../../mk/setup.mk

all:
	$(yacc) -d -b rac -p rac show_attributes.y
	flex --header-file=lex.rac.h -f -8 -P rac -i show_attributes.l
	$(CC) $(CFLAGS) -c lex.rac.c
	$(CC) $(CFLAGS) -c rac.tab.c
	$(yacc) -d -b adf -p adf attribute_descriptions.y
	flex --header-file=lex.adf.h -f -8 -P adf -i attribute_descriptions.l
	$(CC) $(CFLAGS) -c lex.adf.c
	$(CC) $(CFLAGS) -c adf.tab.c
	$(CC) $(CFLAGS) -c attr_makexml.c
	$(CC) $(CFLAGS) -c ../ccan/json/json.c
	ar rc libshow_attr.a lex.rac.o rac.tab.o lex.adf.o adf.tab.o attr_makexml.o json.o
	ranlib libshow_attr.a
	cp libshow_attr.a $(LIBDIR)

test:
	$(CC) -o show_attributes test.c libshow_attr.a $(LIBDIR)/libds.a

clean:
	rm -rf show_attributes rac.tab.[ch] lex.rac.c rac.output adf.tab.[ch] lex.adf.c adf.output *.o libshow_attr.a
