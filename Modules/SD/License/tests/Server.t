use strict;
use warnings;

BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . "/Modules";
}

use Test::More tests => 4;
use SD::License::Server;

my %keys;
my $ls;
my $hardware_hash;

# init
$keys{'private'} = "AIFSBA-UDAZI5-Z9EYS5-H5N";
$keys{'public'}  = "AI7QW8-UXM5DR-UFVUP6";
$hardware_hash   = "79F689-V43TAZ-WYYZ5T-V468A7-7N";

$ls = SD::License::Server->new(
	$ENV{'BOITHOHOME'} . "/bin/bb_generatekeys",
	$ENV{'BOITHOHOME'} . "/bin/bb_sign_msg",
);



## tests

#sub test_bb_sign_msg
{
	my $signature = $ls->bb_sign_msg($keys{'private'}, $hardware_hash);
	like($signature, qr{^((\w{4})-){7}(\w{4})$},
		"Signing with bb_sign_msg");

	eval {
		$ls->bb_sign_msg(undef, undef);
	};
	ok ($@, "bb_sign_msg throws exception");
	
}

#sub test_bb_generatekeys
{
	my ($private, $public) = $ls->bb_generatekeys();
	like( $private,
		qr{^\w+-\w+-\w+-\w+$}, 
		"bb_generatekeys private key syntax" );

	like( $public,
		qr{^\w+-\w+-\w+$}, 
		"bb_generatekeys public key syntax" );
}

