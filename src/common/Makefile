
# Uncomment if you want debugging info 

CFLAGS?=	-O2
CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DDISK_PROTECTOR -DBLACK_BOKS -DIIACL -DWITH_THREAD -DWITH_RANK_FILTER \
		-DWITH_CONFIG

LIB=	libcommon.a
OBJS=	adultWeight.o chtbl.o debug.o iconv.o ir.o logs.o poprank.o sha1.o timediff.o \
	bfileutil.o config.o DocumentIndex.o iindex.o langdetect.o lot.o reposetory.o stdlib.o url.o \
	bsocket.o crc32.o error.o integerindex.o langToNr.o lotlist.o reposetoryNET.o strlcat.o vid.o \
	bstr.o daemon.o exeoc.o ipdb.o list.o mgsort.o revindex.o strlcpy.o re.o ht.o doc_cache.o \
	attributes.o getpath.o atomicallyio.o io.o utf8-strings.o dp.o cgi.o gcwhisper.o sigaction.o \
	pidfile.o collection.o

include ../../mk/lib.mk
