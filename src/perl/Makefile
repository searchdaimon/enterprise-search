include ../../mk/setup.mk

WANT_PERLEMBED= true

PROG=perl

CFLAGS?=
#CFLAGS+=	$(PERL_EMBED_INC) 
CFLAGS+=	-D NO_64_BIT_DIV -m32 -D__USE_BSD -std=c99 -m32 -D_REENTRANT -D_GNU_SOURCE -fno-strict-aliasing -pipe -Wdeclaration-after-statement -I/usr/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/gdbm -D_REENTRANT -D_GNU_SOURCE -I/usr/local/include -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I/usr/include/gdbm -g -DATTRIBUTES -I/home/eirik/.root/include -D_FILE_OFFSET_BITS=64 -O2 -DIIACL -DWITH_OPENLDAP -DBLACK_BOKS  -I/usr/include/db4 -lpthread -DBLACK_BOKS -DIIACL -DWITH_CONFIG

OBJS=main.o perlxsi.o

PROG=perl

include ../../mk/prog.mk
