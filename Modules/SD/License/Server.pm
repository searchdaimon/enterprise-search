##
# Wrapper for server side of license software.
package SD::License::Server;
use strict;
use warnings;
use Carp;

my $generatekeys_path;
my $sign_msg_path;

##
# Default constructor	
sub new {
	my $class = shift;
	if (scalar @_ != 2) {
		croak __PACKAGE__, " requires 2 arguments, you provided ", scalar @_;
	}
	($generatekeys_path, $sign_msg_path) = @_;

	unless (-x $generatekeys_path) {
		croak "bb_generatekeys program", $generatekeys_path, " is not executable.";
	}
	unless (-x $sign_msg_path) {
		croak "bb_sign_msg program, ", $sign_msg_path, " is not executable.";
	}
	return bless {}, $class;
}

##
# Gets output from bb_generatekeys program.
#
# Returns:
#	private_key - Private key
#   public_key  - Public key (server license)
sub bb_generatekeys {
	my $self = shift;

	# run genkeys
	open my $genkeys_h, "$generatekeys_path |"
		or croak "Unable to execute $generatekeys_path";
	my @data = <$genkeys_h>;
	close $genkeys_h 
		or croak "generatekeys exited with error", @data;
	
	# read keys
	my $private_key;
	{
		$data[0] =~ /(\w+)-(\w+)-(\w+)-(\w+)/;
		$private_key = "$1-$2-$3-$4";
	}

	my $public_key;
	{
		$data[1] =~ /(\w+)-(\w+)-(\w+)/;
		$public_key = "$1-$2-$3";
	}

	return ($private_key, $public_key);
}

##
# Sign hardware hash to key.
#
# Parameters:
#	private_key - Private key for a license
#	hardware_hash - Hardware hash for a black box
#
# Returns:
#	signature - Signature
sub bb_sign_msg {
	my ($self, $private_key, $hardware_hash) = @_;

	croak "private_key ($private_key) missing / not valid"
		unless defined $private_key and $private_key =~ /^\w+-\w+-\w+-\w+$/;

	croak "hardware hash missing"
		unless defined $hardware_hash;

	my $exec = "$sign_msg_path $private_key $hardware_hash";	
	#print $exec;
	open my $sign_msg_h, "$exec |"
		or croak "Unable to execute $sign_msg_path";

	my $output = <$sign_msg_h>;
	close $sign_msg_h
		or croak "bb_sign_msg exited with error", $output;

	# example output: RNRT-DA97-8X6M-K7BB-X97J-DEZG-JTXY-9D8F (signature)
	my $signature;
	{ 
		$output =~ /^(\S+)/ ;
		$signature = $1;
	}
	return $signature;
}

1;
