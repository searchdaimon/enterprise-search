##
# Tool for importing jayde userdb to searchdaimon's userdb.
# dvj, 2007
#
# Usage: ./jayde_user_import.pl file1.xml [file2.xml ...]
use strict;
use warnings;
use Carp;
use Data::Dumper;
use YAML;
use XML::Parser::PerlSAX;
use Jayde::UrlImport::Handler;
use Log::Log4perl;
use Readonly;
use Date::Format;

BEGIN {
    push @INC, $ENV{BOITHOHOME} . "/Modules";
}
use SD::Sql::ConnSimple qw(sql_setup get_dbh sql_exec);

Readonly::Scalar my $DB_NAME => "boithoweb";
    
# init log
Log::Log4perl::init('log4perl.conf');
my $log = Log::Log4perl->get_logger('jayde.urlimport');

my $dbh;

main();

sub main {
    my @files = @ARGV;

    show_usage() if not @files;

    # Sanity check.
    foreach my $file (@files) {
        unless (-f $file) {
            print STDERR "$file is not a file\n";
            show_usage();
        }
    }

    connect_to_db();

    # parse contents to db.
    foreach my $file (@files) {
        my $handler = Jayde::UrlImport::Handler->new($file, \&url_record);
        my $parser = XML::Parser::PerlSAX->new(Handler => $handler);
        $parser->parse(Source => {SystemId => $file});
    }

    $log->info("Done parsing ", scalar @files, " documents.");
    1;
}

sub show_usage {
    die "Usage: jayde_url_import.pl file1.xml [file2.xml ...]\n";
}


sub url_record {
    my ($document, %data) = @_;
    $log->info("Got url ", $data{sUrl}, " for user ", $data{sUser});

    eval {
        sql_insert_user_url($data{sUser}, $data{sUrl});
    };
    if ($@) {
        $log->warn($@);
    }
}




##
# Connect to db.
sub connect_to_db {
    my %setup = sql_setup();
    $setup{database} = $DB_NAME;
    $dbh = get_dbh(%setup);
    1;
}


sub sql_insert_url {
    my $url = shift;
    my $query = "INSERT INTO submission_url (url) VALUES (?)";
    my $sth = sql_exec($dbh, $query, $url);
    return $dbh->{ q{mysql_insertid} };
}

sub sql_get_url_id {
    my $url = shift;
    my $query = "SELECT id FROM submission_url
        WHERE url = ?";
    my $sth = sql_exec($dbh, $query, $url);
    my ($id) = $sth->fetchrow;
    return $id;
}


sub sql_insert_user_url {
    my ($user, $url) = @_;
    my $url_id = sql_get_url_id($url)
        || sql_insert_url($url);

    my $query = "INSERT INTO submission_userurl (bruker_navn, url, added)
        VALUES(?, ?, NOW())";

    my $sth = sql_exec($dbh, $query, $user, $url_id);
}


1;
