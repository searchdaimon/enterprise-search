# Class: Page::Setup::Integration
# Network integration
package Page::Setup::Integration;
use strict;
use warnings;
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Carp;
use Params::Validate;
use Data::Dumper;

use Page::Abstract;
use Boitho::Infoquery;
#use Sql::Config;
use Sql::System;
use Sql::SystemParam;
use Data::UserSys;

our @ISA = qw(Page::Abstract);
use config qw(%CONFIG);

use constant TPL_INTEGRATION     => "setup_integration_method.html";
use constant TPL_INTEGRATIOM_VAL => "setup_integration_values.html";

sub _init {
	my $s = shift;
	$s->{'infoQuery'}	 = Boitho::Infoquery->new;
	$s->{sql_sys} = Sql::System->new($s->{dbh});

	$s;
}

sub show {
	my ($s, $vars) = @_;
	$vars->{user_systems} = $CONFIG{user_systems};
	$vars->{primary} = $s->{sql_sys}->get({ is_primary => 1 });
	return TPL_INTEGRATION;
}


sub process_integration {
	my ($s, $vars, $sys_ref) = @_;


	croak "FATAL: A primary system already exists"
		if  $s->{sql_sys}->exists({ is_primary => 1 }) 
		and $s->{sql_sys}->get({ is_primary => 1})->{ip} ne '127.0.0.1';

	$s->{sql_sys}->delete({ is_primary => 1 })
		if $s->{sql_sys}->exists({ is_primary => 1 });

	$sys_ref->{is_primary} = 1;
	eval {
		my $sysobj = Data::UserSys->create($s->{dbh}, %{$sys_ref});
	};
	if ($@) {
		$vars->{error_msg} = $@;
		$vars->{error_msg} =~ s/at .*? line \d+$//;
		return $s->show_integration_values(
			$vars, 
			$sys_ref->{connector},
			$sys_ref->{name},
			$sys_ref,
		);
	}
	
	return undef;
}



sub show_integration_values {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) }, 1, 0);
	my ($s, $vars, $conn_id, $sys_name, $sys_attr) = @_;
	
	$vars->{user_systems} = $CONFIG{user_systems};
	if ($sys_attr) {
		$vars->{sys} = $sys_attr;
	}
	my %param_values = $sys_attr->{params} 
		? %{$sys_attr->{params}} : ( );
	#croak Dumper(\%param_values);
	my @params = Sql::SystemParam->new($s->{dbh})->get({
		connector => $conn_id,
	});
	#croak Dumper(\@params);


	$vars->{sys}{params} = { map {
		$_->{param} => {
			note => $_->{note},
			required => $_->{required},
			value => $param_values{$_->{param}}
		}
	} @params };
	#croak Dumper($vars);
	$vars->{sys}{connector} = $conn_id;
	$vars->{sys}{name} = $sys_name;
	
	return TPL_INTEGRATIOM_VAL;
}

1;
