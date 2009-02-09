#!/usr/bin/env perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use Carp;
use Page::Updates;
use Data::Dumper;
use Switch;

my $page = Page::Updates->new();

my %state = $page->get_state();
my $tpl_file;
my $vars = { };

# Actions
if (defined $state{btn}) {
    my $btn = $state{btn};

    if ($btn->{list_installed}) {
        $tpl_file = $page->show_list($vars, 1);
    }
    elsif ($btn->{check_updates}) {
        $tpl_file = $page->show_updates($vars, 1);
    }
    elsif ($btn->{install_updates}) {
        $tpl_file = $page->update_packages($vars);
    }
    elsif ($btn->{upload}) {
        $tpl_file = $page->upload_pkg($vars, $state{upload}->{pkg_file});
    }
    elsif ($btn->{run_install}) {
        $tpl_file = $page->install_uploaded($vars);
    }
    elsif ($btn->{del_pkg}) {
        $tpl_file = $page->del_pkg($vars, $state{file});
    }
    else { croak "Unknown button '$state{btn}'" }
}

# Views
if ($state{view}) {
    my $v = $state{view};
    if ($v eq 'advanced') {
        $tpl_file = $page->show_advanced($vars);
    }
    elsif ($v eq 'updates') {
        $tpl_file = $page->show_updates($vars);
    }
    else { croak "unknown view" }
}

$tpl_file = $page->show_list($vars, 0)
    unless $tpl_file;

$page->process_tpl($tpl_file, $vars, 
    tpl_folders => ['updates', 'common/network'],
    ANYCASE => 0
);


