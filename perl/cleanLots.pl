
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

				$command = qq{bin/IndexerLotbb $lot $subname};
				print "runing $command\n";
				system($command);

				$command = qq{bin/LotInvertetIndexMaker2 Main $lot $subname};
				print "runing $command\n";
                                system($command);

				$command = qq{bin/mergeUserToSubname $lot $subname};
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

	$command = qq{bin/mergeIIndex 1 $hiestinlot{$key} Main aa $key};

	print "runing $command\n";
	system($command);
}

