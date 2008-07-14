#!/usr/bin/env perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use Carp;
use Data::Dumper;
use Page::Add;
use Template;

my $page = Page::Add->new();
my %state = $page->get_state();
my $tpl_vars;
my $tpl_file = "";

my %misc_opts;
$misc_opts{from_scan} = $state{from_scan};
if ($state{submit_first_form}) {
	#User submittet the first form.
	
	my $share = $state{share};
	($tpl_vars, $tpl_file) = 
		$page->submit_first_form($tpl_vars, $share, %misc_opts);
}

elsif ($state{submit_second_form}) {
	# Form "wizard" complete, add to database.
	
	my $share = $state{share};
	($tpl_vars, $tpl_file) = $page->submit_second_form($tpl_vars, $share, %misc_opts);
}

else {
	if (defined $state{from_scan_result}) {
		# User clicked add share from a scan result (scan.cgi)
		#carp Dumper($state);
		my $share = $state{share};
		$tpl_vars->{share} = $share;
		$tpl_vars->{from_scan} = $state{from_scan_result};
		$page->vars_from_scan($share, $tpl_vars->{from_scan});
	}
	($tpl_vars, $tpl_file) = $page->show_first_form($tpl_vars);
}

$page->process_tpl($tpl_file, $tpl_vars, (tpl_folders => "add"));
