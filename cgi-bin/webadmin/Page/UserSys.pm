package Page::UserSys;
use strict;
use warnings;

use Data::Dumper;
use Params::Validate qw(validate_pos);
use Readonly;
use Carp;

BEGIN { unshift @INC, "$ENV{BOITHOHOME}/Modules" }
use Boitho::Infoquery;
use Page::Abstract;
use Data::UserSys;
use Sql::System;
use Sql::SystemMapping;
our @ISA = qw(Page::Abstract);

Readonly::Scalar my $TPL_LIST    => 'usersys_main.html';
Readonly::Scalar my $TPL_MAPPING => 'usersys_mapping.html';
Readonly::Scalar my $TPL_EDIT    => 'usersys_edit.html';
Readonly::Scalar my $TPL_ADD     => 'usersys_add.html';

use config qw(%CONFIG);

sub _init {
	my $s = shift;
	$s->{iq} = Boitho::Infoquery->new($CONFIG{infoquery});
	$s->{sql_sys} = Sql::System->new($s->{dbh});
	$s->{sql_mapping} = Sql::SystemMapping->new($s->{dbh});
	$s->{sql_param} = Sql::SystemParam->new($s->{dbh});
}

sub show {
	my ($s, $vars) = @_;
	$vars->{systems} = [$s->{sql_sys}->list()];
	
	$TPL_LIST;
}

sub show_add { 
	my ($s, $vars, $part, $sys_ref) = @_;
	if (!$part) { #part 1
		return $TPL_ADD
	}
	croak "invalid part '$part'"
		unless $part =~ /^2$/;
	
	croak "invalid system '$sys_ref->{connector}'"
		unless $CONFIG{user_systems}{$sys_ref->{connector}};

	my @params = $s->{sql_param}->get({ 
		connector => $sys_ref->{connector} 
	});
	$vars->{sys}{params} = { map {
		 $_->{param} => { note => $_->{note} } 
	} @params };

	$vars->{part2} = 1;
	$vars->{sys}{connector} = $sys_ref->{connector};
	$vars->{sys}{name} = $sys_ref->{name};

	return $TPL_ADD;
}

sub show_edit {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
	my ($s, $vars, $sys_id) = @_;
	my $sys = Data::UserSys->new($s->{dbh}, $sys_id);
	$vars->{sys} = { $sys->get() };
	
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

	$vars->{prim}{users} = [ $s->list_users($prim_id) ];
	$vars->{sec}{users}  = [ $s->list_users($sec_id) ];

	 my @mapping = $s->{sql_mapping}->get({ 'system' => $sec_id });

	$vars->{mapping} = \@mapping;
	$vars->{prim}{mapped} = { map { $_->{prim_usr} => 1 } @mapping };
	$vars->{sec}{mapped}  = { map { $_->{secnd_usr} => 1 } @mapping };

	$vars->{sec_sys_id} = $sec_id;

	$TPL_MAPPING;
}

sub upd_mapping {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) }, 1);
	my ($s, $api_vars, $system_id, $mapping_ref) = @_;

	$mapping_ref ||= { };
	
	# TODO: Wrap into commit when
	# 	we upgrade sql.

	$s->{sql_mapping}->delete({}); # del all.

	while (my ($prim_usr, $sec_usr) = each %{$mapping_ref}) {
		$s->{sql_mapping}->insert({ 
			prim_usr => $prim_usr, 
			secnd_usr => $sec_usr,
			'system' => $system_id
		});
	}
	$api_vars->{ok} = "User mapping updated.";
	
}

sub upd_usersys {
	validate_pos(@_, 1, 1, { regex => qr{^\d+$} }, 1);
	my ($s, $vars, $sys_id, $sys_attr) = @_;

	if (defined $sys_attr->{password}
	    && $sys_attr->{password} eq "") {
		# blank input, ignore.
		delete $sys_attr->{password} 
	}
		

	
	my $sys = Data::UserSys->new($s->{dbh}, $sys_id);
	$sys->update(%{$sys_attr});
	
	$vars->{ok} = "System updated.";
	return $s->show_edit($vars, $sys_id);
}

sub list_users {
	validate_pos(@_, 1, { regex => qr(^\d+$) });
	my ($s, $system_id) = @_;
	my $iq = Boitho::Infoquery->new($CONFIG{infoquery});
	return @{$iq->listUsers($system_id)}
}

sub add {
	my ($s, $vars, $sys_ref) = @_;
	my $sys = Data::UserSys->create($s->{dbh}, %{$sys_ref});
	$vars->{ok} = "System created.";
	return $s->show($vars);
}

1;
