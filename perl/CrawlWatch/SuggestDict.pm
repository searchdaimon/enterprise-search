#!/usr/bin/perl
package CrawlWatch::SuggestDict;
use strict;
use warnings;
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Data::Dumper;
use Time::localtime;
use CrawlWatch::Config qw(bb_config_get bb_config_update);
use constant CONF_SUGGDICT_RUN => "suggdict_run_hour";
use constant CONF_SUGGDICT_LAST => "suggdict_last_run";
use constant HOURS => 3600;
use constant ONE_DAY => 24 * HOURS;

my ($dbh, $iq, $log);

sub new {
    croak "missing arguments" unless scalar @_ == 4;
    my $class = shift;
    ($dbh, $iq, $log) = @_;

    bless {}, $class;
}

sub name { "Suggest and spelling dictionary builder" }

sub run {
    bb_config_update($dbh, CONF_SUGGDICT_LAST, time);
    system("sh " . $ENV{'BOITHOHOME'} . "/script/eachNight.sh");
    1;
}

sub next_run {
    my $self = shift;
    my $run_hour = bb_config_get($dbh, CONF_SUGGDICT_RUN);
    my $last_run = bb_config_get($dbh, CONF_SUGGDICT_LAST);
    $log->write("run hour: $run_hour, last_run: $last_run, hour now: ", localtime->hour);

    return -1 unless valid_run_hour($run_hour) 
                 and valid_last_run($last_run);

    my $time_now = time;
    if ($time_now >= ($last_run + (1 * HOURS))) {
        # at least 1 hour since last run.
        
        if ($run_hour == localtime->hour) {
            return 0; #run now.
        }
    }

    my $wait = scnds_till_next($run_hour);
    $wait = 1 * HOURS
        if $wait < (1 * HOURS);
    return $wait;
}

sub scnds_till_next {
    my $run_hour = shift;
    my $next = $run_hour - localtime->hour;
    
    return $next * HOURS
        if $next >= 0;
    return (24 - $next) * HOURS;
}

1;

sub valid_run_hour {
    my $run_hour = shift;
    unless (defined $run_hour
            and $run_hour =~ /^\d+$/
            and $run_hour >= 1
            and $run_hour <= 24) {
    
        $log->write("WARN: not valid run_hour. db field: ", CONF_SUGGDICT_RUN);
        return;
    }
    return 1;
}

sub valid_last_run {
    my $last_run = shift;
    unless (defined $last_run and $last_run =~ /^\d+$/) {
        $log->write("WARN: db field ", CONF_SUGGDICT_LAST, 
            " is not set, or not unixtime.");
        return;
    }
    return 1;
}
