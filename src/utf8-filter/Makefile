
all:
	flex -f -8 utf8-filter.l
	gcc -O2 -o utf8-filter lex.yy.c

clean:
	rm -rf utf8-filter lex.yy.c

.PHONY:	all clean
