CFLAGS=		-W -Wall -O0 -g -I/usr/local/BerkeleyDB.4.5/include/ -pg -D_FILE_OFFSET_BITS=64
LDFLAGS=	-lm -lz -pg
PROG=		anchorMerge
CC=		gcc

# Hack to get common lib files from boitho
COMMONDIR=	../common

OBJS=		anchorMerge.o ../3pLibs/keyValueHash/hashtable_itr.o ../3pLibs/keyValueHash/hashtable.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -g -o $(PROG) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c -D_FILE_OFFSET_BITS=64
	#$(CC) -o $(PROG) $(CFLAGS) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c


clean:
	rm -f ${OBJS} ${PROG}
