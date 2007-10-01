#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use Carp;
use CGI::State;
use Template;
use Data::Dumper;
use Page::Logs;
use Sql::Sql;

my $cgi = CGI->new;
my $state = CGI::State->state($cgi);
print $cgi->header('text/html');

my $vars = { };
my $template = Template->new(
	{INCLUDE_PATH => './templates:./templates/logs:./templates/common',});
my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();

my $tpl_file = undef;
my $logs = Page::Logs->new($dbh, $state->{lines});


if (defined($state->{'view'})) {
	my $view = $state->{'view'};
	
	if ($view eq 'search') {
		# User is viewing search log
		($vars, $tpl_file) = $logs->show_search_log($vars);
	}
}

if ($state->{'log'}) {
	# User is viewing the content of a logfile.
	$tpl_file = $logs->show_logfile_content($vars, 
            $state->{'log'});
}

$tpl_file = $logs->show_logfiles($vars)
    unless defined $tpl_file;

$template->process($tpl_file, $vars)
        or croak $template->error();
