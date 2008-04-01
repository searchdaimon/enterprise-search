CC=	gcc
LDFLAGS= -lm
CFLAGS=	-g -Wall -DTESTMAIN

OBJS=	spelling.o
PROG=	spelling

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) ../3pLibs/keyValueHash/hashtable.c ../dictionarywordsLot/set.c

clean:
	rm -f $(PROG) $(OBJS)
