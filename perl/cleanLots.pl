BEGIN {
	push @INC, $ENV{BOITHOHOME}."/Modules/";
};

use Getopt::Std;
use Boitho::Lot;

getopts('ls:', \%opts);


if ($opts{l}) {
	print "Will log all output to file\n";

	#dublicate stdout to log what is happening
# Runarb: 07.02.2012: This log is not being rotated. Commenting it out for now.
#	open(STDOUT, ">>$ENV{'BOITHOHOME'}/logs/indexing") || die "Can't dup stdout";
#	open(STDERR, ">>&STDOUT") || die "Can't dup stdout";

}

if (!defined($opts{'s'})) {
	die("Sorry, but you have to spesify subname (-s)");
}
my $subname = $opts{'s'};


my %hiestinlot = ();

my $lockfile = $ENV{'BOITHOHOME'} . '/var/boitho-cleanLots' . $subname . '.lock';
open(LOCKF,">$lockfile") or die("can't lock lock file $lockfile: $!");
flock(LOCKF,2);

print "indexing:\n";

foreach my $lot (1 .. 4096) {

	my $Path = Boitho::Lot::GetFilPathForLot($lot,$subname);
	chop $Path;
	print "$Path\n";

	if (!(-e $Path)) {
		last;
	}
			if (-e $dirtyfile) {


				print "name $dirtyfile\n";
				print "lot $lot, subname $subname\n";

				$command = $ENV{'BOITHOHOME'} . "/bin/IndexerLotbb -i $lot \"$subname\"";
				print "runing $command\n";
				system($command);

				$command = $ENV{'BOITHOHOME'} . "/bin/mergeUserToSubname $lot \"$subname\"";
				print "runing $command\n";
                                system($command);




				unlink($dirtyfile) or warn($!);


				$hiestinlot{$subname} = $lot;
			}


	
}

print "\nmergeIIndex:\n";

foreach my $key (keys %hiestinlot) {
	print "key $key, value \"$hiestinlot{$key}\"\n";

	$command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} Main aa \"$key\"";
	print "runing $command\n";
	system($command);

	$command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} acl_allow aa \"$key\"";
	print "runing $command\n";
	system($command);

	$command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} acl_denied aa \"$key\"";
	print "runing $command\n";
	system($command);

	$command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} attributes aa \"$key\"";
	print "runing $command\n";
	system($command);

        $command = $ENV{'BOITHOHOME'} . "/bin/sortCrc32attrMap \"$key\"";
        print "runing $command\n";
        system($command);

	#kjører garbage collection.
#	$command = $ENV{'BOITHOHOME'} . "/bin/gcRepobb \"$key\"";
#	print "runing $command\n";
#	system($command);


}

# Clean search cache
print "Clining search cache.\n";
recdir($ENV{'BOITHOHOME'} . "/var/cache");


# unlocking
close($lockfile) or warn($!);
unlink($lockfile) or warn($!);


###########################################################################################################
#
# Subroutine that recurs thru and delete all search cache
#
###########################################################################################################
sub recdir {
    my ($path) = @_;


    print "recdir($path)\n";

    if (-e $path) {

      my $DIR;
      opendir($DIR, $path) or warn("can't opendir $path: $!") && return;

        while (my $file = readdir($DIR) ) {

                #skiping . and ..
                if ($file =~ /\.$/) {
                      next;
                }
                my $candidate = $path . "\/" . $file;

	        if (-d $candidate) {
	                recdir($candidate);
	        }
		else {
                	unlink($candidate) or warn("Cant delete cache file \"$candidate\": $!");
		}
      }

      closedir($DIR);


      rmdir($path) or warn("Cant delete dir \"$path\": $!");
    }


    return 1;
}

