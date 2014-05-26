CFLAGS=		-W -Wall -O0 -g -I/usr/local/BerkeleyDB.4.5/include/ -D_FILE_OFFSET_BITS=64 -DWITHOUT_DIWRITE_FSYNC
LDFLAGS=	-lm -lz -D_FILE_OFFSET_BITS=64 -DDI_FILE_CASHE
PROG=		gcsummary
#CC=		gcc

include ../../mk/setup.mk

# Hack to get common lib files from boitho
COMMONDIR=	../common

OBJS=		gcsummary.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c
	@#$(CC) -o $(PROG) $(CFLAGS) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c


clean:
	rm -f ${OBJS} ${PROG}
