#!/usr/bin/env perl -w

use warnings;
use strict;

while (<>) {
	my @a = split;
	if (not substr($a[0],0,1)=~ /[^a-z]/ and not $a[0]=~ /[\'0-9]/)
	    {
		print $a[0]."\n";
	    }
}
