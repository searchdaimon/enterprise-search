#
# Makefile for mod_auth_boitho
#
# 'make all' must be run as root, in order
#   to create an suid root binary.
#

APXS = /usr/sbin/apxs
CC = gcc
INSTBINDIR = /usr/local/sbin
DEFINES = -D INSTBINDIR=\\\"$(INSTBINDIR)\\\"
NAME=`cat mod_auth_boitho.spec | sed -ne "s/^Name: //p"`
VERSION=`cat mod_auth_boitho.spec | sed -ne "s/^Version: //p"`

all:
	@echo "make Apache1 or make Apache2"
Apache1: mod_auth_boitho.soApache1
Apache2: mod_auth_boitho.soApache2

install:

	$(APXS) -i -a -n auth_boitho mod_auth_boitho.la

installv13:

	$(APXS) -i -a -n auth_boitho mod_auth_boitho.so

clean: 
	rm -rf mod_auth_boitho.so *.o *.lo *.slo *.la .libs



mod_auth_boitho.soApache1: mod_auth_boitho.c 
	$(APXS) $(DEFINES) -DAPACHE_V13 -c mod_auth_boitho.c ../crawlManager2/client.c ../common/daemon.c ../common/strlcpy.c ../common/bstr.c ../common/strlcat.c

mod_auth_boitho.soApache2: mod_auth_boitho.c 
	$(APXS) $(DEFINES) -c mod_auth_boitho.c ../crawlManager2/client.c ../common/daemon.c ../common/strlcpy.c ../common/bstr.c ../common/strlcat.c

#rpm:
#	mkdir $(NAME)-$(VERSION)
#	cp $(NAME).so $(NAME)-$(VERSION)
#	echo "all:" > $(NAME)-$(VERSION)/Makefile
#	tar zcvf $(HOME)/redhat/SOURCES/$(NAME)-$(VERSION).tar.gz $(NAME)-$(VERSION)/
#	rm -rf $(NAME)-$(VERSION)/
#	rpm -bb mod_auth_boitho.spec
