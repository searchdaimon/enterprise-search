package Sql::CollectionAuth;
use strict;
use warnings;
use Carp;
use Sql::Sql;
use Data::Dumper;
use Sql::Abstract;
use Common::Validate;
our @ISA = qw(Sql::Abstract);

my $table = "collectionAuth";

##
# Attempts to add user/pass pair.
# If user/pass pair exists, it just returns their id.
# If pair does not exists, it inserts the new pair and returns id.
#
# Attributes:
#	username - Username
#	password - Password
#
# Returns:
#	id - user/pass pair ID.
sub add {
        my $s = shift;
	my ($username, $password, $comment) = @_;

	return unless $username;

        # update if exists
	my $id = $s->get_id($username, $password);
        if (defined $id) {
            $s->update_authentication($id, @_);
	    return $id;
        }

	# insert if new
	$s->insert_authentication($username, $password, $comment);
	return $s->{dbh}->{ q{mysql_insertid} };

}

##
# Returns id for given username/password pair.
sub get_id {
	my $self = shift;
	my ($username, $password) = (@_);
	my $query = "SELECT id FROM $table
			WHERE username = ?
			AND password = ?";
	return $self->sql_single($query, $username, $password);
}

##
# Inserts username/password pair.
sub insert_authentication {
    my ($s, $usr, $pass, $comment) = @_;

    my $query =  "INSERT INTO $table (username, password, comment)
			values(?, ?, ?)";
    $s->sql_insert($query, $usr, $pass, $comment);
}

##
# Updates username/password for given pair id.
sub update_authentication {
	my $self = shift;
	my ($id, $username, $password, $comment) = @_;

	valid_numeric({'id' => $id});

	my $query = "UPDATE $table 
			SET username = ?, password = ?, comment = ?
			WHERE id = ?";
	$self->sql_update($query, $username, $password, $comment, $id);
}

##
# Delete username/password pair.
#
# Attributes:
#	id - pair id
sub delete_authentication {
	my $self = shift;
	my $id = shift;
	my $query = "DELETE FROM $table WHERE id = ?";
	$self->sql_delete($query, $id);
}

sub get_authentication {
	croak "get_authentication is deprecated. Use get_auth_by_id() or get_all_auth() instead.";
};

##
# Get auth data for given id.
#
# Attributes:
#	id - pair id
#
# Returns:
#	auth_data - Hashref with pair data.
sub get_auth_by_id {
	my ($self, $id) = @_;

	valid_numeric({'id' => $id});
	
	my $query = "SELECT * FROM $table WHERE id = ?";

	return $self->sql_hashref($query, $id);

}

##
# Get all auth pair data.
#
# Returns:
#	
sub get_all_auth {
	my $self = shift;

	my $query = "SELECT * FROM $table ORDER BY username";

	return $self->sql_hashref($query);
}

sub get_pair {
	croak "get_pair is deprecated. Use get_pair_by_share() instead.";
}

## 
# Get username/password for a given share id.
# 
# Attributes:
#	share_id - Share id
#
# Returns:
#	pair - (username, password) list
sub get_pair_by_share {
	my $self = shift;
	my $share_id = shift;

	my $query = "SELECT $table.username, $table.password 
			FROM shares, $table
			WHERE $table.id = shares.auth_id
			AND shares.id = ?";

	return $self->sql_single($query, $share_id);
	
}

##
# Get username/password for given auth id.
#
# Attributes:
#	id - auth id
sub get_pair_by_id {
	my $self = shift;
	my $id = shift;
	my $query = "SELECT username, password FROM $table WHERE id = ?";
	return $self->sql_single($query, $id);
}

##
# Delete all contents in table.
sub delete_all {
	my $self = shift;
	my $query = "DELETE FROM $table";
	
	return $self->sql_delete($query);
}

1;
