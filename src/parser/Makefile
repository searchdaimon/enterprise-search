
all:	html_parser

html_parser:
	yacc -d html_parser.yacc
	flex -i html_parser.lex
	gcc -c ../common/bstr.c lex.yy.c y.tab.c
	ar rc libhtml_parser.a bstr.o lex.yy.o y.tab.o
	ranlib libhtml_parser.a

test:
	gcc -o html_parser test.c libhtml_parser.a

clean:
	rm -rf lex.yy.c y.tab.c y.tab.h libhtml_parser.a html_parser *.o

.PHONY: all clean
