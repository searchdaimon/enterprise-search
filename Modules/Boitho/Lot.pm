
package Boitho::Lot;

use strict;
use warnings;

my @paths;

BEGIN {
	# init

	open(my $FH, "< ".$ENV{BOITHOHOME}."/config/maplist.conf");
	my $i = 0;
	while (<$FH>) {
		chomp;
		my $dir = $_;

		$paths[$i] = $dir;
		$i += 1;
	}
	close($FH);
};

sub GetFilPathForLot($$) {
	my ($lotnr, $subname) = @_;
	my $path;

	$path = $paths[$lotnr%64] . "/" . $lotnr . "/";
	if ($subname ne 'www') {
		$path .= $subname . "/";
	}

	return $path;
}



1;
