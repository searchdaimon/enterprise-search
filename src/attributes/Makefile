
CC=/usr/local/bin/gcc -O2 -g

all:
	yacc -d -b rac -p rac show_attributes.y
	flex -f -8 -P rac -i show_attributes.l
	$(CC) -c lex.rac.c
	$(CC) -c rac.tab.c
	$(CC) -c attr_makexml.c
	ar rc libshow_attr.a lex.rac.o rac.tab.o attr_makexml.o
	ranlib libshow_attr.a

test:
	$(CC) -o show_attributes test.c libshow_attr.a ../ds/libds.a

clean:
	rm -rf show_attributes rac.tab.[ch] lex.rac.c rac_test rac.output *.o libshow_attr.a
