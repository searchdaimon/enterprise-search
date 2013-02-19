CFLAGS=		-W -Wall -O0 -g -D_FILE_OFFSET_BITS=64 -D__USE_LARGEFILE=64 -DNO_BRANK_MAIN
#CFLAGS=		-W -Wall -O0 -g -I/usr/local/BerkeleyDB.4.5/include/ -D_FILE_OFFSET_BITS=64 -DWITHOUT_DIWRITE_FSYNC -D__USE_LARGEFILE=64 -D__USE_LARGEFILE64=1
LDFLAGS=	
PROG=		linkdbview
CC=		gcc

# Hack to get common lib files from boitho
#COMMONDIR=	../common

OBJS=		linkdbview.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) $(CFLAGS) ../BrankCalculate5/main.c ../common/lot.c ../common/bstr.c ../common/strlcat.c ../common/bfileutil.c ../common/debug.c  ../banlists/ban.c ../bs/bs.c ../tq/tq.c ../3pLibs/keyValueHash/hashtable.c ../3pLibs/keyValueHash/hashtable_itr.c -lpthread -DDEFLOT -lm
	@#$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c $(CFLAGS)
	@#$(CC) -o $(PROG) $(CFLAGS) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c


clean:
	rm -f ${OBJS} ${PROG}
