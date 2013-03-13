#!/usr/bin/env perl

#########################################################################################################
# Program for starting and stoping the init.d services.							#
#													#
# Thanks to Chas Owens for great exsample code postet online at http://stackoverflow.com/a/911591 	#
#													#
#########################################################################################################
use strict;
use warnings;
my @pids;

if (!$ARGV[0]) {
	print "Usage: stopstart.pl {start|stop|other}\n";
	exit;
}

my @services = qw(boitho-bbdn crawlManager searchdbb suggest boithoad);

foreach my $p (@services) {
    die "could not fork" unless defined(my $pid = fork);
    unless ($pid) { #child execs
	print `$ENV{BOITHOHOME}/init.d/$p $ARGV[0]`;
	exit;
    }
    push @pids, $pid; #parent stores children's pids
}

#wait for all children to finish
foreach my $pid (@pids) {
    waitpid $pid, 0;
}
