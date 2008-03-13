#!/usr/bin/env perl
use strict;
use warnings;
BEGIN {
        push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}

use CGI;
use Carp;
use CGI::State;
use Template;
use Page::Updates;
use Data::Dumper;
use Switch;

use Common::Generic qw(init_root_page);

my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/updates:./templates/common/network', 'Page::Updates');

my %state = %{$state_ptr};
my $tpl_file;

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
        $tpl_file = $page->upload_pkg($vars, $state{pkg_file});
    }
    elsif ($btn->{run_install}) {
        $tpl_file = $page->install_uploaded($vars);
    }
    elsif ($btn->{del_pkg}) {
        $tpl_file = $page->del_pkg($vars, $state{file});
    }
    else { croak "Unknown button" }
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

print $cgi->header('text/html');
$template->process($tpl_file, $vars)
        or croak $template->error() . "\n";


