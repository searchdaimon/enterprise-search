#!/usr/bin/perl
package CrawlWatch::TmpSizeWatch;
use strict;
use warnings;
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Time::localtime;
use Boitho::Infoquery;
use DBI;
use Data::Dumper;
use SD::Sql::ConnSimple qw(sql_exec
        sql_fetch_results sql_fetch_single);
use CrawlWatch::Config qw(bb_config_get bb_config_update);

my $last_run;

sub new {
    croak "missing arguments" unless scalar @_ == 4;
    my $class = shift;
    my %self = map { $_ => shift @_ } qw(dbh iq log);

    bless \%self, $class;
}

sub name { "TmpSizeWatch" }

sub run {
    	my $self = shift;
    	$self->{'log'}->write("Running TmpSizeWatch.");

    	#doint the actual TmpSizeWatch run.

	#lager en array over alle /tmp filer
        my @files = getCandidates("/tmp");

        foreach my $file (@files) {

		#statter, for å finne atime (last access time)
                my ($fdev,$fino,$fmode,$fnlink,$fuid,$fgid,$frdev,$fsize,
                        $fatime,$fmtime,$fctime,$fblksize,$fblocks) = stat($file);

		# get user info. Bur være nåverdende bruker, ikke boitho her.
		my ($uname, $upass, $uuid, $ugid, $uquota, $ucomment, $ugcos,
			 $udir, $ushell, $uexpire) = getpwnam('boitho');

                my $now_string = localtime($fatime);

                if (($fuid == $uuid) && ( $fatime < (time - 600)) && (-f $file)) {
                        print "OLD: file uid $fuid, time $now_string: $file\n";
		    	$self->{'log'}->write("TmpSizeWatch: removing old /tmp file $file. File last accessed at $now_string");

                        #unlink($file) or warn "Can't delete $file: $!\n";
                }
                else {
                        print "NEW: file uid $fuid, time $now_string: $file\n";
                }
        }

    1;
}

use constant MIN => 60;

sub next_run {
	my $s = shift;
	my $time_now = time;

	#if først run, run at ones
	unless (defined $last_run) {
		$last_run = $time_now;
		return 0;
	}

	if ($time_now >= ($last_run + (5 * MIN))) {
		# at least 5 min since last run.
		$last_run = $time_now;
		return 0;
	}

	return $last_run + (5 * MIN) - $time_now;
}

sub getCandidates {
        my $some_dir = shift;
        my @dots;

        opendir(DIR, $some_dir) || die "can't opendir $some_dir: $!";
        @dots = readdir(DIR);
        closedir DIR;

        my $count = 0;
        foreach  (@dots) {
                $dots[$count] = $some_dir . '/' . $dots[$count];
                $count++;
        }

        return @dots;
}


1;


