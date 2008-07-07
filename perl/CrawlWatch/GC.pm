#!/usr/bin/perl
package CrawlWatch::GC;
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

use constant CONF_GC_RATE => "gc_default_rate";
use constant CONF_LAST_GC => "gc_last_run";



sub new {
    croak "missing arguments" unless scalar @_ == 4;
    my $class = shift;
    my %self = map { $_ => shift @_ } qw(dbh iq log);

    bless \%self, $class;
}

sub name { "Garbage collection" }

sub run {
    my $self = shift;
    bb_config_update($self->{dbh}, CONF_LAST_GC, time);
    $self->{'log'}->write("TODO: Add GC collection.");
    #doint the actual GC run.
    system($ENV{'BOITHOHOME'} . "/bin/gcAuthoritybb -l");
    1;
}

sub next_run {
    my $self = shift;
    my $gc_rate = bb_config_get($self->{dbh}, CONF_GC_RATE);
    my $last_gc = bb_config_get($self->{dbh}, CONF_LAST_GC);
    my $log = $self->{'log'};

    unless (defined $gc_rate and $gc_rate =~ /^\d+$/) {
        $log->write("WARN: db field ", CONF_GC_RATE, 
                " not valid. Not running GC.");
        return -1;
    }

    
    unless (defined $last_gc and $last_gc =~ /^\d+$/) {
        $log->write("WARN: db field ", 
            CONF_LAST_GC, " not valid. Not running GC.");
        return -1;
    }

    my $next_run = ($last_gc + $gc_rate) - time;
    ($next_run < 0) ? 0 : $next_run;
}

1;


