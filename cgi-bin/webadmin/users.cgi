#!/usr/bin/env perl
use strict;
use warnings;

use Data::Dumper;

use Page::Users;

my $page = Page::Users->new();
my %state = $page->get_state();

my $tpl_file;
my $vars = { };

my $username = $state{'user'};
if ($username) {
	$tpl_file = $page->show_usr_details($vars, $username);
}
elsif ($state{upd_usr_access}) {
	$tpl_file = $page->upd_usr_access($vars, $state{users});
}
else {
	$tpl_file = $page->show_usr_list($vars);
}

$page->process_tpl($tpl_file, $vars, 
	tpl_folders => ['users', 'common']);
