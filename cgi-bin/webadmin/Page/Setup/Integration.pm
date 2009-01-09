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
		if $s->{sql_sys}->exists({ is_primary => 1 }) and $s->{sql_sys}->get({ is_primary => 1})->{ip} ne '127.0.0.1';
	$s->{sql_sys}->delete({ is_primary => 1 })
		if $s->{sql_sys}->exists({ is_primary => 1 });

	$sys_ref->{is_primary} = 1;
	my $sysobj = Data::UserSys->create($s->{dbh}, %{$sys_ref});
	
	1;
}



sub show_integration_values {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) }, 0);
	my ($s, $vars, $conn_id, $ignore_db_values) = @_;
	
	$vars->{user_systems} = $CONFIG{user_systems};
	my $sql_param = Sql::SystemParam->new($s->{dbh});
	my @params = $sql_param->get({ 
		connector => $conn_id
	});
	$vars->{sys}{params} = { map {
		 $_->{param} => { note => $_->{note} } 
	} @params };
	$vars->{sys}{connector} = $conn_id;
	
#	unless ($ignore_db_values) {
#		$vars->{'dap_settings'} 
#			= $sqlConfig->get_dap_settings($method);
#	}
#	
#	$vars->{'method'} = $method;
	return TPL_INTEGRATIOM_VAL;
}

1;
