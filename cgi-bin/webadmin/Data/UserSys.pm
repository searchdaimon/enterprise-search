package Data::UserSys;
use strict;
use warnings;
use Carp;
use Params::Validate qw(validate_pos HASHREF);
use Readonly;
use Data::Dumper;


use Sql::System;
use Sql::Hash::SystemParam;
use Sql::SystemParamValue;
use Sql::SystemParam;


Readonly::Hash my %USERSYS_ATTR 
	=> map { $_ => 1 } qw(name is_primary connector);

sub new  { 
	validate_pos(@_, 1 , 1, { regex => qr/^\d+$/ });
	my ($class, $dbh, $sys_id) = @_;
	my $sql_sys = Sql::System->new($dbh);
	my $sys_ref = $sql_sys->get({ id => $sys_id })
		or croak "System '$sys_id' does not exist.";
	
	my $p_obj = tie my %p_values, 'Sql::Hash::SystemParam', $sys_id, $dbh
		or confess $!;
	my %params = map {$_ => {
		value => $p_values{$_},
		note => $p_obj->note($_),
		required => $p_obj->required($_),
	}} keys %p_values;
	$sys_ref->{params} = \%params;
	
	bless { 
		sql_sys => $sql_sys,
		sys     => $sys_ref,
		dbh     => $dbh,
		sql_param => Sql::SystemParam->new($dbh),
	}, $class;
}

## 
# Ignore parameter validation. Used when creating
# a test system.
sub create_novalidate {
	my ($class, $dbh, %attr) = @_;
	
	my $params_ref = $attr{params} || { };
	delete $attr{params};

	my $sql = Sql::System->new($dbh);

	# if we have no system this wil be our primary system
	if (!$sql->have_system()) {
		$attr{is_primary} = 1;	
	}

	# Add system
	delete $attr{name}
		unless $attr{name};
	_del_invalid_attr(\%attr);
	
	my $sys_id = $sql->insert(\%attr, 1);

	# Add custom params
	tie my %p_values, 'Sql::Hash::SystemParam', $sys_id, $dbh
		or confess $!;
	%p_values = %{$params_ref};
	return $class->new($dbh, $sys_id);

}

sub create {
	my ($class, $dbh, %attr) = @_;
	_validate_params($dbh, $attr{connector}, $attr{params} || { });
	create_novalidate($class, $dbh, %attr);
}

sub update {
	my ($s, %attr) = @_;

	my $sql_paramval = Sql::SystemParamValue->new($s->{dbh});

	my $params_ref = $attr{params} || { };
	delete $attr{params};

	# magic! keep current pass, if none is provided
	if (!exists($params_ref->{password}) || $params_ref->{password} eq q{}) {
		my $curr_pass  = $sql_paramval->get({
			param => 'password',
			system => $s->{sys}{id}
		});
		$params_ref->{password} = $curr_pass->{value}
			if $curr_pass;
	}

	_validate_params(
		$s->{dbh}, 
		$s->{sys}{connector}, 
		$params_ref, 
	);

	# update params.
	tie my %p_val, 'Sql::Hash::SystemParam', $s->{sys}{id}, $s->{dbh}
		or confess $!;
	%p_val = %{$params_ref};
	
	# update system tbl
	_del_invalid_attr(\%attr);
	if (%attr) {
		$s->{sql_sys}->update(\%attr, { id => $s->{sys}{id} });
	}

	1;
}

sub get {
	my ($s, $attr) = @_;
	if (defined $attr) {
		return $s->{sys}{$attr};
	}
	return %{$s->{sys}};
}

##
# Returns all system parameters, including ones not set.
sub get_param_all {
	my $s = shift;
	my @params = $s->{sql_param}->get({ 
		connector => $s->{sys}{connector}
	});
	return map {
		 $_->{param} => { 
			note     => $_->{note}, 
		 	required => $_->{required},
			value    => $s->{sys}{param}{$_->{param}},
		} 
	} @params;
}

sub del {
	my $s = shift;

	# Delete params
	tie my %p_val, 'Sql::Hash::SystemParam', $s->{sys}{id}, $s->{dbh}
		or confess $!;
	%p_val = ();

	# Delete system
	$s->{sql_sys}->delete({ id => $s->{sys}{id} });
	
	
	# TODO: Delete mapping ?

	$s = undef;
	1;
}

sub _del_invalid_attr {
	my ($attr_ref) = @_;
	
	for my $a (keys %{$attr_ref}) {
		next if $USERSYS_ATTR{$a}; # valid
	
		warn "ignoring invalid attr '$a'";
		delete $attr_ref->{$a};
	}
}

## 
# croaks on missing param
sub _validate_params {
	validate_pos(@_, 1, { regex => qr(^\d+$) }, { type => HASHREF });
	my ($dbh, $conn_id, $params_ref) = @_;

	my $sql_param = Sql::SystemParam->new($dbh);
	my @required_param = map { $_->{param} } $sql_param->get({ 
		connector => $conn_id, 
		required => 1 
	}, 'param');
	
	for my $p (@required_param) {
		croak "Required parameter '$p' missing."
			unless defined($params_ref->{$p})
			       && $params_ref->{$p} ne q{};
	}
	
	1;
}

##
# Croaks if system should not be deleted. This is when:
# * it's still in use.
# * it's primary system.
sub restrictive_del {
	my $s = shift;

	my $prim = $s->{sql_sys}->primary_id;

	my $shares = Sql::Shares->new($s->{dbh});

	if (my @coll = $shares->get({ system => $s->{sys}{id} }, 'collection_name')) {
	
		croak "Can't delete user system. User system is in use by '" 
			. join(", ", map { $_->{collection_name} } @coll)
			. "'";
	}

	$s->del();
}



1;
