#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use Carp;
use Sql::Sql;
use Sql::Shares;
use Sql::Connectors;
#use Common::Collection;
use CGI::State;
use Data::Dumper;
use Page::Add;
use Template;


my $cgi = CGI->new;
my $state = CGI::State->state($cgi);
print $cgi->header('text/html');


my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();

my $add = Page::Add->new($dbh, $state);
my $vars = {};
my $template_file = "";

if ($state->{'submit_first_form'}) {
	#User submittet the first form.
	my $share = $state->{'share'};
	($vars, $template_file) = 
		$add->submit_first_form($vars, $share);
}

elsif ($state->{'submit_second_form'}) {
	# Form "wizard" complete, add to database.
	my $share = $state->{'share'};
	($vars, $template_file) = $add->submit_second_form($vars, $share);
}

else { 
	if (defined($state->{'adding_from_scan'})) {
		# User clicked add share from a scan result (scan.cgi)
		$vars = $add->add_from_scan($vars, $state);
	}
	($vars, $template_file) = $add->show_first_form($vars);
}

my $template  =  Template->new(
	{INCLUDE_PATH => './templates:./templates/add:./templates/common'});

$template->process($template_file, $vars)
		or croak ("template: ", $template->error());