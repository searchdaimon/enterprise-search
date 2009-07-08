package Page::Users;
use strict;
use warnings;

BEGIN { unshift @INC, $ENV{'BOITHOHOME'} . '/Modules' }

use Carp;
use Data::Dumper;
use Readonly;
use SD::SLicense qw(license_info $DB_LICENSE_FIELD);

use Page::Abstract;
use Boitho::Infoquery;
use Sql::ActiveUsers;
use Sql::Config;
use Sql::System;
use config qw(%CONFIG);

our @ISA = qw(Page::Abstract);

Readonly::Scalar my $TPL_USER_DETAILS => 'users_details.html';
Readonly::Scalar my $TPL_USER_LIST    => 'users_main.html';

sub _init {
	my $s = shift;
	$s->{iq} = Boitho::Infoquery->new($CONFIG{infoquery});
	$s->{license_key} = Sql::Config->new($s->{dbh})->get_setting($DB_LICENSE_FIELD);
}

sub show_usr_details {
	my ($s, $vars, $usr) = @_;
	$vars->{username} = $usr;
	$vars->{groups}   = $s->{iq}->groupsAndCollectionForUser($usr);
	return $TPL_USER_DETAILS;
}

sub show_usr_list {
	#my ($s, $vars) = @_;
	my ($s, $vars, $selected_users) = @_;

	my $sqlActive = Sql::ActiveUsers->new($s->get_dbh);
	my %active = map { $_->{user} => 1 } $sqlActive->get({}, 'user');

	my %license = license_info($s->{license_key}, $CONFIG{slicense_info_path});
	if (!$license{valid}) {
		$vars->{error} = "This installation has no license, or "
			. " an invalid license. Search will not work for any users.";
	}
	$vars->{licensed_users} = $license{users};

	if ($selected_users < 0) {
	    $vars->{activated_users} = scalar keys %active;
	}
	else {
	    $vars->{activated_users} = $selected_users;
	}

	my $prim_sys = Sql::System->new($s->{dbh})->primary_id();

	my $iq_users_ref = $s->{iq}->listUsers($prim_sys);
	my @users;
	if (defined $iq_users_ref) {
		for my $usr (@{$iq_users_ref}) {
			push @users, {
				username => $usr,
				active   => $active{$usr},
			};
			delete $active{$usr};
		}
	}
	else {
		$vars->{error} = "Unable to fetch user list. " . $s->{iq}->error;
	}

	$vars->{users} = \@users;
	$vars->{unknown_active} = [ keys %active ];

	return $TPL_USER_LIST;
}

sub upd_usr_access {
	my ($s, $vars, $users_raw) = @_;
	my @users = grep { defined } @{$users_raw};
	my $selected_users = $#users +1;

	# Ikke oppdater hvis antall valgte brukere er større enn lisensierte brukere.
	my %license = license_info($s->{license_key}, $CONFIG{slicense_info_path});

	if ($license{valid} && $selected_users <= $license{users}) {
		my $sqlActive = Sql::ActiveUsers->new($s->get_dbh);

		#TODO : Error prone. Do in a transaction.
		$sqlActive->delete({});
	        $sqlActive->insert({ user => $_ }) for @users;
	}

	return $s->show_usr_list($vars, $selected_users);
}

1;
