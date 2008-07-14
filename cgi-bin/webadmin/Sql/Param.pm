package Sql::Param;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Readonly;

use Sql::Webadmin;

Readonly::Scalar our $TBL => "param";
our @ISA = qw(Sql::Webadmin);

sub exists { shift->SUPER::exists($TBL, 'param', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }



1;
