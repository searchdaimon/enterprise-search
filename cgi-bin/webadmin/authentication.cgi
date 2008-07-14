#!/usr/bin/env perl
use strict;
use warnings;
use Carp;
use Sql::CollectionAuth;
use Data::Dumper;
use Page::Authentication;

my $page = Page::Authentication->new();
my %state = $page->get_state();
my $vars = {};

my $tpl_file;

# Group: Submit actions

if (defined $state{confirm_delete}) {
	# User has confirmed a delete. 
	$tpl_file = $page->delete_auth($vars, 
				   $state{id}, 
				   $ENV{REQUEST_METHOD});
}

if (defined $state{submit_new_pair}) {
	# User submitted new pair.
	my $auth = $state{auth};
	$tpl_file = $page->new_auth($vars,
                    $auth->{username}, 
                    $auth->{password},
                    $auth->{comment});

}

if (defined $state{submit_changes}) {
	# User edited a pair.
	my $auth = $state{auth};
	$tpl_file = $page->edit_auth($vars, 
                    $auth->{id}, 
                    $auth->{username}, 
                    $auth->{password},
                    $auth->{comment});
}


# Group: View page


if (defined $state{add_new_pair}) {
	# User pressed add new pair. Show form.

	($vars, $tpl_file) = $page->show_add_form($vars);
}


if (my $form = $state{show}) {
    if ($form eq 'delete') {
	# User pressed pair delete button. Show confirm delete dialog	
	($vars, $tpl_file) = $page->show_delete_confirm($vars, $state{id});
    }
    elsif ($form eq 'edit') {
	# User pressed edit button. Show edit form.
	($vars, $tpl_file) = $page->show_edit_form($vars, $state{id});
    }
    else { croak "Unknown form '$form'" }
}

unless (defined $tpl_file) {
	# Show default page (list all auth)
	$tpl_file = $page->show_auth_list($vars);
}

$page->process_tpl($tpl_file, $vars, (tpl_folders => 'authentication'));
