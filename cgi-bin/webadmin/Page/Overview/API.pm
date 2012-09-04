package Page::Overview::API;
use strict;
use warnings;

use Carp;
use Params::Validate qw(validate_pos HASHREF);
use Data::Dumper;
use File::Temp qw(tempfile);
use Fcntl ':flock';
use CGI qw(escapeHTML);

use config qw(%CONFIG);

use Page::API;
use Page::Abstract;
use Sql::ShareUsers;
use Sql::Shares;
use File::ReadBackwards;

our @ISA = qw(Page::Abstract Page::API);

use constant COLL_LOG_FILE    	=> '/tmp/coll_log_';
use constant MAX_COLL_LINES	=> 200;

Readonly::Scalar my $CRAWL_STATUS_OK => qr{^(OK\.|Ok)$};

my $sqlShares;

sub _init {
        my $self = shift;
        my $dbh = $self->{dbh};
        $sqlShares     = Sql::Shares->new($dbh);
        #$sqlConnectors = Sql::Connectors->new($dbh);
        #$sqlAuth = Sql::CollectionAuth->new($dbh);
        #$sqlGroups = Sql::ShareGroups->new($dbh);
        #$sqlUsers = Sql::ShareUsers->new($dbh);
        $self->{'infoQuery'}   = Boitho::Infoquery->new($CONFIG{'infoquery'});
        #$self->{sqlConfig} = Sql::Config->new($dbh);
}

sub console_output {
    my ($self, $api_vars, $id) = (@_);

    my $done;
    my @lines;

    my $share = $sqlShares->get_share($id);
    
    my $collection = $share->{collection_name};

    my $output_path = COLL_LOG_FILE . $collection;

    $api_vars->{collection_name} = $share->{collection_name};
    $api_vars->{crawler_message} = $share->{crawler_message};

    $api_vars->{doc_count}  = $self->{'infoQuery'}->documentsInCollection( $collection );

    $api_vars->{running} = 'No';
    if ($share->{crawler_message} !~ $CRAWL_STATUS_OK) {
	$api_vars->{running} = 'Yes';
    }

    # read the log file
    eval {
	my $count = 0;

        croak "Results for testrun do not exist. File: $output_path"
            unless defined $output_path && -e $output_path;

	my $bw = File::ReadBackwards->new( $output_path ) or
             die "can't read '$output_path' $!" ;

	while( defined( my $log_line = $bw->readline ) ) {
		push(@lines,$log_line);

		last unless $count++ < MAX_COLL_LINES;
    	}
	@lines = reverse(@lines);

        $api_vars->{output} = escapeHTML(join q{}, @lines);
    };

    unless ($self->api_error($api_vars, $@)) {
        $api_vars->{ok} = "Output fetched.";
    }


}

1;
