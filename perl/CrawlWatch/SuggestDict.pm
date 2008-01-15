#!/usr/bin/perl
package CrawlWatch::SuggestDict;
use strict;
use warnings;
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Data::Dumper;
use Time::gmtime;
use CrawlWatch::Config qw(bb_config_get bb_config_update);
use constant CONF_SUGGDICT_RUN => "suggdict_run_hour";
use constant CONF_SUGGDICT_LAST => "suggdict_last_run";
use constant ONE_DAY => 24 * 3600;

my ($dbh, $iq, $log);

sub new {
    croak "missing arguments" unless scalar @_ == 4;
    my $class = shift;
    ($dbh, $iq, $log) = @_;

    bless {}, $class;
}

sub name { "Suggest dictionary builder" }

sub run {
    bb_config_update($dbh, CONF_SUGGDICT_LAST, time);
    # fork av dict prosess?
    1;
}

sub next_run {
    my $self = shift;
    my $run_hour = bb_config_get($dbh, CONF_SUGGDICT_RUN);
    my $last_run = bb_config_get($dbh, CONF_SUGGDICT_LAST);

    unless (defined $run_hour
        and $run_hour =~ /^\d+$/    
        and ($run_hour >= 0)
        and ($run_hour <= 23)) {
        $log->write("WARN: invalid run hour set for ", $self->name, 
            ". db field: ", CONF_SUGGDICT_RUN);
        return -1;
    }

    unless (defined $last_run and $last_run =~ /^\d+$/) {
        $log->write("WARN: db field ", CONF_SUGGDICT_LAST, 
            " is not set, or not unixtime.");
        return -1;
    }
    
    my $time_now = time;
    if ($time_now >= $last_run + ONE_DAY) {
        # a day (or more) since last run.
        
        $run_hour = 14;
        if ($run_hour == gmtime->hour) {
            return 0; #run now.
        }
    }
    
    return $last_run ? ($last_run - time) + ONE_DAY : ONE_DAY;
}

1;


