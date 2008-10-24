#!/usr/bin/env perl
##
# Usersystem integration

use strict;
use warnings;

use Data::Dumper;
use Page::UserSys;
use Readonly;
use Carp;
use JSON::XS qw(encode_json);

use config qw(%CONFIG); 

my %vars = (
	user_systems => $CONFIG{user_systems},
);
my $page = Page::UserSys->new();
my %state = $page->get_state();

Readonly::Scalar my $API_UPD_MAPPING
	=> "upd_mapping";
Readonly::Scalar my $ACT_EDIT 
	=> "edit";

my $tpl_file;
my $using_api;

if (my $sys_id = $state{'map'}) {
	$tpl_file = $page->show_mapping(\%vars, $sys_id);
}
elsif ($sys_id = $state{edit}) {
	$tpl_file = $page->show_edit(\%vars, $sys_id);
	warn Dumper(\%vars);
}
elsif (my $api_cmd = $state{api}) {
	$using_api = 1;
	if ($api_cmd eq $API_UPD_MAPPING) {
		$page->upd_mapping(\%vars, 
			$state{id}, $state{mapping});
	}
	else { 
		croak "Unknow api cmd '$api_cmd'" 
	}
}
elsif (my $act = $state{action}) {
	croak "Unknown action '$act'"
		if $act ne $ACT_EDIT;

	warn Dumper(\%state);
	$tpl_file = $page->upd_usersys(\%vars, $state{id}, $state{sys});
}
else {
	$tpl_file = $page->show(\%vars);
}

if ($using_api) {
	print $page->get_cgi->header("application/json");
	print encode_json(\%vars);
}
else {
	$page->process_tpl($tpl_file, \%vars, ( tpl_folders => 'usersys'));
}


1;
