#!/usr/local/bin/perl
package Boitho::PhoneHome;
use strict;
use warnings;
use config qw($CONFIG);
require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(bb_phone_home_start bb_phone_home_stop bb_phone_home_running);
use File::stat;

sub myexec($$) {
	my $cmd = shift;
	my $arg = shift;

	my $execcmd = $cmd . ' ' . $arg;

	stat($cmd) || die "Unable to find $cmd: $!";

	my @output = `$execcmd`;
	return [$?, @output];
}

# return 0 for success, something else for failure
sub bb_phone_home_exec($) {
	my $arg = shift;

	my $retref = myexec($CONFIG->{'phone_home_path'}, $arg);
	my @ret = @{ $retref };
	$ret[0] = int($ret[0]);
	@ret = map { int($_) } @ret if ($ret[0] == 0);
	return @ret;
}

# Return (retval, port)
sub bb_phone_home_start() {
	return bb_phone_home_exec('start');
}

# Return (retval)
sub bb_phone_home_stop() {
	return bb_phone_home_exec('stop');
}

# Return (retval, pid)
sub bb_phone_home_running() {
	return bb_phone_home_exec('running');
}
1;