package Common::Collection;
use strict;
use warnings;
use Data::Dumper;
use Sql::Shares;
use Carp;

my @VALID_COLL_CHRS 
    = ('a'..'z', 'A'..'Z', 0..9, '-', q{ }, '_');

my $sqlShares;

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;
    $self->_init(@_);
    return $self;
}

sub _init {
    my ($self, $dbh) = @_;
    $sqlShares = Sql::Shares->new($dbh);
}

##
# Check if a collection being submitted is valid.
# Returns:
#   valid - true/false
#   err   - reason for error
sub validate {
    my ($self, $share, @checks) = @_;

    unless (@checks) { # default checks
        @checks = ('host', 'collection_name', 'connector');
    }

    while (my $check = shift(@checks)) {

        if ($check eq 'share') {
            next if ($share->{'host'});
            next if ($share->{'resource'});
            return (0, 'error_missing_share');
        }
# 		if ($check eq 'host') {
# 			return (0, 'error_missing_host') 
# 				unless ($share->{'host'});
# 		} 
# 		
# 		elsif ($check eq 'resource') {
# 			return (0, 'error_missing_resource')
# 				unless ($share->{'resource'});
# 		} 

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

            if ($sqlShares->get_id_by_collection($name)) {
                unless ($id and $sqlShares->get_collection_name($id) eq $name) {
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
