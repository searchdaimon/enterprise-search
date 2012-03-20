#!/usr/bin/env perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use Carp;
use Page::Apiinfo;
use Data::Dumper;
use Switch;

my $page = Page::Apiinfo->new();

my %state = $page->get_state();
my $tpl_file;
my $vars = { };

# Actions
if (defined $state{btn}) {
    my $btn = $state{btn};
    my $searchmode = $state{searchmode};
    my $outformat = $state{outformat};
    my $query = $state{query};
    my $username = $state{username};


    if ($btn eq 'Select mode') {
        $tpl_file = $page->show_select_mode($vars, $searchmode);
    }
    elsif ($btn eq 'Generate url') {
        $tpl_file = $page->show_generate_url($vars, $searchmode, $outformat, $query, $username);
    }
    else { croak "Unknown button '$state{btn}'" }
}

if ($state{view}) {
	$tpl_file = $page->show_push($vars, 0)
}

$tpl_file = $page->show_search_select($vars, 0)
    unless $tpl_file;

$page->process_tpl($tpl_file, $vars, 
    tpl_folders => [ 'apiinfo' ],
    ANYCASE => 0
);


