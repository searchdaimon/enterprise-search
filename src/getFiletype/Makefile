
CFLAGS?=	-O2

include ../../mk/setup.mk

all:
	bison -d -b fte -p fte identify_extension.y
	flex -f -8 -P fte -i identify_extension.l
##	$(CC) -o fte_test test.c lex.fte.c fte.tab.c ../ds/libds.a
	$(CC) -c $(CFLAGS) lex.fte.c
	$(CC) -c $(CFLAGS) fte.tab.c
	ar rc libfte.a lex.fte.o fte.tab.o
	ranlib libfte.a
	cp libfte.a $(LIBDIR)

#all_old:
#	flex -f -P getfiletype -i getfiletype.l
#	$(CC) -c -o getfiletype.o lex.getfiletype.c

test:
#	$(CC) -o getfiletype test.c getfiletype.o ../ds/libds.a libfte.a
	$(CC) -o fte_test test.c libfte.a $(LIBDIR)/libds.a


clean:
	rm -rf getfiletype getfiletype.o lex.getfiletype.c fte.tab.[ch] lex.fte.c fte_test fte.output *.o libfte.a
