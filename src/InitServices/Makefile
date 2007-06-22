PROG=   initwrapper
CFLAGS= -g -W -Wall
CC = /usr/local/bin/gcc

all: $(PROG)

#$(PROG): ../common/exeoc.c
$(PROG): initwrapper.c
	$(CC) $(CFLAGS) -o $(PROG) ../common/exeoc.c $?
	#chown root $(PROG)
	#chmod +s $(PROG)
