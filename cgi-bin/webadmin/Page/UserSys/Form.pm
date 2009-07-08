package Page::UserSys::Form;
use strict;
use warnings;

use Data::Dumper;
use Readonly;
use Carp;
use Params::Validate qw(validate_pos);

BEGIN { push @INC, $ENV{BOITHOHOME}, "/Modules" }

use Sql::Shares;
use Data::UserSys;
use Boitho::Infoquery;
use config qw(%CONFIG);

use Page::UserSys;
our @ISA = qw(Page::UserSys);


Readonly::Scalar my $TPL_LIST    => 'usersys_main.html';
Readonly::Scalar my $TPL_MAPPING => 'usersys_mapping.html';
Readonly::Scalar my $TPL_EDIT    => 'usersys_edit.html';
Readonly::Scalar my $TPL_ADD     => 'usersys_add.html';
Readonly::Scalar my $TPL_DEL     => 'usersys_del.html';

Readonly::Scalar my $ADD_PART_2 => 2;
Readonly::Scalar my $CLEAN_ERRMSG_REGEX => qr{at .*? line \d+$};


sub _init {
	my $s = shift;
	$s->{sql_param} = Sql::SystemParam->new($s->{dbh});
	$s->{iq} = Boitho::Infoquery->new($CONFIG{infoquery});
	$s->SUPER::_init(@_);
}

sub show {
	my ($s, $vars) = @_;
	my $iq = $s->{iq};

	$vars->{systems} = [ map {
		$_->{user_count} = $iq->countUsers($_->{id});
		$_->{user_count_error} = $iq->error
			if $_->{user_count} == -1;
		$_;
	} $s->{sql_sys}->list() ];
	
	$TPL_LIST;
}

sub show_add { 
	my ($s, $vars, $part, $sys_ref) = @_;

	if (!$part) { #part 1
		return $TPL_ADD
	}
	croak "invalid part '$part'"
		unless $part =~ /^$ADD_PART_2$/;
	
	croak "invalid system '$sys_ref->{connector}'"
		unless $CONFIG{user_systems}{$sys_ref->{connector}};
	
	$vars->{sys}{params} = { $s->_build_params(
		$sys_ref->{connector}, 
		%{$sys_ref->{params}},
	)};

	$vars->{sys}{name} = $sys_ref->{name};
	$vars->{sys}{connector} = $sys_ref->{connector};

	$vars->{part2} = 1;

	return $TPL_ADD;
}
##
# custom params, as used in HTML template.
sub _build_params {
	my ($s, $conn_id, %param_values) = @_;
	my @params = $s->{sql_param}->get({ 
		connector => $conn_id
	});
	return map {
		 $_->{param} => { 
			note     => $_->{note}, 
		 	required => $_->{required},
			value    => $param_values{$_->{param}},
		} 
	} @params;
}

sub show_edit {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) }, 0);
	my ($s, $vars, $sys_id, $sys_attr) = @_;
	my $sys = Data::UserSys->new($s->{dbh}, $sys_id);
	if (!$sys_attr) {
		$vars->{sys} = { $sys->get() };
	}
	else {
		$vars->{sys} = $sys_attr;
		$vars->{sys}{params} = { $s->_build_params(
			$sys->get('connector'), 
			%{$sys_attr->{params}}
		) };
		$vars->{sys}{id} = $sys_id;
	}
	$TPL_EDIT;
}

sub show_mapping {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
	my ($s, $vars, $sec_id) = @_;

	croak "Invalid id '$sec_id'"
		unless $s->{sql_sys}->exists({ id => $sec_id });

	my $prim_id = $s->{sql_sys}->primary_id();
	unless (defined $prim_id) {
		$vars->{error} = "No primary system has been added. "
			. "There is nothing to map against.";
		return $s->show();
	}

	$vars->{prim} = $s->{sql_sys}->get({ id => $prim_id });
	$vars->{sec}  = $s->{sql_sys}->get({ id => $sec_id });

	#$vars->{prim}{users} = [ $s->list_users($prim_id) ];
	#$vars->{sec}{users}  = [ $s->list_users($sec_id) ];

	$TPL_MAPPING;
}


sub show_del {
	my ($s, $vars, $sys_id) = @_;

#	if (my $err = $s->_del_error($sys_id)) {
#		$vars->{error} = $err;
#		return $s->show($vars);
#	}

	my $name = $s->{sql_sys}->get({ 
		id => $sys_id 
	}, 'name')->{name};

	$vars->{name} = $name;
	$vars->{id} = $sys_id;

	$TPL_DEL;
}

sub del {
	my ($s, $vars, $sys_id) = @_;

	# Validate request
	croak "The operation must be a POST request to work."
		unless $ENV{REQUEST_METHOD} eq 'POST';


	# Del system w/ params
	my $sys = Data::UserSys->new($s->{dbh}, $sys_id);
	my $name = $sys->get('name');
	eval {
		$sys->restrictive_del();
	};
	if ($@) {
		$vars->{error} = $@;
		$vars->{error} =~ s/$CLEAN_ERRMSG_REGEX//;
		return $s->show($vars);
	}

	# Delete mapping
	$s->{sql_mapping}->delete({ system => $s->{sys}{id} });

	$vars->{ok} = "User system '$name' deleted.";
	
	return $s->show($vars);
}

sub upd_usersys {
	validate_pos(@_, 1, 1, { regex => qr{^\d+$} }, 1);
	my ($s, $vars, $sys_id, $sys_attr) = @_;

	if (defined $sys_attr->{name} && $sys_attr->{name} eq q{}) {
		delete $sys_attr->{name};
	}
		
	my $sys = Data::UserSys->new($s->{dbh}, $sys_id);
	eval { $sys->update(%{$sys_attr}) };
	if ($@) {
		$vars->{error} = $@;
		$vars->{error} =~ s/$CLEAN_ERRMSG_REGEX//;
		return $s->show_edit($vars, $sys_id, $sys_attr);
	}
	
	my $users = $s->{iq}->countUsers($sys->get('id'));
	if ($users == -1) {
		$vars->{error} = "System updated, but unable to contact system. "
			. $s->{iq}->error;
	}
	else {
		$vars->{ok} = "System updated. System has $users users.";
	}
	return $s->show_edit($vars, $sys_id);
}



sub add {
	my ($s, $vars, $sys_ref) = @_;
	my $users;
	my $sys;
	eval {
		$sys = Data::UserSys->create($s->{dbh}, %{$sys_ref});
		$users = $s->{iq}->countUsers($sys->get('id'));
		croak "Unable to connect to user system. " . $s->{iq}->error
			if $users == -1;
	};
	if ($@) {
		$vars->{error} = $@;
		$vars->{error} =~ s/$CLEAN_ERRMSG_REGEX//;
		eval { $sys->del() if defined $sys }; 
		if ($@) {
			warn "Error while cleaning up after adding invalid system: " . $@;
		}
		return $s->show_add($vars, $ADD_PART_2, $sys_ref);
	}
	$vars->{ok} = "System created. System has $users users.";
	return $s->show($vars);
}

1;
