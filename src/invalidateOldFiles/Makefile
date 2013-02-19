CFLAGS=		-W -Wall -O2 -g -D_FILE_OFFSET_BITS=64 -DBLACK_BOKS
LDFLAGS=	-lm -lz
PROG=		invalidateOldFiles
CC=		gcc

# Hack to get common lib files from boitho
COMMONDIR=	../common

OBJS=		main.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -g -o $(PROG) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c -D_FILE_OFFSET_BITS=64 -DBLACK_BOKS
	#$(CC) -o $(PROG) $(CFLAGS) $(LDFLAGS) $(OBJS) $(COMMONDIR)/*.c


clean:
	rm -f ${OBJS} ${PROG}
