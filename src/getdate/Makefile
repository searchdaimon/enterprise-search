
CFLAGS= -DTEST -Wall -W -g -DYYDEBUG=1 -O1

all: getdate dateview

getdate: getdate.tab.o test.c #getdate.lex.o
	$(CC) $(LDFLAGS) getdate.tab.o test.c -o getdate


getdate.tab.c: getdate.y
	bison -v getdate.y

getdate.lex.c: getdate.l
	flex getdate.l -o getdate.lex.c

dateview: dateview.c
	$(CC)  -DTEST_DATEVIEW dateview.c getdate.tab.c -o dateview
clean:
	rm -f getdate getdate.tab.c getdate.tab.o

