ifndef SETUP_DONE
SETUP_DONE=	true

include $(BOITHOHOME)/config.mk

# all should be our default target
all:


CFLAGS+=        -I/usr/include/mysql

# Detect compile box
host=$(shell hostname)

ifeq ($(strip $(host)),bbh-001.boitho.com)
# BBH1
BDB_INC=        -I/usr/local/BerkeleyDB.4.5/include/
BDB_LIB=        /usr/local/BerkeleyDB.4.5/lib/libdb.a
PERL_EMBED_INC=	`perl -MExtUtils::Embed -e ccopts`
PERL_EMBED_LIB=	`perl -MExtUtils::Embed -e ldopts`
LDFLAGS+=	/usr/lib/mysql/libmysqlclient.a
CFLAGS+=	-I/usr/include/mysql
#`perl -MExtUtils::Embed -e ccopts -e ldopts` -I/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE   -L/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -lperl
endif

ifeq ($(strip $(host)),bbh-002.boitho.com)
# BBH2
PERL_EMBED_INC=	-I/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -D_GNU_SOURCE
PERL_EMBED_LIB=	-rdynamic -Wl,-E -Wl,-rpath,/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE /usr/lib/perl5/5.8.8/i386-linux-thread-multi/auto/DynaLoader/DynaLoader.a -L/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -lperl
BDB_INC=	-I/usr/include/db4
BDB_LIB=	/usr/lib/libdb-4.6.a
CC=		gcc -m32
LDFLAGS+=	-L/usr/lib/mysql -lmysqlclient
CFLAGS+=	-I/usr/include/mysql
endif

ifdef WITH_PROFILE
CFLAGS+=	-pg
LDFLAGS+=	-pg
endif

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
