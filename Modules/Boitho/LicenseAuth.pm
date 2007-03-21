# Class: Boitho::LicenseAuth
# Methods for license validation.
package Boitho::LicenseAuth;
use strict;
use warnings;
use Carp;
use LWP::Simple qw(get);
use Data::Dumper;


## Constructor: new
#
# Attributes:
#	param_ref = Hash ref of parameters
#
# Parameters:
#	hwgen_path - Path to software that generates hardware hash
#	authvalid_path - Path to the program that validates authcode.
#	interface_url - URL to boitho interface.
sub new {
	my $class = shift;
	my $param_ref = shift;
	my $self = {};
	bless $self, $class;
	
	#init
	my $hwgen     = $param_ref->{'hwgen_path'};
	my $authvalid = $param_ref->{'authvalid_path'};
	my $if_url    = $param_ref->{'interface_url'};
	
	croak "Didn't provide path to hardware-hash generating software."
		unless defined $hwgen;

	croak "Didn't provide path to authentication code validating software."
		unless defined $authvalid;
		
	croak "Didn't provide interface url."
		unless defined $if_url;
	
	foreach my $executable ($hwgen, $authvalid) {
		croak "Unable to execute $executable"
			unless -x $executable;
	}
	
	$self->{'hwgen_path'} = $hwgen;
	$self->{'interface_url'} = $if_url;
	$self->{'authvalid_path'} = $authvalid;	

	return $self;

}

##
# Executes software that generates hardware hash. Croaks on failure.
# Stores hash in instance, and returns it.
#
# Returns:
#	hash - hardware hash
sub generate_hardware_code {
	my $self = shift;
	
	my $success = 1;
	my $path = $self->{'hwgen_path'};
	open my $hwgen_h, "$path |"
		or croak "Unable to execute hardware-hash generating software.";
	
	my $input = <$hwgen_h>;
	close $hwgen_h or $success = 0;
	
	croak "Hardware-hash generating software exited with error"
		unless $success;
	
	$self->{'hardware_hash'} = $input;
	return $input;
}

##
# Authenticates license and hardware against search daimon auth server.
#
# Attributes:
#	license - User license
#	hardware - Hardware hash (optional)
sub authenticate {
	my ($self, $license, $hwhash) = @_;
	my $interface = $self->{'interface_url'};
	$hwhash = ($hwhash) ? $hwhash : $self->{'hardware_hash'};
	
	croak "Hardware hash has not been generated, nor was it provided."
		unless $hwhash;
		
	my $response = get("$interface?auth&hw=$hwhash&license=$license");
    
	unless (defined $response) {
	    die "Unable to connect to authentication server. If the Black Box isn't
		connected to the internet, please use manual activation.";
	}

	my ($r_name, $r_type, $r_data) = $self->_parse_response($response);
	
	if ($r_name eq 'error') {
		if ($r_type eq 'max_act_exceeded') {
			die "This license has been activated on too many search boxes, 
				 and can't be activated on more. "
		}
		
		if ($r_type eq 'invalid_license') {
			die "The license you entered seems to be invalid, 
				please check if you typed it correctly.";
		}
		else {
			die "Could not authenticate due to unknown error \"$r_type $r_data\"";
		}
	}
	elsif ($r_name eq 'auth') {
		if ($r_type eq 'auth_code') {
			# Success. Let's return the authentication code.
			return $r_data;
		}
		else {
			die "Unknown auth-response: $r_type, $r_data";
		}
	}
	else {
		die "Unknown responsetype $r_name, $r_type, $r_data";
	}
}

##
# Validate given auth code.
# Uses return value of executed program to determine if valid.
#
# Attributes:
#	authcode - Authentication code
#
# Returns
#	valid - true (1), false (0)
sub auth_code_is_valid {
	my ($self, $authcode) = @_;
	my $authvalid = $self->{'authvalid_path'};
	open my $auth_h, "$authvalid \Q$authcode\E|"
		or croak "Unable to execute authcode validating software, $!.";
	close $auth_h or return 0; 
	return 1;
}

##
# Parses XML response
#
# Attributes:
#	xml_response - String with XML
#
# Returns:
#	name - Response name
#	type - Response type
#	data - Response text value
sub _parse_response {
	my ($self, $xml_response) = @_;
	
	croak "No XML string provided"
		unless (defined $xml_response);

	my $parser = XML::Parser->new(Style => "tree");
	my $xml = XML::SimpleObject->new($parser->parse($xml_response));
	
	my ($msg_name) = $xml->children_names();
	my $msg_type = $xml->child($msg_name)->attribute('type');
	my $msg_data = $xml->child($msg_name)->value();

	return ($msg_name, $msg_type, $msg_data);
}

1;
