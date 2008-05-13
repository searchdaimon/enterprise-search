#!/usr/local/bin/perl

package SD::Df;
use warnings;
use strict;
use Carp;
use Exporter qw(import);
our @EXPORT = qw(df);

sub df {
	my ($path, $bytes) = @_;
        croak "arg 1 path missing"
            unless defined $path;
        $bytes = 1 
            unless defined $bytes;

	my $partinfo = `df --block-size=$bytes $path | tail -n1`;
	chomp $partinfo;
	#print $partinfo . "\n";

	# Parses: /dev/sda5             40305932   3921520  34336932  11% /
	my ($part, $blocks, $used, $avail, $useper, $mounted) = $partinfo =~ /^([^\s]+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d{1,3})\%\s+(.*)$/;

	return undef unless defined $part and defined $blocks and defined $used and
	    defined $avail and defined $useper and defined $mounted;

	return {
		size => $blocks,
		used => $used,
		useper => $useper,
		mounted => $mounted,
		part => $part
	};
}


#use Data::Dumper;

#print Dumper(df("/boot", 1));

1;
