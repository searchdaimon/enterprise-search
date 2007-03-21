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
my $template_file = "logs_main.html";
my $sql = Sql::Sql->new();
my $dbh = $sql->get_connection();

my $logs = Page::Logs->new($dbh);


if (defined($state->{'view'})) {
	my $view = $state->{'view'};
	
	if ($view eq 'search') {
		# User is viewing search log
		($vars, $template_file) = $logs->show_search_log($vars);
	}
}
else {
	# Default page. Fetch logfiles.
	$vars->{'logs'} = $logs->get_logs;
}

if (defined($state->{'view_log'})) {
	# User is viewing the content of a logfile.

	my $log = $state->{'log'};
	$vars = $logs->show_logfile_content($vars, $log);
}


$template->process($template_file, $vars)
        or croak $template->error();
