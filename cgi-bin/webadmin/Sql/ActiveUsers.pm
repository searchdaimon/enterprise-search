package Sql::ActiveUsers;
use strict;
use warnings;

use Carp;
use Readonly;
use Sql::Webadmin;

Readonly::Scalar our $TBL => "activeUsers";
our @ISA = qw(Sql::Webadmin);

sub exists { shift->SUPER::exists($TBL, 'user', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }

sub is_active { return shift->exists({user => shift }) }

sub num_active {
	my $s = shift;
	my $q = "SELECT count('user') FROM $TBL";
	return $s->sql_single($q);
}

1;
