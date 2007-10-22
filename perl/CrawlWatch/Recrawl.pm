#!/usr/bin/perl
package CrawlWatch::Recrawl;
use strict;
use warnings;
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Time::localtime;
use Data::Dumper;
use SD::Sql::ConnSimple qw(sql_setup get_dbh sql_exec
        sql_fetch_results sql_fetch_single);
use CrawlWatch::Config qw(bb_config_get);

use constant DEFAULT_RECHECK_RATE => 300; # 5 min
use constant DEFAULT_CRAWL_RATE   => 1440; # 1 hour.

use constant CONF_RECHECK_RATE         => 'cm_crawl_recheck_rate';
use constant CONF_DEFAULT_CRAWL_RATE   => 'default_crawl_rate';
use constant CONF_SCHEDULE_START       => 'recrawl_schedule_start';
use constant CONF_SCHEDULE_END         => 'recrawl_schedule_end';


use constant DEBUG => 0;

my %schedule;

my ($dbh, $iq, $log);
my $recheck_rate; # Min. rate between recrawl runs.
my $default_crawl_rate;
my $next_crawl;

sub name { "Collections recrawl" }

sub new {
    my $class = shift;
    ($dbh, $iq, $log) = @_;
    bless {}, $class;
}

sub run {
    my $self = shift;
    $self->_fetch_conf_data();
    $next_crawl = $self->crawl_collections(
            $self->fetch_collections());

    $next_crawl = $recheck_rate unless $next_crawl;
    1;
}

sub next_run {
    my $self = shift;
    $self->_fetch_conf_data();
    
    # Check schedule.
    if (%schedule) {
        my $hour_now = localtime->hour();
        my @scheduled_hours 
            = $self->_gen_scheduled_hours($schedule{start}, $schedule{end});

        unless (grep { $_ == $hour_now } @scheduled_hours) {
            $log->write("Recrawl: Not scheduled to do recrawls.");
            $next_crawl = $recheck_rate;
        }
    }

    return unless $next_crawl;
    return ($next_crawl < $recheck_rate) 
        ? time + $next_crawl : time + $recheck_rate;
}

sub _fetch_conf_data {
    $recheck_rate = 
        conf_int_value(CONF_RECHECK_RATE, 
                bb_config_get($dbh, CONF_RECHECK_RATE),
                DEFAULT_RECHECK_RATE);
    
    $default_crawl_rate =
        conf_int_value(CONF_DEFAULT_CRAWL_RATE,
                bb_config_get($dbh, CONF_DEFAULT_CRAWL_RATE), 
                DEFAULT_CRAWL_RATE);

    my $start = bb_config_get($dbh, CONF_SCHEDULE_START);
    my $end   = bb_config_get($dbh, CONF_SCHEDULE_END);

    unless ($start and $end) {
        %schedule = ();
        return;
    }

    my $schedule_valid = qr(^\d{2}$);
    unless ($start =~ $schedule_valid 
        and $end =~ $schedule_valid) {

        %schedule = ();
        return;
    }

    $schedule{start} = int $start;
    $schedule{end}   = int $end;
    1;
}



sub conf_int_value {
    my ($confkey, $dbvalue, $default_value) = @_;
    if ((not defined $dbvalue) 
            or ($dbvalue !~ /^\d+$/)) {
        $log->write("WARN: configkey $confkey not set, or not valid.",
            " Asuming value $default_value.");
        return $default_value;
    }

    return $dbvalue;
}

sub debug {
    if (DEBUG) { print "DEBUG: ", join('', @_), "\n" }
}

##
# Fetches collections that need to be crawled,
# or need to be crawled soon (soon being less than RECRAWL_RATE)
sub fetch_collections {
    my $self = shift;

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
        my $rate_in_sec = $default_crawl_rate * 60;
        my $query = "" .
            "SELECT collection_name, UNIX_TIMESTAMP(last), rate
            FROM shares
            WHERE active = 1 AND rate = 0 AND ( " . # share is active
                    "(UNIX_TIMESTAMP(last) + $rate_in_sec) <= UNIX_TIMESTAMP(NOW())" . # share needs recrawl
                    "OR " .
                    "(UNIX_TIMESTAMP(last) + $rate_in_sec) <= (UNIX_TIMESTAMP(NOW()) + $recheck_rate)" . # share needs recrawl within RECRAWL_RATE
                ")";
		
	my @results = sql_fetch_results($dbh, $query);
	return $self->add_next_crawl(@results);
    };
	
    ##
    # Collections that have a custom rate set, which timeout is determined from.
    my $fetch_custom_rate = sub {
        my $query = "".
            "SELECT collection_name, UNIX_TIMESTAMP(last), rate
            FROM shares
            WHERE active = 1 AND rate != 0 AND (". # share is active
                "( (UNIX_TIMESTAMP(last) + (rate * 60)) <= UNIX_TIMESTAMP(NOW()) )". # share needs recrawl
                "OR ".
                "( (UNIX_TIMESTAMP(last) + (rate * 60)) <= (UNIX_TIMESTAMP(NOW()) + $recheck_rate) )". # share needs recrawl within RECRAWL_RATE
            ")";
        my @results = sql_fetch_results($dbh, $query);
        return $self->add_next_crawl(@results);
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
    my ($self, @collections) = @_;

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
            my $next_crawl = $last_crawl + ($default_crawl_rate * 60);
            $coll_ref->{'next'} = $next_crawl;
        }

    }

    return @collections;
}

## 
# Crawls the collections that need to be crawled.
# 
# If collections that don't need crawl are found,
# the function returns time in seconds till they should be.
# -- If not, the function returns false.
sub crawl_collections {
    my ($self, @collections) = @_;
    my $now = time();
    my $next_run = undef;

    foreach my $coll_ptr (@collections) {
        my ($collection, $next_crawl) 
            = ($coll_ptr->{'collection_name'}, $coll_ptr->{'next'});

        unless (defined $coll_ptr->{'UNIX_TIMESTAMP(last)'}) {
            # Not crawled yet, needs crawl.
            debug("crawl_collections: $collection not crawled yet. Crawling.");
            $self->crawl($collection);
            next;

        }

        if ($next_crawl <= $now) {
            # Time to crawl it.
            debug("crawl_collection: $collection needs a crawl.");
            $self->crawl($collection);
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
    my ($self, $collection) = @_;
    if ($iq->crawlCollection($collection)) {
        $log->write("Crawling $collection");
    }
    else {
        my $error = "";
        if ($iq->get_error) {
            $error = $iq->get_error;
        }
        $log->write("Unable to crawl $collection, $error");
    }
    1;
}

##
# Generates list of valid hours within given schedule.
#
# Arguments:
#   start - Value (hour) between 1 and 24
#   end   - Value (hour) between 1 and 24
sub _gen_scheduled_hours {
    my ($self, $start, $end) = @_;

    if ($end > $start) {
        return ($start..$end);
    }
    else {
        my @hours = ($start..24);
        @hours = (@hours, (1..$end));
        return @hours;
    }
}





1;
