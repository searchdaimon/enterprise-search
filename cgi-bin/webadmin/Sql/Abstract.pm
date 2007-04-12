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
