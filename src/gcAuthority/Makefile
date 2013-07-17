## Based on makefile for boitho-bbdn
## TODO: Remove uneeded dependencies


VPATH=	.:../maincfg:../bbdocument:../acls:../generateThumbnail

WANT_COMMON=	true
WANT_DS=	true
WANT_LIBCONFIG=	true
WANT_HASHTABLE=	true
WANT_BASE64=	true
WANT_BDB=	true
WANT_PERLEMBED=	true
WANT_MYSQL=	true

PROG=	gcAuthoritybb

CFLAGS?=	

CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DIIACL -BLACK_BOX -DBBDOCUMENT_IMAGE_BY_CONVERT -D_REENTRANT \

LDFLAGS+=	-lm -lz 
		
OBJS=	main.o maincfg.o acls.o generate_thumbnail_by_convert.o bbdocument.o bbfilters.o


include ../../mk/prog.mk
