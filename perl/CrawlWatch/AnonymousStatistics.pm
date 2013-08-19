#!/usr/bin/perl
package CrawlWatch::AnonymousStatistics;
use strict;
use warnings;
use Carp;
BEGIN {
	unshift @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Time::localtime;
use Boitho::Infoquery;
use DBI;
use JSON;
use Data::Dumper;
use LWP::UserAgent;
use HTTP::Request::Common qw(POST);  
use SD::Sql::ConnSimple qw(sql_exec
        sql_fetch_results sql_fetch_single sql_fetch_arrayresults);
use CrawlWatch::Config qw(bb_config_get bb_config_update);

use constant DAYS => 60 * 60 * 24;

use constant CONF_ANOSTAT_RATE => "anostat_default_rate";
use constant CONF_LAST_ANOSTAT => "anostat_last_run";
use constant CONF_PREFERENCE => "anostat_preference";



sub new {
    croak "missing arguments" unless scalar @_ == 4;
    my $class = shift;
    my %self = map { $_ => shift @_ } qw(dbh iq log);

    bless \%self, $class;
}

sub name { "Anonymous usage statistics" }

# Read the uptime.
sub uptime {
	# Read the uptime in seconds from /proc/uptime, skip the idle time...
	open FILE, "< /proc/uptime" or die return ("Cannot open /proc/uptime: $!");
		my ($uptime, undef) = split / /, <FILE>;
	close FILE;
	return ($uptime);
}

sub run {
    my $self = shift;
    bb_config_update($self->{dbh}, CONF_LAST_ANOSTAT, time);
    my $anostat_rate = bb_config_get($self->{dbh}, CONF_ANOSTAT_RATE);
    my $now = time() - 3600; # -3600 to compansate for standar ES time sone.
    #doint the actual anostat run.
    ##############################################################################################################
	if (bb_config_get($self->{dbh}, CONF_PREFERENCE) ne 'legal') {
	    	$self->{'log'}->write("Anonymous usage statistics is disabled.");
		return;
	}	
	$self->{'log'}->write("Runing anonymous usage statistics.");

	my $json;

    	my $qconnectors = "
		select connectors.name,count(*) 
		from 
		shares,connectors 
		where 
		connectors.id=shares.connector  
		group by shares.connector
	";

	my @connectors = sql_fetch_arrayresults($self->{dbh}, $qconnectors) ;


    	my $qsystem = "
		select systemConnector.name,count(*) 
		from 
		systemConnector,system 
		where 
		systemConnector.id=system.connector 
		group by system.connector
	";

	my @system = sql_fetch_arrayresults($self->{dbh}, $qsystem) ;


	$json->{"uptime"} = uptime();
	$json->{"connectors"} = \@connectors;
	$json->{"system"} = \@system;

	# Sends the data to Searchdaimon
	my $userAgent = LWP::UserAgent->new(agent => 'perl post');
	my $response = $userAgent->request(
		POST 'http://www.searchdaimon.com/cgi-bin/anostat/post.cgi',
		Content_Type => 'text/json',
		Content => JSON::to_json($json)
	);


	# Debug: Print the respone
	# print $response->as_string;


	
    ##############################################################################################################
    1;
}

sub next_run {
    my $self = shift;
    my $anostat_rate = bb_config_get($self->{dbh}, CONF_ANOSTAT_RATE);
    my $last_anostat = bb_config_get($self->{dbh}, CONF_LAST_ANOSTAT);
    my $log = $self->{'log'};
    $log->write("INFO: ANOSTAT last run: $last_anostat, anostat rate: $anostat_rate");

    unless (defined $anostat_rate and $anostat_rate =~ /^\d+$/) {
        $log->write("WARN: db field ", CONF_ANOSTAT_RATE, 
                " not valid. Not running ANOSTAT.");
        return -1;
    }

    
    unless (defined $last_anostat and $last_anostat =~ /^\d+$/) {
        $log->write("WARN: db field ", 
            CONF_LAST_ANOSTAT, " not valid. Not running anostat.");
        return -1;
    }

    my $next_run = ($last_anostat + $anostat_rate) - time;
    ($next_run < 0) ? 0 : $next_run;
}

1;


