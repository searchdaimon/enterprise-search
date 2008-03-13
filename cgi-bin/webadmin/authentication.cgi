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
use Common::Generic qw(init_root_page);
use Page::Authentication;



# Group: Init
my ($cgi, $state_ptr, $vars, $template, $dbh, $page)
	= init_root_page('/templates/authentication', 'Page::Authentication');

my %state = %{$state_ptr};
my $tpl_file;

my $sqlAuth = Sql::CollectionAuth->new($dbh);
my $common = Common::Generic->new($dbh);

# Group: Submit actions

if (defined $state{'confirm_delete'}) {
	# User has confirmed a delete. 
	$tpl_file = $page->delete_auth($vars, 
				   $state{'id'}, 
				   $ENV{'REQUEST_METHOD'});
}

if (defined $state{'submit_new_pair'}) {
	# User submitted new pair.
	my $auth = $state{'auth'};
	$tpl_file = $page->new_auth($vars,
                    $auth->{username}, 
                    $auth->{password},
                    $auth->{comment});

}

if (defined $state{'submit_changes'}) {
	# User edited a pair.
	my $auth = $state{'auth'};
	$tpl_file = $page->edit_auth($vars, 
                    $auth->{id}, 
                    $auth->{username}, 
                    $auth->{password},
                    $auth->{comment});
}


# Group: View page


if (defined $state{'add_new_pair'}) {
	# User pressed add new pair. Show form.

	($vars, $tpl_file) = $page->show_add_form($vars);
}


if (my $id = $common->request($state{'delete_pair'})) {
	# User pressed pair delete button. Show confirm delete dialog	

	($vars, $tpl_file) = $page->show_delete_confirm($vars, $id);
}
if (my $id = $common->request($state{'edit_pair'})) {
	# User pressed edit button. Show edit form.

	($vars, $tpl_file) = $page->show_edit_form($vars, $id);
}

unless (defined $tpl_file) {
	# Show default page (list all auth)
	$tpl_file = $page->show_auth_list($vars);
}

print $cgi->header('text/html');
$template->process($tpl_file, $vars)
        or croak $template->error();
