#gcc xml.c url.c main.c -I/usr/include/libxml2/ -L/usr/lib -lxml2 -lz -lm  -lcurl  -lssl -lcrypto -ldl && ./a.out
CFLAGS=		
CFLAGS+=	-W -Wall -O0 -g -BLACK_BOX -I/usr/include/mysql
LDFLAGS=	
PROG=		us_mapback.so
CC=		gcc

# Hack to get common lib files from boitho
COMMONDIR=	../common

#VPATH=		.:$(COMMONDIR):$(COMMONDIR)/../crawl/:../dictionarywordsLot/

OBJS=		main.o
#COMMONSOBJ=	adultWeight.o chtbl.o debug.o iconv.o ir.o logs.o poprank.o sha1.o timediff.o \
#		bfileutil.o config.o DocumentIndex.o iindex.o langdetect.o lot.o reposetory.o stdlib.o url.o \
#		bsocket.o crc32.o error.o integerindex.o langToNr.o lotlist.o reposetoryNET.o strlcat.o vid.o \
#		bstr.o daemon.o exeoc.o ipdb.o list.o mgsort.o revindex.o strlcpy.o io.o sid.o
#
OBJS+=		$(COMMONSOBJ)

include ../../mk/setup.mk

all: $(PROG)

$(PROG): $(OBJS)
	libtool --mode=execute $(CC) -o $(PROG) $(LDFLAGS) $(OBJS) -fPIC -shared -D BLACK_BOX -g $(US_STATIC)
	-mkdir ../../usersystems/us_mapback
	cp $(PROG) ../../usersystems/us_mapback

clean:
	rm -f ${OBJS} ${PROG}
