package Boitho::Scan::Samba;
use strict;
use warnings;
use Carp;
use Net::Domain qw(hostname);
use Socket;

#use NetAddr::IP; 

# NOTE:
# Jeg kommenterte ut NetAddr slik at vi slapp aa installere modulen. Dermed maa vi bruke nmap.
#

# Written to work with nmblookup version 3.0 and nmap 4.10

## Example usage:
## $smb = Boitho::Samba::Scan->new();
## $smb->scan();
## foreach my $ip (@$smb->get_hosts()) {
##	print "name: " . $smb->get_name($ip);
##	print " workspace: " . $smb->get_workspace($ip) ."\n";
## }
 

my $USE_NMAP = 1; # nmap is used to sweep the network for potential shares.
		# If the host is blocking ping (ICMP?), it won't be found.
		# If you disable this, every single host will be checked when searching with mask,
		# and searching by range will be disabled.

#my $sql = Sql::Sql->new();
#my $dbh = $sql->get_connection();
#my $scanlog = Sql::Scanlog->new($dbh);

sub new {
	my $class = shift;
	my $self = { };
	bless $self, $class;
	return $self;
}

## 
## Accepts all of the follwing:
## scan(ip), scan(range), scan(ip/cidr). scan()
##
## Example:
## $smb->scan("10.0.0.2");
## $smb->scan("10.0.1-254.12-13");
## $smb->scan("10.0.0.1/24");
## $smb->scan();
##
## If no attribute is sent to the function, it will scan the local subnet (broadcast).
## For subnets that are not local, nmap will be used to find candidates.

sub scan() {
	my $self = shift;
	my $network = shift;
	if (!$network) { 
		return $self->_scan_by_mask();
	}
	elsif ($self->_is_ipv4($network)) {
		return $self->_scan_by_ip($network);
	}
	elsif ($network =~ /((.*)+)\/(\d+)/) { #cidr
		return ($self->_scan_by_mask($1, $2));
	}
	elsif ($network =~ /\d-\d/) { #range
		return ($self->_scan_by_range($network));
	}

	croak "$network is an invalid input format.";
	return;
}

sub get_hosts {
	my $self = shift;
	return keys(%$self);
}

sub get_name($$) {
	my $self = shift;
	my $ip = shift;

	my $details = $self->{$ip};
	return $details->{'name'};
}

sub get_workgroup($$) {
	my $self = shift;
	my $ip = shift;

	my $details = $self->{$ip};
	return $details->{'workgroup'};
}


sub _scan_by_ip {
	my $self = shift;
	my $ip = shift;

	$self->_log ("Checking $ip", 2);

	if (my $details = $self->_finger($ip)) {
		$self->_log ("Found share on $ip", 1);
		#$self->_hack_print_row($ip); #TODO hack;
		$self->{$ip} = $details;
		return 1;
	}
	else {
		$self->_log ("Didn't find share on $ip", 2);
		return 0;
	}
}

# Returns true if at least one host is found to network share.
sub _scan_by_mask {
	my ($self, $ip, $mask) = (@_);
	$mask = 24 unless ($mask);
	my $local_ip = $self->_get_local_ip();
	my $hosts = ();

	if ($self->_is_local_network($ip, $mask, $local_ip)) {
		$hosts = $self->_broadcast_local_network();
	}
	else {
		$ip = $local_ip unless ($ip);
		if ($USE_NMAP) {
			$hosts = $self->_nmap_scan("$ip/$mask");
		}
		else {
			#my $net = NetAddr::IP->new($ip, $mask);
			#$hosts = $net->hostenumref();
		}
	}
	

	my $host_found = 0;
	foreach my $addr (@$hosts) {
		$addr = $1 if ($addr =~ /^((.*)+)\/32/); # Removes cidr added by hostenumref
		$host_found = 1 if ($self->_scan_by_ip($addr));
	}

	return $host_found;
}

sub _scan_by_range {
	my ($self, $range) = (@_);
	unless($range) {
		carp "No range specified";
		return;
	}
	unless ($USE_NMAP) {
		carp "Can't scan range unless nmap used. 
			Set \$USE_NMAP to true, or use search_by_mask instead.";
	}
	my $hosts = $self->_nmap_scan($range);
	
	my $host_found = 0;
	foreach my $addr (@$hosts) {
		$host_found = 1 if ($self->_scan_by_ip($addr));
	}
	return $host_found;
}


sub _get_local_ip() {
	my $self = shift;
	my $host = hostname();
	my $ip = inet_ntoa(
		scalar gethostbyname( $host || 'localhost' ));

	return $ip;
}

sub _is_local_network {
	my ($self, $ip, $mask, $local_ip) = (@_);
	return (($mask == 24) and (!$ip or ($ip eq $local_ip)));
}

sub _broadcast_local_network {
	
	my $self = shift;
	my $hosts = $self->_nmblookup_scan();
	return $hosts;

}

## Method to scan for candidates with nmap.
## Accepts whatever ip/range/cidr-syntax nmap accepts.
sub _nmap_scan($$) {
	my ($self, $network) = (@_);
	#"nmap", "-oG", "-", "-p", "137", "-n", "\Q$network"
	my @data = `nmap -oG - -p 137 -n \Q$network` 
		or carp "Execution of nmap failed.";
	my @hosts = ();	
	foreach (@data) {
		$self->_log($_, 3);
		# Example input: Host: 129.241.139.45 ()     Ports: 137/closed/tcp//netbios-ns///
		my $ip = $1 if (/Host:\s((.*)+)\s\(/);
		push(@hosts, $ip) if $ip;
	}

	return \@hosts;
}

## Method to scan for candidates with nmblookup.
## Uses broadcast, so it's fast.
sub _nmblookup_scan($$$) {
	my ($self, $net) = (@_);
	my $subnet = '';

	if ($net) {
		unless ($self->_is_ipv4($net)) {
			croak "$net is not a valid ipv4.";
		}
		$subnet = "-B $net" if ($net); 
	}	
	my @data = `nmblookup -d 0 '*' \Q$subnet`
		or carp "Execution of nmblookup failed.";
	my @hosts = ();
	foreach(@data) {
		$self->_log($_, 3);
		my @line = split(' ', $_);
		push (@hosts, $line[0]) if (/^\d/);
	}

	return \@hosts if @hosts;
	return;
}


## Get info from host. Method is used to get name and workgroup
## from host. If there is no respons, it is asumed that there is 
## no samba share on it.
sub _finger ($$) {
	my $self = shift;
	my $ip = shift;

	unless ($self->_is_ipv4($ip)) {
		croak "$ip is not a valid ipv4.";
	}	

	my @data = `nmblookup -d 0 -A \Q$ip` or
		carp "Unable to execute nmblookup.";
	my $netbios = '';
	my $workgroup = '';

	foreach my $line (@data) {
		next unless ($line =~ /<00>/);
		last if ($netbios and $workgroup);

		if ($line =~ /GROUP/) {
			$workgroup = $line;
		}
		else { $netbios = $line; }
	}

	# Asuming there can't be space in name or workgroup.
	$netbios = $1 if ($netbios =~ /^\s(\S+)/);
	$workgroup = $1 if ($workgroup =~ /^\s(\S+)/);

	return unless ($netbios or $workgroup);
	
	my %details = (
		'name' => $netbios,
		'workgroup' => $workgroup,
	);
	
	return \%details;

}


sub _log($$$) {
	my ($self, $message, $level) = (@_);
	chomp($message);
	#$scanlog->insert_log($message, $level);
	print "$message <br />";
	return 1;
}

sub _is_ipv4 {
	my $self = shift;
	my $ip = shift;
	my (@part) = $ip =~ /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
	return if (@part < 4);
	foreach(($1, $2, $3, $4)) {
		return if ($_ < 0 or $_ > 255);
	}
	return 1;
}

1;