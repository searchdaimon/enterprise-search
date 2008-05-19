CC=	gcc
VPATH= .:../boitho-bbdn/:../common/:../base64/

LDFLAGS=	 -lxml2
CFLAGS=	-g -Wall -O2 -I/usr/include/libxml2/


OBJS=	main.o bbdnclient.o daemon.o debug.o error.o bstr.o strlcat.o base64.o
PROG=	bbdocumentWebAdd

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)

clean:
	rm -f $(PROG) $(OBJS)
