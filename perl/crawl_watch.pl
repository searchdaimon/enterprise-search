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

use constant RECHECK_DELAY => 300; # 5 min
use constant LOG_FILE          => $ENV{'BOITHOHOME'} . "/logs/crawl_watch.log";
use constant SQL_CONFIG_FILE   => $ENV{'BOITHOHOME'} . "/config/setup.txt";
use constant PATH_TO_INFOQUERY => $ENV{'BOITHOHOME'} . "/bin/infoquery";

use constant DEBUG => 0;

my $no_logging = 0; # Setting this to true disables logging.

while (1) { #main loop 
	my $next_crawl = undef;
	logwrite("Starting new crawl check.");
	
	my $dbh = get_db_connection();
	my @collections = fetch_collections($dbh);
	$next_crawl = crawl_collections(@collections);
	$dbh->disconnect;

	if (defined $next_crawl and $next_crawl < RECHECK_DELAY) {
	    logwrite("Crawl check done. Next scheduled in in ", 
		int($next_crawl / 60), " minutes\n");

	    sleep $next_crawl;
	}
	else {
	    logwrite("Crawl check done. Next scheduled in in ", 
		int(RECHECK_DELAY / 60), " minutes.\n");
	    
	    sleep RECHECK_DELAY;
	}
}

##
# Write to log file. Warns and disables logging on fail.
#
# Parameters:
#   @data - String(s) to write.
sub logwrite {
    my @data = @_;

    return if $no_logging;

    my $success = open my $logh, ">>", LOG_FILE;

    if ($success) { 
	my $time = gmtime(time());
	print {$logh} "$time - ";
	print {$logh} join(q{}, @data);
    }
    else {
	warn "Unable to write to logfile: $!. Disableing logging.";
	$no_logging = 1;
    }
    
    close $logh;
    1;
}

## 
# Get db connection values from a config file.
# Taken directly from Sql::Sql
sub read_config {
        my %settings;

        open my $setup, SQL_CONFIG_FILE
		 or croak print "Can't open config file: $!";
        my @data = <$setup>;
        close $setup;

        foreach my $line (@data) {
                my ($name, $value) = split(/=/, $line);
                chomp($value) if $value;
                $settings{$name} = $value if ($name and $value);
        }

        $settings{'port'} = 3306 unless($settings{'port'});

        return ( $settings{'database'}, 
		 $settings{'server'}, 
		 $settings{'port'},
		 $settings{'user'},
		 $settings{'Password'} );
}

sub get_db_connection {
	my ($db, $host, $port, $user, $pass) = &read_config;
        my $dbh = DBI->connect("DBI:mysql:database=$db;host=$host;port=$port", $user, $pass)
                or croak("$DBI::errstr");

	return $dbh;
}

##
# Fetches collections that need to be crawled,
# or need to be crawled soon (soon being less than RECHECK_DELAY)
sub fetch_collections {
	my $dbh = shift;

	##
	# Collections that have not been crawled yet.  my $fetch_uncrawled = sub {
	my $fetch_uncrawled = sub {
		my $query = "SELECT collection_name, UNIX_TIMESTAMP(last)
			FROM shares
			WHERE active = 1 AND
			ISNULL(last)";
		return fetch_results($dbh, $query);
	};

	##
	# Collections that don't have a rate set. Timeout is determined by default rate.
	my $fetch_default_rate = sub {
		my $default_rate = fetch_default_rate($dbh);
		my $rate_in_sec = $default_rate * 60;
		my $recheck = RECHECK_DELAY;
		my $query = "" .
		"SELECT collection_name, UNIX_TIMESTAMP(last), rate
		 FROM shares
		 WHERE active = 1 AND rate = 0 AND ( " . # share is active
			"(UNIX_TIMESTAMP(last) + $rate_in_sec) <= UNIX_TIMESTAMP(NOW())" . # share needs recrawl
			"OR " .
			"(UNIX_TIMESTAMP(last) + $rate_in_sec) <= (UNIX_TIMESTAMP(NOW()) + $recheck)" . # share needs recrawl within RECHECK_DELAY
		")";
		
		my @results = fetch_results($dbh, $query);
		return add_next_crawl($dbh, @results);
	};
	
	##
	# Collections that have a custom rate set, which timeout is determined from.
	my $fetch_custom_rate = sub {
		my $recheck = RECHECK_DELAY;
		my $query = "".
		"SELECT collection_name, UNIX_TIMESTAMP(last), rate
		 FROM shares
		 WHERE active = 1 AND rate != 0 AND (". # share is active
			"( (UNIX_TIMESTAMP(last) + (rate * 60)) <= UNIX_TIMESTAMP(NOW()) )". # share needs recrawl
			"OR ".
			"( (UNIX_TIMESTAMP(last) + (rate * 60)) <= (UNIX_TIMESTAMP(NOW()) + $recheck) )". # share needs recrawl within RECHECK_DELAY
		")";
		my @results = fetch_results($dbh, $query);
		return add_next_crawl($dbh, @results);
	};

	my @uncrawled    = &{$fetch_uncrawled};
	my @default_rate = &{$fetch_default_rate};
	my @custom_rate  = &{$fetch_custom_rate};
	
	logwrite("Collections needing crawl, now or soon:", 
		"\nUncrawled: ", Dumper(\@uncrawled),
		"\nTime by default rate: ", Dumper(\@default_rate),
		"\nTime by custom rate: ", Dumper(\@custom_rate));

	my @collections = (@uncrawled, @default_rate, @custom_rate);
	
	return @collections;
}

##
# Fetch db results for given query and binds
# to arrayref
sub fetch_results {
	my ($dbh, $query, @binds) = @_;
	my $sth = $dbh->prepare($query)
		or croak "prepare:", $dbh->errstr;
	$sth->execute(@binds);
	
	my @results;
	while (my $res = $sth->fetchrow_hashref) {
		push @results, $res;
	}

	return @results;
}

##
# Adds when a collection shall be crawled next.
#
# Attributes:
#	dbh - db handler
#	collections - array with collection hashrefs (from fetch_results).
#
# See <fetch_results>
sub add_next_crawl {
	my $dbh = shift;
	my @collections = @_;
	my $default_rate = fetch_default_rate($dbh);
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
# Gets default rate from db.
sub fetch_default_rate {
	my $dbh = shift;
	my $query = "SELECT configvalue FROM config
			WHERE configkey = 'default_crawl_rate'";
	my $sth = $dbh->prepare($query)
		or croak "prepare" , $dbh->errstr;
	$sth->execute;
	my $rate = $sth->fetchrow_array;
	carp "Default crawl rate not set" unless $rate;
	return $rate;
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
		my ($collection, $next_crawl) = ($coll_ptr->{'collection_name'}, $coll_ptr->{'next'});
		
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
			logwrite("Not crawling $collection. Shall be crawled in ", $crawled_in / 60, " minutes.");

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
		logwrite("Crawling $collection\n");
	}
	else {
		my $error = "";
		if ($infoquery->get_error) {
			$error = $infoquery->get_error;
		}
		logwrite("Unable to crawl $collection, $error \n");
	}
	1;
}

sub debug {

	if (DEBUG) { print "DEBUG: ", join('', @_), "\n" }
}
