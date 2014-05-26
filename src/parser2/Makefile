
#CC=/usr/local/bin/gcc -p -g -DDEBUG -DDEBUG_2
#CC=gcc -O2 -g -D NOWARNINGS -m32
#CC=/usr/local/bin/gcc
# -O2 -Wall
yacc=bison

include ../../mk/setup.mk

all: css html

html:
	awk -f tags.awk tags.conf > html_parser_tags.h
	$(yacc) -d -b bhpm -p bhpm html_parser.y
	flex -s -f -8 -P bhpm -i html_parser.l
	$(CC) -c lex.bhpm.c bhpm.tab.c -g
	ar rc libhtml_parser.a lex.bhpm.o bhpm.tab.o
	ranlib libhtml_parser.a
	$(CC) -o html_parser test.c libhtml_parser.a libcss_parser.a ../../lib/libds.a ../common/utf8-strings.c -g

css:
	$(yacc) -d -b bcpm -p bcpm css_parser.y
	flex -f -8 -P bcpm -i css_parser.l
	$(CC) -c lex.bcpm.c bcpm.tab.c -g
	ar rc libcss_parser.a lex.bcpm.o bcpm.tab.o
	ranlib libcss_parser.a
	$(CC) -o css_test css_test.c libcss_parser.a ../../lib/libds.a -g

clean:
	rm -rf lex.bhpm.c lex.bcpm.c *.tab.c *.tab.h libhtml_parser.a libcss_parser.a html_parser css_test *.o

.PHONY: all clean css
