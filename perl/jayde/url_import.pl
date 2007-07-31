##
# Tool for importing jayde url-userdb to searchdaimon's url-userdb.
#
# Usage: ./jayde_url_import.pl file1.xml [file2.xml ...]
use strict;
use warnings;
use Carp;
use Data::Dumper;
use YAML;
use Log::Log4perl;
use Readonly;
use Date::Format;
use XML::Simple;

BEGIN {
    push @INC, $ENV{BOITHOHOME} . "/Modules";
}
use SD::Sql::ConnSimple qw(sql_setup get_dbh sql_exec);

Readonly::Scalar my $DB_NAME => "boithoweb";
Readonly::Scalar my $STRIP_QUOTATIONS => 1;
    
# init log
Log::Log4perl::init('log4perl.conf');
my $log = Log::Log4perl->get_logger('jayde.import.url');

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
        open my $fh, "<", $file
            or die "unable to open $file, ", $!;
       
        my @record;
        my $record_open = 0;
        while (my $line = <$fh>) {

            if ($line =~ /^\s*<UserUrl/) {
                $log->debug("new url");
                $record_open = 1;
                undef @record;

            }
            if ($line =~ /^\s*<\/UserUrl/) {
                $log->debug("url done");
                $record_open = 0;
                push @record, $line;
                url_record(%{ XMLin(join q{}, @record) });
                next;
            }
            push @record, $line;
        }
    }

    $log->info("Done parsing ", scalar @files, " documents.");
    1;
}

sub show_usage {
    die "Usage: jayde_url_import.pl file1.xml [file2.xml ...]\n";
}


sub url_record {
    my %data = @_;
    $log->info("Got url ", $data{sUrl}, " for user ", $data{sUser});

    if ($STRIP_QUOTATIONS) {
        $data{sUser} =~ s/^"//;
        $data{sUser} =~ s/"$//;
        $data{sUrl}  =~ s/^"//;
        $data{sUrl}  =~ s/"$//;
    }

    $data{sUrl} = "http://" . $data{sUrl}
        unless $data{sUrl} =~ /^http:\/\//;

    my $indexed = !$data{datemodified} ? 0
        : time2str("%Y-%m-%d %H-%M-%S", $data{datemodified});

    sql_insert_user_url($data{sUser}, $data{sUrl}, 
            time2str("%Y-%m-%d %H-%M-%S", $data{datecreated}),
            $indexed);
        
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
    my ($url, $indexed) = @_;

    my $query = "INSERT INTO submission_url (url, last_indexed) VALUES (?, ?)";
    my $sth = sql_exec($dbh, $query, $url, $indexed);
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

sub sql_user_url_exists {
    my ($user, $url) = @_;

    my $query = "SELECT bruker_navn FROM submission_userurl
        WHERE bruker_navn = ? and url = ?";
    my $sth = sql_exec($dbh, $query, $user, $url);
    my ($result) = $sth->fetchrow;

   return $result ? 1 : 0; 
}


sub sql_insert_user_url {
    my ($user, $url, $added, $indexed) = @_;
    $log->debug("inserting $user $url\n");
    my $url_id = sql_get_url_id($url)
        || sql_insert_url($url, $indexed);

    if (sql_user_url_exists($user, $url_id)) {
        $log->warn("User url $url for $user exists. Ignoring");
        return;
    }

    $log->debug("adding user $user url $url\n");
    my $query = "INSERT INTO submission_userurl (bruker_navn, url, added)
        VALUES(?, ?, ?)";

    eval {
        sql_exec($dbh, $query, $user, $url_id, $added);
    };
    if ($@) {
        $log->warn($@);
    }
}


1;
