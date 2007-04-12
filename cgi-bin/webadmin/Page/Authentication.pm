package Page::Authentication;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use config qw($CONFIG);
use Page::Abstract;
use Common::Validate;
our @ISA = qw(Page::Abstract);

use constant DEFAULT_TPL        => 'authentication_main.html';
use constant ADD_FORM_TPL       => 'authentication_pair_form.html';
use constant EDIT_FORM_TPL      => 'authentication_pair_form.html';
use constant CONFIRM_DELETE_TPL => 'authentication_delete_pair.html';

my $sqlAuth;

sub _init {
	my $self = shift;
	my $dbh = $self->{'dbh'};

	$sqlAuth = Sql::CollectionAuth->new($dbh);

}

# Group: Submit actions

##
# Delete a auth pair from db
#
# Adds a success message to top of default page.
sub delete_auth {
	my ($self, $vars, $id, $request_method) = @_;

	valid_numeric({'id' => $id});

	# Force POST request.
	croak ("The action must be a POST request to work.") 
		if (defined $request_method) and not ($request_method eq 'POST');

	$sqlAuth->delete_authentication($id);
	$vars->{'success_delete_pair'} = 1;
	return $self->show_default($vars);
}


##
# Add new auth pair to db.
#
# - On error, show form again.
# - On success, show default page.
sub new_auth {
	my ($self, $vars, $user, $pass) = @_;

	unless ($user) {
		$vars->{'error_no_username'} = 1;
		return $self->show_add_form($vars, $user, $pass);
	}

	$sqlAuth->add($user, $pass);
	$vars->{'success_insert_pair'} = 1;

	return $self->show_default($vars);
}

##
# Edit a auth pair in the db.
#
# - On error, show form again.
# - On success, show default page.
sub edit_auth {
	my ($self, $vars, $id, $user, $pass) = @_;

	valid_numeric({'id' => $id});

	unless ($user) {
		$vars->{'error_no_username'} = 1;
		return $self->show_edit_form($vars, $id, $user, $pass);
	}

	$sqlAuth->update_authentication($id, $user, $pass);
	$vars->{'success_submit_changes'} = 1;

	return $self->show_default($vars);
}


# Group: View pages

##
# Alias for default auth page.
sub show_default {
	my $self = shift;
	return $self->show_auth_list(@_);
}



##
# Show confirmation for deleting a pair. 
sub show_delete_confirm {
	my ($self, $vars, $id) = @_;

	valid_numeric({'id' => $id});	

	my ($auth_ref) = $sqlAuth->get_auth_by_id($id);
	
	$vars->{'authentication'} = $auth_ref;

	return ($vars, CONFIRM_DELETE_TPL);
}

##
# Show form for adding new auth pairs.
sub show_add_form {
	my ($self, $vars, $user, $pass) = @_;

	if (defined $user or defined $pass) {
		$vars->{'authentication'} 
			= { 'username' => $user,
			    'password' => $pass,
			};
	}

	$vars->{'action'} = 'new';
	return ($vars, ADD_FORM_TPL);
}

##
# Show form for editing a auth pair.
sub show_edit_form {
	my ($self, $vars, $id, $user, $pass) = @_;
	
	valid_numeric({'id' => $id});	

	if (defined $user or defined $pass) {
		$vars->{'authentication'} 
			= { 'username' => $user,
			    'password' => $pass,
			};
	}
	else {
		my ($auth_ref) = $sqlAuth->get_auth_by_id($id);
		$vars->{'authentication'} = $auth_ref;
	}
	
	$vars->{'action'} = 'edit';

	return ($vars, EDIT_FORM_TPL);

}

##
# List all username/password.
# This is the default page.
sub show_auth_list {
	my ($self, $vars) = @_;
	my @authpairs = $sqlAuth->get_all_auth;
	$vars->{'authentication'} = \@authpairs;

	return ($vars, DEFAULT_TPL);
}

# Group: Private methods


1;