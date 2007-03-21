package Sql::ScanResults;
use strict;
use warnings;
use Carp;
use Sql::Sql;
use Data::Dumper;

my $table = "scanResults";
my @valid = qw(connector time range result_xml done);
my $dbh;

sub new {
	my $class = shift;
	$dbh = shift;
	my $self = {};
	bless $self, $class;
	$self->_init();
	return $self;
}

sub _init($) {
	my $self = shift;
	$self->{'sqlConnectors'} = Sql::Connectors->new($dbh);
}

sub insert_new_results {
	my ($self, $connector, $range) = @_;
	
	$connector
		= $self->_get_connector_id($connector);
	
	unless ($connector) {
		carp "Cannot add result, unknown connector.";
		return;
	}

	my $query = "INSERT INTO $table (connector, range, time)	
			VALUES (?, ?, NOW())";

	Sql::Sql::simple_execute($dbh, $query, [$connector, $range]);
 	return $dbh->{ q{mysql_insertid} };
}

sub update_results {
	my $self = shift;
	my $id = shift;
	my $data = shift;
	
	$data->{'connector'}
		= $self->_get_connector_id($data->{'connector'});
	unless ($data->{'connector'}) {
		carp "Cannot update result, unknown connector.";
		return;
	}


	my ($set_part, $bind_values) = 
		Sql::Sql::construct_update_query($data, \@valid);

	push @$bind_values, $id;

	my $query = "UPDATE $table SET $set_part
			WHERE id = ?";
			
			
	return Sql::Sql::simple_execute($dbh, $query, $bind_values);

}


## Returns array with elements needed to list the results.
sub get_list($) {
	my $self = shift;
	my $query = "SELECT id, time, connector, range, done FROM $table
			ORDER BY 'id'";
	Sql::Sql::get_hashref_array($dbh, $query);
}

## Returns array with elements needed to list the results.
## returns connector name instead of id
sub get_list_and_connector_name {
	my $self = shift;
	my $query = "SELECT $table.id, $table.time, 
			connectors.name AS connector, 
			$table.range, $table.done
			FROM $table, connectors
			WHERE $table.connector = connectors.id";
	return Sql::Sql::get_hashref_array($dbh, $query);
}

## Returns the xml string for a given result.
sub get_xml($$) {
	my $self = shift;
	my $id = shift;
	my $query = "SELECT result_xml FROM $table 
			WHERE id = ?";
	return Sql::Sql::single_fetch($dbh, $query, $id);
}

## Delete a given result
sub delete_result($$) {
	my $self = shift;
	my $id = shift;
	my $query = "DELETE FROM $table 
			WHERE id = ?";

	return Sql::Sql::simple_execute($dbh, $query, $id);
}

sub get_connector($$) {
	my $self = shift;
	my $id = shift;
	my $query = "SELECT connector FROM $table
			WHERE id = ?";
	return Sql::Sql::single_fetch($dbh, $query, $id);
}

sub exists($$) {
	my ($self, $id) = (@_);
	my $query = "SELECT id FROM $table 
			WHERE id = ?";
	return Sql::Sql::single_fetch($dbh, $query, $id);
}

## For some functions, the connector needs to be an ID.
## Helper function to get ID by Name.
sub _get_connector_id($$) {
	my ($self, $connector) = @_;
	my $sqlConnectors = $self->{'sqlConnectors'};
	unless ($connector =~ /\d+/) {
		$connector
			= $sqlConnectors->get_id($connector);
	}
	return $connector;
}

1;