#!/usr/local/bin/perl -w

use strict;

use lib "$ENV{BOITHO_PERL_LIB}/lib/perl";

use IO::Socket;
use Term::ReadLine;
use Data::Dumper;

use Config::General;

my %config = ParseConfig("bb-phone-home-srv.conf");

my $file = $config{'unixsocketlocation'};
my $socket = new IO::Socket::UNIX(Type => SOCK_STREAM,
                                  Peer => $file);

die "Could not open unix socket: $!" unless $socket;



sub get_result() {
	while (<$socket>) {
		my $buf = $_;
		chop $buf;
		last if $buf eq 'done';

		print ">> $buf\n";
	}
}


sub help() {
	print <<EOF

add clientname sshport - Set up a ssh tunnel
dropconn clientname - Shut down a ssh tunnel
getcmd clientname - Get pending commands
exit - Disconnect from server

EOF
}

my $term = new Term::ReadLine 'BB Phone Home - Command central';
my $prompt = "ET commander: ";
my $OUT = $term->OUT || \*STDOUT;
while ( defined ($_ = $term->readline($prompt)) ) {
	my $line = $_;
	if ($line eq 'help') {
		help();
		next;
	}
	print $socket "$line\n";
	last if ($line eq 'exit');
	$term->addhistory($_) if /\S/;
	get_result();
}

close $socket;

print "bye bye!\n";
