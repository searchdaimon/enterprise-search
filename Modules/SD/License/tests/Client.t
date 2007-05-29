use strict;
use warnings;

BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . "/Modules";
}

use Test::More tests => 4;
use SD::License::Client;

# init
my %keys;
#$keys{'private'} = "AIFSBA-UDAZI5-Z9EYS5-H5N";
$keys{'public'}  = "AI7QW8-UXM5DR-UFVUP6";
my $hardware_hash   = "79F689-V43TAZ-WYYZ5T-V468A7-7N";
my $signature = "RRIK-PICI-T4PH-7MQJ-GK2K-ZYEY-PNX3-7CDW";

my $lc = SD::License::Client->new(
	$ENV{'BOITHOHOME'} . "/bin/bb_sha_test",
	$ENV{'BOITHOHOME'} . "/bin/bb_verify_msg"
);


## tests

#sub bb_sha_test
{
	my $hash = $lc->bb_sha_test();
	
	like($hash, qr{^(\w{6}-){4}\w+},
		"Generate hardware hash (bb_sha_test)");
}

#sub verify_msg
{
	my ($verified, $msg) = $lc->bb_verify_msg($keys{'public'}, $hardware_hash, $signature);
	is ($verified, 1, "License valid test");
	($verified, $msg) = $lc->bb_verify_msg("ASDFG-ASDFG-SADFG", $hardware_hash, $signature);
	is ($verified, 0, "Not valid test");
	
	($verified, $msg) = $lc->bb_verify_msg(undef, undef, undef);
	is ($verified, 0, "Not valid test2");
}

