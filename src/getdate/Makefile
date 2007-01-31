
CFLAGS=	-DTEST -Wall -W -g

all: getdate

getdate: getdate.tab.o
	$(CC) $(LDFLAGS) getdate.tab.o -o getdate


getdate.tab.c: getdate.y
	bison getdate.y

clean:
	rm -f getdate getdate.tab.c getdate.tab.o
