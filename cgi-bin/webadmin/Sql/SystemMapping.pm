package Sql::SystemMapping;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use SQL::Abstract;
use Readonly;
use Params::Validate;

use Sql::Webadmin;

our @ISA = qw(Sql::Webadmin);

Readonly::Scalar our $TBL => "systemMapping";


sub exists { shift->SUPER::exists($TBL, 'system', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }
		
sub get_mapping {
	my ($s, $sys, $offset, $limit) = @_;
	my $q = "SELECT * FROM $TBL
		WHERE system = ?";
	if (defined $offset && defined $limit) {
		unless (   $offset =~ /^\d+$/ 
			&& $limit  =~ /^\d+$/) {
			croak "invalid offset, limit"
		}
		$q .= " LIMIT $offset, $limit";
	}
	return $s->sql_hashref($q, $sys);
}

sub count {
	my ($s, $where_ref) = @_;
	my ($q, @binds) = $s->{abstr}->select($TBL, ["COUNT(system)"], $where_ref);
	return $s->sql_single($q, @binds);
}


1;
