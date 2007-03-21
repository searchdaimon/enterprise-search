
PROG=	fdiskwrap
OBJS=	fdiskwrap.o

CFLAGS=	-g -W -Wall
LIBS=	

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LIBS) 
	chown root $(PROG)
	chmod +s $(PROG)

clean:
	rm -f $(PROG) $(OBJS)

