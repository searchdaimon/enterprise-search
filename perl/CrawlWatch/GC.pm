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
use constant DEFAULT_GC_RATE => 30 * DAYS;


my ($dbh, $iq, $log);

my $gc_rate;
my $last_gc;

sub new {
    croak "missing arguments" unless scalar @_ == 4;
    my $class = shift;
    ($dbh, $iq, $log) = @_;

    bless {}, $class;
}

sub name { return "Garbage collection" }

sub run {
    my $self = shift;

    #if ($iq->gc) {
        bb_config_update($dbh, CONF_LAST_GC, time);
    #}

    1;
}

sub _fetch_conf_data {
    $gc_rate = bb_config_get($dbh, CONF_GC_RATE);
    $last_gc = bb_config_get($dbh, CONF_LAST_GC);

    unless (defined $gc_rate and $gc_rate =~ /^\d+/) {
        $log->write("WARN: ", CONF_GC_RATE, 
                " not set. Asuming default value of ", DEFAULT_GC_RATE);
        $gc_rate = DEFAULT_GC_RATE;
    }
}

sub next_run {
    my $self = shift;
    $self->_fetch_conf_data();
    if (not defined $last_gc) {
        $log->write("WARN: Unable to determine next GC run. ",
            "Has configkey ", CONF_LAST_GC, " been created yet?");
    }
    return if not $last_gc; 
    return $last_gc + $gc_rate;
}

1;


