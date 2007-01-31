use Boitho::Lot;

if ($#ARGV != 1) {
        print qq{
	Ingen lot git. 
        ./lotLocalRm.pl 2
};
        exit;
}

my $lotpath = Boitho::Lot::GetFilPathForLot($ARGV[0],$ARGV[1]);

print "removing $lotpath\n";

#kjører recrosiv rm
system("rm -r $lotpath");
