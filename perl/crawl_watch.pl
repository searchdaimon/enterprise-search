#!/usr/bin/perl
package Boitho::CrawlWatch;
use strict;
use warnings;
use POSIX qw(setsid);
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Data::Dumper;
use Boitho::Infoquery;
use SD::SimpleLog;
use SD::Sql::ConnSimple qw(sql_setup get_dbh);
use CrawlWatch::GC;
use CrawlWatch::Recrawl;
use CrawlWatch::SuggestDict;
use CrawlWatch::TmpSizeWatch;
use CrawlWatch::SearchCacheClean;
use CrawlWatch::AnonymousStatistics;

use constant LOG_FILE          => $ENV{'BOITHOHOME'} . "/logs/crawl_watch.log";
use constant PATH_TO_INFOQUERY => $ENV{'BOITHOHOME'} . "/bin/infoquery";

use constant DEBUG => 0;
use constant DEFAULT_WAKEUP_RATE => 300;

my $log = SD::SimpleLog->new(LOG_FILE, 1);
$log->show_in_stdout(1);

my $iq = Boitho::Infoquery->new(PATH_TO_INFOQUERY);
$| = 1;

my %setup = sql_setup();
#$setup{database} = "test_" . $setup{database};
my $dbh = get_dbh(%setup);

my @services = (
    CrawlWatch::GC->new($dbh, $iq, $log),
    CrawlWatch::Recrawl->new($dbh, $iq, $log),
    CrawlWatch::SuggestDict->new($dbh, $iq, $log),
    CrawlWatch::TmpSizeWatch->new($dbh, $iq, $log),
    CrawlWatch::SearchCacheClean->new($dbh, $iq, $log),
    CrawlWatch::AnonymousStatistics->new($dbh, $iq, $log),
);

while (1) {
    $log->write("Running...");
    my $sleep_scnds = DEFAULT_WAKEUP_RATE;

    foreach my $service (@services) {
        my $next_run = $service->next_run();
        croak "return value from next_run is not an int:", $next_run
            unless $next_run =~ /^(-?)\d+$/;

        if ($next_run == -1) {
            $log->write("Error in service ",    
                $service->name, ". Skipping.");
            next;
        }
        if ($next_run == 0) {
            $log->write("Running service ", $service->name);
            $service->run();
            redo; # to get next_run time.
        }
        elsif ($next_run < -1) {
            croak "invalid return value '$next_run' from ", $service->name;
        }
        else {
            $sleep_scnds = $next_run
                unless $sleep_scnds < $next_run;
            $log->write("Estimated run for ", $service->name, ": ", 
                scalar localtime time + $next_run);
        }
    }

    my $gmtime = localtime time + $sleep_scnds;
    $log->write("Done. Sleeping until $gmtime.");
    sleep $sleep_scnds;
}

