# Generated automatically from Makefile.in by configure.
# Your C compilier and flags
SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}
installroot = 

bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
confdir = ${prefix}/etc
datadir = ${prefix}/share
mandir = ${prefix}/man
INSTALL = /usr/bin/install -c
WISH = /usr/bin/wish
CC = gcc
COMPAT_OBJ=
build_targets = catdoc xls2csv catppt wordview
install_targets = install-catdoc install-wordview
CHARSETPATH=${datadir}/catdoc
# Flags to pass to your C compilier
# remove -O2 on HP/UX 9. It is known to cause problems
FLAGS=-I. -g -O2 -Wall  -DHAVE_CONFIG_H -DCATDOC_VERSION=\"0.94.2\" -DCHARSETPATH=\"${CHARSETPATH}\"

#Compile-time configuration of catdoc itself. Edit according to taste

#Directory, where all system-wide stuff resides
DATA_DIR=$(datadir)/catdoc
#directory, where catdoc binary would be placed
#directory where to put man page

#System-wide configuration file
SYSTEMRC=$(confdir)/catdocrc

#Per-user configration file (in user's home directory
USERRC=.catdocrc

# path where to search for charsets. may be colon-separated list of dirs

# Extension for map of special character
SPEC_EXT=.specchars

# Extension for map of substitutes (chars which are missing from target
# charset)
REPL_EXT=.replchars

# Target charset - one used on your system console or xterm
TARGET_CHARSET=koi8-r

# Source charset - one which your Word files are usially in unless they
# are UNICODE
SOURCE_CHARSET=cp1251

# Character which would be printed instead of ones, not found in charset
# or replacement map
UNKNOWN_CHAR=?

# END OF COMPILE-TIME CONFIGURATION

DEFINES=-DSYSTEMRC=\"$(SYSTEMRC)\" -DUSERRC=\"$(USERRC)\"

CFLAGS=$(FLAGS) $(DEFINES) -DHAVE_CONFIG_H

COMMONOBJ=charsets.o substmap.o fileutil.o confutil.o numutils.o ole.o 

OBJ=catdoc.o reader.o writer.o analyze.o rtfread.o $(COMMONOBJ)

OBJXLS=xls2csv.o sheet.o xlsparse.o $(COMMONOBJ)

OBJPPT=catppt.o pptparse.o $(COMMONOBJ)
#.c.o:
#	$(CC) -c $(CFLAGS) $*
all: catdoc xls2csv catppt wordview
catdoc: $(OBJ)
	$(CC)  -o catdoc  $(OBJ)
xls2csv: $(OBJXLS)
	$(CC) -o xls2csv $(OBJXLS) -lm

catppt: $(OBJPPT)
	$(CC) -o catppt $(OBJPPT) -lm

install: install-catdoc install-wordview
install-catdoc:catdoc xls2csv catppt 
	../mkinstalldirs $(installroot)$(bindir)
	$(INSTALL) -m 755 catdoc $(installroot)$(bindir)/catdoc
	$(INSTALL) -m 755 xls2csv $(installroot)$(bindir)/xls2csv
	$(INSTALL) -m 755 catppt $(installroot)$(bindir)/catppt
install-wordview: wordview 
	../mkinstalldirs $(installroot)$(bindir)
	$(INSTALL) -m 755 wordview $(installroot)$(bindir)/wordview
wordview: wordview.tcl
	echo "#! $(WISH)" >wordview
	echo set charset_lib "\"$(CHARSETPATH)\"">>wordview
	cat wordview.tcl >>wordview
	chmod 0755 wordview
strftime.o: ../compat/strftime.c
	$(CC) -c $(FLAGS) -o $@ $*
clean:
	rm -f *.o catdoc wordview xls2csv catppt
distclean: clean
	rm Makefile
