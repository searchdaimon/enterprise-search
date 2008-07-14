package Sql::Shares;
use strict;
use warnings;
use Carp;
use DBI;
use Sql::CollectionAuth;
use Data::Dumper;
use Sql::Webadmin;
use Params::Validate qw(validate_pos);

our @ISA = qw(Sql::Webadmin);

my $table = "shares";
my $dbh;
my $sqlAuth;

#
# Valid fileds in db table shares.
my @valid = qw(host connector active success
		last rate query1 query2
		smb_name smb_workgroup resource domain collection_name auth_id userprefix);

sub _init {
	my $self = shift;
	$dbh = $self->{'dbh'};
	$sqlAuth = Sql::CollectionAuth->new($dbh);
}


##
# Insert a new collection into shares table.
#
# Attributes:
#	attr_val_ref -  A hash ref with db table attributes as: attribute => value.
#					Any attributes not listed in @valid are ignored.
#
# Returns:
#	insert_id - Collection id for this insert.
sub insert_share {
    my $self = shift;
    my $attr_val_ref = shift;

    $attr_val_ref->{'collection_name'} 
        = $self->_gen_coll_name($attr_val_ref);

    my ($attr_str, $val_str, @binds) 
        = $self->construct_insert_query($attr_val_ref, @valid);

    my $query = "INSERT INTO $table ($attr_str) values ($val_str)";

    $self->sql_insert($query, @binds);

    return $dbh->{ q{mysql_insertid} };
}


##
# Update a collection in shares table.
#
# Attributes:
#	attr_val_ref -  A hash ref with db table attributes as: attribute => value.
#					Any attributes not listed in @valid are ignored.
sub update_share {
    my ($self, $attr_val_ref) = @_;

    my ($set_str, @binds) 
        = $self->construct_update_query($attr_val_ref, @valid);

    my $query = "UPDATE $table SET $set_str WHERE id = ?";
    push @binds, $attr_val_ref->{'id'};

    $self->sql_update($query, @binds);

    1;
}

##
# Returns list of all shares and its connector.
sub get_shares_and_connector {
	my $self = shift;
	my $query = "SELECT shares.*, connectors.name
			FROM shares, connectors
			WHERE shares.connector = connectors.id
			ORDER BY shares.connector, shares.host";
	return $self->sql_hashref($query);
}


sub get_active_shares {
	croak "Deprecated. Use get_active_by_connector, or define a new sub.";
}

## 
# Returns shares to be crawled for a given connector.
#
# Attributes:
#	connector - Connector name.
sub get_active_by_connector {
	my ($self, $connector) = @_;

	my $query = "SELECT shares.*
			FROM shares, connectors
			WHERE shares.connector = connectors.id
			AND connectors.name = ?
			AND shares.active = 1
			ORDER BY shares.collection_name";

	return $self->sql_hashref($query, $connector);	
}

sub get_shares {
	croak "Deprecated. Use get_all_by_connector, or define a new sub.";
}

##
# Returns all shares for a given connector.
#
# Attributes:
#	connector - Connector name.
#sub get_all_by_connector {
#	my ($self, $connector) = @_;
#	
#	my $query;
#	
#	$query = "SELECT shares.*
#			FROM shares, connectors
#			WHERE shares.connector = connectors.id
#			AND connectors.name = ?";
#	
#	$query .= " ORDER BY shares.active DESC, shares.collection_name ASC";
#
#	return $self->sql_hashref($query, $connector);
#}

##
# Returns a given share.
# Also fetches its connector name, and auth pair.
#
# Attributes:
#	id - share id.
sub get_share {
	my ($self, $id) = @_;
	
	my $query = "SELECT shares.*, connectors.name AS connector_name
		FROM shares, connectors
		WHERE shares.id = ?
		AND connectors.id = shares.connector";

	my ($share) = $self->sql_hashref($query, $id);
	
	# Get username and password
	($share->{'username'}, $share->{'password'}) 
		= $sqlAuth->get_pair_by_share($id);

	return $share;
}

##
# Returns collections for given list of collection id's.
#
# Attributes:
#	@selected - List of collection ID's
sub get_selected_shares {
	my $self = shift;
	my @selected = @_;

	my @result;
	my $query = "SELECT * FROM shares
			WHERE id = ?";
	
	foreach my $collection_id (@selected) {
		next unless ($collection_id);
		
		push @result, $self->sql_hashref($query, $collection_id);
	}

	return @result;
}

##
# Set a collection to be active.
#
# Attributes:
#	id - collection id
sub set_active {
	my $self = shift;
	my $id = shift;
	
	my $query = "UPDATE shares
		SET active = ?
		WHERE id = ?";
	
	return $self->sql_update($query, 1, $id);
}

##
# Check if collection name is taken
#
# Attributes:
#	collection_name - Collectio name
sub collection_name_exists {
	my ($self, $collection_name) = @_;
	return $self->get_id_by_collection($collection_name);
}

##
# Check if given collection id exists.
#
# Attributes:
#	id - Collection id
sub id_exists {
	my ($self, $id) = @_;
	my $query = "SELECT id FROM shares
			WHERE id = ?";
	return $self->sql_single($query, $id);
}

##
# Get collection id for a given name.
sub get_id_by_collection {
	my ($self, $collection_name) = @_;
	my $query = "SELECT id FROM shares
			WHERE collection_name = ?";
	
	return $self->sql_single($query, $collection_name);
}

sub get_id {
	croak "get_id() is deprecated.";
}

##
# Delete all content in table.
sub delete_all {
	my $self = shift;
	my $query = "DELETE FROM shares";
	
	$self->sql_delete($query);
}



##
# Get collection name for a given collection id.
#
# Parameters:
#	id - Collection id.
sub get_collection_name {
	my ($self, $id) = @_;
	my $query = "SELECT collection_name FROM shares
			WHERE id = ?";
	
	return $self->sql_single($query, $id);
}

##
# Delete a given collection.
#
# Parameters:
#	id - Collection id.
sub delete_share {
	my $self = shift;
	my $id = shift;
	my $query = "DELETE FROM $table
			WHERE id = ?";

	$self->sql_delete($query, $id);
	1;
}

##
# Get connector name for a given collection id.
#
# Parameters:
#	id - Collection id.
sub get_connector_name { 
	my $self = shift;
	my $id = shift;
	my $query = "SELECT connectors.name AS connector_name
			FROM shares, connectors
			WHERE shares.id = ?
			AND shares.connector = connectors.id";
	
	return $self->sql_single($query, $id);
}


# Group : Private methods

sub _gen_coll_name {
    my ($self, $data) = @_;
    return $data->{collection_name}
        if $data->{collection_name};

    my @chars = ('a'..'z', 'A'..'Z', 0..9);
    my $name;
    do {
        $name = "collection_";
        $name .= $chars[rand @chars] for 1..5;
    } while ($self->collection_name_exists($name));
    return $name;
}

sub get_connector {
    validate_pos(@_, 1, { regex => qr(^\d+$) });
    my ($s, $id) = @_;
    return $s->get({ id => $id }, 'connector')->{connector};
}

sub exists { shift->SUPER::exists($table, 'id', @_) }
sub get { shift->SUPER::get($table, @_) }
sub insert { shift->SUPER::insert($table, @_) }
sub update { shift->SUPER::update($table, @_) }
sub delete { shift->SUPER::delete($table, @_) }

1;
