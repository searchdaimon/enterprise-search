#CC=	gcc
LDFLAGS= -lm -L/usr/local/lib -lssl -lcrypto
CFLAGS=	-g -Wall -DWITH_MAKE_LICENSE -std=c99 -O2

include ../../mk/setup.mk

OBJS=	license.o base32.o
IOBJS=	$(OBJS) slicense_info.o
MOBJS=	$(OBJS) slicense_make.o
PROG=	slicense_info slicense_make

all: $(PROG)

slicense_info: $(IOBJS)
	$(CC) $(CFLAGS) -o slicense_info $(IOBJS) $(LDFLAGS)

slicense_make: $(MOBJS)
	$(CC) $(CFLAGS) -o slicense_make $(MOBJS) $(LDFLAGS)


clean:
	rm -f $(PROG) $(MOBJS) $(IOBJS)
