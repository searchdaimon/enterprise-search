#!/usr/bin/env perl
##
# Usersystem integration

use strict;
use warnings;

use Data::Dumper;
use Page::UserSys;
use Readonly;
use Carp;
use JSON::XS;

use config qw(%CONFIG); 

my %vars = (
	user_systems => $CONFIG{user_systems},
);
my $page = Page::UserSys->new();
my %state = $page->get_state();
#warn Dumper(\%state);

Readonly::Scalar my $API_UPD_MAPPING => "upd_mapping";
Readonly::Scalar my $ACT_EDIT => "edit";
Readonly::Scalar my $ACT_ADD  => "add";
Readonly::Scalar my $ACT_DEL  => "del";
Readonly::Scalar my $VIEW_EDIT => "edit";
Readonly::Scalar my $VIEW_MAP  => "map";
Readonly::Scalar my $VIEW_ADD  => "add";
Readonly::Scalar my $VIEW_DEL  => "del";

my $tpl_file;
my $using_api;

if (my $v = $state{view}) {
	
	if ($v eq $VIEW_MAP) {
		$tpl_file = $page->show_mapping(\%vars, $state{id});
	}
	elsif ($v eq $VIEW_ADD) {
		$tpl_file = $page->show_add(\%vars, $state{part}, $state{sys});
	}
	elsif ($v eq $VIEW_EDIT) {
		$tpl_file = $page->show_edit(\%vars, $state{id});
	}
	elsif ($v eq $VIEW_DEL) {
		$tpl_file = $page->show_del(\%vars, $state{id});
	}
	else { croak "Unknown view '$v'" }
}

elsif (my $a = $state{action}) {
	if ($a eq $ACT_EDIT) {
		$tpl_file = $page->upd_usersys(
			\%vars, $state{id}, $state{sys});
	}
	elsif ($a eq $ACT_ADD) {
		$tpl_file = $page->add(\%vars, $state{sys});
	}
	elsif ($a eq $ACT_DEL) {
		$tpl_file = $page->del(\%vars, $state{id});
	}
	else { croak "Unknown action '$a'" }
}
elsif (my $api_cmd = $state{api}) {
	$using_api = 1;

	if ($api_cmd eq $API_UPD_MAPPING) {
		$page->upd_mapping(\%vars, 
			$state{id}, $state{mapping});
	}
	else { croak "Unknow api cmd '$api_cmd'" }
}
else {
	$tpl_file = $page->show(\%vars);
}

if ($using_api) {
	print $page->get_cgi->header("application/json");
	print JSON::XS->new->encode(\%vars);
}
else {
	$page->process_tpl($tpl_file, \%vars, ( tpl_folders => 'usersys'));
}


1;
