use strict;
use warnings;

package Perlcrawl;
use Crawler;
our @ISA = qw(Crawler);

use sdMimeMap;

use Carp;
use Data::Dumper;
use LWP::Simple qw(get);

use SD::Crawl;


use DBI;

sub crawl_update {
	my (undef, $self, $opt ) = @_;	

	print "options:\n";
	foreach my $k (keys %{ $opt }) {
		print "$k: $opt->{$k}\n";
	}

	#print "inc: \n", join(", ", @INC), "\n";

	####################################################################
	# Setup
	####################################################################
	my $user = $opt->{"user"};
	my $password = $opt->{"password"};
	my $server = $opt->{"ip"};
	my $database = $opt->{'database'};

	####################################################################
	
	 my $dsn = join "", (
	 "dbi:ODBC:",
	 "DRIVER={FreeTDS};",
	 "Server=$server;",
	 "Port=1094;",
	 "TDS_Version=8.0;",
	 "UID=$user;",
	 "PWD=$password;",
	 "Database=$database;",
	 );

	 my $db_options = {
		 PrintError => 1,
		 RaiseError => 0,
		 AutoCommit => 1,
	 };

	 my $dbh = DBI->connect($dsn, $user, $password, $db_options)
		 or exit_msg("Can't connect: $DBI::errstr");

	#my $sth = $dbh->prepare('select name from sysobjects where type=\'u\''); # Get table names
	#my $sth = $dbh->prepare('select TABLE_NAME from information_schema.tables where Table_Type = \'BASE TABLE\'');
	my $sth = $dbh->prepare('SELECT TABLE_SCHEMA,TABLE_NAME, OBJECTPROPERTY(object_id(TABLE_NAME), N\'IsUserTable\') AS type  FROM INFORMATION_SCHEMA.TABLES');
	$sth->execute;

	#laster ned sql resultater
	my @tables = ();
	while ((my $r = $sth->fetchrow_hashref)) {

                my $tbl = $r->{TABLE_NAME};
                $self->add_document(
                                url => $tbl,
                                title => "Table name: $tbl",
                                content => $tbl,
                                last_modified => 0,
                                type => 'txt',
                                acl_allow => "Everyone,en,rb",

                );

		push @tables, $r->{TABLE_SCHEMA}.".".$tbl;

	}
	$sth->finish();

	foreach my $tbl (@tables) {
		my $q = "SELECT * FROM $tbl";
		print STDERR "Query: $q\n";
		my $sth2 = $dbh->prepare($q) or next;
		$sth2->execute;

		my $data = '';
		print STDERR "Table: $tbl\n";
		while ((my $r2 = $sth2->fetchrow_hashref)) {
			#print STDERR Dumper($r2);

			$data .= Dumper($r2) . "\n\n";
		}

		$data =~ s/\_/ /g;

                $self->add_document(
                                url => $tbl,
                                title => "Table name: $tbl",
                                content => $data,
                                last_modified => 0,
                                type => 'txt',
                                acl_allow => "Everyone,en,rb",

                );

		$sth2->finish;
	}
}

sub path_access {
    my ($undef, $self, $opt) = @_;
# print Data::Dumper->Dump([$opt]);
    my $user = $opt->{"user"};
    my $password  = $opt->{"password"};
    my $server = $opt->{"ip"};

    return 1; # Authenticated
}
