
CFLAGS+= -Wall -W -g -DYYDEBUG=1 -O1
#CFLAGS+= -DTEST_GETDATE 

all: getdate dateview

getdate: getdate.tab.o test.o
	$(CC) $(LDFLAGS) getdate.tab.o test.o -o getdate


getdate.tab.c: getdate.y
	bison -v getdate.y

getdate.lex.c: getdate.l
	flex getdate.l -o getdate.lex.c

dateview: dateview.c
	$(CC)  -DTEST_DATEVIEW dateview.c getdate.tab.c -o dateview
clean:
	rm -f getdate getdate.tab.c getdate.tab.o test.o

