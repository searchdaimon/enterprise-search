#!/bin/sh


BBH="$BOITHOHOME"
LIBASPELL="$BBH/data/dict"

perl $BBH/perl/createaspellwordlist.pl $BBH/var/dictionarywords | aspell --encoding=utf-8 --lang=bb create master /tmp/bb.rws
mv /tmp/bb.rws $LIBASPELL
