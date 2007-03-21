package Common::Collection;
use strict;
use warnings;
use Data::Dumper;
use Sql::Shares;
use Carp;

# This class provides a set of common features regarding collections.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->_init(@_);
	return $self;
}

sub _init {
	my $self = shift;
	my $dbh = shift;
	$self->{'dbh'} = $dbh;
	$self->{'sqlShares'} = Sql::Shares->new($dbh);
}

# Checks for error in input from a form. Returns two values. 
# One true or false valid value, and one more describing string.
# If the check collection_name is used, $sqlShares needs to be defined.
# Example usage: my ($valid, $msg) = Collection::validate_share($dbh, $share, @checks);
sub validate {
	my $self = shift;
	my $share = shift;
	my $sql = $self->{'sqlShares'};
	my @checks = (@_);
		
	# Add default values if checks aren't provided.
	unless (@checks) {
		@checks = ('host', 'collection_name', 'connector');
	}
	
	# note to self: add uses connector and collection_name in first form.
	
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
			
			if ($sql->get_id_by_collection($name)) {
				# Is it I that hold the collection?
				unless ($id and $sql->get_collection_name($id) eq $name) {
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