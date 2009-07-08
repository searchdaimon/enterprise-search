#!/usr/bin/env perl
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Page::Logs;
use Page::Logs::Statistics;
use Benchmark qw(:all :hireswallclock);


my $vars = { };

my $tpl_file = undef;
my $logs = Page::Logs->new();
my $pageStat = Page::Logs::Statistics->new();
my %state = $logs->get_state();


if (defined($state{view})) {
	my $view = $state{view};
	
	if ($view eq 'search') {
		# User is viewing search log
		($vars, $tpl_file) = $logs->show_search_log($vars);
	}
	elsif ($view eq 'statistics') {
		$tpl_file = $pageStat->show($vars, $state{last}, $state{user});
	}
	else { croak "unknown view '$view'" }
}

if ($state{'log'}) {
	# User is viewing the content of a logfile.
	$tpl_file = $logs->show_logfile_content($vars, 
            $state{'log'});
}

$tpl_file = $logs->show_logfiles($vars)
    unless defined $tpl_file;

$logs->process_tpl($tpl_file, $vars, (tpl_folders => 'logs'));
