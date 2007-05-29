##
# Wrapper for the client side of the license software.
package SD::License::Client;
use strict;
use warnings;
use Carp;

my $sha_test_path;
my $verify_msg_path;

##
# Default constructor
#
# Parameters:
#	sha_test_path - Path to bb_sha_test
#	verify_msg_path - Path to bb_verify_msg
sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;

	if (scalar @_ != 2) {
		croak __PACKAGE__, " takes 2 parameters.\n";
	}

	($sha_test_path, $verify_msg_path) = @_;

	unless (-e $sha_test_path) {
		croak "bb_sha_test program ($sha_test_path) is not executable";
	}
	unless (-e $verify_msg_path) {
		croak "bb_verify_msg program ($verify_msg_path) is not executable";
	}

	return $self;
}


##
# Generates hardware hash
#
# Returns:
#	hash - Hardware hash.
sub bb_sha_test {
	my $self = shift;

	open my $sha_test_h, "$sha_test_path |"
		or croak "Unable to execute $sha_test_path";
	my $hash = <$sha_test_h>;
	close $sha_test_h
		or croak "bb_sha_test exited with error, $hash";

	chomp $hash;
	return $hash;
}

##
# Verifies if license is valid for current black box.
#
# Returns:
#	valid - license is valid, true/false
#	msg   - error message, if any.
sub bb_verify_msg {
	my ($self, $license, $hardware, $signature) = @_;

	my $verified = 1;

	open my $vmsg_h, "$verify_msg_path \Q$license\E \Q$hardware\E \Q$signature\E |"
		or croak "Unable to execute $verify_msg_path";
	my @output = <$vmsg_h>;
	close $vmsg_h
		or $verified = 0;

	my $msg;
	$msg = $output[1] if defined $output[1];
	return ($verified, $msg);
}

1;
