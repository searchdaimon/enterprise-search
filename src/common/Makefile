
# Uncomment if you want debugging info 
# CFLAGS=	-g

CC=	/usr/local/bin/gcc -Wall -O2
LIB=	libcommon.a
OBJS=	adultWeight.o chtbl.o debug.o iconv.o ir.o logs.o poprank.o sha1.o timediff.o \
	bfileutil.o config.o DocumentIndex.o iindex.o langdetect.o lot.o reposetory.o stdlib.o url.o \
	bsocket.o crc32.o error.o integerindex.o langToNr.o lotlist.o reposetoryNET.o strlcat.o vid.o \
	bstr.o daemon.o exeoc.o ipdb.o list.o mgsort.o revindex.o strlcpy.o


all: $(LIB)

$(LIB): $(OBJS)
	ar rc $(LIB) $(OBJS)
	ranlib $(LIB)

clean:
	rm -rf $(OBJS) $(LIBS)

.PHONY:	all clean test
