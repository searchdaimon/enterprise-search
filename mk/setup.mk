ifndef SETUP_DONE
SETUP_DONE=	true

include $(BOITHOHOME)/config.mk

# all should be our default target
all:


CFLAGS+=        -I/usr/include/mysql

ifdef WITH_BLACKBOX
BLACKBOX=	bb
CFLAGS+=	-DBLACK_BOKS
PREFIX=		bb

#CFLAGS+=        -DBBDOCUMENT_IMAGE_BY_CONVERT -DWITH_CONFIG -DDI_FILE_CASHE -DEXPLAIN_RANK \

#		-DWITH_RANK_FILTER -DWITH_THREAD -DWITH_CASHE
#MYSQL_THREAD=	-I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient_r

else
BLACKBOX=	
PREFIX=		
endif

ifdef WITH_PREFIX
USEPREFIX=	$(PREFIX)
else
USEPREFIX=	
endif

OBJDIR=	${CURDIR}/obj$(BLACKBOX)
ifdef CGIBIN
BIN=	${BOITHOHOME}/cgi-bin
else
BIN=	${BOITHOHOME}/bin
endif
LIBDIR=	${BOITHOHOME}/lib

endif
