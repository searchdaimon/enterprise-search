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
	$(APXS) $(DEFINES) -DAPACHE_V13 -c mod_auth_boitho.c ../boithoadClientLib/liboithoaut.a

mod_auth_boitho.soApache2: mod_auth_boitho.c 
	$(APXS) $(DEFINES) -c mod_auth_boitho.c ../boithoadClientLib/liboithoaut.a

