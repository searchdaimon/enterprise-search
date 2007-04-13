PROG=   initwrapper
CFLAGS= -g -W -Wall
CC = gcc

all: $(PROG)

$(PROG): ../common/exeoc.c
	$(CC) $(CFLAGS) -o $@ $(PROG).c $?
	#chown root $(PROG)
	#chmod +s $(PROG)
