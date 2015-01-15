
CFLAGS?=	-O2 -g -Wall -W -Wno-unused-parameter

# Uncomment if you want debugging info
#CFLAGS+=        -DDEBUG

LIB=	libds.a
OBJS=	dcontainer.o dpair.o dtuple.o dvector.o dstack.o dqueue.o dlist.o dmap.o dmultimap.o dset.o

include ../../mk/lib.mk

test:
	$(CC) -o test test.c $(LIBDIR)/libds.a
