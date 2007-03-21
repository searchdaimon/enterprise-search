package Sql::CollectionAuth;
use strict;
use warnings;
use Carp;
use Sql::Sql;
use Data::Dumper;

my $table = "collectionAuth";
my $dbh;

sub new {
	my $class = shift;
	$dbh = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}


# Method to attempt adding username/password pair.
# In both cases of pair existing, or inserting new,
# this method will return the pairs id.
sub add {
	my $self = shift;
	my ($username, $password) = (@_);

	return unless $username;

	my $id = $self->get_id($username, $password);
	return $id if $id; #exists, return it.

	# insert it.
	$self->insert_authentication($username, $password);
	return $dbh->{ q{mysql_insertid} };

}

sub get_id {
	my $self = shift;
	my ($username, $password) = (@_);
	my $query = "SELECT id FROM $table
			WHERE username = ?
			AND password = ?";
	return Sql::Sql::single_fetch($dbh, $query, [$username, $password]);
}

sub insert_authentication {

	my $self = shift;
	my ($username, $password) = (@_);
	my $query =  "INSERT INTO $table (username, password)
			values(?, ?)";
	return Sql::Sql::simple_execute($dbh, $query, [$username, $password]);
}

sub update_authentication {
	my $self = shift;
	my ($id, $username, $password) = (@_);
	my $query = "UPDATE $table 
			SET username = ?, password = ?
			WHERE id = ?";
	return Sql::Sql::simple_execute($dbh, $query, [$username, $password, $id]);
}

sub delete_authentication {
	my $self = shift;
	my $id = shift;
	my $query = "DELETE FROM $table WHERE id = ?";
	return Sql::Sql::simple_execute($dbh, $query, $id);
}


sub get_authentication {
	my $self = shift;
	my $id = shift if (@_);
	my ($bind_value, $query);
	if ($id) {
		$query = "SELECT * FROM $table
				WHERE id = ?";
		$bind_value = $id;
	}
	else {  $query = "SELECT * FROM $table
				ORDER BY username"; }
	return Sql::Sql::get_hashref_array($dbh, $query, $bind_value);
}


## Get pair by share id
sub get_pair {
	my $self = shift;
	my $id = shift;

	my $query = "SELECT $table.username, $table.password 
			FROM shares, $table
			WHERE $table.id = shares.auth_id
			AND shares.id = ?";

	return Sql::Sql::single_fetch($dbh, $query, $id);
}

sub get_pair_by_id {
	my $self = shift;
	my $id = shift;
	my $query = "SELECT username, password FROM $table WHERE id = ?";
	return Sql::Sql::single_fetch($dbh, $query, $id);
}

sub delete_all {
	my $self = shift;
	my $query = "DELETE FROM $table";
	
	return Sql::Sql::simple_execute($dbh, $query);
}
1;