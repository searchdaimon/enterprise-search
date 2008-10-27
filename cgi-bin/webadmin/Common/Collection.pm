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
use Readonly;

#Readonly::Hash my %VALID_COLL_CHRS 
    #=> map { $_ => 1 } ('a'..'z', 'A'..'Z', 0..9, '-', '_');


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


        if ($check eq 'connector') {
            return (0, 'error_missing_connector')
                unless ($share->{'connector'});
        } 

        elsif ($check eq 'collection_name') {
            my $name = $share->{'collection_name'};
            my $id = $share->{'id'};

            return (0, 'error_inv_name')
                    unless $s->valid_name($name);
    
            if ($name ne "" && $s->{sqlShares}->get_id_by_collection($name)) {
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

sub valid_name { 
    my ($s, $name) = @_;
	return $name =~ /^([a-zA-Z0-9]|\.|_|-|)+$/;
#    for my $c (split q{}, $name) {
#        return unless $VALID_COLL_CHRS{$c};
#    }
#    return 1;
}

1;
