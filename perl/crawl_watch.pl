#!/usr/bin/perl

package Boitho::CrawlWatch;
use strict;
use warnings;
use POSIX qw(setsid);
use Carp;
BEGIN {
	#runarb: gjør om slik at denne kan kalles fra hvor som helst
    	#unshift @INC, "../Modules";
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use DBI;
use Data::Dumper;

use constant RECHECK_DELAY => 300; # 5 min
use constant PID_FILE => "/tmp/crawl_watch.pid";
use constant SQL_CONFIG_FILE => $ENV{'BOITHOHOME'} . "/config/setup.txt";
use constant PATH_TO_INFOQUERY => $ENV{'BOITHOHOME'} . "/bin/infoquery";
use constant DEBUG => 1;

my $run_as_daimon = 1;
chomp (my $arg = shift @ARGV) if @ARGV;
if ($arg eq "--single-check") {
	$run_as_daimon = 0;
}

if ($run_as_daimon) {
	my $pid = &fork_to_bg;
	&write_pid($pid);
}


while (1) { #main loop 
	my $next_crawl = undef;
	
	my $dbh = &get_db_connection;
	my $collections_ptr = &fetch_collections($dbh);
	$next_crawl = &crawl_collections($collections_ptr);
	$dbh->disconnect;

	last unless $run_as_daimon; #exit

	if (defined $next_crawl and $next_crawl < RECHECK_DELAY) {
		print "# Next check scheduled in ", 
			int($next_crawl / 60), " minutes\n" if DEBUG;
		sleep $next_crawl;
	}
	else {
		print "# Next check scheduled in ", 
			int(RECHECK_DELAY / 60), " minutes.\n" if DEBUG;
		sleep RECHECK_DELAY;
	}
}

## Functions
sub write_pid($) {
	my $pid = shift;
	open my $pid_file, ">", PID_FILE
		or carp "Unable to write to pid file, $!";
	print {$pid_file} $pid;
	close $pid_file;
	1;
}

sub fork_to_bg() {
	defined (my $pid = fork)
		or croak "Error: Can't fork: $!";
	exit if $pid;
	my $new_pid = setsid 
		or die "Error: Can't start a new session: $!";
	print "Starting crawl watch in the background...\n";
	return $new_pid;
}

## Get db connection values from a config file.
## Taken directly from Sql::Sql
sub read_config() {
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

sub get_db_connection($$$$$) {
	my ($db, $host, $port, $user, $pass) = &read_config;
        my $dbh = DBI->connect("DBI:mysql:database=$db;host=$host;port=$port", $user, $pass)
                or croak("$DBI::errstr");

	return $dbh;
}

## Fetches collections that need to be crawled,
## or need to be crawled soon (soon being less than RECHECK_DELAY)
sub fetch_collections($) {
	my $dbh = shift;
	# Mysql 3 doesn't seem to support sub queries nor IF statements,
	# so we'll have to merge different queries. That might a be a good thing though.
	my $fech_uncrawled = sub {
		my $query = "SELECT collection_name, UNIX_TIMESTAMP(last)
			FROM shares
			WHERE active = 1 AND
			ISNULL(last)";
		return fetch_results($dbh, $query);
	};

	## Collections that don't have a rate set. Timeout is determined by default rate.
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
	
	## Collections that have a custom rate set, which timeout is determined from.
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

	print   "uncrawled: ", Dumper(&$fech_uncrawled), 
		"default rate: ", Dumper(&$fetch_default_rate),
		"custom rate: ", Dumper(&$fetch_custom_rate) if DEBUG;
	my @collections = (@{&$fech_uncrawled}, @{&$fetch_default_rate}, @{&$fetch_custom_rate});
	return \@collections;
}

sub fetch_results($$@) {
	my ($dbh, $query, @binds) = @_;
	my $sth = $dbh->prepare($query)
		or croak "prepare:", $dbh->errstr;
	$sth->execute(@binds);
	return $sth->fetchall_arrayref;
}

sub fetch_default_rate($) {
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

## Crawls the collections that need to be crawled. If there are
## collections that should not be crawled yet, it returns the 
## time in seconds till the next collection should be crawled.
sub crawl_collections($) {
	my $collections_ptr = shift;
	my $infoquery = Boitho::Infoquery->new(PATH_TO_INFOQUERY);
	my $now = time();
	my $next_crawl = undef;
	foreach my $collection_ptr (@$collections_ptr) {
		my ($collection, $last) = @$collection_ptr;
		
		if (not $last or $last < $now) {
			if ($infoquery->crawlCollection($collection)) {
				print "# Crawling $collection\n" if DEBUG;
			}
			else {
				my $error = "";
				$error = $infoquery->get_error 	
					if $infoquery->get_error;
				warn "Unable to crawl $collection, $error \n";
			}
		}
		elsif(not defined $next_crawl or $last < $next_crawl) {
			$next_crawl = $last;
		}
	}
	return undef unless defined $next_crawl;
	return $next_crawl - $now; #Seconds til next collection wants to be crawled.
}
