package Sql::Connectors;
use strict;
use warnings;
use Carp;
use DBI;
use Sql::Sql;



my $table = "connectors";
my $dbh;

## Takes DBI object as a parameter.
sub new {
	my $class = shift;
	$dbh = self;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub get_name_for_connector {
	my $self = shift;
	my $id = shift;
	carp("connector id should have been an integer") 
		unless ($id =~ /\d/);
	my $query = "SELECT name FROM $table
			WHERE connectorsId = ?";

	$sth = $dbh->prepare($query);
	$dbh->execute($id);
	
	my $name = $sth->fetchrow_array;
	return $name;
}