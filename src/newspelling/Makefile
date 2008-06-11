CC=	gcc
LDFLAGS= -lm
CFLAGS=	-g -Wall -DTESTMAIN -O2

OBJS=	spelling.o
PROG=	spelling

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) ../3pLibs/keyValueHash/hashtable.c ../common/ht.c

clean:
	rm -f $(PROG) $(OBJS)
