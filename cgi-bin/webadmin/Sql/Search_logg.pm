package Sql::Search_logg;
use strict;
use warnings;
use Carp;
use DBI;
use Sql::Sql;

my $table = "search_logg";

sub new {
	my $class = shift;
	my $dbh = shift;
	my $self = {};
	bless $self, $class;
	$self->_init($dbh);
	return $self;
}

sub _init {
	my $self = shift;
	my $dbh = shift;
	$self->{'dbh'} = $dbh;
}

sub get_log {
	my $self = shift;
	my $dbh = $self->{'dbh'};
	my $query = "SELECT * FROM $table
			ORDER BY tid DESC
			LIMIT 0,50";
	return Sql::Sql::get_hashref_array($dbh, $query);
}
