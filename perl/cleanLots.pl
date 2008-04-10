
BEGIN {
	push @INC, $ENV{BOITHOHOME}."/Modules/";
};


use Getopt::Std;

getopts('l', \%opts);


if ($opts{l}) {
	print "Will log all output to file\n";

	#dublicate stdout to log what is happening
	open(STDOUT, ">>$ENV{'BOITHOHOME'}/logs/indexing") || die "Can't dup stdout";
	open(STDERR, ">>&STDOUT") || die "Can't dup stdout";

}

use Boitho::Lot;

my %hiestinlot = ();

print "indexing:\n";

foreach my $lot (0 .. 4096) {

	my $Path = Boitho::Lot::GetFilPathForLot($lot,"");
	chop $Path;

	if (-e $Path) {

		opendir(DIR, $Path) or die("can't opendir $some_dir: $!");

		while (my $subname = readdir(DIR) ) {

			my $dirtyfile = $Path . $subname . '/dirty';

			if (-e $dirtyfile) {
				print "name $dirtyfile\n";
				print "lot $lot, subname $subname\n";

				$command = $ENV{'BOITHOHOME'} . "/bin/IndexerLotbb -i -g $lot \"$subname\"";
				print "runing $command\n";
				system($command);

				#$command = $ENV{'BOITHOHOME'} . "/bin/LotInvertetIndexMaker3bb Main $lot \"$subname\"";
				#print "runing $command\n";
                                #system($command);

				$command = $ENV{'BOITHOHOME'} . "/bin/LotInvertetIndexMaker2 acl_allow $lot \"$subname\"";
				print "runing $command\n";
                                system($command);

				$command = $ENV{'BOITHOHOME'} . "/bin/LotInvertetIndexMaker2 acl_denied $lot \"$subname\"";
				print "runing $command\n";
                                system($command);


				$command = $ENV{'BOITHOHOME'} . "/bin/mergeUserToSubname $lot \"$subname\"";
				print "runing $command\n";
                                system($command);




				unlink($dirtyfile) or warn($!);


				$hiestinlot{$subname} = $lot;
			}
		}

		closedir(DIR);

	}
}

print "\nmergeIIndex:\n";

foreach my $key (keys %hiestinlot) {
	print "key $key, value $hiestinlot{$key}\n";

	$command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} Main aa \"$key\"";
	print "runing $command\n";
	system($command);

	$command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} acl_allow aa \"$key\"";
	print "runing $command\n";
	system($command);

	$command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} acl_denied aa \"$key\"";
	print "runing $command\n";
	system($command);
}

