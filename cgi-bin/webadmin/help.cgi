#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use CGI::State;
use Carp;
use Template;
use Data::Dumper;
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::PhoneHome qw(bb_phone_home_start bb_phone_home_stop bb_phone_home_running);

my $cgi = CGI->new;
my $state = CGI::State->state($cgi);
print $cgi->header('text/html');

my $vars = { };
my $template = Template->new({INCLUDE_PATH => './templates:./templates/help:./templates/common'});
my $template_file = "help.html";

if (defined($state->{'action'})) {
	my $action = $state->{'action'};
	
	if ($action eq 'phone_home') {
		# User is working with call home
		my $do = $state->{'do'};
		$do = '' unless $do;
		if ($do eq 'start') {
			# User is starting service
			my ($state, $port) = bb_phone_home_start();
			$vars->{'phone_start_retv'} = $state;
			$vars->{'phone_start_port'} = $port;
		}
		elsif ($do eq 'stop') {
			# User is stopping service
			my $state = bb_phone_home_stop();
			$vars->{'phone_stop_retv'} = $state;
		}
		my ($running, $pid) = bb_phone_home_running();

		$vars->{'phone_running_pid'} = $pid;
		$template_file = 'help_phone_home.html';
	}
}

$template->process($template_file, $vars)
 		or croak $template->error();