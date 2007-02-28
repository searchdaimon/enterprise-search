PROG=	suggest
OBJS=	suggest.o suffixtree.o #redblacktree.o
CFLAGS=	-g -Wall -W -DTEST_SUGGEST
LDFLAGS=
LIBS=	
CC=	gcc


all: ${PROG}

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LDFLAGS} ${LIBS} $(CFLAGS)


clean:
	rm -f ${OBJS} ${PROG}
