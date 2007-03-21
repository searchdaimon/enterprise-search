#!/usr/bin/env perl
package Boitho::Scan;
use strict;
use warnings;
BEGIN {
	push @INC, "Modules";
}
use Boitho::Infoquery;
use Boitho::Scan::Samba;
use XML::Parser;
use XML::SimpleObject;
use XML::Writer;
use IO::String;
use Carp;
use Data::Dumper;

my @supported_connectors = qw(SMB);

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->_initialize;
	return $self;
}

sub _initialize {
	my $self = shift;
	$self->{'smb'} = new Boitho::Scan::Samba;
	$self->{'infoquery'} = new Boitho::Infoquery;
}

## Returns true if there exists a scanning utility for connector.
sub is_supported_connector($$) {
	my ($self, $connector) = @_;
	map { return 1 if $_ eq $connector } @supported_connectors;
	0;
}

## Generic scan method. Send in connector as first parameter, range as second.
sub scan($$$$$) {
	my $self = shift;
	my ($connector, $range, $username, $password) = (@_);
	$self->{'username'} = $username;
	$self->{'password'} = $password;
	$self->{'started'} = time;
	if ($connector eq 'SMB') {
		$self->_smb_scan($range);
	}
	elsif ($connector eq 'MySQL') {
		$self->_mysql_scan($range);
	}
	else {
		carp "Unknown connector $connector";
		return;
	}

	$self->_create_xml;
	return 1;
}

sub _smb_scan($$) {
	my $self = shift;
	my $range =  shift;
	my $smb = $self->{'smb'}; 
	
	$smb->scan($range);
	my @hosts = $smb->get_hosts;
	$self->_scan_for_shares('SMB', \@hosts);

	return 1;
}

## Not needed, just like the above sub
sub _mysql_scan($$) {
	my $self = shift;
	my $range = shift;
	my $mysql = $self->{'mysql'};
	$mysql->scan($range);
	my @hosts = $mysql->get_hosts;
	$self->_scan_for_shares('MySQL', \@hosts);
	1;
}

sub _scan_for_shares($$) {
	my $self = shift;
	my $connector = shift;
	my $hosts = shift;
	my $iq = $self->{'infoquery'};
	my @hosts_with_shares = ();
	foreach my $addr (@$hosts) {
		$self->_log("Looking for shares on $addr", 1);
		my %host;
		if ($connector eq 'SMB') {
			$host{'name'} = $self->{'smb'}->get_name($addr);
			$host{'workgroup'} = $self->{'smb'}->get_workgroup($addr);
		}
		$host{'addr'} = $addr;
		my @shares;
		
		my $found_shares = $iq->scan(
					$connector, 
					$addr, 
					$self->{'username'}, 
					$self->{'password'});
		
		if (!$found_shares) {
			carp ("scan returned an error: " . $iq->get_error);
			next;
		}

		foreach my $share (@$found_shares) {
			$self->_log("Found share $share on $addr");
			push @shares, $share;
		}
		$host{'shares'} = \@shares;
		push @hosts_with_shares, \%host;

	}

	$self->{'hosts'} = \@hosts_with_shares;

	return 1;
}

## Takes datastructure of a scan result, and returns an XML of it.
sub _create_xml($$) {
	my $self = shift;
	my $hosts = $self->{'hosts'};
	my $xml = '';
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
	foreach my $host (@$hosts) {
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
	$self->{'xml'} = $xml;
}

sub get_xml($) {
	my $self = shift;
	return $self->{'xml'};
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

sub get_started {
	my $self = shift;	
	return $self->{'started'};

}

sub _log {
	my ($self, $message, $level) = (@_);
	print "$message <br />";
}

1;
