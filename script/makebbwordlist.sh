#!/bin/sh


BBH="$BOITHOHOME"
LIBASPELL="$BBH/data/dict"

perl $BBH/perl/createaspellwordlist.pl $BBH/var/dictionarywords | aspell --encoding=iso-8859-1 --lang=bb create master /tmp/bb.rws
mv /tmp/bb.rws $LIBASPELL
