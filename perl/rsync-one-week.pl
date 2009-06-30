#!/usr/bin/env perl
use strict;
use warnings;
use DateTime;
use String::CRC32 qw(crc32);
use Fcntl qw(:flock);

#my $SRC_PATH = q{dagurval@bbh-001.boitho.com:/home/dagurval/backuptest};
#my $BACKUP_DIR = q{/tmp/backup};

my @RSYNC_ARGS = (
	"-v", # verbose output
	"-z", # compress during transfer
	"-a", # archive
	"-e ssh", 
	"--delete", # delete files deleted on src
);

unless (@ARGV == 2) {
	print STDERR "Usage: $0 src dst\n";
	print STDERR "Example: $0 test\@example.com:/home/user/data /tmp/backup\n";
	exit 1;
}

my $SRC_PATH = shift;
my $BACKUP_DIR = shift;

die "Backup directory '$BACKUP_DIR does not exist."
	unless -d $BACKUP_DIR;

my $unlocker = lock_backup();
backup_today();
&$unlocker();

sub backup_today {
	my ($new_dir, $old_dir) = backup_folders();

	mkdir $_ for ($BACKUP_DIR, $new_dir, $old_dir);
	
	-d $_ || die "directory '$_' does not exist"
		for ($old_dir, $new_dir);

	my $exec = "rsync " . join(" ", @RSYNC_ARGS) . " --link-dest=$old_dir $SRC_PATH $new_dir";

	print "running $exec\n";
	open my $rsynch, "$exec |"
		or die "Unable to run '$exec': ", $!;
	$|=1;
	print $_ while <$rsynch>;
}

sub backup_folders {
	my $dt = DateTime->now;

	my $today_nr = $dt->day_of_week;
	my $today = $dt->day_name;
	
	$dt->subtract(days => 1);
	my $yesterday = $dt->day_name;
	my $yesterday_nr = $dt->day_of_week;

	return (
		$BACKUP_DIR . "/$today_nr-$today/", 
		$BACKUP_DIR . "/$yesterday_nr-$yesterday/"
	);
}


sub lock_backup {
	my $path = "/tmp/backup" . crc32($SRC_PATH) . ".lock";

	open my $lock_fh, ">", $path
		or die "unable to open lock '$path' ", $!;
	
	flock $lock_fh, LOCK_EX | LOCK_NB
		or die "Unable to lock '$path'. Program already running?";
	return sub { 
		flock $lock_fh, LOCK_UN; 
		unlink $path 
	}
}

1;
