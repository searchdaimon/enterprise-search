#!/usr/bin/env perl
use strict;
use warnings;
use DateTime;
use String::CRC32 qw(crc32);
use Fcntl qw(:flock);
use Carp;
use Data::Dumper;

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
use constant LOG_FILE => "backuplog.log";
use constant LOG_INFO => 1;
use constant LOG_WARN => 2;
use constant DEBUG_SKIP_DAYS => 3;


my $SRC_PATH = shift;
my $BACKUP_DIR = shift;


die "Backup directory '$BACKUP_DIR does not exist."
	unless -d $BACKUP_DIR;

my $log = create_logger();
my $unlocker = lock_backup();
backup_today();
&$unlocker();

sub backup_today {
	my ($new_dir, $old_dir) = backup_folders();

	mkdir $_ for ($BACKUP_DIR, $new_dir);
	
	die "directory '$new_dir' does not exist"
		unless -d $new_dir;
	utime undef, undef, $new_dir; # reset modified time.

	my $exec = "rsync " . join(" ", @RSYNC_ARGS);
 	if ($old_dir) { 
		# hard link against earlier backup
		$exec .= " --link-dest=$old_dir";
	}
	$exec .= " $SRC_PATH $new_dir";

	&$log(LOG_INFO, "Executing '$exec'");
	open my $rsynch, "$exec |"
		or die "Unable to run '$exec': ", $!;
	$|=1;
	&$log(LOG_INFO, $_) while <$rsynch>;
}

sub _folder_path {
	my $dt = shift;
	my $TPL = "%s/%s-%s/";
	return sprintf $TPL, $BACKUP_DIR, $dt->day_of_week, $dt->day_name;
}

sub _newest_backup {
	my $dt = DateTime->now;
	$dt->add(days => DEBUG_SKIP_DAYS)
		if DEBUG_SKIP_DAYS;
	
	my @backups;
	for (1..6) {
		$dt->subtract(days => 1);
		my $path = _folder_path($dt);
		unless (-d $path) {
			&$log(LOG_WARN, "Earlier backup '$path' does not exist.");
			next;
		}
		
		my $mtime = (stat $path)[9];
		&$log(LOG_INFO, "Found backup '$path', modified '$mtime'.");
		push @backups, [ $path, $mtime ];
	}
	@backups = sort { $b->[1] <=> $a->[1] } @backups;
	return unless @backups;
	return $backups[0]->[0];
}


sub backup_folders {
	my $dt = DateTime->now;
	$dt->add(days => DEBUG_SKIP_DAYS)
		if DEBUG_SKIP_DAYS;
	
	my $todays_dir = _folder_path($dt);
	my $earlier_backup = _newest_backup();

	&$log(LOG_WARN, "Found no earlier backup.")
		unless $earlier_backup;

	return ($todays_dir, $earlier_backup);
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


sub create_logger {
	open my $logh, ">>", LOG_FILE
		or die "Unable to open logfile " . LOG_FILE, $!;

	return sub {
		my ($level, @msg) = @_;
		$msg[@msg - 1] =~ s/\n$//;
		if ($level == LOG_INFO) {
			print "LOG INFO: ", @msg, "\n";
			print $logh "LOG INFO: ", @msg, "\n";
		}
		elsif ($level == LOG_WARN) {
			print STDERR "LOG WARN: ", @msg, "\n";
			print $logh "LOG WARN: ", @msg, "\n";
		}
		else { croak "Unknown log level '$level'" }
		1;
	}
}

1;
