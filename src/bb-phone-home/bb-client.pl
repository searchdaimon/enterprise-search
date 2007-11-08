#!/usr/local/bin/perl -w

use strict;
use warnings;

use IO::Socket;
use Config::General;
use Data::Dumper;
use LWP;
use File::stat;

$ENV{PATH} = "/bin:/usr/bin:/usr/local/bin";

my %config = ParseConfig("/etc/bb-phone-home-client.conf");

my $name = $config{'clientname'};

# Better error checking
sub forkandexit($@) {
	my ($cmd, @args) = @_;

	my $pid = fork;
	if (!defined($pid)) {
		die "Unable to fork!";
	} elsif ($pid > 0) {
		# Parent
		return $pid;
	} elsif ($pid == 0) {
		my $logfile = $config{'sshlog'};
		$logfile =~ /(.*)/;
		$logfile = $1;

		chdir '/';                 #or die "Can't chdir to /: $!";
		open STDIN, '/dev/null'; #  or die "Can't read /dev/null: $!";
#		open STDOUT, '>/dev/null'; # or die "Can't write to /dev/null: $!";
#		open STDERR, '>/dev/null'; # or die "Can't write to /dev/null: $!";
#		open STDIN, '/dev/null'; #  or die "Can't read /dev/null: $!";
		open STDOUT, '>'.$logfile."-stdout"; # or die "Can't write to /dev/null: $!";
		open STDERR, '>'.$logfile."-stderr"; # or die "Can't write to /dev/null: $!";
#
#		setsid                    or die "Can't start a new session: $!";
#		umask 0;
	
		$cmd =~ /(.*)/;
		my @newargs;
		foreach my $arg (@args) {
			$arg =~ /(.*)/;
			push @newargs, $1;
		}
		exec $cmd, @newargs;
		# Hopfully not reached
		die "Could not exec program: $cmd: $!";
	}
	else {
		die "Unable to fork: $!";
	}

	die "NOT REACHED";
}

sub ping_program($) {
	my $pid = shift;

	my $ret = kill 0, $pid;

	return $ret;
}


sub write_ssh_pid_file($) {
	my $pid = shift;
	$config{'sshpidfile'} =~ /(.*)/;
	my $pidfile = $1;

	open(PIDFILE, "> $pidfile") or return undef;#$!;# die "Unable to write ssh pidfile: $!";
	print PIDFILE "$pid\n";
	close(PIDFILE);
	
	return 1;
}

sub read_ssh_pid_file() {
	my $pid = undef;
	my $pidfile = $config{'sshpidfile'};

	open(PIDFILE, "< $pidfile") or return undef;
	while (<PIDFILE>) {
		next unless  $_ =~ /(\d+)/;
		$pid = $1;
		last;
	}
	close(PIDFILE);
	return $pid;
}


sub start_forwarding($) {
	my $fwdport = shift;

	if (stat($config{'sshpidfile'})) {
		stop_forwarding();
	}

	my $cmd = "/usr/bin/ssh";
	stat($cmd) || die "Could not find the ssh program";
	my @args = ("-l", $config{'sshuser'},
	            "-i", $config{'sshidentityfile'},
	            "-R", "$fwdport:".$config{'localsshhost'}.":".$config{'localsshport'},
	            $config{'sshhost'}, "-N", "-4"); # Force ipv4 for now

	my $pid = forkandexit($cmd, @args);
	write_ssh_pid_file($pid);

	return $pid;
}


sub stop_process($) {
	my $pid = shift;

	kill 9, $pid unless !defined($pid) || $pid == 0;
}


sub stop_forwarding() {
	my $pid = read_ssh_pid_file();

	stop_process $pid;
	$config{'sshpidfile'} =~ /(.*)/;
	my $pidfile = $1;
	unlink( $1 );
}


sub get_forward_port_http() {
	my $url = $config{forwardport_http};
	my $lwp = LWP::UserAgent->new;
	my $hostname = `hostname`;
	chop($hostname);
	$hostname.= "/". $name; # Add magic identify string
	my $response = $lwp->get($url."?hostname=$hostname");
	unless ($response->is_success) {
		print STDERR "Error: $!";
		return undef;
	}

	my $fwdport = $response->content();
	if ($fwdport =~ /^(\d+)/) {
		$fwdport = $1;
	}
	else {
		print STDERR $fwdport."\n";
		$fwdport = undef;
	}

	return $fwdport;
}

sub get_forward_port_random() {
	my $first = 10111;
	my $range = 5000;

	return int(rand($range))+$first;
}


if ($#ARGV == -1) {
	print STDERR "Possible arguments: start, stop, running\n";
	exit 2;
}

my $arg = $ARGV[0];


if ($arg eq "start") {
	my $fwdport = get_forward_port_http();
	#my $fwdport = get_forward_port_random();

	exit 1 if (!defined($fwdport));
	my $pid = start_forwarding($fwdport);

	# Magic number, number of retires before we declare a program dead
	my $i = 15;
	while (1){
		my $alive = ping_program($pid);
		last if ($alive == 1);
		die "Unable to start the port forwarder" unless ($alive == 1 || $i > 0);
		$i -= 1;
		sleep 1;
	}

	print $fwdport."\n";
}
elsif ($arg eq 'stop') {
	stop_forwarding();
}
elsif ($arg eq 'running') {
	my $pid = read_ssh_pid_file();
	exit 1 if (!defined($pid));

	my $ret = 1;
	$ret = ping_program($pid) if (defined($pid));
	
	print "$pid\n" if $ret;
	exit ($ret == 1 ? 0 : 1);
}
else {
	print STDERR "Unknown command.\n";
	exit 2;
}

