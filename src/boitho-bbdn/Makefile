#shortenurl: shortenurl.c
#	gcc -g shortenurl.c -DWITH_SHORTENURL_MAIN -o shortenurl ../common/*.c -W -Wall -lz -lm


VPATH=	.:../maincfg:../bbdocument:../acls:../generateThumbnail:../perlembed

WANT_COMMON=	true
WANT_DS=	true
WANT_LIBCONFIG=	true
WANT_HASHTABLE=	true
WANT_BASE64=	true
WANT_BDB=	true

PROG=	boitho-bbdn

CFLAGS?=	

CFLAGS+=	-DATTRIBUTES -D_FILE_OFFSET_BITS=64 -DIIACL -DBLACK_BOKS -DBBDOCUMENT_IMAGE_BY_CONVERT -D_REENTRANT \
		-DNO_REUSEADDR $(PERL_EMBED_INC)

LDFLAGS+=	$(PERL_EMBED_LIB) -lm -lz 
		
OBJS=	bbdnserver.o maincfg.o acls.o generate_thumbnail_by_convert.o bbdocument.o perlembed.o perlxsi.o bbfilters.o


include ../../mk/prog.mk
