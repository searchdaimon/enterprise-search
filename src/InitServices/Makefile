PROG=   initwrapper
CFLAGS= -g -W -Wall
CC = gcc

all: $(PROG)

#$(PROG): ../common/exeoc.c
$(PROG):
	$(CC) $(CFLAGS) -o $@ initwrapper.c ../common/exeoc.c $?
	#chown root $(PROG)
	#chmod +s $(PROG)
