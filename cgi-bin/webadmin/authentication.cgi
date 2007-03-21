#!/usr/bin/env perl
use strict;
use warnings;
use CGI;
use Carp;
use Sql::Sql;
use Sql::CollectionAuth;
use CGI::State;
use Template;
use Data::Dumper;
use Common::Generic;

my $cgi = CGI->new;
my $state = CGI::State->state($cgi);
print $cgi->header('text/html');

my $vars = { };
my $template = Template->new({INCLUDE_PATH => './templates:./templates/authentication:./templates/common',});

my $sql = Sql::Sql->new;
my $dbh = $sql->get_connection();
my $sqlAuth = Sql::CollectionAuth->new($dbh);
my $common = Common::Generic->new($dbh);

my $template_file = 'authentication_main.html';

my $pair_submit = 1 if ($state->{'submit_new_pair'} 
			or $state->{'submit_changes'});

#Pages that use the default page (only showing a message at the top).
if ($state->{'confirm_delete'}) {
	# User confirms a delete. Delete from database, show success message.
	croak ("The action must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');

	$sqlAuth->delete_authentication($state->{'id'});
	$vars->{'success_delete_pair'} = 1;
}


# Pages that don't use the default page.
if ($state->{'add_new_pair'}) {
	# Show add form
	$vars->{'action'} = 'new';
	$template_file = 'authentication_pair_form.html';
}
elsif (my $id = $common->request($state->{'delete_pair'})) {
	# Show confirm delete dialog
	my $authpair = $sqlAuth->get_authentication($id);
	$vars->{'authentication'} = $authpair;
	$template_file = 'authentication_delete_pair.html';
}
elsif ($id = $common->request($state->{'edit_pair'})) {
	# Show edit username/password dialog.
	my $authpair = $sqlAuth->get_authentication($id);
	$vars->{'authentication'} = $authpair;
	$vars->{'action'} = 'edit';
	$template_file = 'authentication_pair_form.html';
}
elsif ($pair_submit) {
	# Either a new pair, or changes to a pair were submittet.
	my $auth = $state->{'auth'};
	my $action = $state->{'submit_changes'}	? 'edit' : 'new';
	
	unless($auth->{'username'}) {
		# Error, show form again.
		$template_file = "authentication_pair_form.html";
		$vars->{'action'} = $action;
		$vars->{'error_no_username'} = 1;
		$vars->{'authentication'} = [$auth];
	}
	else {
		#All good
		if ($action eq 'new') {
			# Add new pair. Show success message.
			$sqlAuth->insert_authentication($auth->{'username'}, $auth->{'password'});
			$vars->{'success_insert_pair'} = 1;
			
		}
		elsif ($action eq 'edit') {
			# Edit existing pair. Show success message.
			$sqlAuth->update_authentication(
						$auth->{'id'}, 
						$auth->{'username'}, 
						$auth->{'password'});
			$vars->{'success_submit_changes'} = 1;
		}
		
		$vars->{'authentication'} = 
			$sqlAuth->get_authentication;
	}
}
else { 
	# Showing default page (list usernames)
	my $authpairs = $sqlAuth->get_authentication;
	$vars->{'authentication'} = $authpairs;
}


$template->process($template_file, $vars)
        or croak $template->error();