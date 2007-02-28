#!/usr/local/bin/perl

use strict;
use warnings;


sub myexec($) {
	my $cmd = shift;

	my @output = `$cmd`;
	return [$?, @output];
}

# return 0 for success, something else for failure
sub bb_phone_home_exec($) {
	my $arg = shift;

	my $retref = myexec('/home/eirik/Boitho/boitho/websearch/src/bb-phone-home/setuidcaller ' . $arg);
	my @ret = @{ $retref };
	$ret[0] = int($ret[0]);
	@ret = map { int($_) } @ret if ($ret[0] == 0);
	return \@ret;
}

# Return [retval, port]
sub bb_phone_home_start() {
	return bb_phone_home_exec('start');
}

# Return [retval]
sub bb_phone_home_stop() {
	return bb_phone_home_exec('stop');
}

# Return [retval, pid]
sub bb_phone_home_running() {
	return bb_phone_home_exec('running');
}



# Tests

use Data::Dumper;

sub testoutput(@) {
	my @hmm = shift;

	print Dumper(@hmm);
}

if (1) {
	testoutput(bb_phone_home_running());
	print " 1<br />\n";
	testoutput(bb_phone_home_stop());
	print " 2<br />\n";
	testoutput(bb_phone_home_running());
	print " 3<br />\n";
	testoutput(bb_phone_home_start());
	print " 4<br />\n";
	testoutput(bb_phone_home_running());
	print " 5<br />\n";
	testoutput(bb_phone_home_stop());
	print " 6<br />\n";
	testoutput(bb_phone_home_running());
	print " 7<br />\n";
}

