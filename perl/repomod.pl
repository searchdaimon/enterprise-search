#!/usr/bin/perl -w

use warnings;
use strict;

use Data::Dumper;
use Config::Tiny;

my %repos = ( production => 1,
	testing => 1,
	devel => 1 );

if ($#ARGV != 0) {
	print STDERR "Syntax error\n";
	print STDERR "repomod [production|testing|devel]\n";
	exit(1);
}

my $repo = $ARGV[0];

if (not exists($repos{$repo})) {
	printf STDERR "Unknown repo: $repo\n";
	exit(1);
}


# Create a config
my $config = Config::Tiny->new();

# Open the config
$config = Config::Tiny->read('/etc/yum.repos.d/searchdaimon.repo')
    or die "error opening config: ", $!;


# Set repos
foreach my $_repo (keys %repos) {
	$config->{'boitho-' . $_repo}->{enabled} = ($_repo eq $repo ? '1' : '0');
}

# Save a config
$config->write('/etc/yum.repos.d/searchdaimon.repo.tmp');
# And the final atomic move option
system('mv /etc/yum.repos.d/searchdaimon.repo.tmp /etc/yum.repos.d/searchdaimon.repo');
