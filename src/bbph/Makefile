
PROG=	bbphc
OBJS=	bbphc.o
LDFLAGS=-lssl -lcrypto
CFLAGS=	-Wall -g -W -O1

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)

