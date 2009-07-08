#!/usr/bin/env perl
use strict;
use warnings;
use Page::Synonym::Form;
use Page::Synonym::API;
use Carp;
use JSON::XS;
use Data::Dumper;

my $pageForm = Page::Synonym::Form->new();
my $pageAPI = Page::Synonym::API->new();
my %state = $pageForm->get_state();

my (%tpl_vars, %api_vars);
my $tpl_file;

my $using_api = 0;
if (my $api = $state{api}) {
	$using_api = 1;
	if ($api eq "list") { 
		$pageAPI->synonym_list(\%api_vars);
	}
	elsif ($api eq "add") {
		$pageAPI->add(\%api_vars, $state{list}, $state{language});
	}
	elsif ($api eq "update") {
		carp Dumper(\%state);
		$pageAPI->update(\%api_vars, $state{group}, $state{list}, $state{language});
	}
	elsif ($api eq "delete") {
		$pageAPI->delete(\%api_vars, $state{group});
	}
	else { croak "Unknown api call '$api'" }
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
	$pageForm->process_tpl($tpl_file, \%tpl_vars, ( tpl_folders => 'synonym' ));
}
1;
