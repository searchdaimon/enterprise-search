#!/usr/bin/perl
#
#

use strict;
use warnings;

my $statefile = $ENV{BOITHOHOME} . '/var/phonehome.state';
my $pidfile = $ENV{BOITHOHOME} . '/var/bb-phone-home-keepalive-pid-file';
#my $phonehomeclient = $ENV{BOITHOHOME} . '/bin/bb-client.pl';
my $phonehomeclient = 'bb-client.pl';

sub read_line($) {
	my ($file) = @_;

	open(my $fh, "< $file") || (print STDERR "Unable to open file: $file: $!\n" and return undef);
	my $line = <$fh>;
	close($fh);
	chomp($line);

	return $line;
}

sub alivep {
	my $ret;

	`perl $phonehomeclient running`;
	$ret = $? >> 8; # Grab return code

	return $ret == 0 ? 1 : 0;
}

sub check_state {
	print "Checking state... ";

	my $state = read_line($statefile);
	return undef if !defined($state);

	my $alive = alivep();

	if ($state eq 'alive') {
		# Check that phonehome is up, start it if not.

		print "alive.\n";
		
		# We are running, as we should be
		return 0 if ($alive == 1);

		print "Starting phonehome client.\n";
		`perl $phonehomeclient start`;

		return 1;
	} else {
		# phone home should no longer be alive

		print "dead.\n";

		# We are dead, as we should be
		return 0 if ($alive == 0);

		print "Trying to kill phonehome client.\n";
		`perl $phonehomeclient stop`;
		return 1;
	}
}

# Write pidfile
open(my $fh, "> $pidfile");
print $fh "$$\n";
close($fh);

# Install signal handler
$SIG{USR1}  = 'check_state';

do {
	check_state();
	sleep 60; # Check every minute
} while (1);
