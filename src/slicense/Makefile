CC=	gcc
LDFLAGS= -lm -L/usr/local/lib -lssl -lcrypto
CFLAGS=	-g -Wall -DWITH_MAKE_LICENSE -std=c99 -O2

OBJS=	license.o base32.o
PROG=	slicense_info slicense_make

all: $(PROG)

slicense_info: $(OBJS)
	$(CC) $(CFLAGS) -o slicense_info slicense_info.c $(OBJS) $(LDFLAGS)

slicense_make: $(OBJS)
	$(CC) $(CFLAGS) -o slicense_make slicense_make.c $(OBJS) $(LDFLAGS)


clean:
	rm -f $(PROG) $(OBJS) slicense_make.o slicense_info.o
