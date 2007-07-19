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
use Jayde::UserImport::Handler;
use Log::Log4perl;
use Readonly;
use Date::Format;

BEGIN {
    push @INC, $ENV{BOITHOHOME} . "/Modules";
}
use SD::Sql::ConnSimple qw(sql_setup get_dbh sql_exec);

Readonly::Scalar my $DB_NAME => "boithoweb";
    
Readonly::Hash my %DB_LAYOUT => (
        sUser =>  'bruker_navn',
        sPassword => 'passord',
        sRealname => 'navn',
        sSecretQuestion => 'hemmelig_sporsmaal',
        sSecretAnswer => 'hemmelig_svar',
        iAutoLogin => 'auto_login',
        nCreateDate => 'opprettet',
        nLastLoginDate => 'sist_login',
        nStatus => 'status',
        setProps => 'egenskaper'
    );
    
Readonly::Hash my %IGNORED_ELEMENTS => (
        nTrialExpireDate => 1,
        id => 1,
        iType => 1,
        UserAccount => 1,
        iMaxAlerts => 1,
        setApplications => 1,
    );


# init log
Log::Log4perl::init('log4perl.conf');
my $log = Log::Log4perl->get_logger('jayde.userimport');

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
        my $handler = Jayde::UserImport::Handler->new($file, \&add_user_record, \&handler_log);
        my $parser = XML::Parser::PerlSAX->new(Handler => $handler);
        $parser->parse(Source => {SystemId => $file});
    }

    $log->info("Done parsing ", scalar @files, " documents.");
    1;
}

sub show_usage {
    die "Usage: jayde_user_import.pl file1.xml [file2.xml ...]\n";
}

##
# Logger for SAX handler.
# Handles log-events by passing them onto log4perl.
sub handler_log {
    my ($level, @msg) = @_;
    my $log = Log::Log4perl->get_logger('jayde.userimport.handler');
    $log->$level(@msg);
}

##
# User record hook for SAX handler.
# Adds user record to DB.
sub add_user_record {
    my ($document, %user_data) = @_;
    $log->info("Got user record for user ", $user_data{sUser});

    # Transform data to suit our db-setup.
    my %transformed; 
    while (my ($key, $value) = each %user_data) {
        unless (defined $DB_LAYOUT{$key}) {
            next if $IGNORED_ELEMENTS{$key};
            $log->warn("Ignoring unknown element $key.");
            next;
        }

        my $new_key = $DB_LAYOUT{$key};
        if ($key eq "setProps") {
            my %data = (
                    account_notes => $value->[0],
                    company_name  => $value->[2],
                    address1      => $value->[3],
                    address2      => $value->[4],
                    city          => $value->[5],
                    state         => $value->[6],
                    zip           => $value->[7],
                    country       => $value->[8],
                    phone         => $value->[9],
                    referrer      => $value->[16],
                    verified_date => $value->[27],
                    verified_ip   => $value->[28],
                    );
            $transformed{mail} = $value->[1];
            $transformed{$new_key} = Dump(\%data);
        }

        elsif ($key =~ /^nCreateDate$|^nLastLoginDate$/) {
            $transformed{$new_key} = time2str("%Y-%m-%d", $value);
        }

        else {
            $transformed{$new_key} = (ref $value) ? Dump($value) : $value;
        }
    }
    add_to_db(%transformed);
}



##
# Helper method to add a record to db.
sub add_to_db {
    my %data = @_;

    if (user_exists($data{bruker_navn})) {
        $log->warn("User $data{bruker_navn} conflicts with a user already in database. Ignoring.");
        return;
    }

    my @query = ("INSERT INTO brukere (", join(",", keys(%data)), ")",
                    " VALUES(", '?,' x (scalar(values %data) - 1), "?)");
    my $query = join q{}, @query;
    $log->debug("query: ", $query);
    sql_exec($dbh, $query, values %data);
    $log->debug("Done inserting ", $data{bruker_navn}, " to db.");
    1;
}

##
# Checks if given username exists in db
sub user_exists {
    my $user = shift;
    my $query = "SELECT bruker_navn FROM brukere
                    WHERE bruker_navn = ?";
    my $sth = sql_exec($dbh, $query, $user);
    
    return ($sth->fetchrow_array) ? 1 : 0;
}

##
# Connect to db.
sub connect_to_db {
    my %setup = sql_setup();
    $setup{database} = $DB_NAME;
    $dbh = get_dbh(%setup);
    1;
}

1;
