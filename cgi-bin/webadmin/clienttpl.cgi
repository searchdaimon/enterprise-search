#!/usr/bin/env perl
use strict;
use warnings;
use Page::ClientTPL::Form;
use Page::ClientTPL::API;
use Carp;
use JSON::XS;
use Data::Dumper;

my $pageForm = Page::ClientTPL::Form->new();
my $pageAPI = Page::ClientTPL::API->new();
my %state = $pageForm->get_state();

my (%tpl_vars, %api_vars);
my $tpl_file;

my $using_api = 0;
if (my $api = $state{api}) {
	$using_api = 1;
	if ($api eq 'fetch_source') {
		$pageAPI->fetch_file(\%api_vars, $state{tpl}, $state{file}, $state{file_type});
	}
	elsif ($api eq 'save_source') {
		$pageAPI->save_file(\%api_vars, $state{tpl}, $state{file}, $state{source}, $state{file_type});
	}
	elsif ($api eq 'del_source') {
		$pageAPI->del_file(\%api_vars, $state{tpl}, $state{file}, $state{file_type});
	}
	elsif ($api eq 'rename') {
		$pageAPI->rename_template(\%api_vars, $state{tpl}, $state{new_name});
	}
	elsif ($api eq 'new_source') {
		$pageAPI->create_file(\%api_vars, $state{tpl}, $state{new_file}, $state{file_type});
	}
	else { croak "Unknown api call '$api'" }
}
elsif (my $show = $state{show}) {
	if ($show eq 'edit_tpl') {
		$tpl_file = $pageForm->show_edit(\%tpl_vars, $state{tpl});
	}
	elsif ($show eq 'del') {
		$tpl_file = $pageForm->show_del(\%tpl_vars, $state{tpl});
	}
	else { croak "Unknown show '$show'" }
}
elsif (my $action = $state{act}) {
	if ($action eq 'new') {
		$tpl_file = $pageForm->new_template(\%tpl_vars);
	}
	elsif ($action eq 'del') {
    		croak "not a post request"
        		unless $ENV{REQUEST_METHOD} eq 'POST';
		$tpl_file = $pageForm->del_template(\%tpl_vars, $state{id});
	}
	elsif ($action eq 'clone') {
		$tpl_file = $pageForm->clone_template(\%tpl_vars, $state{tpl});
	}
	else { croak "Unknown action '$action'" }
}
else {
	$tpl_file = $pageForm->show_list(\%tpl_vars);
}

if ($using_api) {
    	my $json = JSON::XS->new;
    	$json->pretty(1);
	print $pageAPI->get_cgi()->header(-type => 'text/plain', -charset => "UTF-8");

	print $json->encode(\%api_vars);
}
else {
	$pageForm->process_tpl($tpl_file, \%tpl_vars, ( tpl_folders => 'clienttpl' ));
}
1;
