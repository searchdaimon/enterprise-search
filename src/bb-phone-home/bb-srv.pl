#!/usr/local/bin/perl -w

use strict;
use warnings;

use lib "$ENV{BOITHO_PERL_LIB}/lib/perl";

use IO::Socket;
use threads;
use threads::shared;
#use Thread;
use Data::Dumper;
use Config::General;

my $commander = undef;

my %config = ParseConfig("bb-phone-home-srv.conf");

sub send_commander($) {
	my $msg = shift;

	print $commander $msg if $commander;
}

my %command_table : shared;
my %known_bbs : shared;


sub add_bbs($) {
	my $name = shift;
	lock %known_bbs;

	$known_bbs{$name} = 1;
}

sub add_command($$$) {
	my ($client, $cmd, $arg) = @_;

	lock %command_table;

	$command_table{$client."-".$cmd} = $arg;
}


sub get_command($$) {
	my $client = shift;
	my $delete = shift;
	my @cmds = ( "addconn", "dropconn" );

	lock %command_table;
	
	my %hash;
	foreach my $cmd (@cmds) {
		my $val = $command_table{$client."-".$cmd};

		if (defined($val)) {
			$hash{$cmd} = $val;
			delete $command_table{$client."-".$cmd} if $delete;
		}
	}

	return %hash;
}


sub process_client($) {
	my $socket = shift;

	my $clientinfo;
	$clientinfo = <$socket>;
	chop($clientinfo);
	if (not $clientinfo =~ /^([^\s]+)/) {
		printf STDERR "Got unknown ident-string from client: $clientinfo\n";
		print $socket "done Who are you?\n";
		close($socket);
		return;
	}
	my $clientname = $1;
	add_bbs($clientname);
	my %clienthash = get_command($clientname, 1);
	for my $key ( keys %clienthash ) {
		my $value = $clienthash{$key};

		if ($key eq 'addconn') {
			print $socket "start-forward srv-port: $value\n";
		} elsif ($key eq 'dropconn') {
			print $socket "stop-forward\n";
		} else {
			print STDERR "Unknown command: $key\n";
		}

	}
	print $socket "done Take care now, you hear?\n";
	close($socket);
}


sub read_commands() {
	my $a = $config{'commandqueue'};
	if ($a eq 'fifo') {
		read_commands_fifo();
	} elsif ($a eq 'unixsocket') {
		read_commands_unixsocket();
	} else {
		print STDERR "Unknown command queue: ".$config{'commandqueue'}."\n";
	}
}

sub read_commands_fifo() {
	my $cmdfd;

	my $pid = fork();
	if ($pid != 0) {
		open($cmdfd, "< " . $config{'fifolocation'})
			|| die "Could not open fifo '".$config{'fifolocation'}."': $!";
	} else {
		open($cmdfd, "> " . $config{'fifolocation'})
			|| die "Could not open fifo '".$config{'fifolocation'}."': $!";
		sleep 2;
		exit 0;
	}

	threads->new(\&read_commands_fifo_loop, $cmdfd);
}

sub read_commands_unixsocket_loop($) {
	my $socket = shift;

	while (my ($conn) = $socket->accept()) {
		threads->new( sub {
			$commander = $conn;
			while (my $buf = <$conn>) {
				chop $buf;
				process_command($buf);
			}
		} );
	}

}

sub read_commands_unixsocket() {
	my $socket;
	my $file = $config{'unixsocketlocation'};

	unlink($file);
	$socket = new IO::Socket::UNIX(Type => SOCK_STREAM,
				       Listen => SOMAXCONN,
				       Local => $file);
	die "Could not open unix socket: $!" unless $socket;

	threads->new(\&read_commands_unixsocket_loop, $socket);
}

sub process_command($) {
	my $buf = shift;

	my @cmds = split(/\s+/, $buf);

	if ($cmds[0] eq 'add') {
		if ($#cmds < 2) {
			send_commander "Too few arguments\ndone\n";
			return;
		}
		#add_wanted_connection($cmds[1], $cmds[2]);
		add_command($cmds[1], 'addconn', $cmds[2]);
		send_commander "Adding ".$cmds[1]."\n";
		send_commander "done\n";
	}
	elsif ($cmds[0] eq 'getcmd') {
		if ($#cmds < 1) {
			send_commander "Too few arguments\ndone\n";
			return;
		}
		my %commands = get_command($cmds[1], 0);
		if ($commander) {
			for my $key ( keys %commands ) {
				my $value = $commands{$key};

				print $commander "$key => $value\n";
			}
		}
		send_commander "done\n";
	}
	elsif ($cmds[0] eq 'dropconn') {
		if ($#cmds < 1) {
			send_commander "Too few arguments\n";
		} else {
			add_command($cmds[1], 'dropconn', 1);
		}
		send_commander "done\n";
	}
	elsif ($cmds[0] eq 'exit') {
		close($commander);
	}
	elsif ($cmds[0] eq 'seenbbs') {
		send_commander("Seen blackboxes:\n");
		lock %known_bbs;
		for my $key (keys %known_bbs) {
			send_commander("\t$key\n");
		}
		send_commander("done\n");
	}
	else {
		send_commander "Error: Unknown command\n";
		send_commander "done\n";
	}

}


sub read_commands_fifo_loop($) {
	my $fd = shift;

	while (1) {
		while (my $buf = <$fd>) {
			chop($buf);

			process_command($buf);
		}
	}
}


print "Starting BB Phone Home...\n";

# Set up admin interface
read_commands();

# Listen for incoming connections
my $sock = new IO::Socket::INET(
	LocalPort => $config{'port'},
	LocalHost => $config{'bindaddr'},
	Proto     => 'tcp',
	Listen    => SOMAXCONN,
	Reuse     => 1);

if (!$sock) {
	die $!
}

my($new_sock, $c_addr, $buf);
while (($new_sock, $c_addr) = $sock->accept()) {
	my ($client_port, $c_ip) = sockaddr_in($c_addr);
	my $client_ipnum = inet_ntoa($c_ip);
	my $client_host = gethostbyaddr($c_ip, AF_INET);
	#print "got a connection from: $client_host", " [$client_ipnum]\n";

	threads->new( \&process_client, $new_sock);
}


1;
