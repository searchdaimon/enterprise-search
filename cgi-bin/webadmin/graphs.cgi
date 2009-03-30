#!/usr/bin/env perl
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Graphs;
use Benchmark qw(:all :hireswallclock);

use constant DEFAULT_LAST_VALUE         => 30;
use constant DEFAULT_USERS_VALUE        => 15;

my $vars = { };

my $tpl_file = undef;
my $page = Page::Graphs->new();
my %state = $page->get_state();

$state{last} = DEFAULT_LAST_VALUE unless (defined($state{last}));
$state{last} = int($state{last}) or croak "Last must be an integer";

$tpl_file = 'graphs_main.html';

$vars->{last} = $state{last};

if ($state{user}) {
	$tpl_file = 'graphs_user.html';
	$vars->{user} = $state{user};
} else {
	# Main
	$vars->{users} = $page->get_users($state{last});
}

$page->process_tpl($tpl_file, $vars, (tpl_folders => 'graphs'));
