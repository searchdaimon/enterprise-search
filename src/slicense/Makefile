CC=	gcc
LDFLAGS= -lm -L/usr/local/lib -lssl -lcrypto
CFLAGS=	-g -Wall -DTESTMAIN -DWITH_MAKE_LICENSE -std=c99 -O2

OBJS=	main.o base32.o
PROG=	license

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS)
