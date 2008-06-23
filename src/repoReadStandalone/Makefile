CC=	gcc
CFLAGS=	-Wall -O2 -g -std=c99 -D_FILE_OFFSET_BITS=64
LDFLAGS=-lz
PROG=	reporead
PROG2=	elementoffset
OBJS=	main.o

all: $(PROG) $(PROG2)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(PROG2): elementoffset.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS)
	rm -f $(PROG2)
