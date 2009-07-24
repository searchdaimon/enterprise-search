package Page::Connector::CrawlerUtils;
use strict;
use warnings;
use Readonly;
use Carp;
use Data::Dumper;

BEGIN { push @INC, "$ENV{BOITHOHOME}/Modules" }
use Params::Validate qw(validate_pos OBJECT);
use Boitho::Infoquery;
use Data::Collection;

use config qw(%CONFIG);

Readonly::Array  my @REQ_INPUT_FIELDS => qw(user_system custom_parameters crawling);
Readonly::Scalar my $SOURCE_TPL => "$CONFIG{conn_base_dir}/%s/main.pm";

sub new { bless { } }
sub source_tpl { $SOURCE_TPL }
sub req_input_fields { @REQ_INPUT_FIELDS }

sub conn_path {
	my ($s, $conn_name) = @_;
	return sprintf $SOURCE_TPL, "\Q$conn_name\E";
}

sub del_test_collection {
	validate_pos(@_, 1, { regex => qr(^\d+$) }, { type => OBJECT });
	my ($s, $conn_id, $crawler) = @_;
	my $test_coll = sprintf $CONFIG{test_coll_name}, $crawler->{sql_conn}->get_name($conn_id);
	return unless $crawler->{sql_shares}->exists({collection_name => $test_coll});

	my $coll_id = $crawler->{sql_shares}->get_id_by_collection($test_coll);

	# del from db
	my $c = Data::Collection->new($crawler->{dbh}, { id => $coll_id });
	$c->delete();

	# del from disk
	my $iq = Boitho::Infoquery->new($CONFIG{infoquery});
	if (!$iq->deleteCollection($test_coll)) {
		# likely not a problem, ignoring error.
		carp $iq->error;
	}
	$s;
}

1;
