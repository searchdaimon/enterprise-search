
#CC=/usr/local/bin/gcc -p -g -DDEBUG -DDEBUG_2
CC=/usr/local/bin/gcc -O2 -g -D NOWARNINGS
#CC=/usr/local/bin/gcc
# -O2 -Wall

all: html

html:
	awk -f tags.awk tags.conf > html_parser_tags.h
	yacc -d -b bhpm -p bhpm html_parser.y
	flex -f -8 -P bhpm -i html_parser.l
	$(CC) -c lex.bhpm.c bhpm.tab.c
	ar rc libhtml_parser.a lex.bhpm.o bhpm.tab.o
	ranlib libhtml_parser.a
	$(CC) -o html_parser test.c libhtml_parser.a ../ds/libds.a

clean:
	rm -rf lex.bhpm.c lex.bcpm.c *.tab.c *.tab.h libhtml_parser.a html_parser *.o

.PHONY: all clean
