#!/usr/bin/perl
#
# Get the status of a linux raid monitor system
#
# Written by: Eirik A. Nygaard
#             14.12.06
#
# TODO:
#

use warnings;
use strict;

use Data::Dumper;

my $debug = 0;
#my $mdstatfile = "/home/eirik/mdstat-both-active-output";
#my $mdstatfile = "/home/eirik/mdstat-rebuild-output";
my $mdstatfile = "/proc/mdstat";

# system commands
my $fdisk_cmd = "./fdiskwrap";

# Partitions that is linux raid
my %raiddrives;
my %raiddrivesnotused;
my %raidsetups;

# Hard disks to detect linux raid partitions on
my @disks = ("hda", "hdb");

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

getraiddrives();
getraidsetups();

print "Raid partitions found: ".join(", ", map({ $_ } keys %raiddrives))."\n";
print "Raid partitions not used(probably damanged/faulty): ".
	join(", ", map({ $_ } keys %raiddrivesnotused))."\n\n";
print "The current raid setup:\n".Dumper(\%raidsetups) ."\n";

