## Based on makefile for crawlManager2
## TODO: Remove uneeded dependencies

VPATH=	.:../acls:../maincfg:../key:../crawl:../boitho-bbdn/:../boithoadClientLib:../bbdocument:../crawlManager2:../crawlManager

#WANT_COMMON=	true
WANT_DS=	true
WANT_LIBCONFIG=	true
WANT_HASHTABLE=	true
WANT_BASE64=	true
WANT_BDB=	true
WANT_PERLEMBED=	true
WANT_MYSQL=	true
WANT_LOGGER=	true

PROG=	infoquery

CFLAGS?=	

CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DNO_64_BIT_DIV -DIIACL -BLACK_BOX -D_REENTRANT \
		-DNO_REUSEADDR -DWITH_CONFIG

LDFLAGS+=	-lm -lz -Wl,-E -rdynamic ../common/objbb/*.o

OBJS=	bbdnclient.o maincfg.o acls.o bbdocument.o bbfilters.o shortenurl.o usersystem.o main.o boithoadClientLib.o crawl.o key.o perlcrawl.o client.o


include ../../mk/prog.mk
