CC=	gcc
LDFLAGS= 
CFLAGS=	-g -Wall -DTESTMAIN -std=c99 -O2

OBJS=	main.o
PROG=	logrotate

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)

clean:
	rm -f $(PROG) $(OBJS)
