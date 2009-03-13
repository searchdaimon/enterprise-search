# Uncomment if you want debugging info 
CFLAGS = -g -Wall -O2

PERLEMBED = -rdynamic `perl -MExtUtils::Embed -e ccopts`
CFLAGS += $(PERLEMBED)

CC=	/usr/local/bin/gcc -Wall -O2 $(CFLAGS)
LIB=	libperlembed.a $
OBJS=	perlxsi.o perlembed.o

all: $(LIB)

$(LIB): $(OBJS)
	ar rc $(LIB) $(OBJS)
	ranlib $(LIB)

clean:
	rm -rf $(OBJS) $(LIBS)

.PHONY:	all clean test
