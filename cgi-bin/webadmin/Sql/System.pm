package Sql::System;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use SQL::Abstract;
use Readonly;
use Params::Validate;

use Sql::Webadmin;

our @ISA = qw(Sql::Webadmin);

Readonly::Scalar our $TBL => "system";

sub list {
	my $q = "SELECT * FROM system";
	shift->sql_hashref($q);
}

sub primary_id {
	my $s = shift;
	my $res = $s->get({ is_primary => 1 }, 'id');
	return unless $res;
	return $res->{id};
}

sub exists { shift->SUPER::exists($TBL, 'id', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }



1;
