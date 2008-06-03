package Common::Collection;
use strict;
use warnings;
use Data::Dumper;
use Sql::Shares;
use Sql::CollectionAuth;
use Sql::ShareGroups;
use Sql::ShareUsers;
use Carp;
use config qw(%CONFIG);

my @VALID_COLL_CHRS 
    = ('a'..'z', 'A'..'Z', 0..9, '-', q{ }, '_');


sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;
    $self->_init(@_);
    return $self;
}

sub _init {
    my ($self, $dbh) = @_;
    $self->{sqlShares} = Sql::Shares->new($dbh);
    $self->{dbh} = $dbh;
}

##
# Data needed in a form 
# for editing/managing a collection.
sub coll_form_data {
    my ($s, @input_fields) = @_;

    my $iq = new Boitho::Infoquery($CONFIG{infoquery});
    my $sqlAuth = Sql::CollectionAuth->new($s->{dbh});
    my $sqlConnectors = Sql::Connectors->new($s->{dbh});

    my %form_data = (input_fields => \@input_fields);
    my %fields = map { $_ => 1 } @input_fields;
    $form_data{connectors} = $sqlConnectors->get_connectors()
        if $fields{connectors};
    $form_data{group_list} = $iq->listGroups()
        if $fields{groups};
    $form_data{user_list} = $iq->listMailUsers()
        if $fields{exchange_user_select};
    $form_data{authentication} = [ $sqlAuth->get_all_auth() ]
        if $fields{authentication};

    $form_data{input_fields} = \@input_fields;

    return %form_data;
}

##
# Data for a collection needed in a
# form for deiting/managing.
sub coll_data {
    my ($s, $id, @input_fields) = @_;
    my %fields = map { $_ => 1 } @input_fields;

    my $sqlGroups = Sql::ShareGroups->new($s->{dbh});
    my $sqlUsers  = Sql::ShareUsers->new($s->{dbh});
    my %coll_data = %{ $s->{sqlShares}->get_share($id) };
   
    $coll_data{group_member} = $sqlGroups->get_groups($id)
        if $fields{group};
    $coll_data{user} = [$sqlUsers->get_users($id)]
        if $fields{exchange_user_select};

    return %coll_data;
}

##
# Check if a collection being submitted is valid.
# Returns:
#   valid - true/false
#   err   - reason for error
sub validate {
    my ($s, $share, @checks) = @_;

    unless (@checks) { # default checks
        @checks = ('host', 'collection_name', 'connector');
    }

    while (my $check = shift(@checks)) {

        if ($check eq 'share') {
            next if ($share->{'host'});
            next if ($share->{'resource'});
            return (0, 'error_missing_share');
        }

        elsif ($check eq 'connector') {
            return (0, 'error_missing_connector')
                unless ($share->{'connector'});
        } 

        elsif ($check eq 'collection_name') {
            my $name = $share->{'collection_name'};
            my $id = $share->{'id'};

            foreach my $c (split '', $name) {
                return (0, 'error_inv_name')
                    unless grep { "$c" eq "$_" } @VALID_COLL_CHRS;
            }

            if ($s->{sqlShares}->get_id_by_collection($name)) {
                unless ($id and $s->{sqlShares}->get_collection_name($id) eq $name) {
                    return (0, 'error_collection_exists');
                }
            }
        }

        else {
            carp "Unknown check $check requested.";
        }
    }
    return (1, 'valid');
}

1;
