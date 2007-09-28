#!/usr/bin/env perl

##
# Run bbupdate every x hours.
use strict;
use warnings;

use constant HOUR => 3600;
use constant DAY  => 24 * HOUR;

use constant UPDATE_RATE => 24 * HOUR;
use constant UPDATE_PATH => "$ENV{BOITHOHOME}/setuid/yumwrapper";
use constant LOGFILE     => "$ENV{BOITHOHOME}/logs/bb_auto_update.log";

BEGIN { unshift @INC, "$ENV{BOITHOHOME}/Modules" }
use Carp;
use Boitho::YumWrapper;
use Data::Dumper;
use SD::SimpleLog;

my $log_enabled = 1;
my $log = SD::SimpleLog->new(LOGFILE, $log_enabled);
my $yum = Boitho::YumWrapper->new(undef, UPDATE_PATH);

$log->write("bbAutoUpdate started...");

while (1) { 
    $log->write("Running update");
    my ($stat, @res);

    eval {
        foreach my $act (qw(clean check_update update)) { 
            ($stat, @res) = $yum->$act();
            log_output($act, $stat, @res);
            croak unless $stat;
        }
    };
    if ($@) { $log->write("Something failed, giving up."); }

    my $wakeup = gmtime(time() + UPDATE_RATE);
    $log->write("Sleeping until $wakeup");
    sleep UPDATE_RATE;
}

sub log_output {
    my ($title, $stat, @output) = @_;
    my $res = $stat ? "ran OK" : "FAILED";
    $log->write("$title $res:\n", @output);
}

