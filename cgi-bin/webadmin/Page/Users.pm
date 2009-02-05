package Page::Users;
use strict;
use warnings;

BEGIN { unshift @INC, $ENV{'BOITHOHOME'} . '/Modules' }

use Carp;
use Data::Dumper;
use Readonly;

use Page::Abstract;
use Boitho::Infoquery;
use Sql::ActiveUsers;
use Sql::Config;
use Sql::System;
use config qw(%CONFIG);

our @ISA = qw(Page::Abstract);

Readonly::Scalar my $TPL_USER_DETAILS => 'users_details.html';
Readonly::Scalar my $TPL_USER_LIST    => 'users_main.html';
Readonly::Scalar my $DB_LICENSE_FIELD => "licensekey";

sub _init {
	my $s = shift;
	$s->{iq} = Boitho::Infoquery->new($CONFIG{infoquery});
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

	my %license = $s->get_license_info();
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
	my %license = $s->get_license_info();

	if ($license{valid} && $selected_users <= $license{users}) {
		my $sqlActive = Sql::ActiveUsers->new($s->get_dbh);

		#TODO : Error prone. Do in a transaction.
		$sqlActive->delete({});
	        $sqlActive->insert({ user => $_ }) for @users;
	}

	return $s->show_usr_list($vars, $selected_users);
}

sub get_license_info {
	my $s = shift;
	
	#fetch license from db.
	my $sqlcfg = Sql::Config->new($s->{dbh});
	my $license = $sqlcfg->get_setting($DB_LICENSE_FIELD);
	if (!$license) {
		warn "License not set.";
		return ( valid => 0 );
	}

	# fetch info
	my %lic_info;
	open my $h, "$CONFIG{slicense_info_path} \Q$license\E |"
		or croak "Unable to open $CONFIG{slicense_info_path}";
	while (my $line = <$h>) {
		if ($line =~ /^([a-z]+): ([a-z0-9]+)$/) {
			my ($k, $v) = ($1, $2);
			$v = 1 if $v eq 'yes';
			$v = 0 if $v eq 'no';
			$lic_info{$k} = $v;
		}
		else {
			warn "Unable to parse '$line' from slicense_info";
		}
	}
	close $h or warn "slicense_info exited with error";

	return %lic_info;
}

1;
