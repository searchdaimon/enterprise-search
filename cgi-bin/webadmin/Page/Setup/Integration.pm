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
use Page::Abstract;
use Sql::Config;
use Boitho::Infoquery;
our @ISA = qw(Page::Abstract);

##
# Init.
sub _init {
	my $self = shift;
	my $dbh = $self->{'dbh'};
	$self->{'sqlConfig'} = Sql::Config->new($dbh);
	$self->{'infoQuery'}	 = Boitho::Infoquery->new;

	return $self;
}

sub show_integration_methods($$) {
	my ($self, $vars) = @_;
	my $template_file = "setup_integration_method.html";
	my $sqlConfig = $self->{'sqlConfig'};
	return ($vars, $template_file);
}


sub process_integration {
	my ($self, $vars, $args_ptr) = @_;
	my %args = %{$args_ptr};
	
	my $sql       = $self->{'sqlConfig'};
	my $method    = $args{'auth_method'};
	my $infoQuery = $self->{'infoQuery'};
	
	my @args = ($args{'domain'}, $args{'user'}, 
	$args{'password'}, $args{'ip'}, $args{'port'});
	
	# Validate
	{
		my $valid = 1;
		my $error;
		($valid, $error) = (0, "Integration method has not been set.")
			unless $method;
		
		($valid, $error) = $infoQuery->authTest(@args)
			unless not $valid;
		
		unless ($valid) {
			carp $error;
			$vars->{'error_msg'} = $error;
			$vars->{'dap_settings'} = $args_ptr;
			return ($vars, 0);
			#return $self->show_integration_values($vars, $method, 1);	
		}
	}
	
	# Update
	if ($method eq 'msad') {
		$sql->update_authenticatmethod($method);
		$sql->update_msad(@args);
	
	}
	elsif ($method eq 'ldap') {
		$sql->update_authenticatmethod($method);
		$sql->update_ldap(@args);
	}
	
	elsif ($method eq 'shadow') {
		$sql->update_authenticatmethod("shadow");
	
	}
	else {
		croak ("Unknown authentication method submitted:" , $method);
	}
	
	return ($vars, 1);
}



sub show_integration_values($$$) {
	my ($self, $vars, $method, $ignore_db_values) = @_;
	my $template_file = "setup_integration_values.html";
	my $sqlConfig = $self->{'sqlConfig'};
	
	if ($method eq 'shadow') {
		croak "This method should not have been reached if the 
				method is shadow. Shadow needs no integration values.";	
	}
	
	unless ($ignore_db_values) {
		$vars->{'dap_settings'} 
			= $sqlConfig->get_dap_settings($method);
	}
	
	$vars->{'method'} = $method;
	return ($vars, $template_file);
}

1;
