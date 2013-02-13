# Class: Sql::Webadmin
# Abstract class with method common for all Sql classes. Should not be
# used directly, but inhereted.
#
package Sql::Webadmin;
use strict;
use warnings;
use Data::Dumper;
use SQL::Abstract;
use Params::Validate qw(validate_pos SCALAR HASHREF);
use Carp;

sub new {
	my $class = shift;
	my $dbh   = shift;

	my $self = bless {
		dbh => $dbh,
		abstr => SQL::Abstract->new(),
	}, $class;
	bless $self, $class;
	
	$self->_init(@_)
		if $self->can('_init');
	
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

sub sql_array {
    my $self = shift;
    my $query = shift;
    my @binds = @_;

    my $sth = $self->_prepare_and_execute($query, @binds);
    my @res;
    while (my @row = $sth->fetchrow_array()) {
        if (scalar @row > 1) {
            push @res, \@row;
        }
        else {
            push @res, $row[0];
        }
    }
    return @res;
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
	return wantarray ? @results : shift @results;
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

sub sql_insert_returning_id {
    my $s = shift;
    $s->_prepare_and_execute(@_);
    $s->{dbh}->{ q{mysql_insertid} }
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
	if ($attributes || $values) {
		chop ($attributes, $values);
	}
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
my %sth_cache;
sub _prepare_and_execute {
	my $self = shift;
	my $query = shift;
	my @binds = @_;
	my $sth = $sth_cache{$query};
	if (!$sth) {
		$sth = $self->{dbh}->prepare($query)
			or confess "Prepare: ", $self->{dbh}->errstr;
		$sth_cache{$query} = $sth;
	}
	#warn $query;
	
	my $rv = $sth->execute(@binds)
		or croak "Execute: ", $self->{dbh}->errstr, " Query: ", $query;
	return $sth;
}

# Generic functions to subclass.
my $abstr = SQL::Abstract->new(quote_char => "`", name_sep => ".");
sub exists {
    validate_pos(@_, 1, 1, {type => SCALAR }, { type => HASHREF });
    my ($s, $tbl, $column, $where) = @_;
    return defined $s->sql_single($abstr->select($tbl, $column, $where));
}
sub get {
    validate_pos(@_, 1, { type => SCALAR }, { type => HASHREF }, 0, 0);
    my ($s, $tbl, $where, $what, $order) = @_;
    $what ||= '*';
    my ($q, @binds) = $abstr->select($tbl, $what, $where, $order);
    #warn Dumper([$q, @binds]);
    $s->sql_hashref($q, @binds);
        
}
sub insert {
    validate_pos(@_, 1, { type => SCALAR }, { type => HASHREF }, 0);
    my ($s, $tbl, $val, $return_id) = @_;
    my ($q, @binds) = $abstr->insert($tbl, $val);
    $return_id  ? $s->sql_insert_returning_id($q, @binds) 
                : $s->sql_insert($q, @binds);
}
sub delete {
    validate_pos(@_, 1, 1, { type => HASHREF });
    my ($s, $tbl, $where) = @_;
    $s->sql_delete($abstr->delete($tbl, $where));
}
sub update {
    validate_pos(@_, 1, { type => SCALAR }, { type => HASHREF }, 0);
    my ($s, $tbl, $val, $where) = @_;
    my ($q, @binds) = $abstr->update($tbl, $val, $where);
    $s->sql_update($q, @binds);
}

sub dbh {
	return shift->{dbh}
}

1;
