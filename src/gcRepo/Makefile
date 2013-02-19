CFLAGS=		-W -Wall -O0 -g -I/usr/local/BerkeleyDB.4.5/include/
LDFLAGS=	-lm -lz -D_FILE_OFFSET_BITS=64 -DWITHOUT_DIWRITE_FSYNC
PROG=		gcrepo
CC=		gcc

# Hack to get common lib files from boitho
COMMONDIR=	../common

OBJS=		gcrepo.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c
	#$(CC) -o $(PROG) $(CFLAGS) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c


clean:
	rm -f ${OBJS} ${PROG}
