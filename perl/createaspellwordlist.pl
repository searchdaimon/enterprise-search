#!/usr/bin/env perl -w

use warnings;
use strict;

use Text::Iconv;
my $converter = Text::Iconv->new("utf-8", "iso-8859-1");
#my $converter = Text::Iconv->new("utf-8", "utf-8");

while (<>) {
	my @a = split;
	my $w = $converter->convert($a[0]);
	#my $w = $a[0];
	if (not defined $w) { # Unable to convert
		next;
	}
	# Magnus: Only words that start with a-z and do not contain digits or '.
	# Also drop two-letter words, since aspell suggests many odd spellings for them:
	if (not $w =~ /^[^a-z]/ and not $w =~ /[\'0-9]/ and length($w) > 2)
	# Magnus: We temporarily remove utf8 characters, they seem to cause some problems:
#	if (not $a[0] =~ /[^a-z]/ and length($a[0])>2)
	    {
		print $w."\n";
	    }
}
