package Sql::ShareParam;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Readonly;
use Params::Validate;

use Sql::Webadmin;

Readonly::Scalar our $TBL => "shareParam";
our @ISA = qw(Sql::Webadmin);

sub exists { shift->SUPER::exists($TBL, 'param', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }

sub get_param_names {
    validate_pos(@_, 1, { regex => qr(^\d+$) });
    my ($s, $coll_id) = @_;

    my $q = q{
        SELECT param.param AS 'name'
        FROM param, shareParam
        WHERE 
            shareParam.param = param.id
            AND shareParam.share = ?
    };
    $s->sql_array($q, $coll_id);
}

1;
