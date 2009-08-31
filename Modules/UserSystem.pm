package UserSystem;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use Exporter qw(import);
our @EXPORT = qw(_internal_list_users _internal_list_groups _internal_get_name _internal_authenticate);

sub _internal_list_users {
	my (undef, $params) = @_;

	# Don't send internal references to the API.
	my $users_ref = $params->{_internal_users};
	delete $params->{_internal_users};



 	# return users back to C.
	@{$users_ref} = PerlUserSystem::list_users($params);

	return 1;
}

sub _internal_list_groups {
	my (undef, $params) = @_;

	my $groups_ref = $params->{_internal_groups};
	delete $params->{_internal_groups};

	my $group_user = $params->{_in_username};
	delete $params->{_in_username};
	$group_user = ${$group_user}; 


	@{$groups_ref} = PerlUserSystem::list_groups($params, $group_user);

	return 1;
}

sub _internal_get_name {
	my (undef, $params) = @_;
	croak "Function get_name is deprecated. Name stored in DB (systemConnector)";

#	my $name_ref = $params->{_internal_name};
#	delete $params->{_internal_name};
#	
#	${$name_ref} = PerlUserSystem::get_name($params);

	return 1;
}


sub _internal_authenticate {
	my (undef, $params) = @_;
	my ($user, $pass) = ($params->{_in_user}, $params->{_in_pass});
	my $ret_val = $params->{_out_retval};
	delete $params->{_in_user};
	delete $params->{_in_pass};
	delete $params->{_out_retval};
	${ $ret_val } = PerlUserSystem::authenticate($params, $user, $pass);
	return 1;
}

1;
