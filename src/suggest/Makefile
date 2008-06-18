PROG=	suggest
OBJS=	suggest.o suffixtree.o acl.o #redblacktree.o
CFLAGS=	-g -Wall -W -DTEST_SUGGEST -DWITH_ACL -std=c99
LDFLAGS=
LIBS=	
CC=	/usr/local/bin/gcc


all: ${PROG}

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LDFLAGS} ${LIBS} $(CFLAGS) ../boithoadClientLib/*.c ../common/bstr.c ../common/strlcat.c ../common/debug.c


clean:
	rm -f ${OBJS} ${PROG}
