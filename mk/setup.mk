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
CC=		gcc -D NO_64_BIT_DIV
LIBCONFIG=	/usr/local/lib/libconfig.a
LIBCONFIG_64=	/usr/local/lib64/libconfig.a
CRAWL_STATIC=	-Wl,-static
US_STATIC=	-static
endif

ifeq ($(strip $(host)),bbh-002.boitho.com)
# BBH2
PERL_EMBED_INC=	-I/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -D_GNU_SOURCE
PERL_EMBED_LIB=	-rdynamic -Wl,-E -Wl,-rpath,/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE /usr/lib/perl5/5.8.8/i386-linux-thread-multi/auto/DynaLoader/DynaLoader.a -L/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -lperl
BDB_INC=	-I/usr/include/db4
BDB_LIB=	/usr/lib/libdb-4.6.a
CC=		gcc -m32
MYSQL_LIB+=	/usr/lib/mysql/libmysqlclient.a -lssl
MYSQL_INC+=	-I/usr/include/mysql
LIBCONFIG=	/usr/local/lib/libconfig.a
LIBCONFIG_64=	/usr/local/lib64/libconfig.a
CRAWL_STATIC=	-Wl,-static
US_STATIC=	-static
endif

ifeq ($(strip $(host)),searchdaimon)
# Local on ES
PERL_EMBED_INC=	-I/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -D_GNU_SOURCE
PERL_EMBED_LIB=	-rdynamic -Wl,-E -Wl,-rpath,/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE /usr/lib/perl5/5.8.8/i386-linux-thread-multi/auto/DynaLoader/DynaLoader.a -L/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -lperl
BDB_INC=	-I/usr/include/db4
BDB_LIB=	/usr/lib/libdb-4.6.a
CC=		gcc -m32 -lpthread
MYSQL_LIB+=	/usr/lib/mysql/libmysqlclient.a -lssl
MYSQL_INC+=	-I/usr/include/mysql
LIBCONFIG=	`pkg-config --libs libconfig`
LIBCONFIG_64=	`pkg-config --libs libconfig`
CRAWL_STATIC=	
US_STATIC=	
endif

ifdef NO_DEPRECATED_WARNINGS
CFLAGS+=	-Wno-deprecated-declarations
endif

ifdef WITH_PROFILE
CFLAGS+=	-pg
LDFLAGS+=	-pg
endif

ifdef WITH_BLACKBOX
BLACKBOX=	bb
CFLAGS+=	-DBLACK_BOX
PREFIX=		bb

#CFLAGS+=        -DBBDOCUMENT_IMAGE_BY_CONVERT -DWITH_CONFIG -DDI_FILE_CASHE -DEXPLAIN_RANK \

#		-DWITH_RANK_FILTER -DWITH_THREAD -DWITH_CASHE
#MYSQL_THREAD=	-I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient_r

else
BLACKBOX=	
PREFIX=		
endif

ifndef NO_DEBUG
CFLAGS+=	-g
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
