#shortenurl: shortenurl.c
#	gcc -g shortenurl.c -DWITH_SHORTENURL_MAIN -o shortenurl ../common/*.c -W -Wall -lz -lm


#VPATH=	.:../maincfg:../bbdocument:../acls:../generateThumbnail

WANT_COMMON=	true
#WANT_DS=	true
#WANT_LIBCONFIG=	true
#WANT_HASHTABLE=	true
#WANT_BASE64=	true
#WANT_BDB=	true
#WANT_PERLEMBED=	true

PROG=	logger-test
LIB=    liblogger.a


CFLAGS?=	-pthread

#CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DIIACL -BLACK_BOX -DBBDOCUMENT_IMAGE_BY_CONVERT -D_REENTRANT \
#		-DNO_REUSEADDR -D USE_LIBEXTRACTOR

#LDFLAGS+=	-lm -lz -pthread
		
OBJS=	logger.o


include ../../mk/lib.mk
