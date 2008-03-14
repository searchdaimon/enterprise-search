#!/usr/bin/env perl -w

use warnings;
use strict;

while (<>) {
	my @a = split;
	# Magnus: Kun ord som begynner på a-z, og som ikke inneholder siffer eller '.
	# Fjerner også ord på to bokstaver, da aspell foreslår mange rare rettskrivinger med de:
	if (not substr($a[0],0,1)=~ /[^a-z]/ and not $a[0]=~ /[\'0-9]/ and length($a[0])>2)
	    {
		print $a[0]."\n";
	    }
}
