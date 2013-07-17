#shortenurl: shortenurl.c
#	gcc -g shortenurl.c -DWITH_SHORTENURL_MAIN -o shortenurl ../common/*.c -W -Wall -lz -lm


VPATH=	.:../dp/:../searchFilters:../maincfg:../crawlManager

WITH_PREFIX=	true
WANT_COMMON=	true
WANT_SPELLING=	true
WANT_DS=	true
WANT_GETDATE=	true
WANT_LIBCONFIG=	true
WANT_BDB=	true
WANT_HASHTABLE=	true
WANT_BASE64=	true
WANT_LOGGER=	true

WALL=		true
PROG=	searchd

include ../../mk/setup.mk

CFLAGS+=	-DIIACL -DWITH_SPELLING -DWITH_THREAD -DDISK_PROTECTOR -DATTRIBUTES -D_FILE_OFFSET_BITS=64 \
		-DNEW_CHILD_CATCHER
LDFLAGS+=	\
		$(LIBDIR)/libfte.a $(LIBDIR)/libshow_attr.a \
		$(LIBDIR)/libsnippet_generator.a \
		../utf8-filter/lex.u8fl.o \
		../query/lex.query.o ../query/stemmer.o \
		../parser2/libhtml_parser.a ../parser2/libcss_parser.a \
		-lm -lz \
		-lpthread


#LDFLAGS+=	-DATTRIBUTES ../common/*.c -D_FILE_OFFSET_BITS=64 -DDISK_PROTECTOR -BLACK_BOX -DIIACL -DWITH_SPELLING -DWITH_THREAD

OBJS=	preopen.o search.o searchkernel.o searchd.o htmlstriper.o dp.o searchFilters.o verbose.o maincfg.o \
	shortenurl.o client.o merge.o client.o


include ../../mk/prog.mk
