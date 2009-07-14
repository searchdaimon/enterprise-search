#!/usr/bin/env perl
use strict;
use warnings;
my @services = qw(boitho-bbdn crawlManager searchdbb suggest);
for (@services) {
	next if fork;
	print `$ENV{BOITHOHOME}/init.d/$_ stop`;
	$ARGV[0] || print `$ENV{BOITHOHOME}/init.d/$_ start`;
	exit;
}
