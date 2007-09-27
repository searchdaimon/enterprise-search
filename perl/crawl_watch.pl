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
use DBI;
use Data::Dumper;
use SD::SimpleLog;
use SD::Sql::ConnSimple qw(sql_setup get_dbh sql_exec
        sql_fetch_results sql_fetch_single);

use constant RECRAWL_RATE => 300; # 5 min
use constant GC_RATE      => 30 * 24 * 60 * 60; # 30 days
use constant LOG_FILE          => $ENV{'BOITHOHOME'} . "/logs/crawl_watch.log";
use constant PATH_TO_INFOQUERY => $ENV{'BOITHOHOME'} . "/bin/infoquery";

use constant CONF_LAST_GC    => 'last_infoquery_gc';
use constant CONF_CRAWL_RATE => 'default_crawl_rate';

use constant DEBUG => 0;

my $enable_logging = 1;
my $log = SD::SimpleLog->new(LOG_FILE, $enable_logging);

my %setup = sql_setup();
while (1) {
    my ($next_crawl, $next_gc) = undef;
    $log->write("Starting check.");

    my $dbh = get_dbh(%setup);


    my @collections = fetch_collections($dbh);
    $next_crawl = crawl_collections(@collections);
    $next_gc = garbage_collect($dbh);

    $dbh->disconnect;

    my $wakeup = wakeup_time($next_crawl, $next_gc);
    $log->write("Check done, now sleeping for ", 
        int($wakeup / 60), " minutes.");

    sleep $wakeup;
}

sub wakeup_time {
    my ($next_crawl, $next_gc) = @_;
    
    if ((not defined $next_crawl)
            or ($next_crawl > RECRAWL_RATE)) {
        $next_crawl = RECRAWL_RATE;
    }

    return $next_crawl unless defined $next_gc;

    return ($next_crawl < $next_gc) 
        ? $next_crawl : $next_gc;
}

sub garbage_collect {
    my $dbh = shift;
    my $last_gc = bb_config_get($dbh, CONF_LAST_GC);
    unless ($last_gc) {
        $log->write("WARNING: ", CONF_LAST_GC, " not set. Skipping GC.");
        return;
    }
    
    if ($last_gc + GC_RATE < time()) {
        my $h_last_gc = gmtime $last_gc;
        $log->write("Garbage collecting. (last GC was at $h_last_gc)");
        #TODO: Infoquery GC cmd.
        $last_gc = time();
        bb_config_set($dbh, CONF_LAST_GC, $last_gc);
    }

    return $last_gc + GC_RATE;
}

##
# Fetches collections that need to be crawled,
# or need to be crawled soon (soon being less than RECRAWL_RATE)
sub fetch_collections {
    my $dbh = shift;

    ##
    # Collections that have not been crawled yet.  my $fetch_uncrawled = sub {
    my $fetch_uncrawled = sub {
        my $query = "SELECT collection_name, UNIX_TIMESTAMP(last)
            FROM shares
            WHERE active = 1 AND
            ISNULL(last)";
        return sql_fetch_results($dbh, $query);
    };

    ##
    # Collections that don't have a rate set. Timeout is determined by default rate.
    my $fetch_default_rate = sub {
        my $default_rate = bb_config_get($dbh, CONF_CRAWL_RATE);
        my $rate_in_sec = $default_rate * 60;
        my $recrawl = RECRAWL_RATE;
        my $query = "" .
            "SELECT collection_name, UNIX_TIMESTAMP(last), rate
            FROM shares
            WHERE active = 1 AND rate = 0 AND ( " . # share is active
                    "(UNIX_TIMESTAMP(last) + $rate_in_sec) <= UNIX_TIMESTAMP(NOW())" . # share needs recrawl
                    "OR " .
                    "(UNIX_TIMESTAMP(last) + $rate_in_sec) <= (UNIX_TIMESTAMP(NOW()) + $recrawl)" . # share needs recrawl within RECRAWL_RATE
                ")";
		
	my @results = sql_fetch_results($dbh, $query);
	return add_next_crawl($dbh, @results);
    };
	
    ##
    # Collections that have a custom rate set, which timeout is determined from.
    my $fetch_custom_rate = sub {
        my $recrawl = RECRAWL_RATE;
        my $query = "".
            "SELECT collection_name, UNIX_TIMESTAMP(last), rate
            FROM shares
            WHERE active = 1 AND rate != 0 AND (". # share is active
                "( (UNIX_TIMESTAMP(last) + (rate * 60)) <= UNIX_TIMESTAMP(NOW()) )". # share needs recrawl
                "OR ".
                "( (UNIX_TIMESTAMP(last) + (rate * 60)) <= (UNIX_TIMESTAMP(NOW()) + $recrawl) )". # share needs recrawl within RECRAWL_RATE
            ")";
        my @results = sql_fetch_results($dbh, $query);
        return add_next_crawl($dbh, @results);
    };

    my @uncrawled    = &{$fetch_uncrawled};
    my @default_rate = &{$fetch_default_rate};
    my @custom_rate  = &{$fetch_custom_rate};

    $log->write("Collections needing crawl, now or soon:", 
        "\nUncrawled: ", Dumper(\@uncrawled),
        "\nTime by default rate: ", Dumper(\@default_rate),
        "\nTime by custom rate: ", Dumper(\@custom_rate));

    my @collections = (@uncrawled, @default_rate, @custom_rate);

    return @collections;
}

##
# Adds when a collection shall be crawled next.
#
# Attributes:
#	dbh - db handler
#	collections - array with collection hashrefs
sub add_next_crawl {
    my $dbh = shift;
    my @collections = @_;
    my $default_rate = bb_config_get($dbh, CONF_CRAWL_RATE);
    debug("add_next_crawl: default_rate ", $default_rate);

    foreach my $coll_ref (@collections) {
        my $last_crawl = $coll_ref->{'UNIX_TIMESTAMP(last)'};
        debug("Add next to", Dumper($coll_ref));

        unless ($last_crawl) {
            # coll is uncrawled.
            debug("add_next_crawl: collection ", 
                    $coll_ref->{'collection_name'}, " uncrawled. Ignoring.");
            next;

        }

        if ($coll_ref->{'rate'}) {
            # coll has a custom rate.
            my $next_crawl = $last_crawl + ($coll_ref->{'rate'} * 60);
            $coll_ref->{'next'} = $next_crawl;
        }

        else {
            # we use default rate
            my $next_crawl = $last_crawl + ($default_rate * 60);
            $coll_ref->{'next'} = $next_crawl;
        }

    }

    return @collections;
}

##
# Read configvalue from DB.
sub bb_config_get {
    my ($dbh, $confkey) = @_;
    my $query = "SELECT configvalue FROM config
                    WHERE configkey = ?";
    my $val = sql_fetch_single($dbh, $query, $confkey);
    $log->write("WARNING: Configkey $confkey not set in db.") 
        unless $val;

    return $val;
}

##
# Update configvalue in db.
sub bb_config_set {
    my ($dbh, $confkey, $value) = @_;
    my $query = "UPDATE config
        SET configvalue = ? 
        WHERE configkey = ?";
    
    sql_exec($dbh, $query, $value, $confkey);
    1;
}

## 
# Crawls the collections that need to be crawled.
# 
# If collections that don't need crawl are found,
# the function returns time in seconds till they should be.
# -- If not, the function returns false.
sub crawl_collections {
    my @collections = @_;
    my $now = time();
    my $next_run = undef;

    foreach my $coll_ptr (@collections) {
        my ($collection, $next_crawl) 
            = ($coll_ptr->{'collection_name'}, $coll_ptr->{'next'});

        unless (defined $coll_ptr->{'UNIX_TIMESTAMP(last)'}) {
            # Not crawled yet, needs crawl.
            debug("crawl_collections: $collection not crawled yet. Crawling.");
            crawl($collection);
            next;

        }

        if ($next_crawl <= $now) {
            # Time to crawl it.
            debug("crawl_collection: $collection needs a crawl.");
            crawl($collection);
        }

        else {
            # Should be crawled soon.
            my $crawled_in = ($next_crawl - $now);
            $log->write("Not crawling $collection. Shall be crawled in ", 
                $crawled_in / 60, " minutes.");

            unless (defined $next_run) {
                $next_run = $crawled_in;
            }
            elsif ($crawled_in < $next_run) {
                $next_run = $crawled_in;
            }
        }
    }
    return $next_run;
}

##
# Crawls the given collection.
#
# Attributes:
#	collection - collection name
sub crawl {
    my $collection = shift;
    my $infoquery = Boitho::Infoquery->new(PATH_TO_INFOQUERY);
    if ($infoquery->crawlCollection($collection)) {
        $log->write("Crawling $collection");
    }
    else {
        my $error = "";
        if ($infoquery->get_error) {
            $error = $infoquery->get_error;
        }
        $log->write("Unable to crawl $collection, $error");
    }
    1;
}

sub debug {
    if (DEBUG) { print "DEBUG: ", join('', @_), "\n" }
}
