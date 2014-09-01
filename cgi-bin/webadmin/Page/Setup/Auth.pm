# Class: Page::Setup::Auth
# License validation
package Page::Setup::Auth;
use strict;
use warnings;
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Carp;
use SD::License::Client;
use SD::License::ClientCommunication;
use Page::Abstract;
use Sql::Config;
use config qw($CONFIG);
our @ISA = qw(Page::Abstract);

my $licenseComm;
my $licenseClient;
my $sqlConfig;


##
# Init.
sub _init {
	my $self = shift;
	my $dbh = $self->{'dbh'};

	$sqlConfig = Sql::Config->new($dbh);
	$licenseComm   = SD::License::ClientCommunication->new($CONFIG->{'interface_url'});
	$licenseClient = SD::License::Client->new($CONFIG->{'bb_sha_test_path'}, $CONFIG->{'bb_verify_msg'});
	
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
sub show_activation_dialog {
	my ($self, $vars, $license, $signature) = @_;
	my $template_file = "setup_auth_manual.html";

	$vars->{'activate_url'} = $CONFIG->{'activate_url'};

	$vars->{'license'}   = $license
		if defined $license;
	$vars->{'signature'} = $signature
		if defined $signature;

	$vars->{'hardware_code'} 
		= $licenseClient->bb_sha_test();
	
	return ($vars, $template_file);
}

##
# Authenticates license against our authserver. 
#
# Attributes:
#	vars - Template vars
#	license - User license
#
# Returns:
#	vars - Template vars
#	success - True/False on success.
sub process_license {
	my ($self, $vars, $license) = @_;
	
	# Request signature from server
	my ($signature, $hwhash);
	eval {
		$hwhash = $licenseClient->bb_sha_test();
		$signature = $licenseComm->authenticate($license, $hwhash);
	};
	if ($@) {
		# Error
		my $errmsg = $@;
		$errmsg =~ s/at (.*) line.*//; # strip "at /path/ line .." from errormsg
		$vars->{'error_msg_auth'} = $errmsg;
		return ($vars, 0);
	}

	# License existed for hw in SD db.
	# Check if it's valid for the hardware.
	my ($sig_is_valid, $errmsg) = $licenseClient->bb_verify_msg($license, $hwhash, $signature);
	unless ($sig_is_valid) {
		my $err = "License appears to be invalid.";
		$err .= " Error retuned was <i>$errmsg</i>." if $errmsg;
		$err .= "Please try again, or 
			try <a href=\"setup.cgi?view=manual_activation\">manual activation</a>.";
		$vars->{'error_msg_auth'}  = $err;
		return ($vars, 0);
	}

	$self->_store_license_data($license, $signature);
	
	return ($vars, 1);
}

##
# Validate signature on manual activation.
sub process_signature {
	my ($self, $vars, $license, $hardware, $signature) = @_;
	
	my ($valid, $error) = $licenseClient->bb_verify_msg($license, $hardware, $signature);
	
	if ($valid) {
		$self->_store_license_data($license, $signature);
		return ($vars, 1);
	}
	else {
		my $errmsg = "Signature could not be verified.";
		$errmsg .= " $error." if ($error);
		$vars->{'error_msg_auth'} = $errmsg;
		return ($vars, 0);
	}
}

# Group: Private methods

##
# Store license and signature in a file.
#
# Attributes:
#	license   - User license
#	signature - License/hw signature. 
sub _store_license_data {
	my ($self, $license, $signature) = @_;

	open my $lh, ">", $CONFIG->{'license_file'},
		or croak "Unable to open license file ", $CONFIG->{'license_file'};

	print {$lh} $license;
	close $lh;

	open my $sh, ">", $CONFIG->{'signature_file'}, 
		or croak "Unable to open signature file ", $CONFIG->{'signature_file'};

	print {$sh} $signature;
	close $sh;

	1;
}



1;
