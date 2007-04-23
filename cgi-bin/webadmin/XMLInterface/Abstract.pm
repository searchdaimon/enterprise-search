package XMLInterface::Abstract;
use strict;
use warnings;

# Constructor: new
#
# Attributes:
#	dbh - Database handler
#	@_ - Anything, sent to the childclass' _init function.
sub new {
	my ($class, $dbh) = @_;
	my $self = {};
	bless $self, $class;
	$self->{'dbh'} = $dbh;
	if ($self->can("_init")) { 
		# initialize if supported
		$self->_init(@_);
	}
	return $self;
}

1;