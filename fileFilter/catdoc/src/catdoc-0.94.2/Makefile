# Generated automatically from Makefile.in by configure.

# Your C compilier and flags
SHELL = /bin/sh


all:
	for i in src doc charsets; do\
	   (cd $$i; $(MAKE) all);\
	done

install:
	for i in src doc charsets; do\
	   (cd $$i; $(MAKE) install);\
	done
clean:	
	for i in src doc charsets; do\
	   (cd $$i; $(MAKE) clean);\
	done
distclean:	
	for i in src doc charsets; do\
	   (cd $$i; $(MAKE) distclean);\
	done
	rm Makefile config.*
dist:
	$(MAKE) -C doc dosdoc
	$(MAKE) distclean
