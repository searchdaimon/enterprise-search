PROG=   initwrapper
CFLAGS= -g -W -Wall
CC = /usr/local/bin/gcc

all: $(PROG)

#$(PROG): ../common/exeoc.c
$(PROG): initwrapper.c ../common/exeoc.c
	$(CC) $(CFLAGS) -o $(PROG) $?
	#chown root $(PROG)
	#chmod +s $(PROG)
