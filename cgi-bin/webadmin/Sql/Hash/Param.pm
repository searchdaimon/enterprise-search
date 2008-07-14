package Sql::Hash::Param;
use strict;
use warnings;

use Tie::Hash;
use Carp;
use Data::Dumper;
use Params::Validate;

use Sql::Param;
use Sql::ShareParam;
use Sql::Shares;

our @ISA = qw(Tie::Hash);

sub TIEHASH {
    validate_pos(@_, 1, { regex => qr(^\d+$) }, 1);
    my ($class, $coll_id, $dbh) = @_;
   
    my $sqlColl = Sql::Shares->new($dbh);
    my $sqlShareParam = Sql::ShareParam->new($dbh);

    croak "Collection does not exist"
        unless $sqlColl->exists({id => $coll_id});
    
    bless { 
        id            => $coll_id,
        connector     => $sqlColl->get_connector($coll_id),
        sqlParam      => Sql::Param->new($dbh),
        sqlShareParam => $sqlShareParam,
    }, $class;
}


sub DELETE {
    my ($s, $param) = @_;
    $s->{sqlShareParam}->delete({ 
        id => $s->{id}, 
        param => $s->param_id($param),
    });
    1;
}
sub CLEAR { 
    my $s = shift;
    $s->{sqlShareParam}->delete({ share => $s->{id}});
    1;
}
sub STORE { 
    my ($s, $param, $val) = @_;
    
    croak "Parameter '$param' does not exist for connector '$s->{connector}'"
        unless $s->{sqlParam}->exists({
            param => $param, 
            connector => $s->{connector},
        });
    my $param_id = $s->param_id($param);
    
    my $exists = $s->{sqlShareParam}->exists({
        param => $param_id, 
        share => $s->{id}
    });
    if ($exists) {
        $s->{sqlShareParam}->update({
            value => $val
        }, {
            param => $param_id,
            share => $s->{id}
        })
    }
    else {
        $s->{sqlShareParam}->insert({ 
            param => $param_id,
            value => $val,
            share => $s->{id},
        });
    }
    1;
}
sub FETCH {
    my ($s, $param) = @_;
    my $res_ref = $s->{sqlShareParam}->get({ 
            param => $s->param_id($param), 
            share => $s->{id},
        }, 'value');
    return unless $res_ref;
    return $res_ref->{value};
}

sub FIRSTKEY { 
    my $s = shift;
    my @keys = $s->{sqlShareParam}->get_param_names($s->{id});
    $s->{keys_copy} = \@keys;
    shift @{$s->{keys_copy}};
}

sub NEXTKEY { 
    my $s = shift;
    shift @{$s->{keys_copy}};
}

sub EXISTS { 
    my ($s, $param) = @_;
    $s->{sqlShareParam}->exists({
        share => $s->{id}, 
        param => $s->param_id($param),
    });
}

sub param_id {
    my ($s, $param) = @_;
    return $s->{sqlParam}->get(
        { 
            param => $param,
            connector => $s->{connector},
        }, 
        'id')->{id};
}



1;
