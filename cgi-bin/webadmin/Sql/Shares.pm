package Sql::Shares;
use strict;
use warnings;
use Carp;
use DBI;
use Sql::Sql;
use Sql::CollectionAuth;
use Data::Dumper;
use Common::Generic;
my $table = "shares";
my $dbh;
my @valid = qw(host connector active success
		last rate query1 query2
		smb_name smb_workgroup resource domain collection_name auth_id userprefix);

sub new {
	my $class = shift;
	$dbh = shift;
	my $self = {};
	bless $self, $class;
	$self->_init($dbh);
	return $self;
}

sub _init($$) {
	my ($self, $dbh) = @_;
	$self->{'common'} = Common::Generic->new($dbh);

}


## Because of the varieties of shares, this function accepts a hash
## with attributes and tries to insert the valid ones.
sub insert_share ($\%$) {
	my $self = shift;
	my $data = shift;

	my ($collection_name, $auth_id) = $self->_get_collection_and_auth($data);
	$data->{'collection_name'} = $collection_name;
	$data->{'auth_id'} = $auth_id;
	
	my ($attributes, $values, $bind_values) = 
		$self->_construct_insert_query($data);
	
	my $query = "INSERT INTO $table ($attributes) values ($values)";

	my $sth = $dbh->prepare($query)
		or croak("pepare: " . $dbh->errstr);
	my $rv = $sth->execute(@$bind_values)
		or croak("execute:", $dbh->errstr);

	return $dbh->{ q{mysql_insertid} };

}

## Helper function to generate a collection name if it wasn't provided
## and to get an auth_id, if it wasn't provided.
sub _get_collection_and_auth {
	my $self = shift;
	my $data = shift;
	my $collection = $data->{'collection_name'};
	my $auth_id = $data->{'auth_id'};
	my $common = $self->{'common'};
	
	unless ($collection) {
		 $collection = $self->_generate_random_name;
	}

	$auth_id = $common->get_auth_id($dbh,
					$auth_id, 
					$data->{'username'}, 
					$data->{'password'});
	
	return ($collection, $auth_id);
}


sub update_share {
	my $self = shift;
	my $data = shift;

	
	
	my ($collection_name, $auth_id) = $self->_get_collection_and_auth($data);
	$data->{'collection_name'} = $collection_name;
	$data->{'auth_id'} = $auth_id;

	

	# TODO: Use construct_update_query instead.
	my @bind_values = ();
	my $query = "UPDATE $table SET ";
	while (my ($key, $value) = each(%$data)) {
		next unless grep { /^$key$/ } @valid;
		$query .= "$key=?,";
		push @bind_values, $value;
		
		
	}
	chop ($query);
	
	$query .= " WHERE id = ?";
	push @bind_values, $data->{'id'};
	
	carp "Query: ", $query;
	my $sth = $dbh->prepare($query)
		or croak("pepare: " . $dbh->errstr);
	my $rv = $sth->execute(@bind_values)
		or croak("execute:", $dbh->errstr);

	
	
	return 1;
}

sub _construct_insert_query {
	carp "Deprecated: Use Sql::Sql::construct_insert_query instead.";
	my $self = shift;
	my $data = shift;
	return Sql::Sql::construct_insert_query($data, \@valid);
}

sub get_shares_and_connector {
	my $self = shift;
	my $query = "SELECT $table.*, connectors.name
			FROM $table, connectors
			WHERE $table.connector = connectors.id
			ORDER BY $table.connector, $table.host";
	return Sql::Sql::get_hashref_array($dbh, $query);
}



## Returns shares set to be crawled.
## A protocol name can be passed to this function.
## If that is done, it will only return shares belonging to
## the given protocol.
sub get_active_shares {
	my $self = shift;
	my $protocol = shift;
	my $query;
	if ($protocol) { 
		chomp($protocol);
		$query = "SELECT $table.*
			FROM $table, connectors
			WHERE $table.connector = connectors.id
			AND connectors.name = ?
			AND $table.active = 1";
	} else {
		$query = "SELECT $table.*, connectors
			 FROM $table, connectors
			WHERE $table.connector = connectors.id
			AND active = 1";
	}

	$query .= " ORDER BY $table.collection_name";
	
	return Sql::Sql::get_hashref_array($dbh, $query, $protocol);


}

## Returns all shares.
## A protocol name can be passed to this function.
## If that is done, it will only return shares belonging to
## the given protocol.
sub get_shares {
	my $self = shift;
	my $protocol = shift;
	my $query;
	if ($protocol) { 
		chomp($protocol);
		$query = "SELECT $table.*
			FROM $table, connectors
			WHERE $table.connector = connectors.id
			AND connectors.name = ?";
	} else {
		$query = "SELECT $table.*, connectors
			 FROM $table, connectors
			WHERE $table.connector = connectors.id";
	}
	
	$query .= " ORDER BY $table.active DESC, $table.collection_name ASC";

	return Sql::Sql::get_hashref_array($dbh, $query, $protocol);
}

## Returns one share.
sub get_share {
	my $self = shift;
	my $id = shift;
	my $sqlAuth = Sql::CollectionAuth->new($dbh);
	my $query = "SELECT $table.*, connectors.name AS connector_name
		FROM $table, connectors
		WHERE $table.id = ?
		AND connectors.id = $table.connector";

	my $share = Sql::Sql::single_row_fetch($dbh, $query, $id);
	
	# Get username and password
	($share->{'username'}, $share->{'password'}) = $sqlAuth->get_pair_by_share($id);

	return $share;
}

sub get_selected_shares {
	my $self = shift;
	my $selected = shift;

	my @result = ();
	my $query = "SELECT * FROM $table
			WHERE id = ?";
	my $sth = $dbh->prepare($query)
		or croak ("prepare: ", $dbh->errstr);

	foreach my $share (@$selected) {
		next unless ($share);
		
		$sth->execute($share)
			or croak ("execute:", $dbh->errstr);
		
		push (@result, $sth->fetchrow_hashref);
	}

	return \@result;
}


# sub update_active {
# 	my $self = shift;
# 	my $shares = shift;
# 	
# 
# 	# Disable all.
# 	my $query = "UPDATE $table SET active = ?";
# 	my $sth = $dbh->prepare($query) 
# 		or croak ("prepare: ", $dbh->errstr);
# 	$sth->execute('0') 
# 		or croak ("excecute: ", $dbh->errstr);
# 
# 	# Enable those that should be enabled.
# 	$query = "UPDATE $table	
# 			SET active = ?
# 			WHERE id = ?";
# 	$sth = $dbh->prepare($query) 
# 		or croak ("prepare: ", $dbh->errstr);
# 	foreach my $id (@$shares) {
# 		next unless($id);
# 		$sth->execute('1', $id)
# 			or croak ("excecute: ", $dbh->errstr);
# 	}
# 	return 1;
# }

sub set_active ($$) {
	my $self = shift;
	my $id = shift;
	
	my $query = "UPDATE $table 
		SET active = ?
		WHERE id = ?";
	
	return Sql::Sql::simple_execute($dbh, $query, [1, $id]);
}
sub exists {
	my $self = shift;
	my $collection = shift;
	return $self->get_id_by_collection($collection);

}

sub id_exists {
	my $self = shift;
	my $id = shift;
	my $query = "SELECT id FROM $table
			WHERE id = ?";
	return Sql::Sql::single_fetch($dbh, $query, $id);
}

sub get_id_by_collection {
	my $self = shift;
	my $collection = shift;
	my $query = "SELECT id FROM $table
			WHERE collection_name = ?";
	
	return Sql::Sql::single_fetch($dbh, $query, $collection);
}

# Returns the id for a single host/connector match.
# Method is somewhat misleading, as it is only used by scan to see if host already exists.
#
# If fuzzy match is true, get_id will try to mach $table.resource as a substring
sub get_id {
	my $self = shift;
	my $host = shift;
	my $connector = shift;

	my $query = "";
	if ($connector =~ /\d+/) {
		$query = "SELECT id FROM $table 
			WHERE (host = ? OR $table.resource = ?) AND connector = ?";
	}
	else {
		$query = "SELECT $table.id FROM $table, connectors
			WHERE ($table.host = ? OR $table.resource = ?)
			AND connectors.name = ?";
	}

	my $sth = $dbh->prepare($query) 
		or croak ("prepare", $dbh->errstr);
	$sth->execute($host, $host, $connector)
		or croak ("execute: ", $dbh->errstr);
	
	my $id = $sth->fetchrow_array;
	
	return $id;
}

sub delete_all {
	my $self = shift;
	my $query = "DELETE FROM $table";
	
	my $sth = $dbh->prepare($query)	or croak ("prepare: ", $dbh->errstr);
	$sth->execute			or croak ("execute: ", $dbh->errstr);
}

sub _generate_random_name($) {
	my $self = shift;
	my $name = "";
	my $counter = 0;
	do {
		$name = "random_";
		my @characters = ('a'..'z', 'A'..'Z', 0..9);
	
		for (1..5) {
			$name .= $characters[rand @characters];
		}

		$counter++;
		carp ("_generate_random_name is using too many attempts at finding a name.") if ($counter > 4);
	} while ($self->exists($name));
	return $name;
}

sub get_collection_name {
	my $self = shift;
	my $id = shift;
	my $query = "SELECT collection_name FROM $table
			WHERE id = ?";
	
	my $sth = $dbh->prepare($query)
		or croak "pepare: ", $dbh->errstr;
	my $rv = $sth->execute($id)
		or croak "execute: ", $dbh->errstr;
		
	return $sth->fetchrow_array;
}

sub delete_share {
	my $self = shift;
	my $id = shift;
	my $query = "DELETE FROM $table
			WHERE id = ?";

	my $sth = $dbh->prepare($query)
		or croak "pepare: ", $dbh->errstr;
	my $rv = $sth->execute($id)
		or croak "execute: ", $dbh->errstr;

	return 1;
}

# Get connector name based on share id
sub get_connector_name($$) { 
	my $self = shift;
	my $id = shift;
	my $query = "SELECT connectors.name AS connector_name
			FROM $table, connectors
			WHERE $table.id = ?
			AND $table.connector = connectors.id";
	my @result = Sql::Sql::single_fetch($dbh, $query, $id);
	return $result[0];
}
1;