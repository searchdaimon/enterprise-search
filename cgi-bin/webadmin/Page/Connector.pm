package Page::Connector;
use strict;
use warnings;
use Readonly;
use Carp;
use Data::Dumper;
use Params::Validate;

use Sql::Connectors;
use Sql::Param;
use Sql::Shares;
use Page::Abstract;
our @ISA = qw(Page::Abstract);
use config qw(%CONFIG);

Readonly::Array  our @REQ_INPUT_FIELDS => qw(user_system custom_parameters crawling);
Readonly::Scalar our $SOURCE_TPL => "$CONFIG{conn_base_dir}/%s/main.pm";

sub _init {
    my $s = shift;
    $s->{sql_conn}  = Sql::Connectors->new($s->{dbh});
    $s->{sql_param} = Sql::Param->new($s->{dbh});
    $s->{sql_shares} = Sql::Shares->new($s->{dbh});
}

sub source_to_file {
    validate_pos(@_, 1, 1, 1);
    my ($s, $name, $source) = @_;

    my $path = sprintf $SOURCE_TPL, "\Q$name\E";

    open my $fh, ">", $path
        or croak "Unable to open source file '$path' for writing", $!;
   
    print {$fh} $source;
    close $fh;

    1;
}

sub del_test_collection {
    validate_pos(@_, 1, { regex => qr(^\d+$) });
    my ($s, $conn_id) = @_;
    my $test_coll = sprintf $CONFIG{test_coll_name}, $s->{sql_conn}->get_name($conn_id);
    return unless $s->{sql_shares}->exists({collection_name => $test_coll});

    my $coll_id = $s->{sql_shares}->get_id_by_collection($test_coll);

    my $c = Data::Collection->new($s->{dbh}, { id => $coll_id });
    $c->delete();
}

1;
