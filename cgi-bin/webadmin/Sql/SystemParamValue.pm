package Sql::SystemParamValue;

use Readonly;
use Sql::Webadmin;

Readonly::Scalar our $TBL => "systemParamValue";
our @ISA = qw(Sql::Webadmin);

sub exists { shift->SUPER::exists($TBL, 'system', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }

