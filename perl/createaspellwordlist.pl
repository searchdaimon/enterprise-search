#!/usr/bin/env perl -w

use warnings;
use strict;

while (<>) {
	my @a = split;
#	if ($a[0] >= 'a' and $a[0] <= 'z')
	if (not substr($a[0],0,1)=~ /[^a-z]/ and not $a[0]=~ /[^a-z_\']/)
	    {
		print $a[0]."\n";
	    }
}
