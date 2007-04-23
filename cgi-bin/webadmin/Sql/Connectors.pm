package Sql::Connectors;
use strict;
use warnings;
use Carp;
use Sql::Sql;
use Sql::Shares;
use Data::Dumper;

my $table = "connectors";
my $dbh;

sub new {
	my $class = shift;
	$dbh = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub get_all_ids {
	my $query = "SELECT id FROM $table";
	return Sql::Sql::multi_row_fetch($dbh, $query);
}

sub get_all_names {
	my $self = shift;
	
	my $query = "SELECT name FROM $table";
	my $sth = $dbh->prepare($query)
		or croak "pepare: ", $dbh->errstr;
	my $rv = $sth->execute()
		or croak "execute: ", $dbh->errstr;

	my @names = ();

	while (my ($name) = $sth->fetchrow_array) {
		push (@names, $name);
	}

	return \@names;
}

sub get_name {
	my $self = shift;
	my $id = shift;
	my $query = "SELECT name FROM $table
			WHERE id = ?";
	
	my $sth = $dbh->prepare($query)
		or croak "pepare: ", $dbh->errstr;
	my $rv = $sth->execute($id)
		or croak "execute: ", $dbh->errstr;
		
	return $sth->fetchrow_array;
}

## Get ID for a given name.
## Names are unique, so there will
## only be one result.
sub get_id($$) {
	my $self = shift;
	my $id   = shift;
	my $query = "SELECT id FROM $table
					WHERE name = ?";
	return Sql::Sql::single_fetch($dbh, $query, $id);
}

sub get_connectors {
	my $self = shift;
	my $query = "SELECT * FROM $table";
	return Sql::Sql::get_hashref_array($dbh, $query);
}


sub get_with_shares {
	my $self = shift;
	my $sqlShares = Sql::Shares->new($dbh);
	my $only_active = shift;
	my @connector_list;

 	foreach my $connector (@{$self->get_connectors}) {
		my @shares;
		if ($only_active) {
 			@shares = $sqlShares->get_active_shares($connector->{'name'});
		}
		else {
			@shares = $sqlShares->get_all_by_connector($connector->{'name'});
		}
		push (@connector_list, {
			'name' => $connector->{'name'},
			'comment' => $connector->{'comment'},
			'shares' => \@shares });
 	}
	return @connector_list;

}


sub get_connectors_with_scantool {
	my $self = shift;
	my $query = "SELECT * FROM $table
		WHERE scantoolAvailable = ?";
	return Sql::Sql::get_hashref_array($dbh, $query, 1);

}

sub get_input_fields($$) {
	my $self = shift;
	my $connector = shift;
	my $query;
	unless ($connector) {
		carp "Didn't get a connector as a parameter.";
		return;
	}
	if ($connector =~ /\d+/) { # id
		$query = "SELECT inputFields FROM $table
				WHERE id = ?";
	} else {
		$query = "SELECT inputFields FROM $table
				WHERE name = ?";
	}
	
	my $fields_string  = Sql::Sql::single_fetch($dbh, $query, $connector);
	my @fields = ();
	return unless $fields_string;
	foreach(split(',', $fields_string)) {
		# Remove leading space	
		s/^\s+//;
		s/\s+$//;
		push @fields, $_;
	}

	return \@fields;
}
1;