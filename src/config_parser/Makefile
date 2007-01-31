
CC=gcc

all:	config_parser

config_parser:
	yacc -d config_parser.yacc
	lex -i config_parser.lex
	$(CC) -c lex.yy.c y.tab.c
	ar rc libconfig_parser.a lex.yy.o y.tab.o
	ranlib libconfig_parser.a

test:
	$(CC) -o config_parser test.c libconfig_parser.a

clean:
	rm -rf lex.yy.c y.tab.c y.tab.h libconfig_parser.a config_parser *.o

.PHONY:	all config_parser clean
