#!/bin/sh


BBH="$BOITHOHOME"
LIBASPELL="/home/eirik/.root/lib/aspell-0.60/"

perl $BBH/perl/createaspellwordlist.pl $BBH/var/dictionarywords | aspell --lang=bb create master /tmp/bb.rws
mv /tmp/bb.rws $LIBASPELL
