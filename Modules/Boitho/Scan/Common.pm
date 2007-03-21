package Boitho::Scan::Common;
use strict;
use warnings;

## Method to scan for candidates with nmap.
## Accepts whatever ip/range/cidr-syntax nmap accepts.
sub nmap_scan($$) {
	my ($self, $network, $port) = (@_);
	#"nmap", "-oG", "-", "-p", "", "-n", "\Q$network"
	my @data = `nmap -oG - -p \Q$port\E -n \Q$network\E` 
		or carp "Execution of nmap failed.";
	my @hosts = ();	
	foreach (@data) {
		$self->log($_, 3);
		# Example input: Host: 129.241.139.45 ()     Ports: 137/closed/tcp//netbios-ns///
		my $ip = $1 if (/Host:\s((.*)+)\s\(/);
		push(@hosts, $ip) if $ip;
	}

	return \@hosts;
}

sub log($$$) {
	my $self = shift;
	print "$shift <br />";
}