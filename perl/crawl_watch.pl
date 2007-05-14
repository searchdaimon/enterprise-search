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

my $no_logging = 0; # Setting this to true disables logging.

while (1) { #main loop 
	my $next_crawl = undef;
	logwrite("Starting new crawl check.");
	
	my $dbh = get_db_connection();
	my @collections = fetch_collections($dbh);
	$next_crawl = crawl_collections(@collections);
	$dbh->disconnect;

	#last unless $run_as_daimon; #exit

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
	# Collections that have not been crawled yet.
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
		my $default_rate = &fetch_default_rate($dbh);
		my $query = "
		SELECT collection_name, UNIX_TIMESTAMP( last )
		FROM shares
		WHERE active = 1 AND rate = 0 AND (
			UNIX_TIMESTAMP(last) < UNIX_TIMESTAMP(NOW() + ?)
			OR UNIX_TIMESTAMP(last) < (UNIX_TIMESTAMP(NOW()) + ? + ?)
		)";
		
		return fetch_results($dbh, $query, 
			($default_rate, $default_rate, RECHECK_DELAY));
	};
	
	##
	# Collections that have a custom rate set, which timeout is determined from.
	my $fetch_custom_rate = sub {
		my $query = "
		SELECT collection_name, UNIX_TIMESTAMP(last)
		FROM shares
		WHERE active = 1 AND rate != 0 AND (
			UNIX_TIMESTAMP(last) < UNIX_TIMESTAMP(now() + rate)
			OR UNIX_TIMESTAMP(last) < UNIX_TIMESTAMP(now() + rate + ?)
		)";
		return fetch_results($dbh, $query, RECHECK_DELAY);
	};

	my $uncrawled_ref    = &{$fetch_uncrawled};
	my $default_rate_ref = &{$fetch_default_rate};
	my $custom_rate_ref  = &{$fetch_custom_rate};
	
	logwrite("Collections needing crawl, now or soon:", 
		"\nUncrawled: ", Dumper($uncrawled_ref),
		"\nTime by default rate: ", Dumper($default_rate_ref),
		"\nTime by custom rate: ", Dumper($custom_rate_ref));

	my @collections = (@{$uncrawled_ref}, @{$default_rate_ref}, @{$custom_rate_ref});
	
	return @collections;
}

sub fetch_results {
	my ($dbh, $query, @binds) = @_;
	my $sth = $dbh->prepare($query)
		or croak "prepare:", $dbh->errstr;
	$sth->execute(@binds);
	return $sth->fetchall_arrayref;
}

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
	my $infoquery = Boitho::Infoquery->new(PATH_TO_INFOQUERY);
	my $now = time();
	my $next_crawl = undef;
	foreach my $collection_ptr (@collections) {
		my ($collection, $last) = @{$collection_ptr};
		
		if (not $last or $last < $now) {
			if ($infoquery->crawlCollection($collection)) {
				logwrite("Crawling $collection\n");
			}
			else {
				my $error = "";
				$error = $infoquery->get_error 	
					if $infoquery->get_error;
				logwrite("Unable to crawl $collection, $error \n");
			}
		}
		elsif(not defined $next_crawl or $last < $next_crawl) {
			$next_crawl = $last;
		}
	}
	return unless defined $next_crawl;
	return $next_crawl - $now; #Seconds til next collection wants to be crawled.
}
