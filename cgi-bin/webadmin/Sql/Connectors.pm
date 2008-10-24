package Sql::Connectors;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use SQL::Abstract;
use Readonly;
use Params::Validate;

use Sql::Webadmin;
use Sql::Sql;

our @ISA = qw(Sql::Webadmin);

Readonly::Scalar our $TBL => "connectors";

my $dbh;
my $abstr = SQL::Abstract->new();

sub _init { $dbh = $_[0]->{dbh} }

sub get_active {my $s = shift; $s->get({ active => 1 }); }

sub get_all_ids {
	my $query = "SELECT id FROM $TBL";
	return Sql::Sql::multi_row_fetch($dbh, $query);
}

sub get_all_names {
	my $self = shift;
	
	my $query = "SELECT name FROM $TBL";
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
    validate_pos(@_, 1, { regex => qr(^\d+$) });
    my ($s, $id) = @_;
    return $s->get({ id => $id }, 'name')->{name};
}

## Get ID for a given name.
## Names are unique, so there will
## only be one result.
sub get_id($$) {
	my $self = shift;
	my $id   = shift;
	my $query = "SELECT id FROM $TBL
					WHERE name = ?";
	return Sql::Sql::single_fetch($dbh, $query, $id);
}

sub get_connectors_with_scantool {
	my $self = shift;
	my $query = "SELECT * FROM $TBL
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
		$query = "SELECT inputFields FROM $TBL
				WHERE id = ?";
	} else {
		$query = "SELECT inputFields FROM $TBL
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

sub is_extension { shift->exists({ id => shift, extension => 1 }) }
sub is_readonly { shift->exists({ id => shift, read_only => 1 }) }

sub exists { shift->SUPER::exists($TBL, 'id', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }

1;
