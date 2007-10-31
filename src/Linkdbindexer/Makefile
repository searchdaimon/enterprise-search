CFLAGS=		-W -Wall -O0 -g -D_FILE_OFFSET_BITS=64 -D__USE_LARGEFILE=64
#CFLAGS=		-W -Wall -O0 -g -I/usr/local/BerkeleyDB.4.5/include/ -D_FILE_OFFSET_BITS=64 -DWITHOUT_DIWRITE_FSYNC -D__USE_LARGEFILE=64 -D__USE_LARGEFILE64=1
LDFLAGS=	
PROG=		linkdbindexer
CC=		/usr/local/bin/gcc

# Hack to get common lib files from boitho
#COMMONDIR=	../common

OBJS=		linkdbindexer.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) $(CFLAGS)
	@#$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c $(CFLAGS)
	@#$(CC) -o $(PROG) $(CFLAGS) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c


clean:
	rm -f ${OBJS} ${PROG}
