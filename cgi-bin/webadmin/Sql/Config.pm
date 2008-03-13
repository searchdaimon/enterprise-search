package Sql::Config;
use strict;
use warnings;
use Carp;
use DBI;
use Sql::Sql;
use Data::Dumper;
use config qw($CONFIG);

my $table = "config";
my $dbh;


sub new {
	my $class = shift;
	$dbh = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub update_msad {
	my $self = shift;
	return $self->_update_dap('msad', \@_);
}

sub update_ldap {
	my $self = shift;
	return $self->_update_dap('ldap', \@_);
}

sub get_authenticatmethod($) {
	my $self = shift;
	return $self->get_setting('authenticatmetod');
}

sub update_authenticatmethod {
	my $self = shift;
	my $method = shift;

	croak ("$method is an unknown authentication method.")
		unless($self->_is_valid_authmethod($method));

	my $query = "UPDATE $table SET configvalue = ?
		WHERE configkey = 'authenticatmetod'";

	my $sth = $dbh->prepare($query)
		or croak("Pepare: " . $dbh->errstr);
	my $rv = $sth->execute($method)
		or croak("Update: " . $dbh->errstr);

	return 1;
}

## Helper function to update dap configurations.
## Used by update_msad and update_ldap functions.
sub _update_dap {
	my $self = shift;
	my $dap = shift;
	my $vars = shift;
	my ($domain, $user, $password, $ip, $port) = (@$vars);
	my %values = (
		'domain' => $domain,
		'user' => $user,
		'password' => $password,
		'ip' => $ip,
		'port' => $port);
	
	foreach my $var (("domain", "user", "password", "ip", "port")) {
		my $query = "UPDATE $table SET configvalue = ?
				WHERE configkey = '$dap"."_"."$var'";

		my $sth = $dbh->prepare($query) 
			or croak("Pepare: " . $dbh->errstr);

		my $rv = $sth->execute($values{$var}) 
			or croak("Update: " . $dbh->errstr);	
	}	
	return 1;
}


## Method returns true if there authentication method
## allready has been configured.
sub config_exists {
	my $self = shift;
	my $query = "SELECT configvalue
			FROM config
		WHERE configkey = 'authenticatmetod' 
		AND configvalue != ''";

	my $sth = $dbh->prepare($query)
		or croak("prepare:" . $dbh->errstr);
	$sth->execute()
		or croak("execute:" . $dbh->errstr);
	
	return ($sth->rows) ? 1 : 0;
}


sub _is_valid_authmethod($$) {
	my $self = shift;
	my $method = shift;
	carp "Deprecated method _is_valid_authmethod used. Use is_valid_authmethod.";
	return $self->is_valid_authmethod($method);
}

sub is_valid_authmethod($$) {
	my ($self, $method) = @_;
	
	
	croak "Need to define 'valid_auth_methods' in config file"
		unless defined $CONFIG->{'valid_auth_methods'};
		
	my $valid_ptr = $CONFIG->{'valid_auth_methods'};
	
	map { return 1 if ($_ eq $method); } @$valid_ptr;
	return 0;
}

#
# Return everything in the table.
sub get_all {
	my $query = "SELECT * FROM $table";
	return map { $_->{configkey} => $_->{configvalue} } 
            @{Sql::Sql::get_hashref_array($dbh, $query)};
}

##
# Updates given setting.
# TODO: Make private. Use insert_setting instead outside this class.
#
# Attributes:
#	key - Name of setting.
#	value - Value to set it to.
sub update_setting {
	my ($self, $key, $value) = (@_);
	my $query = "UPDATE $table 
			SET configvalue = ?
			WHERE configkey = ?";
	my $sth = $dbh->prepare($query)
		or croak("prepare: " . $dbh->errstr);
	$sth->execute($value, $key)
		or croak("execute: " . $dbh->errstr);

	return 1;
}

##
# Inserts setting, or updates it if it already exists.
#
# Attributes:
#	key - Name of setting.
#	value - Value to set it to.
sub insert_setting {
	my ($self, $key, $value) = @_;
	
	if ($self->_exists($key)) {
		return $self->update_setting($key, $value);
	}
	else {
		my $query = "INSERT INTO $table (configkey, configvalue) VALUES (?, ?)";
		return Sql::Sql::simple_execute($dbh, $query, [$key, $value]);
	}
}

## 
# Check if setting exists.
#
# Attributes:
#	key - Name of setting
#
# Returns:
#	result - key (true), nothing (false).
sub _exists {
	my ($self, $key) = @_;
	my $query = "SELECT configkey FROM $table
				WHERE configkey = ?";
	
	return Sql::Sql::single_fetch($dbh, $query, $key);
}

sub get_setting($$) {
	my ($self, $key) = (@_);
	my $query = "SELECT configvalue 
			FROM $table
			WHERE configkey = ?";
	return Sql::Sql::single_fetch($dbh, $query, $key);
}


sub get_dap_settings($$) {
	my ($self, $method) = @_;
	
	my @valid_methods = ('msad', 'ldap');
	unless (grep /^$method$/, @valid_methods) {
		carp "Unknown/invalid method used in get_dap_settings.";
		return;
	}
	
	my $query = "
		SELECT configkey, configvalue FROM $table
			WHERE configkey LIKE '${method}_%'";
	my $raw_settings_ptr 
		= Sql::Sql::get_hashref_array($dbh, $query);
	
	
	#transform into "key => value", without prefix in key.
	my %settings = map {	
		my $key_without_prefix = substr $_->{'configkey'}, (length $method) + 1;	
		
		($key_without_prefix, $_->{'configvalue'})
	} @$raw_settings_ptr;
		
	return \%settings;
	
}


sub delete_all {
	my ($self, $keep_configkeys) = @_;
	my $query;
	
	if ($keep_configkeys) {
		$query = "UPDATE $table SET configvalue = ''";
	}
	else {
		$query = "DELETE FROM $table";
	}

	return Sql::Sql::simple_execute($dbh, $query);
}

1;
