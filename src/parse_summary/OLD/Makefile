
CFLAGS=-O3

all:
	yacc -d summary.yacc
	flex -i summary.lex
	gcc $(CFLAGS) -c lex.yy.c y.tab.c
	ar rc libsummary.a lex.yy.o y.tab.o
	ranlib libsummary.a

test:
	gcc $(CFLAGS) -o summary test.c libsummary.a

clean:
	rm -rf lex.yy.c y.tab.c y.tab.h libsummary.a summary *.o

.PHONY: all clean
