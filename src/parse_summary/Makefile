

# Legg til -O2 og -DNOWARNINGS, fiks <b></b>

LEMON=./lemon -s
LEX=flex -f -8
CC=gcc -p -g
SUMMARY=summary.parser
HIGHLIGHT=highlight.parser

all: lemon hi_l sum

sum:
	$(LEMON) $(SUMMARY).y
	$(LEX) -Psummary -i $(SUMMARY).l
	$(CC) -c $(SUMMARY).code.c lex.summary.c
	ar rc libsummary.a $(SUMMARY).code.o lex.summary.o
	ranlib libsummary.a

hi_l:
	$(LEMON) $(HIGHLIGHT).y
	$(LEX) -Phighlight $(HIGHLIGHT).l
	$(CC) -c $(HIGHLIGHT).code.c lex.highlight.c
	ar rc libhighlight.a $(HIGHLIGHT).code.o lex.highlight.o
	ranlib libhighlight.a

lemon:
	$(CC) -o lemon lemon.c

test:
	$(CC) -o summary test_highlighter.c -L. -lsummary -lhighlight ./../query/lex.query.o

test1:
	$(CC) -o summary test.c -L. -lsummary

test2:
	$(CC) -o hl test2.c -L. -lhighlight

clean:
	rm -rf summary $(SUMMARY).[ch] $(SUMMARY).out $(HIGHLIGHT).[ch] $(HIGHLIGHT).out libsummary.a libhighlight.a lex.* *.o

distclean: clean
	rm -rf lemon

.PHONY: all clean distclean
