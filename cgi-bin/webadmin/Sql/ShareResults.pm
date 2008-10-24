package Sql::ShareResults;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Readonly;
use Params::Validate;

use Sql::Webadmin;

Readonly::Scalar our $TBL => "shareResults";
our @ISA = qw(Sql::Webadmin);

sub exists { shift->SUPER::exists($TBL, 'share', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }



1;
