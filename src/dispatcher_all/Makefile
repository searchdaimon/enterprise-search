## Based on makefile for boitho-bbdn
## TODO: Remove uneeded dependencies


VPATH=	.:../maincfg:../bbdocument:../acls:../boithoadClientLib:../crawlManager2:../searchFilters:../cgi-util:../key

WANT_COMMON=	true
WANT_DS=	true
WANT_LIBCONFIG=	true
WANT_HASHTABLE=	true
WANT_BASE64=	true
WANT_BDB=	true
#WANT_PERLEMBED=	true
WANT_MYSQL=     true
WANT_LOGGER=	true

CGIBIN=	true
PROG=	dispatcher_allbb

CFLAGS?=	

CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DIIACL -BLACK_BOX -DWITH_SPELLING -DWITH_CASHE

LDFLAGS+=	-lm -lz ../getFiletype/libfte.a ../query/lex.query.o -lpthread

OBJS=	main.o maincfg.o acls.o out_opensearch.o  out_sdxml.o library.o qrewrite.o cgihandler.o client.o \
	boithoadClientLib.o searchFilters.o cgi-util.o key.o


include ../../mk/prog.mk
