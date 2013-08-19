##
# Functions for talking to mysql server.
package SD::Sql::ConnSimple;
use strict;
use warnings;
use Carp;
use DBI;
use Data::Dumper;
use Exporter;
our @EXPORT_OK = qw(sql_setup get_dbh sql_exec sql_fetch_results sql_fetch_single sql_fetch_arrayresults);
our @ISA = qw(Exporter);

use constant SQL_CONFIG_FILE => $ENV{'BOITHOHOME'} . "/config/setup.txt";

sub sql_setup {
    my %settings;

    open my $setup, SQL_CONFIG_FILE
        or croak print "Can't open config file ". SQL_CONFIG_FILE .": $!";
    my @data = <$setup>;
    close $setup;

    foreach my $line (@data) {
        my ($name, $value) = split(/=/, $line);
        chomp($value) if $value;
        $settings{$name} = $value if ($name and $value);
    }

    $settings{'port'} = 3306 unless($settings{'port'});

    return %settings;
}

sub get_dbh {
    my %setup = @_;
    my ($db, $host, $port, $user, $pass) = ( $setup{database}, 
            $setup{server}, $setup{port}, 
            $setup{user}, $setup{Password} );

    my $dbh = DBI->connect("DBI:mysql:database=$db;host=$host;port=$port", $user, $pass)
        or croak("$DBI::errstr");

    return $dbh;
}

sub sql_exec {
    my ($dbh, $query, @binds) = @_;
    my $sth = $dbh->prepare($query)
        or croak "prepare:", $dbh->errstr;
    $sth->execute(@binds)
        or croak "exec: ", $dbh->errstr;
    return $sth;
}

##
# Fetch db results for given query and binds
# to arrayref
sub sql_fetch_results {
    my $sth = sql_exec(@_);
    my @results;
    while (my $res = $sth->fetchrow_hashref) {
        push @results, $res;
    }
    return @results;
}

##
# Fetch only one scalar value.
#
# Arguments:
#   dbh - db connection.
#   query - SQL Query.
#   @binds - Bind values.
sub sql_fetch_single {
    my $sth = sql_exec(@_);
    my ($value) = $sth->fetchrow_array;
    return $value;
}

##
# Fetch db results for given query and binds
# to arrayref
sub sql_fetch_arrayresults {
    my $sth = sql_exec(@_);
    my @results;
    while (my @res = $sth->fetchrow_array) {
        push @results, \@res;
    }
    return @results;
}

1;
