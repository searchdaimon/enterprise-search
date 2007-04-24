
CFLAGS= -DTEST -Wall -W -g -DYYDEBUG=1 -O1

all: getdate

getdate: getdate.tab.o #getdate.lex.o
	$(CC) $(LDFLAGS) getdate.tab.o test.c -o getdate


getdate.tab.c: getdate.y
	bison -v getdate.y

getdate.lex.c: getdate.l
	flex getdate.l -o getdate.lex.c

clean:
	rm -f getdate getdate.tab.c getdate.tab.o

