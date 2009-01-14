package Sql::Hash::SystemParam;
use strict;
use warnings;
use Tie::Hash;
use Carp;
use Data::Dumper;
use Params::Validate;

use Sql::SystemParam;
use Sql::SystemParamValue;


our @ISA = qw(Tie::Hash);

sub TIEHASH {
	validate_pos(@_, 1, { regex => qr(^\d+$) }, 1);
	my ($class, $sys_id, $dbh) = @_;

	my $sql_sys = Sql::System->new($dbh);
	my $conn_id = $sql_sys->get({ 
		id => $sys_id 
	}, 'connector')->{connector};
	
	bless {
		sys_id => $sys_id,
		conn_id => $conn_id,
		sql_param => Sql::SystemParam->new($dbh),
		sql_param_val => Sql::SystemParamValue->new($dbh),
	}, $class;
}

sub DELETE {
	my ($s, $param) = @_;
	$s->{sql_param_val}->delete({
		system => $s->{sys_id},
		param  => $param,
	});
}

sub CLEAR { 
	my $s = shift;
	$s->{sql_param_val}->delete({
		system => $s->{sys_id},
	});
	1;
}
sub STORE { 
	my ($s, $param, $val) = @_;
	my $exists = $s->{sql_param}->exists({
		param => $param,
		connector => $s->{conn_id},
	});
	croak "param '$param' does not exist for connector $s->{conn_id}"
		unless $exists;
#	unless ($exists) {
#		$s->{sql_param}->insert({
#			connector => $param,
#			system => $s->{sys_id},
#		});
#	}

	$s->{sql_param_val}->insert({
		param => $param,
		value => $val,
		system => $s->{sys_id},
	});
	1;
}
sub FETCH { 
	my ($s, $param) = @_;
	my $val = $s->{sql_param_val}->get({
		param => $param,
		system => $s->{sys_id},
	}, 'value');
	return unless $val;
	return $val->{value};
}
	
sub FIRSTKEY { 
	my $s = shift;
	my @params = $s->{sql_param}->get({ 
		connector => $s->{conn_id}}, 'param');
	my @keys = map { $_->{param} } @params;

	$s->{keys_copy} = \@keys;
	shift @{$s->{keys_copy}};
}

sub NEXTKEY { 
	my $s = shift;
	shift @{$s->{keys_copy}};
}

sub EXISTS { 
	my ($s, $param) = @_;
	return $s->{sql_param_val}->exists({
			param => $param,
			system => $s->{sys_id},
			});
}

sub note {
	my ($s, $param) = @_;
	return $s->{sql_param}->get({ 
		param => $param,
		connector => $s->{conn_id},
	}, 'note')->{note};
}

sub required { 
	my ($s, $param) = @_;
	return $s->{sql_param}->get({ 
		param => $param,
		connector => $s->{conn_id},
	}, 'required')->{required};
}



1;
