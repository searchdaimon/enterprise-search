package Sql::SystemParam;

use Readonly;
use Sql::Webadmin;

Readonly::Scalar our $TBL => "systemParam";
our @ISA = qw(Sql::Webadmin);

sub exists { shift->SUPER::exists($TBL, 'connector', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }


