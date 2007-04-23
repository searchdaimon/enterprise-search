# Class: Sql::Abstract
# Abstract class with method common for all Sql classes. Should not be
# used directly, but raher inhereted.
#
package Sql::Abstract;
use strict;
use warnings;
use Data::Dumper;
use Carp;

# Constructor: new
#
# Attributes:
#	dbh - Database handler (connection)
sub new($$) {
	my $class = shift;
	my $dbh   = shift;

	my $self = {};
	bless $self, $class;
	
	#init
	$self->{'dbh'} = $dbh;
	if ($self->can("_init")) {
		$self->_init(@_);
	}
	
	return $self;
}

# Group: Sql functions

##
# Returns a single SELECT-result.
#
# Attributes:
#	query - Sql select query
#	@binds - Bind values.
#
# Returns:
#	result - Single scalar or list-result.
sub sql_single {
	my $self = shift;
	my $query = shift;
	my @binds = @_;
	
	my $sth = $self->_prepare_and_execute($query, @binds);
	return $sth->fetchrow_array();
}


##
# Returns array with SELECT-results as hashref. One hashref per result.
#
# Attributes:
#	query - Sql select query
#	@binds - Bind values.
#
# Returns:
#	result - list with hashrefs.
sub sql_hashref {
	my $self  = shift;
	my $query = shift;
	my @binds = @_;
	
	my $sth = $self->_prepare_and_execute($query, @binds);
	
	my @results = ();
	while (my $r = $sth->fetchrow_hashref) {
		push @results, $r;
	}
	return @results;
}

##
# For INSERT-queries.
#
# Attributes:
#	query - Sql insert query
#	@binds - Bind values.
sub sql_insert {
	my $self = shift;
	my $query = shift;
	my @binds = @_;
	
	$self->_prepare_and_execute($query, @binds);
	1;
}

sub sql_update {
	my $self = shift;
	my $query = shift;
	my @binds = @_;
	$self->_prepare_and_execute($query, @binds);
	1;
}

sub sql_delete {
	my $self = shift;
	my $query = shift;
	my @binds = @_;
	$self->_prepare_and_execute($query, @binds);
	1;
}



##
# Constructs part of an insert query.
#
# Parameters:
#	attr_data_ref - Hashref with db tables: attribute => value
#	@valid        - List of valid db table attributes.
#
# Returns:
#	sql_attr_string   - String with SQL attribute.
#	sql_values_string - String with SQL values.
#	@binds			  - SQL bind values.
#
# Example usage:
# > my ($attr_str, $val_str, @binds) 
# > 	= $self->construct_insert_query($data_ref);
# > $self->sql_insert("INSERT INTO $table ($attr_str) VALUES ($val_str)", @binds);
sub construct_insert_query {
	my $self = shift;
	my $attr_data_ref = shift;
	my @valid = @_;
	
	my ($attributes, $values);
	my @bind_values;

	while (my ($key, $value) = each(%{$attr_data_ref})) {
		next unless grep { /^$key$/ } @valid;
	
		$attributes .= 	"$key,";
		$values .= "?,";
		push @bind_values, $value;
	}
	chop ($attributes, $values);
	return ($attributes, $values, @bind_values);
}


## 
# Constructs part of an update query.
#
# Parameters:
#	attr_data_ref - Hashref with db tables: attribute => value
#	@valid        - List of valid db table attributes.
#
# Returns:
#	sql_set_str       - Set part of an SQL UPDATE query.
#	@binds			  - SQL bind values.
#
# Example usage:
# > my ($set_str, @binds) 
# > 	= $self->construct_update_query($data_ref);
# > $self->sql_update("UPDATE $table SET $set_str WHERE id = ?", @binds, $id);
sub construct_update_query {
	my $self = shift;
	my $attr_data_ref = shift;
	my @valid = @_;

	my @bind_values;
	my $set_str;

	while (my ($key, $value) = each(%{$attr_data_ref})) {
		next unless grep /^$key$/, @valid;

		$set_str .= "$key=?,";
		push @bind_values, $value;
	}
	chop $set_str;

	return ($set_str, @bind_values);
}



# Group: Private Methods

##
# Run perpare and execute on sql query
#
# Attributes:
#	query - Sql query
#	@binds - Bind values
#
# Returns:
#	sth - Statement
sub _prepare_and_execute {
	my $self = shift;
	my $query = shift;
	my @binds = @_;
	my $dbh = $self->{'dbh'};
	
	my $sth = $dbh->prepare($query)
		or croak "Pepare: ", $dbh->errstr;
	
	my $rv = $sth->execute(@binds)
		or croak "Execute: ", $dbh->errstr;
	return $sth;
}

1;
