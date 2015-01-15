#CC=	gcc
#LDFLAGS= -lm 
#CFLAGS=	-g -Wall -DTESTMAIN -std=c99 -O2

WANT_LOGGER=    true

OBJS=	spelling.o dmetaphone.o levenshtein.o
LIB=	libspelling.a
CFLAGS+=        -D_FILE_OFFSET_BITS=64
CFLAGS+=	-W -Wall

#PROG=	spelling

#all: $(PROG)

#$(PROG): $(OBJS)
#	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) ../3pLibs/keyValueHash/hashtable.c ../3pLibs/keyValueHash/hashtable_itr.c ../common/ht.c ../ds/libds.a

#clean:
#	rm -f $(PROG) $(OBJS)
#
#dmp:
#	gcc -o dmetaphone dmetaphone.c -Wall -std=c99 -g -DDMP_TEST
#
# Make test: 
# gcc  objbb/dmetaphone.o spelling.c ../logger/logger.c ../3pLibs/keyValueHash/hashtable.c ../3pLibs/keyValueHash/hashtable_itr.c ../ds/d*.c ../common/ht.c ../common/bstr.c ../common/strlcat.c ../newspelling/levenshtein.c -lm -D TESTMAIN -g -O0 -W -Wall -Wno-unused-parameter -Wno-unused-variable -D DEBUG
include ../../mk/lib.mk
