#!/usr/bin/env perl

package Graph::UserQueries;

BEGIN {
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
	push @INC, $ENV{'BOITHOHOME'} . '/cgi-bin/webadmin';
}

use strict;
use warnings;

use CGI;
use Data::Dumper;
use constant DEFAULT_LAST_VALUE		=> 30;
use constant DEFAULT_USERS_VALUE	=> 15;
use Carp;

use Page::Logs::Statistics;

my $page = Page::Logs::Statistics->new();
my $dbh = $page->get_dbh();
my %state = $page->get_state();

#$state{last} = DEFAULT_LAST_VALUE unless (defined($state{last}));
$state{last} = int($state{last}) or croak "Last must be an integer";
croak "need an action" unless defined $state{action};
#croak "need a username" unless defined $state{user};

print "Content-Type: text/plain\n\n";

if ($state{action} eq 'queries') {
	print $page->get_data_queries($state{last}, 10, $state{user});
} elsif ($state{action} eq 'searchesday') {
	print $page->get_searches_day($state{last}, $state{user});
} elsif ($state{action} eq 'users') {
	print $page->get_users_stat($state{last}, 20);
} elsif ($state{action} eq 'crawleddocs') {
	$state{crawler} = int($state{crawler}) or croak "Crawler must be an integer";
	$state{sessid} = int($state{sessid}) or croak "Session ID must be an integer";
	print $page->get_crawled_docs($state{crawler}, $state{sessid});
} else {
	croak "Unknown action: " . $state{action};
}
