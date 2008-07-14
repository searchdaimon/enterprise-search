# Class: Common::Data::Overview
# Methods for data retrieval that both XMLInterface, and Page use.
package Common::Data::Overview;
use strict;
use warnings;

BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . "/Modules";
}

use Boitho::Infoquery;
use Common::Data::Abstract;
use Sql::Connectors;
use Sql::Config;
use Sql::Shares;

use Carp;
use Data::Dumper;
use Time::Local qw(timelocal);
use config qw(%CONFIG);

BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . "Modules";
}

our @ISA = qw(Common::Data::Abstract);

my $sqlConnectors;
my $sqlConfig;
my $dbh;
my $default_crawl_rate;
my $infoQuery;

sub _init {
	my $self = shift;
	$dbh = $self->{'dbh'};

	$sqlConnectors = Sql::Connectors->new($dbh);
	$sqlConfig     = Sql::Config->new($dbh);

	$default_crawl_rate
		= $sqlConfig->get_setting('default_crawl_rate');

	$infoQuery = Boitho::Infoquery->new($CONFIG{'infoquery'});
}

##
# Fetches all connectors and all their shares. 
sub get_connectors_with_collections {
    my $self = shift;
    my $sqlShares = Sql::Shares->new($self->{dbh});

    my @connectors = $sqlConnectors->get({ active => 1 });
    for my $c (@connectors) {
        my $test_coll = sprintf $CONFIG{test_coll_name}, $c->{name};
        my @collections = $sqlShares->get({ 
            connector => $c->{id},  
            collection_name => {"!=", $test_coll }, # Ignore test collection.
        });
        $c->{shares} = \@collections;
        next unless @collections;

        for my $coll (@collections) {
            next unless $coll->{active};
            my $rate = $coll->{rate} || $default_crawl_rate;

            $coll->{smart_rate} = $self->_minutes_to_text($rate);
            $coll->{next_crawl} = $self->_get_next_crawl($rate, $coll->{'last'});
            $coll->{doc_count}  = $infoQuery->documentsInCollection(
                    $coll->{collection_name});
        }
    }

    return @connectors;
}

# Group: Private methods

sub _get_next_crawl {
	my $self = shift;
	my ($rate, $last) = @_;
	return unless $last; # Need last to find next.
        return if $last eq '0000-00-00 00:00:00';
	
	#make $last into unixtime, if it isn't.
	unless ($last =~ /^\d+$/) {
		$last = $self->_mysql_to_unixtime($last);
	}

	my $time_ago = time - $last;
	my $time_left = ($rate * 60) - $time_ago; # * 60 to get seconds.
	
	my $to_text = $self->_minutes_to_text(abs($time_left / 60));
	if ($time_left < 0) { return "Should have been crawled $to_text ago"; }
	return "Next crawl in $to_text";

}

##
# Change to mysql time string to unixstamp
sub _mysql_to_unixtime {
	my $self = shift;
	my $mysql_time = shift;
	return unless $mysql_time;
	#/(\d\d\d\d)-?(\d\d)-?(\d\d) ?(\d\d):?(\d\d):?(\d\d)/;
	my ($year, $month, $day, $hour, $minute, $second) 
		= $mysql_time 
		=~ /(\d\d\d\d)-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d)/;
	my $unixtime = eval { timelocal($second, $minute, $hour, $day, 
			 ($month - 1), ($year - 1900)) };
	carp $@ if $@;
	
	$unixtime;
}

##
# Method to change seconds into text.
# Example outputs: "1 day", "5 minutes", "12 hours"
sub _minutes_to_text {
	my ($self, $minutes) = @_;
        return unless defined $minutes;
    
        return "now" if $minutes == 0;

	if ($minutes < 60) { # less than an hour
		$minutes = int $minutes;
		return "$minutes minute" if ($minutes == 1);
		return "$minutes minutes";
	}
	elsif ($minutes < 1440) { #less than an day
		my $hours = int($minutes / 60);
		return "$hours hour" if ($hours == 1);
		return "$hours hours";
	}
	
	my $days = int($minutes / 60 / 24);
	return "$days day" if ($days == 1);
	return "$days days";
}

1;
