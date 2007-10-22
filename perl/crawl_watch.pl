#!/usr/bin/perl
package Boitho::CrawlWatch;
use strict;
use warnings;
use POSIX qw(setsid);
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use SD::SimpleLog;
use SD::Sql::ConnSimple qw(sql_setup get_dbh);
use CrawlWatch::GC;
use CrawlWatch::Recrawl;

use constant LOG_FILE          => $ENV{'BOITHOHOME'} . "/logs/crawl_watch.log";
use constant PATH_TO_INFOQUERY => $ENV{'BOITHOHOME'} . "/bin/infoquery";

use constant DEBUG => 0;
use constant DEFAULT_WAKEUP_RATE => 300;

my $log = SD::SimpleLog->new(LOG_FILE, 1);
my $iq = Boitho::Infoquery->new(PATH_TO_INFOQUERY);

my $dbh = get_dbh(sql_setup());

my @services = (
    CrawlWatch::GC->new($dbh, $iq, $log),
    CrawlWatch::Recrawl->new($dbh, $iq, $log),
);

while (1) {
    $log->write("Running...");
    my $wakeup_time = time + DEFAULT_WAKEUP_RATE;

    foreach my $service (@services) {
        my $next_run = $service->next_run();

        if ((not defined $next_run)
                or ($next_run < time)) {
            $log->write("Running service ", $service->name());
            $service->run();
        
            $next_run = $service->next_run();
        }

        if (not defined $next_run) {
            $log->write("WARN: ", $service->name(), " has no next_run set.");
        }
        else {
            $wakeup_time = $next_run
                if $next_run < $wakeup_time;
        }
    }

    if ($wakeup_time < time) {
        $log->write("WARN: Can't wake up in the past ($wakeup_time). ", 
                "Using default wakup rate instead.");
        $wakeup_time = time + DEFAULT_WAKEUP_RATE;
    }
    my $gmtime = gmtime $wakeup_time;
    $log->write("Done. Sleeping until $gmtime.");
    sleep $wakeup_time;
}

