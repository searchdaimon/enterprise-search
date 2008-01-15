#!/usr/bin/perl
package CrawlWatch::Recrawl;
use strict;
use warnings;
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Time::gmtime;
use Data::Dumper;
use SD::Sql::ConnSimple qw(sql_setup get_dbh sql_exec
        sql_fetch_results sql_fetch_single);
use CrawlWatch::Config qw(bb_config_get);
use List::Util qw(min);

use constant CONF_RECHECK_RATE         => 'recrawl_recheck_rate';
use constant CONF_DEFAULT_CRAWL_RATE   => 'default_crawl_rate';
use constant CONF_SCHEDULE_START       => 'recrawl_schedule_start';
use constant CONF_SCHEDULE_END         => 'recrawl_schedule_end';
use constant ONE_DAY => 24 * 3600;


use constant DEBUG => 0;

my %schedule;

my $recheck_rate; # Min. rate between recrawl runs.
my $default_crawl_rate;
my $next_crawl;
my ($dbh, $iq, $log);

sub name { "Collections recrawl" }

sub new {
    my $class = shift;
    ($dbh, $iq, $log) = @_;

    bless {}, $class;
}

sub run {
    my $self = shift;
    $self->fetch_conf_data();
    $self->crawl_collections();
    1;
}

sub next_run {
    my $s = shift;
    eval { $s->fetch_conf_data() };
    if ($@) {
        $log->write("WARN: ", $@);
        return -1;
    }
    # Check schedule.
    if (%schedule) {
        my @scheduled_hours = $s->_gen_scheduled_hours(
            $schedule{start}, $schedule{end});

        my $hour_now = gmtime->hour();
        if (grep { $_ == $hour_now } @scheduled_hours) {
            return $s->calc_next_crawl();
        }
        else {
            return $s->calc_schedl_start($schedule{start});
        }
    }

    return $s->calc_next_crawl();
}

sub calc_schedl_start {
    my ($s, $start) = @_;

    my $hour_now = gmtime->hour();
    if ($start < $hour_now) {
        return ((24 - $hour_now) + $start) * 60; #60 seconds;
    }
    return ($start - $hour_now) * 60;
}

sub fetch_conf_data {
    my $self = shift;

    $default_crawl_rate
        = bb_config_get($dbh, CONF_DEFAULT_CRAWL_RATE), 
    $self->valid_int_field(
        $default_crawl_rate, CONF_DEFAULT_CRAWL_RATE);

    my $start = bb_config_get($dbh, CONF_SCHEDULE_START);
    my $end   = bb_config_get($dbh, CONF_SCHEDULE_END);

    unless ($start and $end) {
        %schedule = ();
        return 1;
    }

    my $schedule_valid = qr(^\d{1,2}$);
    unless ($start =~ $schedule_valid 
        and $end =~ $schedule_valid) {
        %schedule = ();
        return 1;
    }

    $schedule{start} = int $start;
    $schedule{end}   = int $end;
    1;
}

sub valid_int_field {
    my ($self, $field, $field_name) = @_;
    unless (defined $field
                and $field =~ /^\d+$/) {
        croak "ERROR: not valid value for db field $field_name";
    }
    1;
}

my $COLL_LAST = "UNIX_TIMESTAMP(last)";
my $COLL_NOW  = "UNIX_TIMESTAMP(NOW())";

##
# Create SQL query for fetching collections based on rate.
sub coll_create_sql {
    my ($s, $fetch_all, $limit, $next, $rate) = @_;

    my $q = "
    SELECT collection_name,
        $COLL_LAST AS last,
        $COLL_NOW  AS now,
        $next AS next
    FROM shares
    WHERE active = 1
        AND $rate
        AND {where_clause}
    ORDER BY next DESC";
    
    my $clause = ($fetch_all) ? "1 = 1"
        : "$next <= $COLL_NOW";
    $q =~ s/{where_clause}/$clause/;

    $q .= " LIMIT 0, $limit" 
        if defined $limit;
    return $q;
}

##
# Fetch collections that use default rate.
# 
# Returns:
#   by default - collections that need recrawl
#   when fetch_all - All collections
sub coll_def_rate {
    my ($s, $fetch_all, $limit) = @_;
    croak unless (not defined $limit) or $limit =~ /^\d+$/ ;
    my $rate_in_sec = $default_crawl_rate * 60;

    my $query = $s->coll_create_sql($fetch_all, $limit,
            "($COLL_LAST + $rate_in_sec)", "rate = 0");
    return sql_fetch_results($dbh, $query);
}

##
# Fetch collections that use custom rate.
# 
# Returns:
#   by default - collections that need recrawl
#   when fetch_all - All collections
sub coll_custom_rate {
    my ($s, $fetch_all, $limit) = @_;
    my $query = $s->coll_create_sql($fetch_all, $limit,
        "($COLL_LAST + (rate * 60))", "rate != 0");
    return sql_fetch_results($dbh, $query);
}

##
# Fetch uncrawled collections.
sub coll_uncrawled {
    my ($s, $limit) = @_;
    my $q = "SELECT collection_name FROM shares
        WHERE active = 1 AND (last = 0 OR ISNULL(last))";
    $q .= " LIMIT 0, $limit" 
        if defined $limit;

    return sql_fetch_results($dbh, $q);
}

## 
# Calculate time to next crawl.
sub calc_next_crawl {
    my $s = shift;

    return 0 if $s->coll_uncrawled();

    my @coll = ($s->coll_def_rate(1, 1), $s->coll_custom_rate(1, 1));
    return ONE_DAY unless @coll;

    @coll = sort { $a->{'next'} <=> $b->{'next'} } @coll;
    my $next_crawl = $coll[0]->{'next'} - $coll[0]->{'now'};
    return ($next_crawl < 0) ? 0 : $next_crawl;
}

##
# Crawl all collections in need of recrawl.
sub crawl_collections {
    my $s = shift;

    my @uncrawled    = map { $_->{collection_name} } $s->coll_uncrawled();
    my @default_rate = map { $_->{collection_name} } $s->coll_def_rate();
    my @custom_rate  = map { $_->{collection_name} } $s->coll_custom_rate();

    $log->write("Collections needing crawl: ",
        "\nUncrawled: ", Dumper(\@uncrawled),
        "\nTime by default rate: ", Dumper(\@default_rate),
        "\nTime by custom rate: ", Dumper(\@custom_rate));

    $s->crawl($_) for @uncrawled, @default_rate, @custom_rate;
    $s;
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
