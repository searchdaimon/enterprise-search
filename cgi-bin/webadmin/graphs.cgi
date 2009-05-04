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

$tpl_file = 'graphs.html';

$vars->{last} = $state{last};
my @A = sort mysort @{$page->get_users($state{last})};
$vars->{users} = \@A;

if ($state{user}) {
	$vars->{user} = $state{user};
	$vars->{userstring} = "user=" . $state{user} . "&";
} else {
	$vars->{user} = '';
}

$page->process_tpl($tpl_file, $vars, (tpl_folders => 'graphs'));


sub mysort {
    lc($a->{user}) cmp lc($b->{user});
}
