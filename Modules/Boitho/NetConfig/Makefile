PROG=   configwrite
CFLAGS= -g -W -Wall
CC = gcc

all: $(PROG)

$(PROG): ../../../../websearch/src/common/exeoc.c
	$(CC) $(CFLAGS) -o $@ $(PROG).c $?
	chown root $(PROG)
	chmod +s $(PROG)
