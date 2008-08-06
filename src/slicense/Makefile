CC=	gcc
LDFLAGS= -lm -lssl
CFLAGS=	-g -Wall -DTESTMAIN -std=c99 -O2

OBJS=	main.o base32.o
PROG=	license

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)

clean:
	rm -f $(PROG) $(OBJS)
