CC=	gcc
VPATH= .:../boitho-bbdn/:../common/:../base64/:../maincfg/:../crawlManager/:.

LDFLAGS=	 -lxml2 /usr/lib/mysql/libmysqlclient.a
CFLAGS=	-g -Wall -O2 -I/usr/include/libxml2/ -DDEBUG -I/usr/include/mysql


OBJS=	main.o bbdnclient.o daemon.o debug.o error.o bstr.o strlcat.o base64.o maincfg.o client.o timediff.o
PROG=	bbdocumentWebAdd

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS) /usr/local/lib/libconfig.a

clean:
	rm -f $(PROG) $(OBJS)
