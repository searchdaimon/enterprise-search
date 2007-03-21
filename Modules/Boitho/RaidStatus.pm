#!/usr/bin/perl
#
# Get the status of a linux raid monitor system
#
# Written by: Eirik A. Nygaard
#             14.12.06
#
# Modified by: Dagur V. Johannsson
#			17.01.2007
# 	- Added package name, getraidinfo, export.
#
# TODO:
#
package Boitho::RaidStatus;
use warnings;
use strict;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(getraidinfo);

use Data::Dumper;

my $debug = 0;
#my $mdstatfile = "./Modules/Boitho/RaidStatus/mdstat-both-active-output";
#my $mdstatfile = "./Modules/Boitho/RaidStatus/mdstat-rebuild-output";
my $mdstatfile = "/proc/mdstat";
# system commands
my $fdisk_cmd = "./Modules/Boitho/RaidStatus/fdiskwrap";

# Partitions that is linux raid
my %raiddrives;
my %raiddrivesnotused;
my %raidsetups;

# Hard disks to detect linux raid partitions on
my @disks = ("sda"); #, "hdb");

sub getraiddrives() {
	
	sub getdrivefdisk($) {
		my $str = shift;

		my @arr = split(/\s+/, $str, 2);
		die "Unable to parse fdisk output" unless ($arr[0] =~ /\/dev\/(\w+\d+)/);
		$raiddrives{$1} = 1;
		return $1;
	}

	foreach (@disks) {
		my $cmd = $fdisk_cmd . " ". $_;
		my @raid = grep(/Linux raid autodetect$/, split(/\n/, `$cmd`));
		
		map{ getdrivefdisk($_); } @raid;
		#print "Raid partitions found for $_: ". join(", ", map{ getdrivefdisk($_); } @raid) . "\n"
		#	if $debug;
		
		#fake data:
		#getdrivefdisk("/dev/hda2");
		#getdrivefdisk("/dev/hdb2");
	}
	%raiddrivesnotused = %raiddrives;
}

sub getraidsetups() {
	open(MDSTAT, "< $mdstatfile") or die "Unable to open $mdstatfile: $!";

	my $state = 0;
	my ($curmd, $raid, $raidstate);
	while (<MDSTAT>) {
		if ($state == 0 and $_ =~ /^md(\d+)\s+:\s+(\w+)\s+\w+(\d)\s+(.*)$/) {
			my %raid;
			my @parts;

			$state+=1;
			$curmd = $1;
			$raidstate = $2;
			$raid = $3;

			@parts = split(/\s+/, $4);
			foreach (@parts) {
				die "Unable to parse mdstat" unless $_ =~ /([^\[]+)\[\d+\](?:\((\w)\))?/;
				if (defined($2)) {
					delete($raiddrivesnotused{$1}) if not ($2 eq 'F');
				} else {
					delete($raiddrivesnotused{$1});
				}
			}
			$raid{disks} = \@parts;
			$raid{state} = $raidstate;
			
			$raidsetups{$curmd} = \%raid;
			print STDERR "Found new raid setup: md$1 which is $2 in raid$3\n" if $debug;
		}
		elsif ($state > 0 and $_ =~/^\s+$/) {
			$state = 0;
			print STDERR "End of current raid setup\n" if $debug;
			next;
		}
		elsif ($state == 1 and $_ =~ /^\s{5,}/) {
			chop;
			$_ =~ s/^\s+(.+)$/$1/;
			if ($_ =~ /(\d+)\s+blocks\s+\[(\d+)\/(\d+)\]\s+\[(.*)\]/) {
				print STDERR "Raid setup has $1 blocks [$2/$3]-($4)\n" if $debug;
				if ($2 != $3) {
					$raidsetups{$curmd}->{degraded} = 1;
				}
			}
			elsif ($_ =~ /^\[[^\]]+\]\s+recovery\s+=\s+([^\s]+)\s+\([^)]+\)\s+finish=([^\s]+)/) {
				$raidsetups{$curmd}->{rebuildstatus} = $1;
				$raidsetups{$curmd}->{rebuildtimeleft} = $2;
				print STDERR "Raid(md$curmd) is rebuilding\n" if $debug;
			}
			else {
				print STDERR "Could not parse '$_'\n" if $debug;
			}
		}
	}

	close(MDSTAT);
}

sub getraidinfo() {
	getraiddrives();
	getraidsetups();

	#print "raiddrives: ", Dumper(\%raiddrives), "<br />\n";
	#print "raiddrives not used: ", Dumper(\%raiddrivesnotused), "<br />\n";
	#print "setup: ", Dumper(\%raidsetups);

	return (\%raiddrives, \%raiddrivesnotused, \%raidsetups);
}
# getraiddrives();
# getraidsetups();
# print "\n\n";
# print "Raid partitions found: ".join(", ", map({ $_ } keys %raiddrives))."\n";
# print "Raid partitions not used(probably damanged/faulty): ".
#        #join(", ", map({ $_ } keys %raiddrivesnotused))."\n\n";
# print "The current raid setup:\n".Dumper(\%raidsetups) ."\n";
# 
# print join(" ", map({ $_ } keys %raiddrivesnotused))."\n";

1;
