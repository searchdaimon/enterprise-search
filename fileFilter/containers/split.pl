#!/usr/bin/perl

use strict;
use warnings;

use Archive::Extract;
use File::Temp;

my $file = shift @ARGV or die "Usage: ./split.pl file" ;

my $ae = Archive::Extract->new(archive => $file);

my $tmpdir = mkdtemp('/tmp/containersplitXXXXXX');
my $ok = $ae->extract(to => $tmpdir);

foreach (@{ $ae->files }) {
	print 'dat';
	print ' ';
	print $tmpdir . "/" . $_;
	print "\n";
}

