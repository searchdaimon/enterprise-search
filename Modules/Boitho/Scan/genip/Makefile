##
## $Id: Makefile,v 1.1 2007/06/06 22:04:08 dagurval Exp $
##

## Uncomment for debugging
#DEBUG = -g

##
## General definitions
##
CFLAGS += -Wall $(DEBUG)
OBJS = targets.o
EXES = genip
LDLIBS = $(OBJS)

##
## Front end targets
##
all: $(OBJS) $(EXES)

install:
	install -s genip /usr/local/bin
	install -m 644 genip.1 /usr/local/man/man1

uninstall:
	-rm /usr/local/bin/genip
	-rm /usr/local/man/man1/genip.1

manhtml:
	@nroff -man genip.1 | man2html -compress > genip.html

clean:
	rm -f $(OBJS) $(EXES) core *.core genip.html
