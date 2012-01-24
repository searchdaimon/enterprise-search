#!/usr/bin/perl
package CrawlWatch::SearchCacheClean;
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

use constant DAYS => 60 * 60 * 24;

use constant CONF_SCC_RATE => "scc_default_rate";
use constant CONF_LAST_SCC => "scc_last_run";



sub new {
    croak "missing arguments" unless scalar @_ == 4;
    my $class = shift;
    my %self = map { $_ => shift @_ } qw(dbh iq log);

    bless \%self, $class;
}

sub name { "Search Cache Clean" }


sub recdir {
    my ($path, $scc_rate, $now) = @_;


    print "recdir($path)\n";

    if (-e $path) {

	my $DIR;
    	opendir($DIR, $path) or warn("can't opendir $path: $!") && return;

        while (my $file = readdir($DIR) ) {

                #skipper . og ..
                if ($file =~ /\.$/) {
                      next;
                }
        	my $candidate = $path . "\/" . $file;

		if (-d $candidate) {
			recdir($candidate, $scc_rate, $now);
		}	
		elsif (-f $candidate && (stat($candidate))[9] + $scc_rate < $now) {
			print "Deleting $candidate\n";
			unlink($candidate) or warn("Cant delete cache file \"$candidate\": $!");
		}
	}

	closedir($DIR);
    }


    return 1;
}

sub run {
    my $self = shift;
    bb_config_update($self->{dbh}, CONF_LAST_SCC, time);
    my $scc_rate = bb_config_get($self->{dbh}, CONF_SCC_RATE);
    my $now = time();
    $self->{'log'}->write("Runing search cache clean.");
    #doint the actual SCC run.


    my $path = $ENV{'BOITHOHOME'} . "/var/cache";
    recdir($path, $scc_rate, $now);

    1;
}

sub next_run {
    my $self = shift;
    my $scc_rate = bb_config_get($self->{dbh}, CONF_SCC_RATE);
    my $last_scc = bb_config_get($self->{dbh}, CONF_LAST_SCC);
    my $log = $self->{'log'};
    $log->write("INFO: SCC last crawl: $last_scc, SCC crawl rate: $scc_rate");

    unless (defined $scc_rate and $scc_rate =~ /^\d+$/) {
        $log->write("WARN: db field ", CONF_SCC_RATE, 
                " not valid. Not running SCC.");
        return -1;
    }

    
    unless (defined $last_scc and $last_scc =~ /^\d+$/) {
        $log->write("WARN: db field ", 
            CONF_LAST_SCC, " not valid. Not running SCC.");
        return -1;
    }

    my $next_run = ($last_scc + $scc_rate) - time;
    ($next_run < 0) ? 0 : $next_run;
}

1;


