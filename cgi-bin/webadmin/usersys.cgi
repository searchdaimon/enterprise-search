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
use Page::UserSys;
use Page::UserSys::Form;
use Page::UserSys::API;

use config qw(%CONFIG); 

my %vars;
my $page = Page::UserSys->new();
my %state = $page->get_state();
#warn Dumper(\%state);

Readonly::Scalar my $API_ADD_MAPPING   => "add_mapping";
Readonly::Scalar my $API_DEL_MAPPING   => "del_mapping";
Readonly::Scalar my $API_LIST_MAPPING  => "list_mapping";
Readonly::Scalar my $API_LIST_UNMAPPED => "list_unmapped";
Readonly::Scalar my $API_AUTOMAP       => "automap";
Readonly::Scalar my $API_CLEAR_MAPPING => "clear_mapping";

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
	my $page = Page::UserSys::Form->new();
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
	my $page = Page::UserSys::Form->new();
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
	my $api = Page::UserSys::API->new();
	if ($api_cmd eq $API_ADD_MAPPING) {
		$api->add_mapping(\%vars, 
			$state{id}, $state{prim_usr}, $state{sec_usr});
	}
	elsif ($api_cmd eq $API_DEL_MAPPING) {
		$api->del_mapping(\%vars,
			$state{id}, $state{prim_usr}, $state{sec_usr});
	}
	elsif ($api_cmd eq $API_LIST_MAPPING) {
		$api->list_mapping(\%vars, $state{id}, $state{offset}, $state{limit});
	}
	elsif ($api_cmd eq $API_LIST_UNMAPPED) {
		$api->list_unmapped(\%vars, 
			$state{id}, 
			$state{system},
			$state{offset}, 
			$state{limit});
	}
	elsif ($api_cmd eq $API_CLEAR_MAPPING) {
		$api->clear_mapping(\%vars, $state{id});
	}
	elsif ($api_cmd eq $API_AUTOMAP) {
		$api->auto_map(\%vars, $state{id});
	}
	else { croak "Unknow api cmd '$api_cmd'" }
}
else {
	my $page = Page::UserSys::Form->new();
	$tpl_file = $page->show(\%vars);
}

if ($using_api) {
	print CGI::header(-type => "application/javascript", -charset => 'UTF8');
	print JSON::XS->new->encode(\%vars);
}
else {
	croak "user systems already set"
		if defined $vars{user_systems};
	$vars{user_systems} = $CONFIG{user_systems};

	$page->process_tpl($tpl_file, \%vars, ( tpl_folders => 'usersys'));
}



1;
