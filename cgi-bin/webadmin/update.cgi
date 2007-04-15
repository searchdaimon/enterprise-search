#!/usr/bin/perl
use strict;
use warnings;

print "Content-type: text/html\n\n";

my $command = $ENV{'BOITHOHOME'} . '/setuid/yumupdate';


my $in = `$command`;

my @inarr = split(/\n/,$in);

foreach my $i (@inarr) {
	print "$i<br>\n";
}
