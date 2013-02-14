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
use SD::Sql::ConnSimple qw(sql_exec
        sql_fetch_results sql_fetch_single);
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

    	my $q = "
		select connectors.name,count(*),read_only 
		from 
		shares,connectors 
		where 
		connectors.id=shares.connector  
		group by shares.connector
	";

	my @a = sql_fetch_results($self->{dbh}, $q) ;
	$json->{"connectors"} = \@a;


	print to_json($json);
exit;
	bb_config_get($self->{dbh}, 'setup_wizard_done');
	
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


