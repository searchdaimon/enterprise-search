
CC=/usr/local/bin/gcc -p -g
#CC=/usr/local/bin/gcc -O2
# -O2 -Wall

all:
	yacc -d -p bhpm html_parser.y
	flex -f -8 -P bhpm -i html_parser.l
	$(CC) -c lex.bhpm.c y.tab.c
	ar rc libhtml_parser.a lex.bhpm.o y.tab.o
	ranlib libhtml_parser.a

test:
	$(CC) -o html_parser test.c libhtml_parser.a

clean:
	rm -rf lex.bhpm.c y.tab.c y.tab.h libhtml_parser.a html_parser *.o

.PHONY: all clean
