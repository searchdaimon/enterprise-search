## 
# Class for communaction with SD license server.
package SD::License::ClientCommunication;
use strict;
use warnings;
use Carp;
use LWP::Simple qw(get);
use Data::Dumper;
use XML::SimpleObject;
use XML::Parser;


my $interface_url;

## 
# Default constructor.
#
# Parameters:
#	interface_url = HTTP interface to SD license server.
sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;

	if (scalar @_ != 1) {
		croak __PACKAGE__, " requires 1 argument.";
	}
	
	#init
	$interface_url = shift;
	
	croak "Didn't provide interface url."
		unless defined $interface_url;
	
	return $self;

}

##
# Authenticates license and hardware against search daimon auth server.
#
# Attributes:
#	license - User license
#	hardware - Hardware hash
sub authenticate {
	my ($self, $license, $hwhash) = @_;

	#sanity check
	my $i = 0;
	for my $arg ($license, $hwhash) {
		croak "argument $i missing" unless defined $arg;
		$i++;
	}
	

	# read response
	my $response = get("$interface_url?auth&hw=$hwhash&license=$license");
    
	unless (defined $response) {
	    croak "Unable to connect to authentication server. If the Black Box isn't
		connected to the internet, please use manual activation.";
	}

	my ($r_name, $r_type, $r_data) = $self->_parse_response($response);
	
	if ($r_name eq 'error') {
		if ($r_type eq 'max_act_exceeded') {
			croak "This license has been activated on too many search boxes, 
				 and can't be activated on more. "
		}
		
		if ($r_type eq 'invalid_license') {
			croak "The license you entered seems to be invalid, 
				please check if you typed it correctly.";
		}
		else {
			croak "Could not authenticate due to unknown error \"$r_type $r_data\"";
		}
	}
	elsif ($r_name eq 'auth') {
		if ($r_type eq 'signature') {
			# Success. Let's return the signature. 
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
