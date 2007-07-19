##
# Export URL submissions.

package JaydeURLExport;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Switch;
use Log::Log4perl;
use XML::Writer;
use Readonly;

BEGIN {
    push @INC, $ENV{BOITHOHOME} . "/Modules";
}
use SD::Sql::ConnSimple qw(sql_exec sql_setup get_dbh);

Readonly::Scalar my $DB_DATABASE => "boithoweb";

# init log
Log::Log4perl::init('log4perl.conf');
my $log = Log::Log4perl->get_logger('jayde.urlexport');

my $arg = $ARGV[0] || 'verified';

main();

sub main {
    my $user_status;
    switch ($arg) {
        case 'all' { $user_status = undef }
        case 'verified' { $user_status = 0 }
        case 'banned' { $user_status = 4 }

        else { show_usage() }
    }

    # get dbh
    my %setup = sql_setup();
    $setup{database} = $DB_DATABASE;
    my $dbh = get_dbh(%setup);
    
    # get, and print urls.
    my $sth = do_url_query($dbh, $user_status);
    sth_to_xml($sth);
}

##
# Generates SQL query and executes it.
#
# Attributes:
#   dbh - Database handler
#   user_status - User status (optional)
#
# Returns:
#   sth - Sql statement.
sub do_url_query {
    my ($dbh, $user_status) = @_;
    my @binds;
    my $query = "SELECT DISTINCT submission_url.url
        FROM submission_url, submission_userurl, brukere

        WHERE brukere.bruker_navn = submission_userurl.bruker_navn
        AND   submission_url.id = submission_userurl.url";
    if (defined $user_status) {
        $query .= " AND brukere.status = ?";
        push @binds, $user_status;
    }
    $log->debug("query: ", $query);

    my $sth = sql_exec($dbh, $query, @binds);
}

##
# Generate XML output for given url-query statement.
# See <do_url_query>.
sub sth_to_xml {
    my $sth = shift;
    
    my $wr = XML::Writer->new(DATA_MODE => 1, DATA_INDENT => 2);
    $wr->startTag('urls');
    while (my $data_ref = $sth->fetchrow_hashref) {
        $log->debug(Dumper($data_ref));
        $wr->dataElement('url', $data_ref->{url});
    }
    $wr->endTag('urls');
    $wr->end();

    1;
}

sub show_usage {
    print STDERR "Usage: ./jayde_url_export [all|verified|banned]\n";
    print STDERR "Second argument is user status. Defaults to 'verified'\n";
    exit 1;
}

1;
