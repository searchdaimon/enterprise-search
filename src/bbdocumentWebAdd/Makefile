CC=	gcc
VPATH= .:../boitho-bbdn/:../common/:../base64/:../maincfg/

LDFLAGS=	 -lxml2
CFLAGS=	-g -Wall -O2 -I/usr/include/libxml2/ -DDEBUG


OBJS=	main.o bbdnclient.o daemon.o debug.o error.o bstr.o strlcat.o base64.o maincfg.o
PROG=	bbdocumentWebAdd

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) /usr/local/lib/libconfig.a

clean:
	rm -f $(PROG) $(OBJS)
