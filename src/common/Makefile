
# Uncomment if you want debugging info 
CFLAGS=	-g

CFLAGS+=	-I/usr/include/mysql
ifdef BLACKBOX
CFLAGS+=	-DBLACK_BOKS -DBBDOCUMENT_IMAGE_BY_CONVERT -DWITH_CONFIG -DDI_FILE_CASHE -DEXPLAIN_RANK \
		-DWITH_RANK_FILTER -DWITH_THREAD -DWITH_CASHE
MYSQL_THREAD = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient_r

endif

CC=	/usr/local/bin/gcc -Wall -O2 $(CFLAGS)
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
