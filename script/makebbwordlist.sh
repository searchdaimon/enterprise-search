#!/bin/sh


BBH="$BOITHOHOME"
LIBASPELL="$BBH/data/dict"

perl $BBH/perl/createaspellwordlist.pl $BBH/var/dictionarywords | aspell --data-dir=$BBH/data/dict --encoding=iso-8859-1 --lang=bb create master /tmp/bb.rws
cp /tmp/bb.rws $LIBASPELL
rm /tmp/bb.rws
