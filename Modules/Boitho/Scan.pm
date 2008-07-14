#!/usr/bin/env perl
package Boitho::Scan;
use strict;
use warnings;
BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use XML::Parser;
use XML::SimpleObject;
use XML::Writer;
use IO::String;
use Carp;
use Data::Dumper;

use constant HOOK_SCAN_START  => "scan start";
use constant HOOK_SCAN_DONE   => "scan done";
use constant HOOK_SHARE_FOUND => "share found";

use constant MAX_RANGE_SIZE => 256 * 256;

my @scan_start_hooks;
my @scan_done_hooks;
my @share_found_hooks;

my $genip_path;

use constant DEBUG => 0;

# TODO: Ask infoquery what is supported, rather than hardcode.
my @supported_connectors = qw(SMB);

my $infoquery;

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->_init(@_);
	return $self;
}

sub _init {
	my ($self, $infoquery_path, $l_genip_path) = @_;

	croak "Infoquery path given is not executable. Path: ", $infoquery_path
	    unless -x $infoquery_path;

	croak "Genip is not executable. Path: ", $l_genip_path
		unless -x $l_genip_path;
	
	$genip_path = $l_genip_path;

	$infoquery  = Boitho::Infoquery->new($infoquery_path);

	if (DEBUG) {
		$self->add_hook_scan_start(\&debug);
		$self->add_hook_scan_done(\&debug);
		$self->add_hook_share_found(\&debug);
	}
}

##
# Arguments:
#	connector - Connector name
#
# Returns:
#	is_supported - True/false on supported connector
sub is_supported_connector {
	my ($self, $connector) = @_;
	map { return 1 if $_ eq $connector } @supported_connectors;
	0;
}

##
# Scan range.
sub scan {
	my $self = shift;
	my ($connector, $range, $username, $password, $use_icmp_scan) = (@_);

	croak "scan requires 4 arguments" unless (scalar @_ == 5);

	croak "Connector $connector is not supported"
		unless ($self->is_supported_connector($connector));


	my @hosts;
	if ($use_icmp_scan) {
		@hosts = $self->_nmap_icmp_scan($range);
	}
	else {
		@hosts = $self->_get_hosts_in_range($range);
	}

	if (DEBUG) { debug("hosts: ", Dumper(\@hosts)) };

	my @scan_results;
	foreach my $host (@hosts) {
		my @shares = $self->_scan_host($connector, $host, $username, $password);
		push @scan_results, { 
			'addr'   => $host,
			'shares' => \@shares
		};
		
	}

	my $xml_result = $self->_create_xml(@scan_results);
	return $xml_result;
}

##
# Add function to run when scanning a host.
#
# Function should take arguments: connector, host
#
# Arguments:
#	function_ptr - Pointer to function
sub add_hook_scan_start {
	my ($self, $function_ptr) = @_;
	push @scan_start_hooks, $function_ptr;
}

sub add_hook_share_found {
	my ($self, $function_ptr) = @_;
	push @share_found_hooks, $function_ptr;
}

##
# Add function to run when done scanning a host.
#
# Function should take arguments: connector, host, result, msg
#
# Arguments:
#	function_ptr - Pointer to function
sub add_hook_scan_done {
	my ($self, $function_ptr) = @_;
	push @scan_done_hooks, $function_ptr;
}

##
# Find shares on given host.
#
# Arguments:
#	connector - Connector name
#	host - Host ip/addr
#
# Returns:
#	found_shares - List with found shares.
sub _scan_host {
	my ($self, $connector, $host, $username, $password) = @_;

	croak "given connector not supported"
		unless $self->is_supported_connector($connector);

	for my $nr (1..4) {
		croak "Missing argument $nr"
			unless defined $_[$nr];
	}
	
	$self->_feed_hooks(HOOK_SCAN_START, $connector, $host);

	debug("To infoquery: ", $connector, $host, $username, $password);
		
	my $found_shares_ref = $infoquery->scan(
						$connector, 
						$host, 
						$username,
						$password);

	debug("Found shares: ", Dumper($found_shares_ref));
		
	unless ($found_shares_ref) {
		my $errmsg;
		$errmsg = "infoquery scan returned an error: " . $infoquery->get_error;
		$self->_feed_hooks(HOOK_SCAN_DONE, $connector, $host, undef, $errmsg);
		return;
	}

	foreach my $share (@{$found_shares_ref}) {
		$self->_feed_hooks(HOOK_SHARE_FOUND, $connector, $host, $share);
	}

	$self->_feed_hooks(HOOK_SCAN_DONE, $connector, $host, $found_shares_ref);
	return @{$found_shares_ref}; 
}

##
# Generate XML data for scan results.
#
# Arguments:
#	scan_results - list with scan results
#
# Returns:
#	xml - results as xml data
sub _create_xml {
	my ($self, @scan_results) = @_;
	my $xml = q{};
	my $output = IO::String->new($xml);
	my $wr = new XML::Writer(OUTPUT => $output, DATA_MODE => 1, DATA_INDENT => 2);

	# Format:
	# <result>
	# 	<host addr="127.0.0.1" name="BOITHO" workgroup="BOITHOS" />
	#		<share>$C</share>
	#		<share>wikipedia</share>
	#	</host>
	# </result>

	$wr->startTag('result');
	foreach my $host (@scan_results) {
		next unless($host);
		#print Dumper 
		$wr->startTag('host', 
			addr => $host->{'addr'}, 
			name => $host->{'name'},
			workgroup => $host->{'workgroup'} );
		
		SHARE:	
		foreach my $share (@{$host->{'shares'}}) {
			next SHARE unless($share);
			$wr->dataElement('share', $share);
		}
		
		$wr->endTag('host');
	}
	$wr->endTag('result');
	return $xml;
}

## Read result xml into a datastucture that Template can parse.
sub parse_xml($$) {
	my $self = shift;
	my $xml_string = shift;
	unless ($xml_string) {
		carp "Didn't get an xml string";
		return;
	}

	my $parser = XML::Parser->new(Style => "tree");
	my $xml = XML::SimpleObject->new($parser->parse($xml_string));

	my @hosts = ();

	foreach my $host ($xml->child('result')->children('host')) {
		last unless ($host);
		my %host;
		$host{'addr'} = $host->attribute('addr');
		$host{'name'} = $host->attribute('name');
		$host{'workgroup'} = $host->attribute('workgroup');
		my @shares = ();
	
		foreach my $share ($host->children('share')) {
			push @shares, $share->value if $share;
		}
		$host{'shares'} = \@shares;
		push @hosts, \%host;
	}

	return \@hosts;
	
}

##
# Uses genip to get all hosts in given range. Range can be the same syntax as nmap uses.
#
# Arguments:
#	range - Ip range
#
# Returns:
#	hosts - List with all hosts in range
sub _get_hosts_in_range {
	my ($self, $range) = @_;

	my @hosts;
	
	my $exec = $genip_path . " \Q$range\E";
	open my $geniph, "$exec |"
		or croak "Unable to execute genip, $!";

        my $range_size = 0;
	while (my $ip = <$geniph>) {
           
                croak "IP-range is too large.\n"
                    if $range_size++ > MAX_RANGE_SIZE;

		chomp $ip;
		push @hosts, $ip;
	}
	close $geniph or croak "Genip exited with error $!";

	return @hosts;
}

##
# Use nmap to ping-scan range. Finds hosts that respond.
#
# Arguments:
#	range - Ip range
#
# Returns:
#	hosts - List of hosts that responded to ping-scan
sub _nmap_icmp_scan {
	my ($self, $range) = @_;

        # slooow way of checking if range is too large.
        open my $geniph, "$genip_path \Q$range\E|"  
            or croak "Unable to execute genip, $!";
        my $size = 0;
        while (<$geniph>) {
            croak "IP-range is too large.\n" if $size++ > MAX_RANGE_SIZE;
        }


	my @hosts;

	my $exec = "nmap -sP -oG - -n \Q$range\E";
	debug("nmap exec ", $exec);

	open my $nmaph, "$exec|"
		or croak "Unable to execute nmap $!";

	while (my $line = <$nmaph>) {
		debug("nmap: ", $line);
		my $ip = $1 if $line =~ m{Host:\s((.*)+)\s\(}xms;
		next unless $ip;
		push @hosts, $ip;
	}
	
	close $nmaph;

	return @hosts;
}

##
# Run hook-functions on given event.
sub _feed_hooks {
	my $self = shift;
	my $hook = shift;

	my ($connector, $host, $result, $msg, $share);
	if ($hook eq HOOK_SCAN_START) { 
		($connector, $host) = @_;
		&{$_}($connector, $host)
			 foreach @scan_start_hooks;
	}
	elsif ($hook eq HOOK_SCAN_DONE) {
		($connector, $host, $result, $msg) = @_;
		&{$_}($connector, $host, $result, $msg)
				 foreach @scan_done_hooks;

	}
	elsif ($hook eq HOOK_SHARE_FOUND) {
		($connector, $host, $share) = @_;
		&{$_}($connector, $host, $share)
			foreach @share_found_hooks;
	}
	else {
		croak "Unknown hook $hook";
	}
}

sub debug {
	print "DEBUG: ", join(" ", @_), "\n"
            if DEBUG;
}

use constant test => 0;
if (test) {
	my $s = Boitho::Scan->new($ENV{'BOITHOHOME'} . "/bin/infoquery");
	my $res = $s->scan("SMB", "213.179.58.120-125", "Boitho", "1234Asd", 1);
	carp $res;
}


1;
