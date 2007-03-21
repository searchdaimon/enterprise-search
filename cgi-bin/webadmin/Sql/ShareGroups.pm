package Sql::ShareGroups;
use strict;
use warnings;
use Carp;
use Sql::Sql;
use Data::Dumper;

my $table = "shareGroups";
my $dbh;

sub new {
	my $class = shift;
	$dbh = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub set_groups($$$) {
	my $self = shift;
	my $share = shift;
	my $groups = shift;
	$self->_delete_groups($share);
	foreach my $group (@$groups) {
		next unless $group;
		$self->_insert($share, $group);
	}

	return 1;
}

sub get_groups($$) {
	my $self = shift;
	my $share = shift;
	
	my $query = "SELECT name 
			FROM shareGroups
			WHERE share = ?";
	return Sql::Sql::multi_row_fetch($dbh, $query, $share);
}

sub delete_all {
	my $self = shift;
	my $query = "DELETE FROM $table";
	
	return Sql::Sql::simple_execute($dbh, $query);
}

sub _delete_groups {

	my $self = shift;
	my $share = shift;

	my $query = "DELETE FROM $table 
			WHERE share = ?";

	return Sql::Sql::simple_execute($dbh, $query, $share);
}

sub _insert {
	my $self = shift;
	my $share = shift;
	my $group = shift;
	my $query = "INSERT INTO $table (name, share)
			VALUES (?, ?)";

	return Sql::Sql::simple_execute($dbh, $query, [$group, $share]);
}



1;
