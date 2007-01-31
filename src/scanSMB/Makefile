#
CC = gcc

#SAMBA_INCL = ../../source/include
#EXTLIB_INCL = -I/usr/include/gtk-1.2 \
#	      -I/usr/include/glib-1.2 \
#	      -I/usr/lib/glib/include
#
#DEFS = -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
#CFLAGS = -O0 -g -I$(SAMBA_INCL) $(EXTLIB_INCL) $(DEFS)

DEFS = -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
CFLAGS = -O0 -g 

LDFLAGS = -L/usr/local/samba/lib

all:	testbrowse


testbrowse: testbrowse.o
	@echo Linking testbrowse
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -lsmbclient -lpopt $<


clean:
	@rm -f *.o *~ $(TESTS)
	@make -C smbwrapper clean
