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
	# Magnus: Kun ord som begynner på a-z, og som ikke inneholder siffer eller '.
	# Fjerner også ord på to bokstaver, da aspell foreslår mange rare rettskrivinger med de:
	if (not $w =~ /^[^a-z]/ and not $w =~ /[\'0-9]/ and length($w) > 2)
	# Magnus: Vi fjerner utf8-tegn midlertidig, ser ut til å skape en del problemer:
#	if (not $a[0] =~ /[^a-z]/ and length($a[0])>2)
	    {
		print $w."\n";
	    }
}
