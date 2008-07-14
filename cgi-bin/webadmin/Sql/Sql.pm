# Class: Sql::Sql
# TODO: Deprecate. Replace with Sql::Webadmin that everything inherits.
package Sql::Sql;
use DBI;
use strict;
use warnings;
use config (qw($CONFIG));
use Carp;

## Creates a new connection and stores it in $self->{'dbh'}
sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	my $config = $self->read_config();
	my ($db, $host, $port) = ($config->{'database'}, 
				  $config->{'server'}, 
				  $config->{'port'});

	my $dbh = DBI->connect(
		"DBI:mysql:database=$db;host=$host;port=$port",
		$config->{'user'},
		$config->{'Password'})
		or croak("$DBI::errstr");

	$self->{'dbh'} = $dbh;

	return $self;

}

## Returns the db connection
sub get_connection {
	my $self = shift;
	return $self->{'dbh'};
}



## Helper function to put all rows of a query into a hash,
## and the hashes into an array.
## returns pointer to the array.
sub get_hashref_array {
	my ($dbh, $self);
	if (ref($_[0] eq 'Sql::Sql')) {
		$self = shift;
		$dbh = $self->get_connection();
	}
	else { $dbh = shift; }
	my $query = shift;
	my $bind_ref = shift;
	my @bind_values = Sql::Sql::_get_bind_values($bind_ref);

	my $sth = $dbh->prepare($query)
		or croak("pepare: " . $dbh->errstr);
	$sth->execute(@bind_values);
	
	my @results = ();
	while (my $r = $sth->fetchrow_hashref) {
		
		push (@results, $r);
	}
	return \@results;
}

# Method to simplify UPDATE and INSERT (and other) queries.
# Send in query as a first parameter
# and bind values as a second.
# The second parmaeter can eather be a single value, or a pointer to an array.
sub simple_execute {
	my ($dbh, $self);
	if (ref($_[0] eq 'Sql::Sql')) {
		$self = shift;
		$dbh = $self->get_connection();
	}
	else { $dbh = shift; }

	my ($query, $bind_ref) = (@_);
	my @bind_values = Sql::Sql::_get_bind_values($bind_ref);

	if (0) { #debug
		print $query, " ", @bind_values;
	}

	my $sth = $dbh->prepare($query)
		or croak "pepare: ", $dbh->errstr;
	my $rv = $sth->execute(@bind_values)
		or croak "execute:", $dbh->errstr;

	return 1; #Todo: change to rows updated. Maby. Return 0 might not be so good.
}

# Method to fetch a single value
sub single_fetch {
	my ($dbh, $self);
	if (ref($_[0] eq 'Sql::Sql')) {
		$self = shift;
		$dbh = $self->get_connection();
	}
	else { $dbh = shift; }

	my ($query, $bind_ref) = (@_);
	my @bind_values = Sql::Sql::_get_bind_values($bind_ref);
	my $sth = $dbh->prepare($query)
		or croak("pepare: " . $dbh->errstr);
	my $rv = $sth->execute(@bind_values)
		or croak("execute:", $dbh->errstr);

	return $sth->fetchrow_array;

}

sub single_row_fetch {
	my ($dbh, $self);
	if (ref($_[0] eq 'Sql::Sql')) {
		$self = shift;
		$dbh = $self->get_connection();
	}
	else { $dbh = shift; }

	my ($query, $bind_ref) = (@_);
	my @bind_values = Sql::Sql::_get_bind_values($bind_ref);
	
	my $sth = $dbh->prepare($query)
		or croak("pepare: " . $dbh->errstr);
	my $rv = $sth->execute(@bind_values)
		or croak("execute:", $dbh->errstr);
	
	my $data = $sth->fetchrow_hashref;

	return $data;
}

sub multi_row_fetch {
	my ($dbh, $self);
	if (ref($_[0] eq 'Sql::Sql')) {
		$self = shift;
		$dbh = $self->get_connection();
	}
	else { $dbh = shift; }
	
	my ($query, $bind_ref) = (@_);
	my $sth = Sql::Sql::_prepare_and_execute($dbh, $query, $bind_ref);
	my @data;
	while(my $row = $sth->fetchrow_array) {
		push @data, $row;
	}
	return \@data;

}


## Helper function to construct part of
## an insert query.
## Returns two string, and bind values.
## If you have two bind values, result is like this:
## 	attribute: "('key1', 'key2')"
## 	values: "(?, ?)"
## 	bind_values: ['value1', 'value2']
sub construct_insert_query($$) {
	#my $self = shift if ref($_[0] eq 'Sql::Sql');
	my $data = shift;
	my $valid = shift;
	
	my $attributes = "";
	my $values = "";
	my @bind_values = ();

	while (my ($key, $value) = each(%$data)) {
		next unless (grep /^$key$/, @$valid);
	
		$attributes .= 	"$key,";
		$values .= "?,";
		push @bind_values, $value;
	}
	chop ($attributes, $values);
	return ($attributes, $values, \@bind_values);
}


## Helper function to construct part of 
## an update query.
## Returns one string and one array pointer with bind values
## query: key1=?,key2=?
## bind_values = ['value1', 'value2']
sub construct_update_query($$) {
	my $data = shift;
	my $valid = shift;
	my @bind_values = ();
	my $query = '';
	while (my ($key, $value) = each(%$data)) {
		next unless (grep /^$key$/, @$valid);
		$query .= "$key=?,";
		push @bind_values, $value;
		
	}
	chop ($query);

	return ($query, \@bind_values);
}

## Get db connection values from a config file.
sub read_config($) {
	my $self = shift;
	my %settings = ();

	open my $setup, $CONFIG->{'config_path'} or croak "Can't open setup from $CONFIG->{'config_path'}: $!";
	my @data = <$setup>;
	close $setup;
	
	foreach my $line (@data) {
	
		my ($name, $value) = split(/=/, $line);
		chomp($value) if $value;
		$settings{$name} = $value if ($name and $value);

	}

	$settings{'port'} = 3306 unless($settings{'port'});
	
	return \%settings;
}


# Helper method to get sent in bind values. Returns an array (not a pointer).
# Grabage in garbage out.
sub _get_bind_values {
	my $self = shift if ($_ and ref($_[0] eq 'Sql::Sql'));
	my $bind_values = shift;

	return unless $bind_values;

		

	if (ref($bind_values) eq 'ARRAY') {
		return @$bind_values;
	}
	return $bind_values;
}

# Helper method to execute a query.
sub _prepare_and_execute {
	my $self = shift if (ref($_[0] eq 'Sql::Sql'));
	my $dbh = shift;
	my ($query, $bind_ref) = (@_);
	my @bind_values = Sql::Sql::_get_bind_values($bind_ref);
	
	my $sth = $dbh->prepare($query)
		or croak("pepare: " . $dbh->errstr);
	my $rv = $sth->execute(@bind_values)
		or croak("execute:", $dbh->errstr);
	return $sth;
}

1;
