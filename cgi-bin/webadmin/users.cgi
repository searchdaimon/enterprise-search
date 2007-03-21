#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use CGI::State;
use Carp;
use Template;
use Modules::Boitho::Infoquery;
use Data::Dumper;

my $cgi = CGI->new;
my $state = CGI::State->state($cgi);
print $cgi->header('text/html');

my $vars = { };
my $template = Template->new({INCLUDE_PATH => './templates:./templates/users:./templates/common',});

my $template_file = 'users_main.html';

#my $users = Class::Users->new;
my $iq = Boitho::Infoquery->new;
my $username = $state->{'user'};
if ($username) {
	# Show details for user
	$vars->{'username'} = $username;
	$vars->{'groups'} = $iq->groupsAndCollectionForUser($username);
	$template_file = 'users_details.html';
}
else {
	# List all users

	$vars->{'users'} = $iq->listUsers;


}
$template->process($template_file, $vars)
        or croak $template->error();
