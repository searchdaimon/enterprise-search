BEGIN {
	unshift @INC, $ENV{BOITHOHOME}."/Modules/";
};

use strict;
use Carp;
use Getopt::Std;
use Boitho::Lot;
use File::Path qw(remove_tree);

my %opts;
getopts('lir', \%opts);


if ($opts{l}) {
	print "Will log all output to file\n";

	open(STDOUT, ">>$ENV{'BOITHOHOME'}/logs/indexing") || die "Can't dup stdout";
	open(STDERR, ">>&STDOUT") || die "Can't dup stdout";
}

my $subname = shift @ARGV or die("Sorry, but you have to spesify subname as arg 1");


my %hiestinlot = ();

my $lockfile = $ENV{'BOITHOHOME'} . '/var/boitho-cleanLots' . $subname . '.lock';
open(LOCKF,">$lockfile") or die("can't lock lock file $lockfile: $!");
flock(LOCKF,2);

if ($opts{'r'}) {
	print "Deleting:\n";
	foreach my $lot (1 .. 4096) {

		my $path = Boitho::Lot::GetFilPathForLot($lot,$subname);
		chop $path;

		my $dirtyfile = $path . '/dirty';

		print "Reseting lot $lot, subname $subname\n";


		if (!(-e $path)) {
			last;
		}

		# Remove the iindex and revindex folders
		remove_tree( $path . '/iindex', $path . '/revindex', {error => \my $err} );
	  	if (@$err) {
	  	    for my $diag (@$err) {
	  	        my ($file, $message) = %$diag;
	  	        if ($file eq '') {
	  	            print "general error: $message\n";
	  	        }
	  	        else {
	  	            print "problem unlinking $file: $message\n";
	  	        }
	  	    }
	  	}

		# Delete all but: dirty  DocID  reposetory  urls.db reposetory.attribute_columns and *new_attribute_keys
	      	my $DIR;
	      	opendir($DIR, $path) or warn("can't opendir $path: $!") && return;

	        while (my $file = readdir($DIR) ) {

	                #skiping . and ..
	                if ($file =~ /\.$/) {
	                      next;
	                }

			if ($file eq 'dirty' || $file eq 'DocID' || $file eq 'reposetory' || $file eq 'urls.db' || $file eq 'reposetory.attribute_columns') {
				next;
			}

	                if ($file =~ /new_attribute_keys/) {
	                      next;
	                }

	                my $candidate = $path . "\/" . $file;

			#print "rm $candidate\n";
	                unlink($candidate) or warn("Cant delete cache file \"$candidate\": $!");

	      	}

	      	closedir($DIR);

		# Creat dirty file if it dos not exist.
		if (!(-e $dirtyfile)) {
			open(OUT, ">",$dirtyfile) or die("Can't open $dirtyfile: ");
			print OUT "1";
			close(OUT);
		}

	}
}

if ($opts{'i'}) {
	print "indexing:\n";

	foreach my $lot (1 .. 4096) {

		my $path = Boitho::Lot::GetFilPathForLot($lot,$subname);
		chop $path;

		my $dirtyfile = $path . '/dirty';

		print "Indexing lot $lot, subname $subname\n";

		if (!(-e $path)) {
			last;
		}


		if (-e $dirtyfile) {


			
			my $command = $ENV{'BOITHOHOME'} . "/bin/IndexerLotbb -i $lot \"$subname\"";
			print "runing $command\n";
			system($command);
			exitstatus($?);

			my $command = $ENV{'BOITHOHOME'} . "/bin/mergeUserToSubname $lot \"$subname\"";
			print "runing $command\n";
			system($command);
			exitstatus($?);



			unlink($dirtyfile) or warn($!);
			$hiestinlot{$subname} = $lot;
		}

	}

	print "\nmergeIIndex:\n";

	foreach my $key (keys %hiestinlot) {
		print "key $key, value \"$hiestinlot{$key}\"\n";

		my $command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} Main aa \"$key\"";
		print "runing $command\n";
		system($command);
		exitstatus($?);

		my $command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} acl_allow aa \"$key\"";
		print "runing $command\n";
		system($command);
		exitstatus($?);

		my $command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} acl_denied aa \"$key\"";
		print "runing $command\n";
		system($command);
		exitstatus($?);

		my $command = $ENV{'BOITHOHOME'} . "/bin/mergeIIndex 1 $hiestinlot{$key} attributes aa \"$key\"";
		print "runing $command\n";
		system($command);
		exitstatus($?);

			my $command = $ENV{'BOITHOHOME'} . "/bin/sortCrc32attrMap \"$key\"";
			print "runing $command\n";
			system($command);
		exitstatus($?);

		# Running garbage collection.
		#	my $command = $ENV{'BOITHOHOME'} . "/bin/gcRepobb \"$key\"";
		#	print "runing $command\n";
		#	system($command);

	}
}
# Clean search cache
print "Cleaning search cache.\n";
recdir($ENV{'BOITHOHOME'} . "/var/cache");


# Release the collection lock
close(LOCKF) or warn($!);
unlink($lockfile) or warn($!);

print "~cleanLots.pl\n";

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

sub exitstatus {
	my $n = shift;

	if ($n == -1) {
		carp "Warn: failed to execute: $!";
	}
	elsif ($n & 127) {
		carp sprintf("Warn: child died with signal %d, %s coredump", ($n & 127),  ($n & 128) ? 'with' : 'without');
	}
	elsif (($n >> 8) == 0) {
		# ok
	}
	else {
		carp sprintf("Warn: child exited with value %d", $n >> 8);
	}
	
}
