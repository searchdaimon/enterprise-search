#!/usr/bin/env perl
use strict;
use warnings;
use Carp;
use DBI; #bruker DBI databse interfase

####################################################################
#settup
#####
#MySQL settup
my $user = "boitho";
my $Password = "G7J7v5L5Y7";
my $server = "localhost";
my $database = "boithobb";


use constant CFG => 
q{graph_info This graph shows search querys response time pr 5 min .
graph_title Response times
graph_vlabel Response times
graph_category Searchdaimon
min.label Best response time
min.draw LINE2
max.label Worst response time
max.draw LINE2
avg.label Average response time
avg.draw LINE2
};
use constant DEBUG => 0;

if (my $a = shift) {
	if ($a eq 'autoconf') {
		#print "yes\n";
		exit 1;
	}
	elsif ($a eq 'config') {
		print CFG;
	}
	else {
		die "Usage $0 [autoconf|config]\n"
			unless $a eq 'config';
	}
	exit;
}



# conetc to mysql
my $dbh = DBI->connect("DBI:mysql:database=$database;host=$server;port=3306",
                             $user, $Password) or warn("Can`t connect: $DBI::errstr");  #




#count querys last 5 min
my $sth = $dbh->prepare("SELECT AVG(search_tid), MIN(search_tid), MAX(search_tid) from search_logg WHERE DATE_SUB(NOW(),INTERVAL 5 MINUTE) < tid") or dienice("Can`t prepare statment: ", $dbh->errstr);
my $rv = $sth->execute;


my ($avg,$min,$max) = $sth->fetchrow_array;

if (!defined($avg)) { $avg = 0; }
if (!defined($min)) { $min = 0; }
if (!defined($max)) { $max = 0; }

print qq{min.value $min
max.value $max
avg.value $avg
};

