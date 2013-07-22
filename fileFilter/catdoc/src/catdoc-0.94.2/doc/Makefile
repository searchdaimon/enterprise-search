# Generated automatically from Makefile.in by configure.

# Your C compilier and flags
SHELL = /bin/sh
installroot=
srcdir = .
top_srcdir = ..
INSTALL = /usr/bin/install -c
prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
mandir = ${prefix}/man/man1
manext = .1 
%.txt: %.1
	nroff -mandoc $< |col -bx >$@
%.ps: %.1
	groff -Tps -mandoc $< >$@	

all:
	touch build

dosdoc: catdoc.txt catdoc.ps xls2csv.txt xls2csv.ps catppt.txt catppt.ps
clean:
	rm build
install: install-catdoc install-wordview
install-catdoc: catdoc.1 xls2csv.1 catppt.1
	../mkinstalldirs $(installroot)$(mandir)
	$(INSTALL) -m 644 catdoc.1 $(installroot)$(mandir)/catdoc$(manext)
	$(INSTALL) -m 644 xls2csv.1 $(installroot)$(mandir)/xls2csv$(manext)
	$(INSTALL) -m 644 catppt.1 $(installroot)$(mandir)/catppt$(manext)
install-wordview:	
	../mkinstalldirs $(installroot)$(mandir)
	$(INSTALL) -m 644 wordview.1 $(installroot)$(mandir)/wordview$(manext)
# Following rules are primarily for developers use. I doubt that you
# need to rebuild these versions of documentation
distclean: catppt.txt catppt.ps catdoc.1 catdoc.txt catdoc.ps xls2csv.1  xls2csv.txt xls2csv.ps
	rm Makefile
