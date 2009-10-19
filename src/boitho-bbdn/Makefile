#shortenurl: shortenurl.c
#	gcc -g shortenurl.c -DWITH_SHORTENURL_MAIN -o shortenurl ../common/*.c -W -Wall -lz -lm


VPATH=	.:../maincfg:../bbdocument:../acls:../generateThumbnail

WANT_COMMON=	true
WANT_DS=	true
WANT_LIBCONFIG=	true
WANT_HASHTABLE=	true
WANT_BASE64=	true
WANT_BDB=	true
WANT_PERLEMBED=	true
WANT_LOGGER=	true

PROG=	boitho-bbdn

CFLAGS?=	

CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DIIACL -DBLACK_BOKS -DBBDOCUMENT_IMAGE_BY_CONVERT -D_REENTRANT \
		-DNO_REUSEADDR -D USE_LIBEXTRACTOR

LDFLAGS+=	-lm -lz 
		
OBJS=	bbdnserver.o maincfg.o acls.o generate_thumbnail_by_convert.o bbdocument.o bbfilters.o


include ../../mk/prog.mk
