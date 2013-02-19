CFLAGS=		-W -Wall -O0 -g -I/usr/local/BerkeleyDB.4.5/include/
LDFLAGS=	-lm
PROG=		detectedParkedDomains
PROG2=		query
CC=		gcc

# Hack to get common lib files from boitho
COMMONDIR=	../common

OBJS=		detectParkedDomains.o
OBJS2=		query.o

all: $(PROG) $(PROG2)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(CFLAGS) $(LDFLAGS) $(OBJS) ../3pLibs/keyValueHash/hashtable_itr.c ../3pLibs/keyValueHash/hashtable.c \
	../common/crc32.c /usr/local/BerkeleyDB.4.5/lib/libdb.a


$(PROG2): $(OBJS2)
	$(CC) -o $(PROG2) $(CFLAGS) $(LDFLAGS) $(OBJS2) ../3pLibs/keyValueHash/hashtable_itr.c ../3pLibs/keyValueHash/hashtable.c \
	../common/crc32.c /usr/local/BerkeleyDB.4.5/lib/libdb.a



clean:
	rm -f ${OBJS} ${PROG} $(PROG2) $(OBJS2)
