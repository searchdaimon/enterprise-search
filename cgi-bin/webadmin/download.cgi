#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use Carp;;
use CGI::State;
use Page::Logs;

my $cgi = CGI->new;
my $state = CGI::State->state($cgi);
print $cgi->header('text/plain');

if ($state->{'logfile'}) {
	Page::Logs::download($state->{'logfile'});
}
else {
	print "No logfile selected.";
}
