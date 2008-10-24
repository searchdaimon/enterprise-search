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

use config qw(%CONFIG);

sub _init {
	my $s = shift;
	$s->{iq} = Boitho::Infoquery->new($CONFIG{infoquery});
	$s->{sql_sys} = Sql::System->new($s->{dbh});
	$s->{sql_mapping} = Sql::SystemMapping->new($s->{dbh});
}

sub show {
	my ($s, $vars) = @_;
	$vars->{systems} = [$s->{sql_sys}->list()];
	
	$TPL_LIST;
}

sub show_edit {
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
	
	my $sys = Data::UserSys->new($s->{dbh}, $sys_id);
	$sys->update(%{$sys_attr});
	
	$vars->{ok} = "System updated.";
	return $s->show_edit($vars, $sys_id);
}

sub list_users {
	validate_pos(@_, 1, { regex => qr(^\d+$) });
	my ($s, $system_id) = @_;

#	my @users = $s->{iq}->listUsers($system_id)
#		or croak "infoquery: ", $s->{iq}->error();
#	return @users;

	# DEBUG
	if ($system_id == 1) {
		open my $fh, "/home/dagurval/websearch/example_list" or die $!;
		my @usr;
		for (0..300) {
			$usr[$_] = <$fh>;
			chomp $usr[$_];
		}
		return @usr;
	}
	else {
		open my $fh, "/home/dagurval/websearch/example_list2" or die $!;
		my @usr;
		for (0..300) {
			$usr[$_] = <$fh>;
			chomp $usr[$_];
		}
		return @usr;
	}
}


1;
