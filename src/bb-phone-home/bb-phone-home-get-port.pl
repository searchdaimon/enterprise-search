#!/usr/local/bin/perl

use strict;
use warnings;

use IO::Socket;
use Sys::Syslog;

use Fcntl ':flock';

use POSIX qw(strftime);

my $sshportstart = 10000;
my $sshportend = 10500;

my $logfile = "/tmp/phone-home-port-log";
my $delegatedports = "/tmp/delegated-ports";


#
# XXX: Race condition, make sure we don't delegate the same port within a given
# interval.
#

# Does not scale, rewrite in C?

sub get_port() {
	my $num = $sshportend - $sshportstart;
	my $port = int(rand($num)) + $sshportstart;

	my $fh;
	open($fh, "+<", $delegatedports) or return int(rand($num)) + $sshportstart;
	flock($fh, LOCK_EX);
	seek($fh, 0, 0);

	my $oldport = <$fh>;
	#while (<$fh>) { print $_; $oldport = $_ if ($_ =~ /\d+/); }

	if (defined($oldport) && $oldport =~ /(\d+)/) {
		$oldport = $1;
		$port = (($oldport - $sshportend + 1) % $num) + $sshportstart;
	}
	else {
		$port = $sshportstart;
	}

	seek($fh, 0, 0);
	truncate($fh, 0);
	print $fh "$port\n";

	flock($fh, LOCK_UN);
	close($fh);

	return $port;
}

sub port_in_use($) {
	my $port = shift;

	my $s = IO::Socket::INET->new(PeerAddr => 'localhost',
				      PeerPort => $port,
				      Proto    => 'tcp',
				      Timeout  => 10) or return 0;
	close($s);
	return 1;
}


my $port;
do {
	$port = get_port();
} while  (port_in_use($port));

print $port."\n";

if ($#ARGV != -1) {
	openlog("bb-phone-home-get-port.pl", 'cons,pid', 'user');
	syslog('info', '%s was delegated to port %d', $ARGV[0], time);
	closelog();

	while (1) {
		open(my $fh, '>>', $logfile) or last;
		my $time = strftime "%a %b %e %H:%M:%S %Y", localtime;
		print $fh "$time: " . $ARGV[0] . " was delegated to port '$port'.\n";
		close $fh;
		last;
	}
}


