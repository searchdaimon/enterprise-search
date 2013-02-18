#shortenurl: shortenurl.c
#	gcc -g shortenurl.c -DWITH_SHORTENURL_MAIN -o shortenurl ../common/*.c -W -Wall -lz -lm


VPATH=	.:../acls:../maincfg:../key:../crawl:../boitho-bbdn/:../boithoadClientLib:../bbdocument:../libcache

#WANT_COMMON=	true
WANT_DS=	true
WANT_LIBCONFIG=	true
WANT_HASHTABLE=	true
WANT_BASE64=	true
WANT_BDB=	true
WANT_PERLEMBED=	true
WANT_MYSQL=	true
WANT_LOGGER=	true

PROG=	crawlManager2

CFLAGS?=	

CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DNO_64_BIT_DIV -DIIACL -DBLACK_BOKS -D_REENTRANT \
		-DNO_REUSEADDR -DWITH_CONFIG -DWITH_US_CACHE -DDEBUG

LDFLAGS+=	-lm -lz -Wl,-E -rdynamic ../common/objbb/*.o

OBJS=	bbdnclient.o maincfg.o acls.o bbdocument.o bbfilters.o shortenurl.o usersystem.o main.o boithoadClientLib.o crawl.o key.o perlcrawl.o libcache.o


include ../../mk/prog.mk
