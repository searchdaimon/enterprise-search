#!/usr/local/bin/perl -w

use strict;
use warnings;

use IO::Socket;
use AppConfig qw(:expand :argcount);
use Data::Dumper;
use LWP;
use File::stat;

# Keep alive daemon files
my $dstatefile = $ENV{BOITHOHOME} . '/var/phonehome.state';
my $dpidfile = $ENV{BOITHOHOME} . '/var/bb-phone-home-keepalive-pid-file';

$ENV{PATH} = "/bin:/usr/bin:/usr/local/bin";

my $config = AppConfig->new({
	CASE   => 1,
	GLOBAL => {
		DEFAULT  => "<unset>",
		ARGCOUNT => ARGCOUNT_ONE,
	},
});

$config->define('clientname');
$config->define('localsshhost');
$config->define('localsshport');
$config->define('sshhost');
$config->define('sshuser');
$config->define('sshlog');
$config->define('sshpidfile');
$config->define('forwardport_http');
$config->define('sshidentityfile');
$config->define('clientname');

$config->file($ENV{BOITHOHOME}."/config/bb-phone-home-client.conf");

my $name = $config->get('clientname');

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
		my $logfile = $config->get('sshlog');
		$logfile =~ /(.*)/;
		$logfile = $1;

		chdir '/';                 #or die "Can't chdir to /: $!";
		open STDIN, '/dev/null'; #  or die "Can't read /dev/null: $!";
		open STDOUT, '>/dev/null'; # or die "Can't write to /dev/null: $!";
		open STDERR, '>/dev/null'; # or die "Can't write to /dev/null: $!";
#		open STDIN, '/dev/null'; #  or die "Can't read /dev/null: $!";
#		open STDOUT, '>'.$logfile."-stdout"; # or die "Can't write to /dev/null: $!";
#		open STDERR, '>'.$logfile."-stderr"; # or die "Can't write to /dev/null: $!";
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
	my $_pidfile = $config->get('sshpidfile');
	$_pidfile =~ /(.*)/;
	my $pidfile = $1;

	open(PIDFILE, "> $pidfile") or return undef;#$!;# die "Unable to write ssh pidfile: $!";
	print PIDFILE "$pid\n";
	close(PIDFILE);
	
	return 1;
}

sub read_ssh_pid_file() {
	my $pid = undef;
	my $pidfile = $config->get('sshpidfile');

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

	if (stat($config->get('sshpidfile'))) {
		stop_forwarding();
	}

	my $cmd = "/usr/bin/ssh";
	stat($cmd) || die "Could not find the ssh program";
	my @args = ("-l", $config->get('sshuser'),
	            "-i", $config->get('sshidentityfile'),
	            "-R", "$fwdport:".$config->get('localsshhost').":".$config->get('localsshport'),
	            $config->get('sshhost'), "-N", "-4"); # Force ipv4 for now

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
	$config->get('sshpidfile') =~ /(.*)/;
	my $pidfile = $1;
	unlink( $1 );
}


sub get_forward_port_http() {
	my $url = $config->get('forwardport_http');
	my $lwp = LWP::UserAgent->new;
	my $hostname = `hostname`;
	chop($hostname);
	$hostname.= "/". $name; # Add magic identify string
	my $response = $lwp->get($url."?hostname=$hostname");
	print Dumper($config->get('forwardport_http'));
	unless ($response->is_success) {
		print STDERR "Error: lwp::useragent: $!";
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

sub writedstate {
	my ($newstate) = @_;

	open (my $fh, "> $dstatefile");
	print $fh "$newstate\n";
	close($fh);
}

sub getdpid {
	open (my $fh, "< $dpidfile");
	my $pid = <$fh>;
	close($fh);
	chomp($pid);

	return $pid;
}


if ($#ARGV == -1) {
	print STDERR "Possible arguments: start, stop, running\n";
	exit 2;
}

my $arg = $ARGV[0];


if ($arg eq "start") {
	my $fwdport = get_forward_port_http();

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
elsif ($arg eq 'start2') {
	writedstate("alive");
	kill("USR1", getdpid());
}
elsif ($arg eq 'stop2') {
	writedstate("dead");
	kill("USR1", getdpid());
}
else{
	print STDERR "Unknown command.\n";
	exit 2;
}

