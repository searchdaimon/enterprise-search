CC=	gcc
CFLAGS=	-Wall -O2 -g -std=c99 -D_FILE_OFFSET_BITS=64
LDFLAGS=-lz
PROG=	reporead
OBJS=	main.o

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS)
