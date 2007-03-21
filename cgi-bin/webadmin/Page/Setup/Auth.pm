# Class: Page::Setup::Auth
# License validation
package Page::Setup::Auth;
use strict;
use warnings;
BEGIN {
	push @INC, "Modules";
}
use Carp;
use Boitho::LicenseAuth;
use Page::Abstract;
use Sql::Config;
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

##
# Init.
sub _init {
	my $self = shift;
	my $dbh = $self->{'dbh'};
	my $licauth_params = {
			'hwgen_path'     => $CONFIG->{'hwgen_path'},
			'interface_url'  => $CONFIG->{'interface_url'},
			'authvalid_path' => $CONFIG->{'authvalid_path'},
		};
	$self->{'licenseAuth'} = Boitho::LicenseAuth->new($licauth_params);
	$self->{'sqlConfig'} = Sql::Config->new($dbh);
	return $self;
}
##
# Displays form to input license.
#
# Attributes:
#	vars - Template vars
#
# Returns:
#	vars - Template vars
#	template_file - Template file
sub show_license_dialog {
	my ($self, $vars, $license) = @_;
	my $template_file = "setup_auth.html";
	$vars->{'license'} = $license
		if defined $license;
	return ($vars, $template_file);
}

##
# Display link to manual activation, field to insert auth-code.
#
# Attributes:
#	vars - Template vars
#
# Returns:
#	vars - Template vars
#	template-file - Template file
sub show_activation_dialog {
	my ($self, $vars, $auth_code) = @_;
	my $template_file = "setup_auth_manual.html";
	my $licenseAuth = $self->{'licenseAuth'};

	$vars->{'activate_url'} = $CONFIG->{'activate_url'};
	
	$vars->{'auth_code'} = $auth_code
		if defined $auth_code;

	$vars->{'hardware_code'} 
		= $licenseAuth->generate_hardware_code();
	
	return ($vars, $template_file);
}

##
# Authenticates license against search daimon authserver. 
# Calls <show_license_dialog> on fail, or <????> on success.
#
# Attributes:
#	vars - Template vars
#	license - User license
sub process_license {
	my ($self, $vars, $license) = @_;
	my $licenseAuth = $self->{'licenseAuth'};
	my $sqlConfig = $self->{'sqlConfig'};
	
	my ($auth_code, $erromsg);
	eval {
		return 1; # DEBUG: Slik vi får testet alt annet.
		$licenseAuth->generate_hardware_code();
		$auth_code = $licenseAuth->authenticate($license);
	};
	if ($@) {
		# Error
		$vars->{'error_msg_auth'} = $@;
		return ($vars, 0);
	}

	unless ($licenseAuth->auth_code_is_valid($auth_code)) {
		$vars->{'error_msg_auth'} 
			= "License appeared to be valid, but the response from
			activation server could not be verified. Please try again, or 
			try <a href=\"setup.cgi?view=manual_activation\">manual activation</a>.";
		return ($vars, 0);
	}

	# Success! Store auth code and proceed.
	$sqlConfig->insert_setting('auth_code', $auth_code, $license);
	return ($vars, 1);
}

##
# Validate authcode on manual activation.
# Calls <show_activation_dialog> on fail, or <????> on success.
# Attributes:
#	vars - Template vars
#	auth_code - Authentication code
sub process_auth_code {
	my ($self, $vars, $auth_code) = @_;
	my $licenseAuth = $self->{'licenseAuth'};
	
	if ($licenseAuth->auth_code_is_valid($auth_code)) {
		return ($vars, 1);
	}
	else {
		$vars->{'error_msg_auth'} = "Authentication code is not correct.";
		return ($vars, 0);
	}
}



1;