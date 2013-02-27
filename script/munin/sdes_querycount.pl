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
q{graph_info This graph shows search querys pr 5 min .
graph_title Number of search querys
graph_vlabel querycount
graph_category Searchdaimon
querycount.label Number
querycount.draw LINE2
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
my $sth = $dbh->prepare("SELECT count(*) FROM search_logg WHERE DATE_SUB(NOW(),INTERVAL 5 MINUTE) < tid;") or dienice("Can`t prepare statment: ", $dbh->errstr);
my $rv = $sth->execute;


my $querycount = $sth->fetchrow_array;


print qq{querycount.value $querycount
};

