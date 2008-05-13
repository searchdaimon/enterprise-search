#gcc xml.c url.c main.c -I/usr/include/libxml2/ -L/usr/lib -lxml2 -lz -lm  -lcurl  -lssl -lcrypto -ldl && ./a.out
CFLAGS=		-I/usr/include/libxml2/ -L/usr/lib -I/home/eirik/.root/include
CFLAGS+=	-W -Wall -O0 -g
LDFLAGS=	-lxml2 -lz -lm  -lcurl  -lssl -lcrypto -ldl -L/home/eirik/.root/lib
PROG=		crawlExchange.so
CC=		/usr/local/bin/gcc

# Hack to get common lib files from boitho
COMMONDIR=	../common

VPATH=		.:$(COMMONDIR):$(COMMONDIR)/../crawl/:../dictionarywordsLot/

OBJS=		xml.o webdav.o main.o
COMMONSOBJ=	adultWeight.o chtbl.o debug.o iconv.o ir.o logs.o poprank.o sha1.o timediff.o \
		bfileutil.o config.o DocumentIndex.o iindex.o langdetect.o lot.o reposetory.o stdlib.o url.o \
		bsocket.o crc32.o error.o integerindex.o langToNr.o lotlist.o reposetoryNET.o strlcat.o vid.o \
		bstr.o daemon.o exeoc.o ipdb.o list.o mgsort.o revindex.o strlcpy.o io.o sid.o

OBJS+=		$(COMMONSOBJ)
OBJS+=		crawl.o ../base64/base64.o ../crawl/mailsubject.o set.o


all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) -fPIC -shared -D BLACK_BOKS -g -Wl,-static 
	-mkdir ../..//crawlers/crawlExchange
	cp $(PROG) ../..//crawlers/crawlExchange

clean:
	rm -f ${OBJS} ${PROG}
