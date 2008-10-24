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


1;
