package Data::UserSys;
use strict;
use warnings;
use Carp;
use Params::Validate;
use Readonly;
use Data::Dumper;


use Sql::System;
use Sql::Hash::SystemParam;


Readonly::Hash my %USERSYS_ATTR 
	=> map { $_ => 1 } qw(name ip user password is_primary connector);

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
	}} keys %p_values;
	$sys_ref->{params} = \%params;
	
	bless { 
		sql_sys => $sql_sys,
		sys     => $sys_ref,
		dbh     => $dbh,
	}, $class;
}



sub create {
	my ($class, $dbh, %attr) = @_;

	my $params_ref = $attr{params} || { };
	delete $attr{params};

	# Add system
	delete $attr{name}
		unless $attr{name};
	_del_invalid_attr(\%attr);
	
	my $sql = Sql::System->new($dbh);
	my $sys_id = $sql->insert(\%attr, 1);

	# Add custom params
	tie my %p_values, 'Sql::Hash::SystemParam', $sys_id, $dbh
		or confess $!;
	%p_values = %{$params_ref};
	return $class->new($dbh, $sys_id);
}

sub update {
	my ($s, %attr) = @_;
	
	# update params.
	tie my %p_val, 'Sql::Hash::SystemParam', $s->{sys}{id}, $s->{dbh}
		or confess $!;
	$attr{params} ||= { };
	%p_val = %{$attr{params}};
	delete $attr{params};
	
	# update system tbl
	$s->_del_invalid_attr(\%attr);
	$s->{sql_sys}->update(\%attr, { id => $s->{sys}{id} });

	1;
}

sub get {
	my ($s, $attr) = @_;
	if (defined $attr) {
		return $s->{sys}{$attr};
	}
	return %{$s->{sys}};
}

sub _del_invalid_attr {
	my ($s, $attr_ref) = @_;
	
	for my $a (keys %{$attr_ref}) {
		next if $USERSYS_ATTR{$a}; # valid
	
		warn "ignoring invalid attr '$a'";
		delete $attr_ref->{$a};
	}
}


1;
